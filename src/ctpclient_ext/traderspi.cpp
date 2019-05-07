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
    _client->OnTdFrontConnected();
}

void TraderSpi::OnFrontDisconnected(int nReason)
{
    _client->OnTdFrontDisconnected(nReason);
}

void TraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField * /* pRspAuthenticateField */, CThostFtdcRspInfoField * /* pRspInfo */, int /* nRequestID */, bool /* bIsLast */)
{
    //
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnTdUserLogin(pRspUserLogin, pRspInfo);
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnTdUserLogout(pUserLogout, pRspInfo);
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnRspSettlementInfoConfirm(pSettlementInfoConfirm, pRspInfo);
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnErrOrderInsert(pInputOrder, pRspInfo);
}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnRspOrderAction(pInputOrderAction, pRspInfo);
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    _client->OnErrOrderInsert(pInputOrder, pRspInfo);
}

void TraderSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    _client->OnErrOrderAction(pOrderAction, pRspInfo);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    CThostFtdcOrderField *pNewOrder = new CThostFtdcOrderField;
    memcpy(pNewOrder, pOrder, sizeof(CThostFtdcOrderField));
    _client->OnRtnOrder(boost::shared_ptr<CThostFtdcOrderField>(pNewOrder));
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    _client->OnRtnTrade(pTrade);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnTdError(pRspInfo);
}

void TraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryOrder(pOrder, pRspInfo, bIsLast);    
}

void TraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryTrade(pTrade, pRspInfo, bIsLast);
}

void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryTradingAccount(pTradingAccount, pRspInfo, bIsLast);   
}

void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryInvestorPosition(pInvestorPosition, pRspInfo, bIsLast);
}

void TraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryDepthMarketData(pDepthMarketData, pRspInfo, bIsLast);
}

void TraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnRspQrySettlementInfo(pSettlementInfo, pRspInfo);
}

void TraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool bIsLast)
{
    _client->OnRspQryInvestorPositionDetail(pInvestorPositionDetail, pRspInfo, bIsLast);
}
