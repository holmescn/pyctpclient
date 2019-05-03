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
#include <sstream>
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

	std::string mdFlowPath, tdFlowPath;
	if (_flowPath == "") {
		auto tmpPath = temp_directory_path();
		auto rootPath = tmpPath / "ctp/";
		auto mdPath = rootPath / "md/";
		auto tdPath = rootPath / "td/";
		create_directory(rootPath);
		create_directory(mdPath);
		create_directory(tdPath);

		mdFlowPath = mdPath.string();
		tdFlowPath = tdPath.string();
	} else {
		path p(_flowPath);
		if (!exists(p)) {
			create_directory(p);
		}
		auto mdPath = p / "md/";
		if (!exists(mdPath)) {
			create_directory(mdPath);
		}
		auto tdPath = p / "td/";
		if (!exists(tdPath)) {
			create_directory(tdPath);
		}

		mdFlowPath = mdPath.string();
		tdFlowPath = tdPath.string();
	}

	_mdApi = CThostFtdcMdApi::CreateFtdcMdApi(mdFlowPath.c_str(), /*using udp*/false, /*multicast*/false);
	_mdSpi = new MdSpi(this);
	_mdApi->RegisterSpi(_mdSpi);
	_mdApi->RegisterFront(const_cast<char*>(_mdAddr.c_str()));
	_mdApi->Init();

	// _tdApi = CThostFtdcTraderApi::CreateFtdcTraderApi(tdFlowPath.c_str());
	// _tdSpi = new TraderSpi(this);
	// _tdApi->RegisterSpi(_tdSpi);
	// _tdApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	// _tdApi->SubscribePublicTopic(THOST_TERT_QUICK);
	// _tdApi->RegisterFront(const_cast<char*>(_tdAddr.c_str()));
	// _tdApi->Init();

	std::this_thread::sleep_for(std::chrono::seconds(5));
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

void CtpClient::QueryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
	strncpy(req.CurrencyID, "CNY", sizeof req.CurrencyID);

	assert_request(_tdApi->QueryTradingAccount(&req, 0));
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

void CtpClientWrap::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
	if (override fn = get_override("on_rsp_trading_account")) {
		fn(pTradingAccount, pRspInfo, bIsLast);
	}
}

#pragma endregion // Trader SPI