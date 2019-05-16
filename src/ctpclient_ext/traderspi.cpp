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
#include <iostream>
#include "traderspi.h"
#include "ctpclient.h"

TraderSpi::TraderSpi(CtpClient *client) : _client(client)
{
    //
}

TraderSpi::~TraderSpi()
{
    //
}

void TraderSpi::OnFrontConnected()
{
    CtpClient::Response r;
    memset(&r, 0, sizeof r);
    r.type = CtpClient::ResponseType::OnTdFrontDisconnected;
    _client->Enqueue(r);
}

void TraderSpi::OnFrontDisconnected(int nReason)
{
    CtpClient::Response r;
    memset(&r, 0, sizeof r);
    r.type = CtpClient::ResponseType::OnTdFrontDisconnected;
    r.nReason = nReason;
    _client->Enqueue(r);
}

void TraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField * /* pRspAuthenticateField */, CThostFtdcRspInfoField * /* pRspInfo */, int /* nRequestID */, bool /* bIsLast */)
{
    //
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnTdUserLogin, pRspUserLogin, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnTdUserLogout, pUserLogout, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnSettlementInfoConfirm, pSettlementInfoConfirm, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspOrderInsert, pInputOrder, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspOrderAction, pInputOrderAction, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    _client->Enqueue(CtpClient::ResponseType::OnErrRtnOrderInsert, pInputOrder, pRspInfo);
}

void TraderSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    _client->Enqueue(CtpClient::ResponseType::OnErrRtnOrderAction, pOrderAction, pRspInfo);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    _client->Enqueue(CtpClient::ResponseType::OnRtnOrder, pOrder);
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    _client->Enqueue(CtpClient::ResponseType::OnRtnTrade, pTrade);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnTdError, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspQryOrder, pOrder, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspQryTrade, pTrade, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspQryTradingAccount, pTradingAccount, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    
    _client->Enqueue(CtpClient::ResponseType::OnRspQryInvestorPosition, pInvestorPosition, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    CtpClient::Response r;
    r.Init(CtpClient::ResponseType::OnRspQryDepthMarketData, pRspInfo, nRequestID, bIsLast);
    if (pDepthMarketData) {
        memcpy(&r.base, pDepthMarketData, sizeof r.DepthMarketData);
    } else if (pRspInfo->ErrorID == 0) {
        r.RspInfo.ErrorID = 16;
    }
    _client->Enqueue(r);
}

void TraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnRspQryInvestorPositionDetail, pInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);
}
