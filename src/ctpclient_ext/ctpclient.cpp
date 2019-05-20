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
#include <string>
#include <future>
#include <sstream>
#include <iostream>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include "mdspi.h"
#include "traderspi.h"
#include "ctpclient.h"

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
    if (rc == 0) {
        // 发送成功
    } else {
        std::stringstream ss;
        ss << request << " failed because of ";
        switch (rc) {
        case -1:
            // 因网络原因发送失败
            ss << "network error.";
            throw std::runtime_error(ss.str());
        case -2:
            // 未处理请求队列总数量超限
            ss << "excessing the limit of request queue.";
            throw std::runtime_error(ss.str());
        case -3:
            // 每秒发送请求数量超限
            ss << "too frequently request.";
            throw std::runtime_error(ss.str());
        default:
            ss << "unknown reason: " << rc;
            throw std::runtime_error(ss.str());
        }
    }
}

#pragma region General functions

CtpClient::CtpClient(const std::string &mdAddr, const std::string &tdAddr, const std::string &brokerId, const std::string &userId, const std::string &password)
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

py::tuple CtpClient::GetApiVersion()
{
    std::string v1 = CThostFtdcMdApi::GetApiVersion();
    std::string v2 = CThostFtdcTraderApi::GetApiVersion();
    return py::make_tuple(v1, v2);
}

void CtpClient::Init()
{
#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

    if (_mdAddr != "") {
        auto mdFlowPath = _flowPath + PATH_SEP "md-";

        _mdApi = CThostFtdcMdApi::CreateFtdcMdApi(mdFlowPath.c_str(), /*using udp*/false, /*multicast*/false);
        _mdSpi = new MdSpi(this);
        _mdApi->RegisterSpi(_mdSpi);
        _mdApi->RegisterFront(const_cast<char*>(_mdAddr.c_str()));
        _mdApi->Init();
    }

    if (_tdAddr != "") {
        auto tdFlowPath = _flowPath + PATH_SEP "td-";

        _tdApi = CThostFtdcTraderApi::CreateFtdcTraderApi(tdFlowPath.c_str());
        _tdSpi = new TraderSpi(this);
        _tdApi->RegisterSpi(_tdSpi);
        _tdApi->SubscribePrivateTopic(THOST_TERT_QUICK);
        _tdApi->SubscribePublicTopic(THOST_TERT_QUICK);
        _tdApi->RegisterFront(const_cast<char*>(_tdAddr.c_str()));
        _tdApi->Init();
    }

    _requestResponsed = true;
    _thread = std::thread([this](std::shared_future<void> exitSignal) {
        while (exitSignal.wait_for(1100ms) == std::future_status::timeout) {
            if (_requestResponsed.load(std::memory_order_acquire)) {
                CtpClient::Request req;
                if (_requestQueue.try_dequeue(req)) {
                    _requestResponsed.store(false, std::memory_order_release);
                    ProcessRequest(&req);
                }
            }
        }
    }, g_exitSignal);
}

void CtpClient::Join()
{
    auto timer = std::chrono::steady_clock::now();
    while (g_exitSignal.wait_for(10ms) == std::future_status::timeout) {
        CtpClient::Response rsp;
        while (_responseQueue.try_dequeue(rsp)) {
            ProcessResponse(&rsp);
        }

        {
            auto duration = std::chrono::steady_clock::now() - timer;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(duration) > _idleDelay * 1ms) {
                OnIdle();
                timer = std::chrono::steady_clock::now();
            }
        }
    }

    _thread.join();
}

void CtpClient::Exit()
{
    g_exitPromise.set_value();
}

void CtpClient::Enqueue(ResponseType type, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    Response r;
    r.Init(type, pRspInfo, nRequestID, bIsLast);
    Enqueue(r);
}

void CtpClient::Enqueue(const CtpClient::Response &r)
{
    _responseQueue.enqueue(r);
}

void CtpClient::ProcessRequest(CtpClient::Request *r)
{
    switch (r->type) {
    case RequestType::QueryOrder:
        assert_request(_tdApi->ReqQryOrder(&r->QryOrder, r->nRequestID));
        break;
    case RequestType::QueryTrade:
        assert_request(_tdApi->ReqQryTrade(&r->QryTrade, r->nRequestID));
        break;
    case RequestType::QueryTradingAccount:
        assert_request(_tdApi->QueryTradingAccount(&r->QryTradingAccount, r->nRequestID));
        break;
    case RequestType::QueryInvestorPosition:
        assert_request(_tdApi->QueryInvestorPosition(&r->QryInvestorPosition, r->nRequestID));
        break;
    case RequestType::QueryInvestorPositionDetail:
        assert_request(_tdApi->ReqQryInvestorPositionDetail(&r->QryInvestorPositionDetail, r->nRequestID));
        break;
    case RequestType::QueryMarketData:
        assert_request(_tdApi->ReqQryDepthMarketData(&r->QryDepthMarketData, r->nRequestID));
        break;
    default:
        throw std::invalid_argument("unhandled request type.");
    }
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
        OnMdUserLogin(r->RspUserLogin, r->RspInfo);
        break;
    case ResponseType::OnMdUserLogout:
        OnMdUserLogout(r->UserLogout, r->RspInfo);
        break;
    case ResponseType::OnSubMarketData:
        OnSubscribeMarketData(r->SpecificInstrument, r->RspInfo, r->bIsLast);
        break;
    case ResponseType::OnUnSubMarketData:
        OnUnsubscribeMarketData(r->SpecificInstrument, r->RspInfo, r->bIsLast);
        break;
    case ResponseType::OnRtnMarketData:
    {
        auto pDepthMarketData = std::make_shared<CThostFtdcDepthMarketDataField>();
        memcpy(pDepthMarketData.get(), &r->DepthMarketData, sizeof r->DepthMarketData);
        OnRtnMarketData(pDepthMarketData);
    }
        break;
    case ResponseType::OnTick:
    {
        auto pTickBar = std::make_shared<TickBar>();
        memcpy(pTickBar.get(), &r->tick, sizeof r->tick);
        OnTick(pTickBar);
    }
        break;
    case ResponseType::On1Min:
    {
        auto pM1Bar = std::make_shared<M1Bar>();
        memcpy(pM1Bar.get(), &r->m1, sizeof r->m1);
        On1Min(pM1Bar);
    }
        break;
    case ResponseType::On1MinTick:
    {
        auto pM1Bar = std::make_shared<M1Bar>();
        memcpy(pM1Bar.get(), &r->m1, sizeof r->m1);
        On1MinTick(pM1Bar);
    }
        break;
    case ResponseType::OnMdError:
        OnMdError(r->RspInfo);
        break;
    case ResponseType::OnTdFrontConnected:
        OnTdFrontConnected();
        break;
    case ResponseType::OnTdFrontDisconnected:
        OnTdFrontDisconnected(r->nReason);
        break;
    case ResponseType::OnTdUserLogin:
        OnTdUserLogin(r->RspUserLogin, r->RspInfo);
        break;
    case ResponseType::OnTdUserLogout:
        OnTdUserLogout(r->UserLogout, r->RspInfo);
        break;
    case ResponseType::OnSettlementInfoConfirm:
        OnRspSettlementInfoConfirm(r->SettlementInfoConfirm, r->RspInfo);
        break;
    case ResponseType::OnRspOrderInsert:
        OnErrOrderInsert(r->InputOrder, r->RspInfo);
        break;
    case ResponseType::OnRspOrderAction:
        OnErrOrderAction(r->InputOrderAction, r->OrderAction, r->RspInfo);
        break;
    case ResponseType::OnErrRtnOrderInsert:
        OnErrOrderInsert(r->InputOrder, r->RspInfo);
        break;
    case ResponseType::OnErrRtnOrderAction:
        OnErrOrderAction(r->InputOrderAction, r->OrderAction, r->RspInfo);
        break;
    case ResponseType::OnRtnOrder:
    {
        auto pOrder = std::make_shared<CThostFtdcOrderField>();
        memcpy(pOrder.get(), &r->Order, sizeof r->Order);
        OnRtnOrder(pOrder);
    }
        break;
    case ResponseType::OnRtnTrade:
        OnRtnTrade(r->Trade);
        break;
    case ResponseType::OnTdError:
        OnTdError(r->RspInfo);
        break;
    case ResponseType::OnRspQryOrder:
    {
        auto pOrder = std::make_shared<CThostFtdcOrderField>();
        memcpy(pOrder.get(), &r->Order, sizeof r->Order);
        OnRspQryOrder(pOrder, r->RspInfo, r->bIsLast);
    }
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryTrade:
        OnRspQryTrade(r->Trade, r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryTradingAccount:
        OnRspQryTradingAccount(r->TradingAccount, r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryInvestorPosition:
        OnRspQryInvestorPosition(r->InvestorPosition, r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryDepthMarketData:
        OnRspQryDepthMarketData(r->DepthMarketData, r->RspInfo, r->nRequestID, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryInvestorPositionDetail:
        OnRspQryInvestorPositionDetail(r->InvestorPositionDetail, r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    default:
        throw std::invalid_argument("unhandled response type.");
    }
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

void CtpClient::SubscribeMarketData(const std::vector<std::string> &instrumentIds)
{
    size_t N = instrumentIds.size();
    if (N == 0) return;

    char **ppInstrumentIDs = new char*[N];
    for (size_t i = 0; i < N; i++) {
        ppInstrumentIDs[i] = const_cast<char*>(instrumentIds[i].c_str());
    }
    _mdApi->SubscribeMarketData(ppInstrumentIDs, N);
    delete[] ppInstrumentIDs;
}

void CtpClient::UnsubscribeMarketData(const std::vector<std::string> &instrumentIds)
{
    size_t N = instrumentIds.size();
    if (N == 0) return;

    char **ppInstrumentIDs = new char*[N];
    for (size_t i = 0; i < N; i++) {
        ppInstrumentIDs[i] = const_cast<char*>(instrumentIds[i].c_str());
    }
    _mdApi->UnSubscribeMarketData(ppInstrumentIDs, N);
    delete[] ppInstrumentIDs;
}

#pragma endregion // Market Data API


#pragma region Market Data SPI

void CtpClientWrap::OnMdFrontConnected()
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_front_connected",
        OnMdFrontConnected
    );
}

void CtpClientWrap::OnMdFrontDisconnected(int nReason)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_front_disconnected",
        OnMdFrontDisconnected,
        nReason
    );
}

void CtpClientWrap::OnMdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_user_login",
        OnMdUserLogin,
        RspUserLogin,
        RspInfo
    );
}

void CtpClientWrap::OnMdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_user_logout",
        OnMdUserLogout,
        UserLogout,
        RspInfo
    );
}

void CtpClientWrap::OnSubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_subscribe_market_data",
        OnSubscribeMarketData,
        SpecificInstrument,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnUnsubscribeMarketData(const CThostFtdcSpecificInstrumentField &SpecificInstrument, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_unsubscribe_market_data",
        OnUnsubscribeMarketData,
        SpecificInstrument,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRtnMarketData(std::shared_ptr<CThostFtdcDepthMarketDataField> pDepthMarketData)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_rtn_market_data",
        OnRtnMarketData,
        pDepthMarketData
    );
}

void CtpClientWrap::OnTick(std::shared_ptr<TickBar> pBar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_tick",
        OnTick,
        pBar
    );
}

void CtpClientWrap::On1Min(std::shared_ptr<M1Bar> pBar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_1min",
        On1Min,
        pBar
    );
}

void CtpClientWrap::On1MinTick(std::shared_ptr<M1Bar> pBar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_1min_tick",
        On1MinTick,
        pBar
    );
}

void CtpClientWrap::OnMdError(const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_error",
        OnMdError,
        RspInfo
    );
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
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryOrder;
    strncpy(r.QryOrder.BrokerID, _brokerId.c_str(), sizeof r.QryOrder.BrokerID);
    strncpy(r.QryOrder.InvestorID, _userId.c_str(), sizeof r.QryOrder.InvestorID);

    _requestQueue.enqueue(r);
}

void CtpClient::QueryTrade()
{
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryTrade;
    strncpy(r.QryTrade.BrokerID, _brokerId.c_str(), sizeof r.QryTrade.BrokerID);
    strncpy(r.QryTrade.InvestorID, _userId.c_str(), sizeof r.QryTrade.InvestorID);

    _requestQueue.enqueue(r);
}

void CtpClient::QueryTradingAccount()
{
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryTradingAccount;
    strncpy(r.QryTradingAccount.BrokerID, _brokerId.c_str(), sizeof r.QryTradingAccount.BrokerID);
    strncpy(r.QryTradingAccount.InvestorID, _userId.c_str(), sizeof r.QryTradingAccount.InvestorID);
    strncpy(r.QryTradingAccount.CurrencyID, "CNY", sizeof r.QryTradingAccount.CurrencyID);

    _requestQueue.enqueue(r);
}

void CtpClient::QueryInvestorPosition()
{
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryInvestorPosition;
    strncpy(r.QryInvestorPosition.BrokerID, _brokerId.c_str(), sizeof r.QryInvestorPosition.BrokerID);
    strncpy(r.QryInvestorPosition.InvestorID, _userId.c_str(), sizeof r.QryInvestorPosition.InvestorID);
    // 不填写合约则返回所有持仓
    // strncpy(r.QryInvestorPosition.InstrumentID, "", sizeof r.QryInvestorPosition.InstrumentID);

    _requestQueue.enqueue(r);
}

void CtpClient::QueryInvestorPositionDetail()
{
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryInvestorPositionDetail;
    strncpy(r.QryInvestorPositionDetail.BrokerID, _brokerId.c_str(), sizeof r.QryInvestorPositionDetail.BrokerID);
    strncpy(r.QryInvestorPositionDetail.InvestorID, _userId.c_str(), sizeof r.QryInvestorPositionDetail.InvestorID);
    // 不填写合约则返回所有持仓
    // strncpy(r.QryInvestorPositionDetail.InstrumentID, "", sizeof r.QryInvestorPositionDetail.InstrumentID);

    _requestQueue.enqueue(r);
}

void CtpClient::QueryMarketData(std::string instrumentId, int nRequestID)
{
    CtpClient::Request r;
    memset(&r, 0, sizeof r);
    r.type = RequestType::QueryMarketData;
    strncpy(r.QryDepthMarketData.InstrumentID, instrumentId.c_str(), sizeof r.QryDepthMarketData.InstrumentID);
    r.nRequestID = nRequestID;

    _requestQueue.enqueue(r);
}

void CtpClient::InsertOrder(const std::string &instrumentId, Direction direction, OffsetFlag offsetFlag, TThostFtdcPriceType limitPrice, TThostFtdcVolumeType volume, py::kwargs kwargs)
{
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof req);
    strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
    strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);

    strncpy(req.InstrumentID, instrumentId.c_str(), sizeof req.InstrumentID);
    req.VolumeTotalOriginal = volume;
    req.LimitPrice = limitPrice;
    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    req.TimeCondition = THOST_FTDC_TC_GFD;
    req.VolumeCondition = THOST_FTDC_VC_AV;
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    req.MinVolume = 1;
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    req.RequestID = 0;

    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    req.TimeCondition = THOST_FTDC_TC_GFD;
    req.VolumeCondition = THOST_FTDC_VC_AV;
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    req.MinVolume = 1;
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    req.RequestID = 0;

    req.Direction = (TThostFtdcDirectionType)direction;
    req.CombOffsetFlag[0] = (TThostFtdcOffsetFlagType)offsetFlag;

    if (kwargs.contains("order_price_type")) {
        req.OrderPriceType = (TThostFtdcOrderPriceTypeType)kwargs["order_price_type"].cast<OrderPriceType>();
    }

    if (kwargs.contains("hedge_flag")) {
        req.CombHedgeFlag[0] = (TThostFtdcHedgeFlagType)kwargs["hedge_flag"].cast<HedgeFlag>();
    }

    if (kwargs.contains("time_condition")) {
        req.TimeCondition = (TThostFtdcTimeConditionType)kwargs["time_conditino"].cast<TimeCondition>();
    }

    if (kwargs.contains("volume_condition")) {
        req.VolumeCondition = (TThostFtdcVolumeConditionType)kwargs["volume_condition"].cast<VolumeCondition>();
    }

    if (kwargs.contains("contingent_condition")) {
        req.ContingentCondition = (TThostFtdcContingentConditionType)kwargs["contingent_condition"].cast<ContingentCondition>();
    }

    if (kwargs.contains("min_volume")) {
        req.MinVolume = kwargs["min_volume"].cast<int>();
    }

    if (kwargs.contains("request_id")) {
        req.RequestID = kwargs["request_id"].cast<int>();
    }

    assert_request(_tdApi->ReqOrderInsert(&req, req.RequestID));
}

void CtpClient::OrderAction(
    std::shared_ptr<CThostFtdcOrderField> pOrder,
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
    req.ActionFlag = (TThostFtdcActionFlagType)actionFlag;
    req.LimitPrice = limitPrice;
    req.VolumeChange = volumeChange;

    assert_request(_tdApi->ReqOrderAction(&req, requestId));
}

void CtpClient::DeleteOrder(std::shared_ptr<CThostFtdcOrderField> pOrder, int requestId)
{
    OrderAction(pOrder, OrderActionFlag::AF_Delete, 0.0, 0, requestId);
}

#pragma endregion // Trader API


#pragma region Trader SPI

void CtpClientWrap::OnTdFrontConnected()
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_front_connected",
        OnTdFrontConnected
    );
}

void CtpClientWrap::OnTdFrontDisconnected(int nReason)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_front_disconnected",
        OnTdFrontDisconnected,
        nReason
    );
}

void CtpClientWrap::OnTdUserLogin(const CThostFtdcRspUserLoginField &RspUserLogin, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_user_login",
        OnTdUserLogin,
        RspUserLogin,
        RspInfo
    );
}

void CtpClientWrap::OnTdUserLogout(const CThostFtdcUserLogoutField &UserLogout, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_user_logout",
        OnTdUserLogout,
        UserLogout,
        RspInfo
    );
}

void CtpClientWrap::OnRspSettlementInfoConfirm(const CThostFtdcSettlementInfoConfirmField &SettlementInfoConfirm, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_settlement_info_confirm",
        OnRspSettlementInfoConfirm,
        SettlementInfoConfirm,
        RspInfo
    );
}

void CtpClientWrap::OnErrOrderInsert(const CThostFtdcInputOrderField &InputOrder, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_err_order_insert",
        OnErrOrderInsert,
        InputOrder,
        RspInfo
    );
}

void CtpClientWrap::OnErrOrderAction(const CThostFtdcInputOrderActionField &InputOrderAction, const CThostFtdcOrderActionField &OrderAction, const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_err_order_action",
        OnErrOrderAction,
        InputOrderAction,
        OrderAction,
        RspInfo
    );
}

void CtpClientWrap::OnRtnOrder(std::shared_ptr<CThostFtdcOrderField> pOrder)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_rtn_order",
        OnRtnOrder,
        pOrder
    );
}

void CtpClientWrap::OnRtnTrade(const CThostFtdcTradeField &Trade)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_rtn_trade",
        OnRtnTrade,
        Trade
    );
}

void CtpClientWrap::OnTdError(const CThostFtdcRspInfoField &RspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_error",
        OnTdError,
        RspInfo
    );
}

void CtpClientWrap::OnRspQryOrder(std::shared_ptr<CThostFtdcOrderField> pOrder, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_order",
        OnRspQryOrder,
        pOrder,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryTrade(const CThostFtdcTradeField &Trade, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_trade",
        OnRspQryTrade,
        Trade,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryTradingAccount(const CThostFtdcTradingAccountField &TradingAccount, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_trading_account",
        OnRspQryTradingAccount,
        TradingAccount,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryInvestorPosition(const CThostFtdcInvestorPositionField &InvestorPosition, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_investor_position",
        OnRspQryInvestorPosition,
        InvestorPosition,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField &InvestorPositionDetail, const CThostFtdcRspInfoField &RspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_investor_position_detail",
        OnRspQryInvestorPositionDetail,
        InvestorPositionDetail,
        RspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryDepthMarketData(const CThostFtdcDepthMarketDataField &DepthMarketData, const CThostFtdcRspInfoField &RspInfo, int nRequestID, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_market_data",
        OnRspQryDepthMarketData,
        DepthMarketData,
        RspInfo,
        nRequestID,
        bIsLast
    );
}

#pragma endregion // Trader SPI

void CtpClientWrap::OnIdle()
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_idle",
        OnIdle
    );
}
