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
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnTdFrontConnected);
    _client->Push(r);
}

void TraderSpi::OnFrontDisconnected(int nReason)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnTdFrontDisconnected);
    r->nReason = nReason;
    _client->Push(r);
}

void TraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField * /* pRspAuthenticateField */, CThostFtdcRspInfoField * /* pRspInfo */, int /* nRequestID */, bool /* bIsLast */)
{
    //
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnTdUserLogin, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pRspUserLogin);
    _client->Push(r);
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnTdUserLogout, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pUserLogout);
    _client->Push(r);
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnSettlementInfoConfirm, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pSettlementInfoConfirm);
    _client->Push(r);
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspOrderInsert, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pInputOrder);
    _client->Push(r);
}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspOrderAction, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pInputOrderAction);
    _client->Push(r);
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnErrRtnOrderInsert, pRspInfo);
    r->SetRsp(pInputOrder);
    _client->Push(r);
}

void TraderSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnErrRtnOrderAction, pRspInfo);
    r->SetRsp(pOrderAction);
    _client->Push(r);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRtnOrder);
    r->SetRsp(pOrder);
    _client->Push(r);
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRtnTrade);
    r->SetRsp(pTrade);
    _client->Push(r);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnTdError, pRspInfo, nRequestID, bIsLast);
    _client->Push(r);
}

void TraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryOrder, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pOrder);
    _client->Push(r);
}

void TraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryTrade, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pTrade);
    _client->Push(r);
}

void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryTradingAccount, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pTradingAccount);
    _client->Push(r);
}

void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryInvestorPosition, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pInvestorPosition);
    _client->Push(r);
}

void TraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryDepthMarketData, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pDepthMarketData);
    if (pDepthMarketData == nullptr && r->RspInfo.ErrorID == 0) {
        r->RspInfo.ErrorID = 16;
    }
    _client->Push(r);
}

void TraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQrySettlementInfo, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pSettlementInfo);
    _client->Push(r);
}

void TraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRspQryInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pInvestorPositionDetail);
    _client->Push(r);
}
