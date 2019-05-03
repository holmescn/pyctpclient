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
#pragma once
#include <boost/python.hpp>
#include "ThostFtdcUserApiStruct.h"

class MdSpi;
class TraderSpi;
class CThostFtdcMdApi;
class CThostFtdcTraderApi;

#pragma region Exception
struct RequestNetworkException
{
    std::string request;
};

struct FullRequestQueueException
{
    std::string request;
};

struct RequestTooFrequentlyException
{
    std::string request;
};

struct UnknownRequestException
{
    int rc;
    std::string request;
};
#pragma endregion

class CtpClient
{
protected:
    MdSpi *_mdSpi = nullptr;
    CThostFtdcMdApi *_mdApi = nullptr;
    TraderSpi *_tdSpi = nullptr;
    CThostFtdcTraderApi *_tdApi = nullptr;
    std::string _flowPath;
    std::string _mdAddr;
    std::string _tdAddr;
    std::string _brokerId;
    std::string _userId;
    std::string _password;
    boost::python::list _instrumentIds;

public:
    CtpClient(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password);
    CtpClient(const CtpClient&) = delete;
    CtpClient(CtpClient&&) = delete;
    CtpClient& operator=(const CtpClient&) = delete;
    CtpClient& operator=(CtpClient&&) = delete;
    virtual ~CtpClient();

    void Run();

public:
    // Getter/Setter
    inline std::string GetFlowPath() const { return _flowPath; }
    inline void SetFlowPath(std::string flowPath) { _flowPath = flowPath; }
    inline std::string GetMdAddr() const { return _mdAddr; }
    inline void SetMdAddr(std::string addr) { _mdAddr = addr; }
    inline std::string GetTdAddr() const { return _tdAddr; }
    inline void SetTdAddr(std::string addr) { _tdAddr = addr; }
    inline std::string GetBrokerId() const { return _brokerId; }
    inline void SetBrokerId(std::string brokerId) { _brokerId = brokerId; }
    inline std::string GetUserId() const { return _userId; }
    inline void SetUserId(std::string userId) { _userId = userId; }
    inline std::string GetPassword() const { return _password; }
    inline void SetPassword(std::string password) { _password = password; }
    inline boost::python::list GetInstrumentIds() const { return _instrumentIds; }
    inline void SetInstrumentIds(boost::python::list instrumentIds) { _instrumentIds = instrumentIds; }

public:
    static boost::python::tuple GetApiVersion();

public:
    // MdApi
    void MdLogin();
    void SubscribeMarketData(boost::python::list instrumentIds);
    void UnsubscribeMarketData(boost::python::list instrumentIds);

public:
    // MdSpi
	virtual void OnMdFrontConnected() = 0;
	virtual void OnMdFrontDisconnected(int nReason) = 0;
	virtual void OnMdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnMdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo) = 0;
    virtual void OnSubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo) = 0;
    virtual void OnUnsubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo) = 0;
    virtual void OnRtnMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) = 0;
    virtual void OnTick(std::string instrumentId, float price, int volume, std::string time) = 0;
    virtual void On1Min(std::string instrumentId, float priceOpen, float priceHigh, float priceLow, float priceClose, int volume, std::string time) = 0;
	virtual void OnMdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) = 0;

public:
    // TraderApi
    void TdLogin();
    void ConfirmSettlementInfo();
    void QueryOrder();
    void QueryTrade();
    void QueryTradingAccount();
    void QueryInvestorPosition();
    void QueryMarketData(std::string instrumentId);
    void QuerySettlementInfo();

public:
    // TraderSpi
	virtual void OnTdFrontConnected() = 0;
    virtual void OnTdFrontDisconnected(int nReason) = 0;
	virtual void OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) = 0;
	virtual void OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) = 0;
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnTdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) = 0;

    virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo)	= 0;
};


class CtpClientWrap : public CtpClient, public boost::python::wrapper<CtpClient>
{
public:
	CtpClientWrap(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password);
	~CtpClientWrap();

	void OnMdFrontConnected() override;
	void OnMdFrontDisconnected(int nReason) override;
	void OnMdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo) override;
	void OnMdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo) override;
    void OnSubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo) override;
    void OnUnsubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo) override;
    void OnRtnMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;
    void OnTick(std::string instrumentId, float price, int volume, std::string time) override;
    void On1Min(std::string instrumentId, float priceOpen, float priceHigh, float priceLow, float priceClose, int volume, std::string time) override;
	void OnMdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	void OnTdFrontConnected() override;
	void OnTdFrontDisconnected(int nReason) override;
	void OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	void OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo) override;

	void OnTdError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
	void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo)	override;

};
