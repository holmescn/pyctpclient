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
    _client->OnMdFrontConnected();
}

void MdSpi::OnFrontDisconnected(int nReason)
{
    _client->OnMdFrontDisconnected(nReason);
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnMdUserLogin(pRspUserLogin, pRspInfo);
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnMdUserLogout(pUserLogout, pRspInfo);
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnSubscribeMarketData(pSpecificInstrument, pRspInfo);
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnUnsubscribeMarketData(pSpecificInstrument, pRspInfo);
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    char tickTime[16] = { 0 };
    snprintf(tickTime, sizeof tickTime, "%s.%03d", pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);

    M1Bar bar;
    memcpy(bar.InstrumentID, pDepthMarketData->InstrumentID, sizeof bar.InstrumentID);
    memset(bar.UpdateTime, 0, sizeof bar.UpdateTime);
    memcpy(bar.UpdateTime, pDepthMarketData->UpdateTime, 5);

    std::string m1_now(bar.UpdateTime);
    std::string instrumentId(pDepthMarketData->InstrumentID);
    auto price = pDepthMarketData->LastPrice;
    bar.Position = pDepthMarketData->OpenInterest;

    auto iter = _m1Bars.find(instrumentId);
    if (iter == _m1Bars.end()) {
        bar.OpenPrice = bar.HighestPrice = bar.LowestPrice = bar.ClosePrice = price;
    } else {
        auto &b = iter->second;
        if (m1_now == b.UpdateTime) {
            bar.OpenPrice = b.OpenPrice;
            bar.HighestPrice = price > b.HighestPrice ? price : b.HighestPrice;
            bar.LowestPrice = price < b.LowestPrice ? price : b.LowestPrice;
            bar.ClosePrice = price;
            bar.PreVolume = b.PreVolume;
            bar.PreTurnover = b.PreTurnover;
        } else {
            bar.OpenPrice = bar.HighestPrice = bar.LowestPrice = bar.ClosePrice = price;
            bar.PreVolume = b.Volume;
            bar.PreTurnover = b.Turnover;
        }

        bar.Volume = pDepthMarketData->Volume - b.PreVolume;
        bar.Turnover = pDepthMarketData->Turnover - b.PreTurnover;
    }

    if (iter != _m1Bars.end() && std::string(iter->second.UpdateTime) != m1_now) {
        _client->On1Min(iter->second);
    }

    _m1Bars[pDepthMarketData->InstrumentID] = bar;

    _client->OnRtnMarketData(pDepthMarketData);
    _client->OnTick(instrumentId, pDepthMarketData->LastPrice, pDepthMarketData->Volume, tickTime);
    _client->On1MinTick(bar);
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int /* nRequestID */, bool /* bIsLast */)
{
    _client->OnMdError(pRspInfo);
}
