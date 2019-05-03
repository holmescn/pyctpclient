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

void TraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnTdUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnTdUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnRspSettlementInfoConfirm(pSettlementInfoConfirm, pRspInfo);
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnTdError(pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnRspQryOrder(pOrder, pRspInfo, bIsLast);    
}

void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnRspQryTradingAccount(pTradingAccount, pRspInfo, bIsLast);   
}
