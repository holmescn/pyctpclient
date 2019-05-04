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
#include <cstring>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include "mdspi.h"
#include "traderspi.h"
#include "ctpclient.h"

using namespace boost::python;

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
	//
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

	std::this_thread::sleep_for(std::chrono::seconds(10));
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
	char **ppInstrumentIDs = new char*[N];
	for (size_t i = 0; i < N; i++) {
		std::string instrumentId = extract<std::string>(instrumentIds[i]);
		ppInstrumentIDs[i] = new char[instrumentId.size()+1];
		memcpy(ppInstrumentIDs[i], instrumentId.data(), instrumentId.size()+1);
	}

	_mdApi->SubscribeMarketData(ppInstrumentIDs, N);

	for (size_t i = 0; i < N; i++) {
		delete[] ppInstrumentIDs[i];
	}
	delete[] ppInstrumentIDs;
}

void CtpClient::UnsubscribeMarketData(boost::python::list instrumentIds)
{
	size_t N = len(instrumentIds);
	char **ppInstrumentIDs = new char*[N];
	for (size_t i = 0; i < N; i++) {
		std::string instrumentId = extract<std::string>(instrumentIds[i]);
		ppInstrumentIDs[i] = new char[instrumentId.size()+1];
		memcpy(ppInstrumentIDs[i], instrumentId.data(), instrumentId.size()+1);
	}

	_mdApi->UnSubscribeMarketData(ppInstrumentIDs, N);

	for (size_t i = 0; i < N; i++) {
		delete[] ppInstrumentIDs[i];
	}
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
	} else {
		std::cerr << "Market Data User Logout" << std::endl;
	}
}

void CtpClientWrap::OnTick(std::string instrumentId, float price, int volume, std::string time)
{
	if (override fn = get_override("on_tick")) {
		fn(instrumentId, price, volume, time);
	}
}

void CtpClientWrap::On1Min(std::string instrumentId, float priceOpen, float priceHigh, float priceLow, float priceClose, int volume, std::string time)
{
	if (override fn = get_override("on_1min")) {
		fn(instrumentId, priceOpen, priceHigh, priceLow, priceClose, volume, time);
	}
}

void CtpClientWrap::OnMdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (override fn = get_override("on_md_error")) {
		fn(pRspInfo, nRequestID, bIsLast);
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
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	assert_request(_tdApi->ReqQryOrder(&req, 0));
}

void CtpClient::QueryTrade()
{
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	assert_request(_tdApi->ReqQryTrade(&req, 0));
}

void CtpClient::QueryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	strncpy(req.CurrencyID, "CNY", sizeof req.CurrencyID);

	assert_request(_tdApi->QueryTradingAccount(&req, 0));
}

void CtpClient::QueryInvestorPosition()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InstrumentID, _userId.c_str(), sizeof req.InvestorID);

	// 不填写合约则返回所有持仓
	strncpy(req.InstrumentID, "", sizeof req.InstrumentID);

	assert_request(_tdApi->QueryInvestorPosition(&req, 0));
}

void CtpClient::QueryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InstrumentID, _userId.c_str(), sizeof req.InvestorID);

	// 不填写合约则返回所有持仓
	strncpy(req.InstrumentID, "", sizeof req.InstrumentID);

	assert_request(_tdApi->ReqQryInvestorPositionDetail(&req, 0));
}

void CtpClient::QueryMarketData(std::string instrumentId)
{
	CThostFtdcQryDepthMarketDataField req;
	memset(&req, 0, sizeof req);
	strncpy(req.InstrumentID, instrumentId.c_str(), sizeof req.InstrumentID);

	assert_request(_tdApi->ReqQryDepthMarketData(&req, 0));
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
		case Buy:
			req.Direction = THOST_FTDC_D_Buy;
			break;
		case Sell:
			req.Direction = THOST_FTDC_D_Sell;
			break;
		default:
			throw InvalidArgument{"direction", "unknwon"};
	}

	switch (offsetFlag) {
		case Open:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
			break;
		case Close:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			break;
		case ForceClose:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceClose;
			break;
		case CloseToday:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			break;
		case CloseYesterday:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
			break;
		case ForceOff:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceOff;
			break;
		case LocalForceClose:
			req.CombOffsetFlag[0] = THOST_FTDC_OF_LocalForceClose;
			break;
		default:
			throw InvalidArgument{"offset_flag", "unknwon"};
	}

	switch (orderPriceType) {
		case AnyPrice:
			req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
			break;
		case LimitPrice:
			req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			break;
		case BestPrice:
			req.OrderPriceType = THOST_FTDC_OPT_BestPrice;
			break;
    	case LastPrice:
			req.OrderPriceType = THOST_FTDC_OPT_LastPrice;
			break;
		case LastPricePlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusOneTicks;
			break;
		case LastPricePlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusTwoTicks;
			break;
		case LastPricePlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusThreeTicks;
			break;
    	case AskPrice1:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1;
			break;
		case AskPrice1PlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusOneTicks;
			break;
		case AskPrice1PlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusTwoTicks;
			break;
		case AskPrice1PlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusThreeTicks;
			break;
    	case BidPrice1:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1;
			break;
		case BidPrice1PlusOneTick:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusOneTicks;
			break;
		case BidPrice1PlusTwoTicks:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusTwoTicks;
			break;
		case BidPrice1PlusThreeTicks:
			req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusThreeTicks;
			break;
    	case FiveLevelPrice:
			req.OrderPriceType = THOST_FTDC_OPT_FiveLevelPrice;
			break;
		default:
			throw InvalidArgument{"order_price_type", "unknown"};
	}

	switch (hedgeFlag) {
		case Speculation:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
			break;
		case Arbitrage:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Arbitrage;
			break;
		case Hedge:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Hedge;
			break;
		case MarketMaker:
			req.CombHedgeFlag[0] = THOST_FTDC_HF_MarketMaker;
			break;
		default:
			throw InvalidArgument{"hedge_flag", "unknown"};
	}

	switch (timeCondition) {
		case IOC:
			req.TimeCondition = THOST_FTDC_TC_IOC;
			break;
		case GFS:
			req.TimeCondition = THOST_FTDC_TC_GFS;
			break;
		case GFD:
			req.TimeCondition = THOST_FTDC_TC_GFD;
			break;
		case GTD:
			req.TimeCondition = THOST_FTDC_TC_GTD;
			break;
		case GTC:
			req.TimeCondition = THOST_FTDC_TC_GTC;
			break;
		case GFA:
			req.TimeCondition = THOST_FTDC_TC_GFA;
			break;
		default:
			throw InvalidArgument{"time_condition", "unknown"};
	}

	switch (volumeCondition) {
		case AV:
			req.VolumeCondition = THOST_FTDC_VC_AV;
			break;
		case MV:
			req.VolumeCondition = THOST_FTDC_VC_MV;
			break;
		case CV:
			req.VolumeCondition = THOST_FTDC_VC_CV;
			break;
		default:
			throw InvalidArgument{"volume_condition", "unknown"};
	}

	switch (contingentCondition) {
		case Immediately:
			req.ContingentCondition = THOST_FTDC_CC_Immediately;
			break;
		case Touch:
			req.ContingentCondition = THOST_FTDC_CC_Touch;
			break;
		case TouchProfit:
			req.ContingentCondition = THOST_FTDC_CC_TouchProfit;
			break;
		case ParkedOrder:
			req.ContingentCondition = THOST_FTDC_CC_ParkedOrder;
			break;
    	case LastPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;
			break;
		case LastPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice;
			break;
    	case LastPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserThanStopPrice;
			break;
		case LastPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
			break;
    	case AskPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterThanStopPrice;
			break;
		case AskPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice;
			break;
    	case AskPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserThanStopPrice;
			break;
		case AskPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserEqualStopPrice;
			break;
    	case BidPriceGreaterThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterThanStopPrice;
			break;
		case BidPriceGreaterEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice;
			break;
    	case BidPriceLesserThanStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserThanStopPrice;
			break;
		case BidPriceLesserEqualStopPrice:
			req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserEqualStopPrice;
			break;
		default:
			throw InvalidArgument{"contingent_condition", "unknown"};
	}

	req.MinVolume = minVolume;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = isAutoSuspend;
	req.UserForceClose = userForceClose;
	req.VolumeTotalOriginal = volume;
	req.LimitPrice = limitPrice;

	assert_request(_tdApi->ReqOrderInsert(&req, requestID));
}

void CtpClient::OrderAction(boost::python::dict kwargs)
{

}

void CancelOrder(boost::python::dict kwargs)
{

}

void ModifyOrder(boost::python::dict kwargs)
{

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

void CtpClientWrap::OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (override fn = get_override("on_td_user_login")) {
		fn(pRspUserLogin, pRspInfo, nRequestID, bIsLast);
	} else {
		std::cerr << "Trader User Login" << std::endl;
		ConfirmSettlementInfo();
	}
}

void CtpClientWrap::OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (override fn = get_override("on_td_user_logout")) {
		fn(pUserLogout, pRspInfo, nRequestID, bIsLast);
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

void CtpClientWrap::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_rsp_order_insert")) {
		fn(pInputOrder, pRspInfo);
	}
}

void CtpClientWrap::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_rsp_order_action")) {
		fn(pInputOrderAction, pRspInfo);
	}
}

void CtpClientWrap::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_err_order_insert")) {
		fn(pInputOrder, pRspInfo);
	}
}

void CtpClientWrap::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (override fn = get_override("on_err_order_action")) {
		fn(pOrderAction, pRspInfo);
	}
}

void CtpClientWrap::OnRtnOrder(CThostFtdcOrderField *pOrder)
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

void CtpClientWrap::OnTdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (override fn = get_override("on_td_error")) {
		fn(pRspInfo, nRequestID, bIsLast);
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