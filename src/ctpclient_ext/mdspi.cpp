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
#include "mdspi.h"
#include "ctpclient.h"

MdSpi::MdSpi(CtpClient *client) : _client(client)
{
    //
}

MdSpi::~MdSpi()
{
    //
}

void MdSpi::OnFrontConnected()
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnMdFrontConnected);
    _client->Push(r);
}

void MdSpi::OnFrontDisconnected(int nReason)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnMdFrontDisconnected);
    r->nReason = nReason;
    _client->Push(r);
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnMdUserLogin, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pRspUserLogin);
    _client->Push(r);
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnMdUserLogout, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pUserLogout);
    _client->Push(r);
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnSubMarketData, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pSpecificInstrument);
    _client->Push(r);
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnUnSubMarketData, pRspInfo, nRequestID, bIsLast);
    r->SetRsp(pSpecificInstrument);
    _client->Push(r);
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRtnMarketData);
    r->SetRsp(pDepthMarketData);
    _client->Push(r);
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::OnMdError, pRspInfo, nRequestID, bIsLast);
    _client->Push(r);
}
