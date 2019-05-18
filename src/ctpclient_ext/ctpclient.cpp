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
            if (std::chrono::duration_cast<std::chrono::milliseconds>(duration) > 1s) {
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
        OnRtnOrder(std::shared_ptr<CThostFtdcOrderField>(pNewOrder));
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
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryTrade:
        OnRspQryTrade(&r->Trade, &r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryTradingAccount:
        OnRspQryTradingAccount(&r->TradingAccount, &r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryInvestorPosition:
        OnRspQryInvestorPosition(&r->InvestorPosition, &r->RspInfo, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryDepthMarketData:
        OnRspQryDepthMarketData(&r->DepthMarketData, &r->RspInfo, r->nRequestID, r->bIsLast);
        _requestResponsed.store(true, std::memory_order_release);
        break;
    case ResponseType::OnRspQryInvestorPositionDetail:
        OnRspQryInvestorPositionDetail(&r->InvestorPositionDetail, &r->RspInfo, r->bIsLast);
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

void CtpClientWrap::OnMdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_user_login",
        OnMdUserLogin,
        pRspUserLogin,
        pRspInfo
    );
}

void CtpClientWrap::OnMdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_user_logout",
        OnMdUserLogout,
        pUserLogout,
        pRspInfo
    );
}

void CtpClientWrap::OnSubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_subscribe_market_data",
        OnSubscribeMarketData,
        pSpecificInstrument,
        pRspInfo
    );
}

void CtpClientWrap::OnUnsubscribeMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_unsubscribe_market_data",
        OnUnsubscribeMarketData,
        pSpecificInstrument,
        pRspInfo
    );
}

void CtpClientWrap::OnRtnMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_rtn_market_data",
        OnRtnMarketData,
        pDepthMarketData
    );
}

void CtpClientWrap::OnTick(TickBar *bar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_tick",
        OnTick,
        bar
    );
}

void CtpClientWrap::On1Min(M1Bar *bar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_1min",
        On1Min,
        bar
    );
}

void CtpClientWrap::On1MinTick(M1Bar *bar)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_1min_tick",
        On1MinTick,
        bar
    );
}

void CtpClientWrap::OnMdError(CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_md_error",
        OnMdError,
        pRspInfo
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

// void CtpClient::InsertOrder(
//     std::string instrumentId,
//     Direction direction,
//     OffsetFlag offsetFlag,
//     TThostFtdcPriceType limitPrice,
//     TThostFtdcVolumeType volume,
//     int requestId,
//     boost::python::dict extraOptions)
// {
//     CThostFtdcInputOrderField req;
//     memset(&req, 0, sizeof req);
//     strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
//     strncpy(req.InvestorID, _userId.c_str(), sizeof req.InvestorID);
//     strncpy(req.InstrumentID, instrumentId.c_str(), sizeof req.InstrumentID);
//     req.VolumeTotalOriginal = volume;
//     req.LimitPrice = limitPrice;

//     switch(direction) {
//         case D_Buy:
//             req.Direction = THOST_FTDC_D_Buy;
//             break;
//         case D_Sell:
//             req.Direction = THOST_FTDC_D_Sell;
//             break;
//         default:
//             throw std::invalid_argument("direction");
//     }

//     switch (offsetFlag) {
//         case OF_Open:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
//             break;
//         case OF_Close:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
//             break;
//         case OF_ForceClose:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceClose;
//             break;
//         case OF_CloseToday:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
//             break;
//         case OF_CloseYesterday:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
//             break;
//         case OF_ForceOff:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_ForceOff;
//             break;
//         case OF_LocalForceClose:
//             req.CombOffsetFlag[0] = THOST_FTDC_OF_LocalForceClose;
//             break;
//         default:
//             throw std::invalid_argument("offset_flag");
//     }

//     if (extraOptions.has_key("order_price_type")) {
//         OrderPriceType opt = extract<OrderPriceType>(extraOptions["order_price_type"]);
//         switch (opt) {
//         case OPT_AnyPrice:
//             req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
//             break;
//         case OPT_LimitPrice:
//             req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
//             break;
//         case OPT_BestPrice:
//             req.OrderPriceType = THOST_FTDC_OPT_BestPrice;
//             break;
//         case OPT_LastPrice:
//             req.OrderPriceType = THOST_FTDC_OPT_LastPrice;
//             break;
//         case OPT_LastPricePlusOneTick:
//             req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusOneTicks;
//             break;
//         case OPT_LastPricePlusTwoTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusTwoTicks;
//             break;
//         case OPT_LastPricePlusThreeTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_LastPricePlusThreeTicks;
//             break;
//         case OPT_AskPrice1:
//             req.OrderPriceType = THOST_FTDC_OPT_AskPrice1;
//             break;
//         case OPT_AskPrice1PlusOneTick:
//             req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusOneTicks;
//             break;
//         case OPT_AskPrice1PlusTwoTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusTwoTicks;
//             break;
//         case OPT_AskPrice1PlusThreeTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_AskPrice1PlusThreeTicks;
//             break;
//         case OPT_BidPrice1:
//             req.OrderPriceType = THOST_FTDC_OPT_BidPrice1;
//             break;
//         case OPT_BidPrice1PlusOneTick:
//             req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusOneTicks;
//             break;
//         case OPT_BidPrice1PlusTwoTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusTwoTicks;
//             break;
//         case OPT_BidPrice1PlusThreeTicks:
//             req.OrderPriceType = THOST_FTDC_OPT_BidPrice1PlusThreeTicks;
//             break;
//         case OPT_FiveLevelPrice:
//             req.OrderPriceType = THOST_FTDC_OPT_FiveLevelPrice;
//             break;
//         default:
//             throw std::invalid_argument("order_price_type");
//         }
//     } else {
//         req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
//     }

//     if (extraOptions.has_key("hedge_flag")) {
//         HedgeFlag hf = extract<HedgeFlag>(extraOptions["hedge_flag"]);
//         switch (hf) {
//         case HF_Speculation:
//             req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//             break;
//         case HF_Arbitrage:
//             req.CombHedgeFlag[0] = THOST_FTDC_HF_Arbitrage;
//             break;
//         case HF_Hedge:
//             req.CombHedgeFlag[0] = THOST_FTDC_HF_Hedge;
//             break;
//         case HF_MarketMaker:
//             req.CombHedgeFlag[0] = THOST_FTDC_HF_MarketMaker;
//             break;
//         default:
//             throw std::invalid_argument("hedge_flag");
//         }
//     } else {
//         req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//     }

//     if (extraOptions.has_key("time_condition")) {
//         TimeCondition tc = extract<TimeCondition>(extraOptions["time_conditino"]);
//         switch (tc) {
//         case TC_IOC:
//             req.TimeCondition = THOST_FTDC_TC_IOC;
//             break;
//         case TC_GFS:
//             req.TimeCondition = THOST_FTDC_TC_GFS;
//             break;
//         case TC_GFD:
//             req.TimeCondition = THOST_FTDC_TC_GFD;
//             break;
//         case TC_GTD:
//             req.TimeCondition = THOST_FTDC_TC_GTD;
//             break;
//         case TC_GTC:
//             req.TimeCondition = THOST_FTDC_TC_GTC;
//             break;
//         case TC_GFA:
//             req.TimeCondition = THOST_FTDC_TC_GFA;
//             break;
//         default:
//             throw std::invalid_argument("time_condition");
//         }
//     } else {
//         req.TimeCondition = THOST_FTDC_TC_GFD;
//     }

//     if (extraOptions.has_key("volume_condition")) {
//         VolumeCondition vc = extract<VolumeCondition>(extraOptions["volume_condition"]);
//         switch (vc) {
//         case VC_AV:
//             req.VolumeCondition = THOST_FTDC_VC_AV;
//             break;
//         case VC_MV:
//             req.VolumeCondition = THOST_FTDC_VC_MV;
//             break;
//         case VC_CV:
//             req.VolumeCondition = THOST_FTDC_VC_CV;
//             break;
//         default:
//             throw std::invalid_argument("volume_condition");
//         }
//     } else {
//         req.VolumeCondition = THOST_FTDC_VC_AV;
//     }

//     if (extraOptions.has_key("contingent_condition")) {
//         ContingentCondition cc = extract<ContingentCondition>(extraOptions["contingent_condition"]);
//         switch (cc) {
//         case CC_Immediately:
//             req.ContingentCondition = THOST_FTDC_CC_Immediately;
//             break;
//         case CC_Touch:
//             req.ContingentCondition = THOST_FTDC_CC_Touch;
//             break;
//         case CC_TouchProfit:
//             req.ContingentCondition = THOST_FTDC_CC_TouchProfit;
//             break;
//         case CC_ParkedOrder:
//             req.ContingentCondition = THOST_FTDC_CC_ParkedOrder;
//             break;
//         case CC_LastPriceGreaterThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;
//             break;
//         case CC_LastPriceGreaterEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice;
//             break;
//         case CC_LastPriceLesserThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserThanStopPrice;
//             break;
//         case CC_LastPriceLesserEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
//             break;
//         case CC_AskPriceGreaterThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterThanStopPrice;
//             break;
//         case CC_AskPriceGreaterEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice;
//             break;
//         case CC_AskPriceLesserThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserThanStopPrice;
//             break;
//         case CC_AskPriceLesserEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_AskPriceLesserEqualStopPrice;
//             break;
//         case CC_BidPriceGreaterThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterThanStopPrice;
//             break;
//         case CC_BidPriceGreaterEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice;
//             break;
//         case CC_BidPriceLesserThanStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserThanStopPrice;
//             break;
//         case CC_BidPriceLesserEqualStopPrice:
//             req.ContingentCondition = THOST_FTDC_CC_BidPriceLesserEqualStopPrice;
//             break;
//         default:
//             throw std::invalid_argument("contingent_condition");
//         }
//     } else {
//         req.ContingentCondition = THOST_FTDC_CC_Immediately;
//     }

//     if (extraOptions.has_key("min_volume")) {
//         req.MinVolume = extract<int>(extraOptions["min_volume"]);
//     } else {
//         req.MinVolume = 1;
//     }

//     assert_request(_tdApi->ReqOrderInsert(&req, requestId));
// }

// void CtpClient::OrderAction(
//     boost::shared_ptr<CThostFtdcOrderField> pOrder,
//     OrderActionFlag actionFlag,
//     TThostFtdcPriceType limitPrice,
//     TThostFtdcVolumeType volumeChange,
//     int requestId)
// {
//     CThostFtdcInputOrderActionField req;
//     memset(&req, 0, sizeof req);

//     strncpy(req.BrokerID, pOrder->BrokerID, sizeof req.BrokerID);
//     strncpy(req.InvestorID, pOrder->InvestorID, sizeof req.InvestorID);
//     strncpy(req.OrderRef, pOrder->OrderRef, sizeof req.OrderRef);
//     strncpy(req.ExchangeID, pOrder->ExchangeID, sizeof req.ExchangeID);
//     strncpy(req.OrderSysID, pOrder->OrderSysID, sizeof req.OrderSysID);
//     strncpy(req.InstrumentID, pOrder->InstrumentID, sizeof req.InstrumentID);
//     req.FrontID = pOrder->FrontID;
//     req.SessionID = pOrder->SessionID;

//     switch (actionFlag) {
//         case AF_Delete:
//             req.ActionFlag = THOST_FTDC_AF_Delete;
//             break;
//         case AF_Modify:
//             req.ActionFlag = THOST_FTDC_AF_Modify;
//             break;
//         default:
//             throw std::invalid_argument("action_flag");
//     }
//     req.LimitPrice = limitPrice;
//     req.VolumeChange = volumeChange;

//     assert_request(_tdApi->ReqOrderAction(&req, requestId));
// }

// void CtpClient::DeleteOrder(boost::shared_ptr<CThostFtdcOrderField> pOrder, int requestId)
// {
//     OrderAction(pOrder, AF_Delete, 0.0, 0, requestId);
// }

// void CtpClient::ModifyOrder(
//     boost::shared_ptr<CThostFtdcOrderField> pOrder,
//     TThostFtdcPriceType limitPrice,
//     TThostFtdcVolumeType volumeChange,
//     int requestId)
// {
//     OrderAction(pOrder, AF_Modify, limitPrice, volumeChange, requestId);
// }

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

void CtpClientWrap::OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_user_login",
        OnTdUserLogin,
        pRspUserLogin,
        pRspInfo
    );
}

void CtpClientWrap::OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_user_logout",
        OnTdUserLogout,
        pUserLogout,
        pRspInfo
    );
}

void CtpClientWrap::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_settlement_info_confirm",
        OnRspSettlementInfoConfirm,
        pSettlementInfoConfirm,
        pRspInfo
    );
}

void CtpClientWrap::OnErrOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_err_order_insert",
        OnErrOrderInsert,
        pInputOrder,
        pRspInfo
    );
}

void CtpClientWrap::OnErrOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_err_order_action",
        OnErrOrderAction,
        pInputOrderAction,
        pOrderAction,
        pRspInfo
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

void CtpClientWrap::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_rtn_trade",
        OnRtnTrade,
        pTrade
    );
}

void CtpClientWrap::OnTdError(CThostFtdcRspInfoField *pRspInfo)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,
        CtpClient,
        "on_td_error",
        OnTdError,
        pRspInfo
    );
}

void CtpClientWrap::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_order",
        OnRspQryOrder,
        pOrder,
        pRspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_trade",
        OnRspQryTrade,
        pTrade,
        pRspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_trading_account",
        OnRspQryTradingAccount,
        pTradingAccount,
        pRspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_investor_position",
        OnRspQryInvestorPosition,
        pInvestorPosition,
        pRspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_investor_position_detail",
        OnRspQryInvestorPositionDetail,
        pInvestorPositionDetail,
        pRspInfo,
        bIsLast
    );
}

void CtpClientWrap::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PYBIND11_OVERLOAD_PURE_NAME(
        void,        /* Return type */
        CtpClient,
        "on_rsp_market_data",
        OnRspQryDepthMarketData,
        pDepthMarketData,
        pRspInfo,
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
