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
#include <map>
#include <atomic>
#include <chrono>
#include <thread>
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include "ThostFtdcUserApiStruct.h"
#include "bar.h"
#include "concurrentqueue.h"

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

#pragma region Enums

enum Direction { D_Unknown, D_Buy, D_Sell };
enum OffsetFlag {
        OF_Open, OF_Close, OF_ForceClose, OF_CloseToday, OF_CloseYesterday,
        OF_ForceOff, OF_LocalForceClose };
enum OrderPriceType {
    OPT_AnyPrice, OPT_LimitPrice, OPT_BestPrice,
    OPT_LastPrice, OPT_LastPricePlusOneTick, OPT_LastPricePlusTwoTicks, OPT_LastPricePlusThreeTicks,
    OPT_AskPrice1, OPT_AskPrice1PlusOneTick, OPT_AskPrice1PlusTwoTicks, OPT_AskPrice1PlusThreeTicks,
    OPT_BidPrice1, OPT_BidPrice1PlusOneTick, OPT_BidPrice1PlusTwoTicks, OPT_BidPrice1PlusThreeTicks,
    OPT_FiveLevelPrice
};
enum HedgeFlag { HF_Speculation, HF_Arbitrage, HF_Hedge, HF_MarketMaker };
enum TimeCondition { TC_IOC, TC_GFS, TC_GFD, TC_GTD, TC_GTC, TC_GFA };
enum VolumeCondition { VC_AV, VC_MV, VC_CV };
enum ContingentCondition {
    CC_Immediately, CC_Touch, CC_TouchProfit, CC_ParkedOrder,
    CC_LastPriceGreaterThanStopPrice, CC_LastPriceGreaterEqualStopPrice,
    CC_LastPriceLesserThanStopPrice, CC_LastPriceLesserEqualStopPrice,
    CC_AskPriceGreaterThanStopPrice, CC_AskPriceGreaterEqualStopPrice,
    CC_AskPriceLesserThanStopPrice, CC_AskPriceLesserEqualStopPrice,
    CC_BidPriceGreaterThanStopPrice, CC_BidPriceGreaterEqualStopPrice,
    CC_BidPriceLesserThanStopPrice, CC_BidPriceLesserEqualStopPrice
};
enum OrderActionFlag { AF_Delete, AF_Modify };
enum OrderStatus {
    OST_AllTraded,
    OST_PartTradedQueueing,
    OST_PartTradedNotQueueing,
    OST_NoTradeQueueing,
    OST_NoTradeNotQueueing,
    OST_Canceled,
    OST_Unknown,
    OST_NotTouched,
    OST_Touched
};
enum OrderSubmitStatus {
    OSS_InsertSubmitted,
    OSS_CancelSubmitted,
    OSS_ModifySubmitted,
    OSS_Accepted,
    OSS_InsertRejected,
    OSS_CancelRejected,
    OSS_ModifyRejected
};
enum OrderActionStatus {
    OAS_Submitted,
    OAS_Accepted,
    OAS_Rejected
};

#pragma endregion // Enums

class CtpClient
{
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
    std::thread _thread;

    enum class RequestType {
        QueryOrder,
        QueryTrade,
        QueryTradingAccount,
        QueryInvestorPosition,
        QueryInvestorPositionDetail,
        QueryMarketData
    };

    enum class ResponseType : uint32_t {
        OnMdFrontConnected,
        OnMdFrontDisconnected,
        OnMdUserLogin,
        OnMdUserLogout,
        OnSubMarketData,
        OnUnSubMarketData,
        OnRtnMarketData,
        OnMdError,

        OnTdFrontConnected,
        OnTdFrontDisconnected,
        OnTdUserLogin,
        OnTdUserLogout,
        OnSettlementInfoConfirm,
        OnRspOrderInsert,
        OnRspOrderAction,
        OnErrRtnOrderInsert,
        OnErrRtnOrderAction,
        OnRtnOrder,
        OnRtnTrade,
        OnTdError,
        OnRspQryOrder,
        OnRspQryTrade,
        OnRspQryTradingAccount,
        OnRspQryInvestorPosition,
        OnRspQryInvestorPositionDetail,
        OnRspQryDepthMarketData
    };

    struct Request {
        RequestType type;
        union {
            CThostFtdcQryOrderField QryOrder;
            CThostFtdcQryTradeField QryTrade;
            CThostFtdcQryTradingAccountField QryTradingAccount;
            CThostFtdcQryInvestorPositionField QryInvestorPosition;
            CThostFtdcQryInvestorPositionDetailField QryInvestorPositionDetail;
            CThostFtdcQryDepthMarketDataField QryDepthMarketData;
        };
        int nRequestID;
    };

    struct Response {
        ResponseType type;
        union {
            char base;
            CThostFtdcRspUserLoginField RspUserLogin;
            CThostFtdcUserLogoutField UserLogout;
            CThostFtdcSpecificInstrumentField SpecificInstrument;
            CThostFtdcDepthMarketDataField DepthMarketData;
            M1Bar m1;
            TickBar tick;
            CThostFtdcSettlementInfoConfirmField SettlementInfoConfirm;
            CThostFtdcInputOrderField InputOrder;
            CThostFtdcInputOrderActionField InputOrderAction;
            CThostFtdcOrderActionField OrderAction;
            CThostFtdcOrderField Order;
            CThostFtdcTradeField Trade;
            CThostFtdcTradingAccountField TradingAccount;
            CThostFtdcInvestorPositionField InvestorPosition;
            CThostFtdcSettlementInfoField SettlementInfo;
            CThostFtdcInvestorPositionDetailField InvestorPositionDetail;
        };
        CThostFtdcRspInfoField RspInfo;
        int nRequestID;
        int nReason;
        bool bIsLast;

        Response(ResponseType type_, CThostFtdcRspInfoField *pRspInfo=nullptr, int nRequestID_=0, bool bIsLast_=true)
        : type(type_), nRequestID(nRequestID_), nReason(0), bIsLast(bIsLast_) {
            if (pRspInfo) {
                memcpy(&RspInfo, pRspInfo, sizeof RspInfo);
            } else {
                memset(&RspInfo, 0, sizeof RspInfo);
            }
        }
        Response(const Response &other) = default;
        Response(Response &&other) = default;
        Response& operator=(const Response &rhs) = default;
        Response& operator=(Response &&rhs) = default;
        ~Response() = default;

        template<class T>
        inline void SetRsp(T *pRsp) {
            if (pRsp) {
                memcpy(&base, pRsp, sizeof *pRsp);
            } else {
                memset(&base, 0, sizeof *pRsp);
            }
        }
    };

    std::atomic_bool _requestResponsed;
    moodycamel::ConcurrentQueue<CtpClient::Request*>  _requestQueue;
    moodycamel::ConcurrentQueue<CtpClient::Response*> _responseQueue;
    void ProcessRequest(CtpClient::Request *r);
    void ProcessResponse(CtpClient::Response *r);
    void Push(CtpClient::Response *r);

    friend class MdSpi;
    friend class TraderSpi;
protected:
    boost::python::list _instrumentIds;
	std::map<std::string, M1Bar> _m1Bars;

public:
    CtpClient(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password);
    CtpClient(const CtpClient&) = delete;
    CtpClient(CtpClient&&) = delete;
    CtpClient& operator=(const CtpClient&) = delete;
    CtpClient& operator=(CtpClient&&) = delete;
    virtual ~CtpClient();

    void Init();
    void Join();
    void Exit();

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
    virtual void OnTick(TickBar* bar) = 0;
    virtual void On1Min(M1Bar* bar) = 0;
    virtual void On1MinTick(M1Bar* bar) = 0;
	virtual void OnMdError(CThostFtdcRspInfoField *pRspInfo) = 0;

    virtual void OnIdle() = 0;

public:
    // TraderApi
    void QueryOrder();
    void QueryTrade();
    void QueryTradingAccount();
    void QueryInvestorPosition();
    void QueryInvestorPositionDetail();
    void QueryMarketData(std::string instrumentId, int requestId);

    void TdLogin();
    void ConfirmSettlementInfo();
    void InsertOrder(std::string instrumentId,
                     Direction direction,
                     OffsetFlag offsetFlag,
                     TThostFtdcPriceType limitPrice,
                     TThostFtdcVolumeType volume,
                     int requestId,
                     boost::python::dict extraOptions
                    );
    void OrderAction(boost::shared_ptr<CThostFtdcOrderField> pOrder,
                     OrderActionFlag actionFlag,
                     TThostFtdcPriceType limitPrice,
                     TThostFtdcVolumeType volumeChange,
                     int requestId);
    void DeleteOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder, int requestId);
    void ModifyOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder,
                     TThostFtdcPriceType limitPrice,
                     TThostFtdcVolumeType volumeChange,
                     int requestId);

public:
    // TraderSpi
	virtual void OnTdFrontConnected() = 0;
    virtual void OnTdFrontDisconnected(int nReason) = 0;
	virtual void OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnErrOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnErrOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) = 0;
	virtual void OnRtnOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder) = 0;
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) = 0;
	virtual void OnTdError(CThostFtdcRspInfoField *pRspInfo) = 0;

    virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) = 0;
    virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) = 0;
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
    void OnTick(TickBar *bar) override;
    void On1Min(M1Bar *bar) override;
    void On1MinTick(M1Bar *bar) override;
	void OnMdError(CThostFtdcRspInfoField *pRspInfo) override;
    
	void OnTdFrontConnected() override;
	void OnTdFrontDisconnected(int nReason) override;
	void OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo) override;
	void OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo) override;
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo) override;
    void OnErrOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) override;
    void OnErrOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) override;
	void OnRtnOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder) override;
	void OnRtnTrade(CThostFtdcTradeField *pTrade) override;
	void OnTdError(CThostFtdcRspInfoField *pRspInfo) override;

    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, bool bIsLast) override;
    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnIdle() override;
};
