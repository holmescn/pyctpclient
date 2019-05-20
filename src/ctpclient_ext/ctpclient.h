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
#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ThostFtdcUserApiStruct.h"
#include "bar.h"
#include "concurrentqueue.h"

namespace py = pybind11;

class MdSpi;
class TraderSpi;
class CThostFtdcMdApi;
class CThostFtdcTraderApi;

#pragma region Enums

enum Direction {
    D_Buy = THOST_FTDC_D_Buy,
    D_Sell = THOST_FTDC_D_Sell
};

enum OffsetFlag {
    OF_Open = THOST_FTDC_OF_Open,
    OF_Close = THOST_FTDC_OF_Close,
    OF_ForceClose = THOST_FTDC_OF_ForceClose,
    OF_CloseToday = THOST_FTDC_OF_CloseToday,
    OF_CloseYesterday = THOST_FTDC_OF_CloseYesterday,
    OF_ForceOff = THOST_FTDC_OF_ForceOff,
    OF_LocalForceClose = THOST_FTDC_OF_LocalForceClose
};

enum OrderPriceType {
    OPT_AnyPrice, OPT_LimitPrice, OPT_BestPrice,
    OPT_LastPrice, OPT_LastPricePlusOneTick, OPT_LastPricePlusTwoTicks, OPT_LastPricePlusThreeTicks,
    OPT_AskPrice1, OPT_AskPrice1PlusOneTick, OPT_AskPrice1PlusTwoTicks, OPT_AskPrice1PlusThreeTicks,
    OPT_BidPrice1, OPT_BidPrice1PlusOneTick, OPT_BidPrice1PlusTwoTicks, OPT_BidPrice1PlusThreeTicks,
    OPT_FiveLevelPrice
};

enum HedgeFlag {
    HF_Speculation = THOST_FTDC_HF_Speculation,
    HF_Arbitrage = THOST_FTDC_HF_Arbitrage,
    HF_Hedge = THOST_FTDC_HF_Hedge,
    HF_MarketMaker = THOST_FTDC_HF_MarketMaker
};

enum TimeCondition {
    TC_IOC = THOST_FTDC_TC_IOC,
    TC_GFS = THOST_FTDC_TC_GFS,
    TC_GFD = THOST_FTDC_TC_GFD,
    TC_GTD = THOST_FTDC_TC_GTD,
    TC_GTC = THOST_FTDC_TC_GTC,
    TC_GFA = THOST_FTDC_TC_GFA
};

enum VolumeCondition {
    VC_AV = THOST_FTDC_VC_AV,
    VC_MV = THOST_FTDC_VC_MV,
    VC_CV = THOST_FTDC_VC_CV
};

enum ContingentCondition {
    CC_Immediately = THOST_FTDC_CC_Immediately,
    CC_Touch = THOST_FTDC_CC_Touch,
    CC_TouchProfit = THOST_FTDC_CC_TouchProfit,
    CC_ParkedOrder = THOST_FTDC_CC_ParkedOrder,
    CC_LastPriceGreaterThanStopPrice = THOST_FTDC_CC_LastPriceGreaterThanStopPrice,
    CC_LastPriceGreaterEqualStopPrice = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice,
    CC_LastPriceLesserThanStopPrice = THOST_FTDC_CC_LastPriceLesserThanStopPrice,
    CC_LastPriceLesserEqualStopPrice = THOST_FTDC_CC_LastPriceLesserEqualStopPrice,
    CC_AskPriceGreaterThanStopPrice = THOST_FTDC_CC_AskPriceGreaterThanStopPrice,
    CC_AskPriceGreaterEqualStopPrice = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice,
    CC_AskPriceLesserThanStopPrice = THOST_FTDC_CC_AskPriceLesserThanStopPrice,
    CC_AskPriceLesserEqualStopPrice = THOST_FTDC_CC_AskPriceLesserEqualStopPrice,
    CC_BidPriceGreaterThanStopPrice = THOST_FTDC_CC_BidPriceGreaterThanStopPrice,
    CC_BidPriceGreaterEqualStopPrice = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice,
    CC_BidPriceLesserThanStopPrice = THOST_FTDC_CC_BidPriceLesserThanStopPrice,
    CC_BidPriceLesserEqualStopPrice = THOST_FTDC_CC_BidPriceLesserEqualStopPrice
};

enum OrderActionFlag {
    AF_Delete = THOST_FTDC_AF_Delete,
    AF_Modify = THOST_FTDC_AF_Modify
};

enum OrderStatus {
    OST_AllTraded = THOST_FTDC_OST_AllTraded,
    OST_PartTradedQueueing = THOST_FTDC_OST_PartTradedQueueing,
    OST_PartTradedNotQueueing = THOST_FTDC_OST_PartTradedNotQueueing,
    OST_NoTradeQueueing = THOST_FTDC_OST_NoTradeQueueing,
    OST_NoTradeNotQueueing = THOST_FTDC_OST_NoTradeNotQueueing,
    OST_Canceled = THOST_FTDC_OST_Canceled,
    OST_Unknown = THOST_FTDC_OST_Unknown,
    OST_NotTouched = THOST_FTDC_OST_NotTouched,
    OST_Touched = THOST_FTDC_OST_Touched
};

enum OrderSubmitStatus {
    OSS_InsertSubmitted = THOST_FTDC_OSS_InsertSubmitted,
    OSS_CancelSubmitted = THOST_FTDC_OSS_CancelSubmitted,
    OSS_ModifySubmitted = THOST_FTDC_OSS_ModifySubmitted,
    OSS_Accepted = THOST_FTDC_OSS_Accepted,
    OSS_InsertRejected = THOST_FTDC_OSS_InsertRejected,
    OSS_CancelRejected = THOST_FTDC_OSS_CancelRejected,
    OSS_ModifyRejected = THOST_FTDC_OSS_ModifyRejected
};

enum OrderActionStatus {
    OAS_Submitted = THOST_FTDC_OAS_Submitted,
    OAS_Accepted = THOST_FTDC_OAS_Accepted,
    OAS_Rejected = THOST_FTDC_OAS_Rejected
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
    size_t _idleDelay = 1000;

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
        OnTick,
        On1Min,
        On1MinTick,
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

        inline void Init(ResponseType type, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            memset(this, 0, sizeof *this);
            this->type = type;
            this->nRequestID = nRequestID;
            this->bIsLast = bIsLast;    
            if (pRspInfo) {
                memcpy(&this->RspInfo, pRspInfo, sizeof this->RspInfo);
            } else {
                this->RspInfo.ErrorID = -1;
            }
        }
    };

    std::atomic_bool _requestResponsed;
    moodycamel::ConcurrentQueue<CtpClient::Request>  _requestQueue;
    moodycamel::ConcurrentQueue<CtpClient::Response> _responseQueue;
    void ProcessRequest(CtpClient::Request *r);
    void ProcessResponse(CtpClient::Response *r);

    template<class T>
    void Enqueue(ResponseType type, T *pRsp, CThostFtdcRspInfoField *pRspInfo=nullptr, int nRequestID=0, bool bIsLast=true) {
        Response r;
        r.Init(type, pRspInfo, nRequestID, bIsLast);
        if (pRsp) {
            memcpy(&r.base, pRsp, sizeof *pRsp);
        }

        Enqueue(r);
    }
    void Enqueue(ResponseType type, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void Enqueue(const CtpClient::Response &r);

    friend class MdSpi;
    friend class TraderSpi;
protected:
    std::vector<std::string> _instrumentIds;

public:
    CtpClient(const std::string &mdAddr, const std::string &tdAddr, const std::string &brokerId, const std::string &userId, const std::string &password);
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
    inline const std::vector<std::string>& GetInstrumentIds() const { return _instrumentIds; }
    inline void SetInstrumentIds(const std::vector<std::string> &instrumentIds) {
        _instrumentIds.clear();
        for (auto &id : instrumentIds) {
            _instrumentIds.push_back(id);
        }
    }
    inline size_t GetIdleDelay() const { return _idleDelay; }
    inline void SetIdleDelay(size_t delay) { _idleDelay = delay; }

    static py::tuple GetApiVersion();

public:
    // MdApi
    void MdLogin();
    void SubscribeMarketData(const std::vector<std::string> &instrumentIds);
    void UnsubscribeMarketData(const std::vector<std::string> &instrumentIds);

public:
    // MdSpi
	virtual void OnMdFrontConnected() = 0;
	virtual void OnMdFrontDisconnected(int nReason) = 0;
	virtual void OnMdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnMdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo) = 0;
    virtual void OnSubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnUnsubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRtnMarketData(std::shared_ptr<CThostFtdcDepthMarketDataField> pDepthMarketData) = 0;
    virtual void OnTick(std::shared_ptr<TickBar> pBar) = 0;
    virtual void On1Min(std::shared_ptr<M1Bar> pBar) = 0;
    virtual void On1MinTick(std::shared_ptr<M1Bar> pBar) = 0;
	virtual void OnMdError(const CThostFtdcRspInfoField &RspInfo) = 0;

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
    void InsertOrder(
        const std::string &instrumentId,
        Direction direction,
        OffsetFlag offsetFlag,
        TThostFtdcPriceType limitPrice,
        TThostFtdcVolumeType volume,
        py::kwargs kwargs);
    void OrderAction(std::shared_ptr<CThostFtdcOrderField> pOrder, 
        OrderActionFlag actionFlag,
        TThostFtdcPriceType limitPrice,
        TThostFtdcVolumeType volumeChange,
        int requestId);
    void DeleteOrder(std::shared_ptr<CThostFtdcOrderField> pOrder, int requestId);

public:
    // TraderSpi
	virtual void OnTdFrontConnected() = 0;
    virtual void OnTdFrontDisconnected(int nReason) = 0;
	virtual void OnTdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnTdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnRspSettlementInfoConfirm(const CThostFtdcSettlementInfoConfirmField &SettlementInfoConfirm, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnErrOrderInsert(const CThostFtdcInputOrderField &InputOrder, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnErrOrderAction(const CThostFtdcInputOrderActionField &InputOrderAction, const CThostFtdcOrderActionField &OrderAction, const CThostFtdcRspInfoField &RspInfo) = 0;
	virtual void OnRtnOrder(std::shared_ptr<CThostFtdcOrderField> pOrder) = 0;
	virtual void OnRtnTrade(const CThostFtdcTradeField &Trade) = 0;
	virtual void OnTdError(const CThostFtdcRspInfoField &RspInfo) = 0;

    virtual void OnRspQryOrder(std::shared_ptr<CThostFtdcOrderField> pOrder, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTrade(const CThostFtdcTradeField &Trade, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRspQryTradingAccount(const CThostFtdcTradingAccountField &TradingAccount, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRspQryInvestorPosition(const CThostFtdcInvestorPositionField &InvestorPosition, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRspQryInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField &InvestorPositionDetail, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) = 0;
    virtual void OnRspQryDepthMarketData(const CThostFtdcDepthMarketDataField &DepthMarketData, const CThostFtdcRspInfoField &RspInfo, int nRequestID, bool bIsLast) = 0;
};


struct CtpClientWrap : CtpClient
{
    /* Inherit the constructors */
    using CtpClient::CtpClient;

	void OnMdFrontConnected() override;
	void OnMdFrontDisconnected(int nReason) override;
	void OnMdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo) override;
	void OnMdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo) override;
    void OnSubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnUnsubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRtnMarketData(std::shared_ptr<CThostFtdcDepthMarketDataField> pDepthMarketData) override;
    void OnTick(std::shared_ptr<TickBar> pBar) override;
    void On1Min(std::shared_ptr<M1Bar> pBar) override;
    void On1MinTick(std::shared_ptr<M1Bar> pBar) override;
	void OnMdError(const CThostFtdcRspInfoField &RspInfo) override;

	void OnTdFrontConnected() override;
	void OnTdFrontDisconnected(int nReason) override;
	void OnTdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo) override;
	void OnTdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo) override;
	void OnRspSettlementInfoConfirm(const CThostFtdcSettlementInfoConfirmField &SettlementInfoConfirm, const CThostFtdcRspInfoField &RspInfo) override;
    void OnErrOrderInsert(const CThostFtdcInputOrderField &InputOrder, const CThostFtdcRspInfoField &RspInfo) override;
    void OnErrOrderAction(const CThostFtdcInputOrderActionField &InputOrderAction, const CThostFtdcOrderActionField &OrderAction, const CThostFtdcRspInfoField &RspInfo) override;
	void OnRtnOrder(std::shared_ptr<CThostFtdcOrderField> pOrder) override;
	void OnRtnTrade(const CThostFtdcTradeField &Trade) override;
	void OnTdError(const CThostFtdcRspInfoField &RspInfo) override;

    void OnRspQryOrder(std::shared_ptr<CThostFtdcOrderField> pOrder, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRspQryTrade(const CThostFtdcTradeField &Trade, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRspQryTradingAccount(const CThostFtdcTradingAccountField &TradingAccount, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRspQryInvestorPosition(const CThostFtdcInvestorPositionField &InvestorPosition, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRspQryInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField &InvestorPositionDetail, const CThostFtdcRspInfoField &RspInfo, bool bIsLast) override;
    void OnRspQryDepthMarketData(const CThostFtdcDepthMarketDataField &DepthMarketData, const CThostFtdcRspInfoField &RspInfo, int nRequestID, bool bIsLast) override;

    void OnIdle() override;
};
