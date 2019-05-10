/*
 * Copyright 2019 Holmes Conan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <ctime>
#include <csignal>
#include <cstring>
#include <chrono>
#include <string>
#include <future>
#include <thread>
#include <iostream>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include "mdspi.h"
#include "traderspi.h"
#include "ctpclient.h"
#include "GILhelper.h"

using namespace boost::python;
using namespace std::chrono_literals;

std::promise<void> g_exitPromise;
std::shared_future<void> g_exitSignal(g_exitPromise.get_future());

void signal_handler(int signal)
{
	g_exitPromise.set_value();
}

#define assert_request(request) _assertRequest((request), #request)

void _assertRequest(int rc, const char *request)
{
	switch (rc) {
	case 0:
		// 发送成功
		break;
	case -1:
		// 因网络原因发送失败
		throw RequestNetworkException{request};
	case -2:
		// 未处理请求队列总数量超限
		throw FullRequestQueueException{request};
	case -3:
		// 每秒发送请求数量超限
		throw RequestTooFrequentlyException{request};
	default:
		throw UnknownRequestException{rc, request};
	}
}

#pragma region General functions

CtpClient::CtpClient(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password)
: _mdAddr(mdAddr), _tdAddr(tdAddr), _brokerId(brokerId), _userId(userId), _password(password)
{
	std::signal(SIGINT, signal_handler);
}

CtpClient::~CtpClient()
{
	if (_mdSpi) {
		delete _mdSpi;
	}

	if (_tdSpi) {
		delete _tdSpi;
	}
}

tuple CtpClient::GetApiVersion()
{
	std::string v1 = CThostFtdcMdApi::GetApiVersion();
	std::string v2 = CThostFtdcTraderApi::GetApiVersion();
	return boost::python::make_tuple(v1, v2);
}

void CtpClient::Run()
{
	using namespace boost::filesystem;

	_thread = std::thread([this](std::shared_future<void> exitSignal) {
		size_t timeout = 1024;
		auto timer = std::chrono::steady_clock::now();
    	while (exitSignal.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
			CtpClient::Response *r = nullptr;
			{
				std::unique_lock<std::mutex> lk(_queueMutex);
				if(_queueConditionVariable.wait_for(lk, timeout * 1ms, [this]{ return !_q.empty(); })) {
					r = _q.front();
					_q.pop();
					auto duration = std::chrono::steady_clock::now() - timer;
					if (std::chrono::duration_cast<std::chrono::milliseconds>(duration) > 5s && timeout > 2) {
						timeout /= 2;
					}
				} else {
					OnIdle();

					auto duration = std::chrono::steady_clock::now() - timer;
					if (std::chrono::duration_cast<std::chrono::milliseconds>(duration) < 1s) {
						timeout *= 2;
					}
					timer = std::chrono::steady_clock::now();
				}
			}

			if (r) {
				ProcessResponse(r);
				delete r;
			}
		}
	}, g_exitSignal);

	if (_flowPath == "") {
		auto tmpPath = temp_directory_path() / "ctp";
		_flowPath = tmpPath.string();
	}

	auto rootPath = path(_flowPath);
	auto mdPath = rootPath / "md/";
	auto tdPath = rootPath / "td/";
	create_directory(rootPath);

	if (_mdAddr != "") {
		create_directory(mdPath);
		auto mdFlowPath = mdPath.string();

		_mdApi = CThostFtdcMdApi::CreateFtdcMdApi(mdFlowPath.c_str(), /*using udp*/false, /*multicast*/false);
		_mdSpi = new MdSpi(this);
		_mdApi->RegisterSpi(_mdSpi);
		_mdApi->RegisterFront(const_cast<char*>(_mdAddr.c_str()));
		_mdApi->Init();
	}

	if (_tdAddr != "") {
		create_directory(tdPath);
		auto tdFlowPath = tdPath.string();

		_tdApi = CThostFtdcTraderApi::CreateFtdcTraderApi(tdFlowPath.c_str());
		_tdSpi = new TraderSpi(this);
		_tdApi->RegisterSpi(_tdSpi);
		_tdApi->SubscribePrivateTopic(THOST_TERT_QUICK);
		_tdApi->SubscribePublicTopic(THOST_TERT_QUICK);
		_tdApi->RegisterFront(const_cast<char*>(_tdAddr.c_str()));
		_tdApi->Init();
	}
}

void CtpClient::Join()
{
	_thread.join();
}

void CtpClient::Exit()
{
	g_exitPromise.set_value();
}

void CtpClient::Push(CtpClient::Response *r)
{
	std::lock_guard<std::mutex> lk(_queueMutex);
	_q.push(r);
	_queueConditionVariable.notify_one();
}

void CtpClient::ProcessResponse(CtpClient::Response *r)
{
	switch (r->type) {
	case ResponseType::OnMdFrontConnected:
		OnMdFrontConnected();
		break;
  case ResponseType::OnMdFrontDisconnected:
		OnMdFrontDisconnected(r->nReason);
		break;
  case ResponseType::OnMdUserLogin:
		OnMdUserLogin(&r->RspUserLogin, &r->RspInfo);
		break;
  case ResponseType::OnMdUserLogout:
		OnMdUserLogout(&r->UserLogout, &r->RspInfo);
		break;
  case ResponseType::OnSubMarketData:
		OnSubscribeMarketData(&r->SpecificInstrument, &r->RspInfo);
		break;
  case ResponseType::OnUnSubMarketData:
		OnUnsubscribeMarketData(&r->SpecificInstrument, &r->RspInfo);
		break;
  case ResponseType::OnRtnMarketData:
		OnRtnMarketData(&r->DepthMarketData);
		break;
  case ResponseType::OnTick:
		OnTick(&r->tick);
		break;
  case ResponseType::On1Min:
		On1Min(&r->m1);
		break;
  case ResponseType::On1MinTick:
		On1MinTick(&r->m1);
		break;
	case ResponseType::OnMdError:
		OnMdError(&r->RspInfo);
		break;
  case ResponseType::OnTdFrontConnected:
		OnTdFrontConnected();
		break;
  case ResponseType::OnTdFrontDisconnected:
		OnTdFrontDisconnected(r->nReason);
		break;
  case ResponseType::OnTdUserLogin:
		OnTdUserLogin(&r->RspUserLogin, &r->RspInfo);
		break;
  case ResponseType::OnTdUserLogout:
		OnTdUserLogout(&r->UserLogout, &r->RspInfo);
		break;
  case ResponseType::OnSettlementInfoConfirm:
		OnRspSettlementInfoConfirm(&r->SettlementInfoConfirm, &r->RspInfo);
		break;
  case ResponseType::OnRspOrderInsert:
		OnErrOrderInsert(&r->InputOrder, &r->RspInfo);
		break;
  case ResponseType::OnRspOrderAction:
		OnErrOrderAction(&r->InputOrderAction, nullptr, &r->RspInfo);
		break;
  case ResponseType::OnErrRtnOrderInsert:
		OnErrOrderInsert(&r->InputOrder, &r->RspInfo);
		break;
  case ResponseType::OnErrRtnOrderAction:
		OnErrOrderAction(nullptr, &r->OrderAction, &r->RspInfo);
		break;
  case ResponseType::OnRtnOrder:
	{
	  CThostFtdcOrderField *pNewOrder = new CThostFtdcOrderField;
    memcpy(pNewOrder, &r->Order, sizeof r->Order);
    OnRtnOrder(boost::shared_ptr<CThostFtdcOrderField>(pNewOrder));
	}
		break;
  case ResponseType::OnRtnTrade:
		OnRtnTrade(&r->Trade);
		break;
  case ResponseType::OnTdError:
		OnTdError(&r->RspInfo);
		break;
  case ResponseType::OnRspQryOrder:
		OnRspQryOrder(&r->Order, &r->RspInfo, r->bIsLast);
		break;
  case ResponseType::OnRspQryTrade:
		OnRspQryTrade(&r->Trade, &r->RspInfo, r->bIsLast);
		break;
  case ResponseType::OnRspQryTradingAccount:
		OnRspQryTradingAccount(&r->TradingAccount, &r->RspInfo, r->bIsLast);
		break;
  case ResponseType::OnRspQryInvestorPosition:
		OnRspQryInvestorPosition(&r->InvestorPosition, &r->RspInfo, r->bIsLast);
		break;
  case ResponseType::OnRspQryDepthMarketData:
		OnRspQryDepthMarketData(&r->DepthMarketData, &r->RspInfo, r->bIsLast);
		break;
  case ResponseType::OnRspQrySettlementInfo:
		OnRspQrySettlementInfo(&r->SettlementInfo, &r->RspInfo);
		break;
  case ResponseType::OnRspQryInvestorPositionDetail:
		OnRspQryInvestorPositionDetail(&r->InvestorPositionDetail, &r->RspInfo, r->bIsLast);
		break;
	default:
		throw std::invalid_argument("unhandled response type.");
	}
}

CtpClientWrap::CtpClientWrap(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password)
: CtpClient(mdAddr, tdAddr, brokerId, userId, password)
{
	//
}

CtpClientWrap::~CtpClientWrap()
{
	//
}

#pragma endregion


#pragma region Market Data API

void CtpClient::MdLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.UserID, _userId.c_str(), sizeof req.UserID);
	strncpy(req.Password, _password.c_str(), sizeof req.Password);
	// strncpy(req.UserProductInfo, _userProductInfo.c_str(), sizeof req.UserProductInfo);

	assert_request(_mdApi->ReqUserLogin(&req, 0));
}

void CtpClient::SubscribeMarketData(boost::python::list instrumentIds)
{
	size_t N = len(instrumentIds);
	if (N == 0) return;

	char **ppInstrumentIDs = new char*[N];
	for (size_t i = 0; i < N; i++) {
		ppInstrumentIDs[i] = extract<char*>(instrumentIds[i]);
	}
	_mdApi->SubscribeMarketData(ppInstrumentIDs, N);
	delete[] ppInstrumentIDs;
}

void CtpClient::UnsubscribeMarketData(boost::python::list instrumentIds)
{
	size_t N = len(instrumentIds);
	if (N == 0) return;

	char **ppInstrumentIDs = new char*[N];
	for (size_t i = 0; i < N; i++) {
		ppInstrumentIDs[i] = extract<char*>(instrumentIds[i]);
	}
	_mdApi->UnSubscribeMarketData(ppInstrumentIDs, N);
	delete[] ppInstrumentIDs;
}

#pragma endregion // Market Data API


#pragma region Market Data SPI

void CtpClientWrap::OnMdFrontConnected()
{
	if (override fn = get_override("on_md_front_connected")) {
		fn();
	} else {
		std::cerr << "Market Data Front Connected" << std::endl;
		MdLogin();
	}
}

void CtpClientWrap::OnMdFrontDisconnected(int nReason)
{
	if (override fn = get_override("on_md_front_disconnected")) {
		fn(nReason);
	} else {
		std::cerr << "Market Data Front Disconnected with reason = " << nReason << std::endl;
	}
}

void CtpClientWrap::OnMdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_md_user_login")) {
		fn(pRspUserLogin, pRspInfo);
	} else {
		std::cerr << "Market Data User Login" << std::endl;
		if (len(_instrumentIds) > 0) {
			SubscribeMarketData(_instrumentIds);
		}
	}
}

void CtpClientWrap::OnMdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_md_user_logout")) {
		fn(pUserLogout, pRspInfo);
	} else {
		std::cerr << "Market Data User Logout" << std::endl;
	}
}

void CtpClientWrap::OnSubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_subscribe_market_data")) {
		fn(pSpecificInstrument, pRspInfo);
	} else {
		std::cerr << "Market Data subscribed " << pSpecificInstrument->InstrumentID << std::endl;
	}
}

void CtpClientWrap::OnUnsubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_unsubscribe_market_data")) {
		fn(pSpecificInstrument, pRspInfo);
	} else {
		std::cerr << "Market Data unsubscribed " << pSpecificInstrument->InstrumentID << std::endl;
	}
}

void CtpClientWrap::OnRtnMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (override fn = get_override("on_rtn_market_data")) {
		fn(pDepthMarketData);
	}
}

void CtpClientWrap::OnTick(TickBar *bar)
{
	if (override fn = get_override("on_tick")) {
		fn(bar);
	}
}

void CtpClientWrap::On1Min(M1Bar *bar)
{
	if (override fn = get_override("on_1min")) {
		fn(bar);
	}
}

void CtpClientWrap::On1MinTick(M1Bar *bar)
{
	if (override fn = get_override("on_1min_tick")) {
		fn(bar);
	}
}

void CtpClientWrap::OnMdError(CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_md_error")) {
		fn(pRspInfo);
	} else {
		std::cerr << "Market Data Error: " << pRspInfo->ErrorID << std::endl;
	}
}

#pragma endregion // Market Data SPI


#pragma region Trader API

void CtpClient::TdLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.UserID, _userId.c_str(), sizeof req.UserID);
	strncpy(req.Password, _password.c_str(), sizeof req.Password);
	// strncpy(req.UserProductInfo, _userProductInfo.c_str(), sizeof req.UserProductInfo);

	assert_request(_tdApi->ReqUserLogin(&req, 0));
}

void CtpClient::ConfirmSettlementInfo()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);

	assert_request(_tdApi->ConfirmSettlementInfo(&req, 0));
}

void CtpClient::QueryOrder()
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryOrderField req;
		memset(&req, 0, sizeof req);
		strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
		strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);

		assert_request(_tdApi->ReqQryOrder(&req, 0));
	}, _queryTick);
}

void CtpClient::QueryTrade()
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryTradeField req;
		memset(&req, 0, sizeof req);
		strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
		strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
		assert_request(_tdApi->ReqQryTrade(&req, 0));
	}, _queryTick);
}

void CtpClient::QueryTradingAccount()
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryTradingAccountField req;
		memset(&req, 0, sizeof req);
		strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
		strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
		strncpy(req.CurrencyID, "CNY", sizeof req.CurrencyID);

		assert_request(_tdApi->QueryTradingAccount(&req, 0));
	}, _queryTick);
}

void CtpClient::QueryInvestorPosition()
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryInvestorPositionField req;
		memset(&req, 0, sizeof req);
		strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
		strncpy(req.InstrumentID, _userId.c_str(), sizeof req.InvestorID);

		// 不填写合约则返回所有持仓
		strncpy(req.InstrumentID, "", sizeof req.InstrumentID);

		assert_request(_tdApi->QueryInvestorPosition(&req, 0));
	}, _queryTick);
}

void CtpClient::QueryInvestorPositionDetail()
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryInvestorPositionDetailField req;
		memset(&req, 0, sizeof req);
		strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
		strncpy(req.InstrumentID, _userId.c_str(), sizeof req.InvestorID);

		// 不填写合约则返回所有持仓
		strncpy(req.InstrumentID, "", sizeof req.InstrumentID);

		assert_request(_tdApi->ReqQryInvestorPositionDetail(&req, 0));
	}, _queryTick);
}

void CtpClient::QueryMarketData(std::string instrumentId)
{
	_queryTick += std::chrono::milliseconds(1200);
	std::async(std::launch::async, [this](std::string instrumentId, std::chrono::steady_clock::time_point until) {
		std::this_thread::sleep_until(until);

		CThostFtdcQryDepthMarketDataField req;
		memset(&req, 0, sizeof req);
		strncpy(req.InstrumentID, instrumentId.c_str(), sizeof req.InstrumentID);

		assert_request(_tdApi->ReqQryDepthMarketData(&req, 0));
	}, instrumentId, _queryTick);
}

void CtpClient::QuerySettlementInfo()
{
	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	strncpy(req.CurrencyID, "CNY", sizeof req.CurrencyID);

	assert_request(_tdApi->QuerySettlementInfo(&req, 0));
}

void CtpClient::InsertOrder(
	std::string instrumentId,
	Direction direction,
	OffsetFlag offsetFlag,
	TThostFtdcPriceType limitPrice,
	TThostFtdcVolumeType volume,
	OrderPriceType orderPriceType,
	HedgeFlag hedgeFlag,
	TimeCondition timeCondition,
	VolumeCondition volumeCondition,
	ContingentCondition contingentCondition,
	TThostFtdcVolumeType minVolume,
	TThostFtdcPriceType stopPrice,
	bool isAutoSuspend,
	bool userForceClose,
	int requestID)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	strncpy(req.InstrumentID, instrumentId.c_str(), sizeof req.InstrumentID);

	switch(direction) {
		case D_Buy:
			req.Direction = THOST_FTDC_D_Buy;
			break;
		case D_Sell:
			req.Direction = THOST_FTDC_D_Sell;
			break;
		default:
			throw std::invalid_argument("direction");
	}

	switch (offsetFlag) {
		case OF_Open:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
			break;
		case OF_Close:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			break;
		case OF_ForceClose:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceClose;
			break;
		case OF_CloseToday:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			break;
		case OF_CloseYesterday:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
			break;
		case OF_ForceOff:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceOff;
			break;
		case OF_LocalForceClose:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_LocalForceClose;
			break;
		default:
			throw std::invalid_argument("offset_flag");
	}

	switch (orderPriceType) {
		case OPT_AnyPrice:
			req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
			break;
		case OPT_LimitPrice:
			req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			break;
		case OPT_BestPrice:
			req.OrderPriceType = THOST_FTDC_OPT_BestPrice;
			break;
    	case OPT_LastPrice:
			req.OrderPriceType = THOST_FTDC_OPT_LastPrice;
			break;
		case OPT_LastPricePlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusOneTicks;
			break;
		case OPT_LastPricePlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusTwoTicks;
			break;
		case OPT_LastPricePlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusThreeTicks;
			break;
    	case OPT_AskPrice1:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1;
			break;
		case OPT_AskPrice1PlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusOneTicks;
			break;
		case OPT_AskPrice1PlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusTwoTicks;
			break;
		case OPT_AskPrice1PlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusThreeTicks;
			break;
    	case OPT_BidPrice1:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1;
			break;
		case OPT_BidPrice1PlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusOneTicks;
			break;
		case OPT_BidPrice1PlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusTwoTicks;
			break;
		case OPT_BidPrice1PlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusThreeTicks;
			break;
    	case OPT_FiveLevelPrice:
			req.OrderPriceType = THOST_FTDC_OPT_FiveLevelPrice;
			break;
		default:
			throw std::invalid_argument("order_price_type");
	}

	switch (hedgeFlag) {
		case HF_Speculation:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
			break;
		case HF_Arbitrage:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Arbitrage;
			break;
		case HF_Hedge:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Hedge;
			break;
		case HF_MarketMaker:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_MarketMaker;
			break;
		default:
			throw std::invalid_argument("hedge_flag");
	}

	switch (timeCondition) {
		case TC_IOC:
			req.TimeCondition = THOST_FTDC_TC_IOC;
			break;
		case TC_GFS:
			req.TimeCondition = THOST_FTDC_TC_GFS;
			break;
		case TC_GFD:
			req.TimeCondition = THOST_FTDC_TC_GFD;
			break;
		case TC_GTD:
			req.TimeCondition = THOST_FTDC_TC_GTD;
			break;
		case TC_GTC:
			req.TimeCondition = THOST_FTDC_TC_GTC;
			break;
		case TC_GFA:
			req.TimeCondition = THOST_FTDC_TC_GFA;
			break;
		default:
			throw std::invalid_argument("time_condition");
	}

	switch (volumeCondition) {
		case VC_AV:
			req.VolumeCondition = THOST_FTDC_VC_AV;
			break;
		case VC_MV:
			req.VolumeCondition = THOST_FTDC_VC_MV;
			break;
		case VC_CV:
			req.VolumeCondition = THOST_FTDC_VC_CV;
			break;
		default:
			throw std::invalid_argument("volume_condition");
	}

	switch (contingentCondition) {
		case CC_Immediately:
			req.ContingentCondition = THOST_FTDC_CC_Immediately;
			break;
		case CC_Touch:
			req.ContingentCondition = THOST_FTDC_CC_Touch;
			break;
		case CC_TouchProfit:
			req.ContingentCondition = THOST_FTDC_CC_TouchProfit;
			break;
		case CC_ParkedOrder:
			req.ContingentCondition = THOST_FTDC_CC_ParkedOrder;
			break;
    	case CC_LastPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;
			break;
		case CC_LastPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice;
			break;
    	case CC_LastPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserThanStopPrice;
			break;
		case CC_LastPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
			break;
    	case CC_AskPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterThanStopPrice;
			break;
		case CC_AskPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice;
			break;
    	case CC_AskPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserThanStopPrice;
			break;
		case CC_AskPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserEqualStopPrice;
			break;
    	case CC_BidPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterThanStopPrice;
			break;
		case CC_BidPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice;
			break;
    	case CC_BidPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserThanStopPrice;
			break;
		case CC_BidPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserEqualStopPrice;
			break;
		default:
			throw std::invalid_argument("contingent_condition");
	}

	req.MinVolume = minVolume;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = isAutoSuspend;
	req.UserForceClose = userForceClose;
	req.VolumeTotalOriginal = volume;
	req.LimitPrice = limitPrice;

	assert_request(_tdApi->ReqOrderInsert(&req, requestID));
}

void CtpClient::OrderAction(
	boost::shared_ptr<CThostFtdcOrderField> pOrder,
	OrderActionFlag actionFlag,
    TThostFtdcPriceType limitPrice,
    TThostFtdcVolumeType volumeChange,
	int requestId)
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof req);

	strncpy(req.BrokerID, pOrder->BrokerID, sizeof req.BrokerID);
	strncpy(req.InvestorID, pOrder->InvestorID, sizeof req.InvestorID);
	strncpy(req.OrderRef, pOrder->OrderRef, sizeof req.OrderRef);
	strncpy(req.ExchangeID, pOrder->ExchangeID, sizeof req.ExchangeID);
	strncpy(req.OrderSysID, pOrder->OrderSysID, sizeof req.OrderSysID);
	strncpy(req.InstrumentID, pOrder->InstrumentID, sizeof req.InstrumentID);
	req.FrontID = pOrder->FrontID;
	req.SessionID = pOrder->SessionID;

	switch (actionFlag) {
		case AF_Delete:
			req.ActionFlag = THOST_FTDC_AF_Delete;
			break;
		case AF_Modify:
			req.ActionFlag = THOST_FTDC_AF_Modify;
			break;
		default:
			throw std::invalid_argument("action_flag");
	}
	req.LimitPrice = limitPrice;
	req.VolumeChange = volumeChange;

	assert_request(_tdApi->ReqOrderAction(&req, requestId));
}

void CtpClient::DeleteOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder, int requestId)
{
	OrderAction(pOrder, AF_Delete, 0.0, 0, requestId);
}

void CtpClient::ModifyOrder(
	boost::shared_ptr<CThostFtdcOrderField> pOrder,
    TThostFtdcPriceType limitPrice,
    TThostFtdcVolumeType volumeChange,
    int requestId)
{
	OrderAction(pOrder, AF_Modify, limitPrice, volumeChange, requestId);
}

#pragma endregion // Trader API


#pragma region Trader SPI

void CtpClientWrap::OnTdFrontConnected()
{
	std::cerr << "Trader Front Connected" << std::endl;

	if (override fn = get_override("on_td_front_connected")) {
		fn();
	} else {
		TdLogin();
	}
}

void CtpClientWrap::OnTdFrontDisconnected(int nReason)
{
	if (override fn = get_override("on_md_front_disconnected")) {
		fn(nReason);
	} else {
		std::cerr << "Trader Front Disconnected with reason = " << nReason << std::endl;
	}
}

void CtpClientWrap::OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_td_user_login")) {
		fn(pRspUserLogin, pRspInfo);
	} else {
		std::cerr << "Trader User Login" << std::endl;
		ConfirmSettlementInfo();
	}
}

void CtpClientWrap::OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_td_user_logout")) {
		fn(pUserLogout, pRspInfo);
	} else {
		std::cerr << "Trader User Logout" << std::endl;
	}
}

void CtpClientWrap::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_settlement_info_confirm")) {
		fn(pSettlementInfoConfirm, pRspInfo);
	} else {
		std::cerr << "SettlementInfoConfirm: " << pRspInfo->ErrorID << std::endl;
	}
}

void CtpClientWrap::OnErrOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_err_order_insert")) {
		fn(pInputOrder, pRspInfo);
	}
}

void CtpClientWrap::OnErrOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_err_order_action")) {
		fn(pInputOrderAction, pOrderAction, pRspInfo);
	}
}

void CtpClientWrap::OnRtnOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder)
{
	if (override fn = get_override("on_rtn_order")) {
		fn(pOrder);
	}
}

void CtpClientWrap::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if (override fn = get_override("on_rtn_trade")) {
		fn(pTrade);
	}
}

void CtpClientWrap::OnTdError(CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_td_error")) {
		fn(pRspInfo);
	} else {
		std::cerr << "Trader Error: " << pRspInfo->ErrorID << std::endl;
	}
}

void CtpClientWrap::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_order")) {
		fn(pOrder, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_trade")) {
		fn(pTrade, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_trading_account")) {
		fn(pTradingAccount, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_investor_position")) {
		fn(pInvestorPosition, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_investor_position_detail")) {
		fn(pInvestorPositionDetail, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_market_data")) {
		fn(pDepthMarketData, pRspInfo, bIsLast);
	}
}

void CtpClientWrap::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_rsp_settlement_info")) {
		fn(pSettlementInfo, pRspInfo);
	}
}

#pragma endregion // Trader SPI

void CtpClientWrap::OnIdle()
{
	if (override fn = get_override("on_idle")) {
		fn();
	}
}
