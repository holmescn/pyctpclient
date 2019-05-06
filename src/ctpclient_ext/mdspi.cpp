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
    std::cerr << "InstrumentID: " << pDepthMarketData->InstrumentID << std::endl;
    
    char tickTime[16] = { 0 };
    snprintf(tickTime, sizeof tickTime, "%s.%03d", pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);

    M1Bar bar;
    bar.time = std::string(pDepthMarketData->UpdateTime, pDepthMarketData->UpdateTime+6);;

    auto instrumentId = pDepthMarketData->InstrumentID;
    auto price = pDepthMarketData->LastPrice;
    auto volume = pDepthMarketData->Volume;
    bar.volume = volume;

    bool update1min = false;
    auto iter = _m1Bars.find(instrumentId);
    if (iter == _m1Bars.end() || iter->second.time != bar.time) {
        bar.priceOpen = bar.priceHigh = bar.priceLow = bar.priceClose = price;
        update1min = iter->second.time != bar.time;
    } else {
        bar.priceHigh = price > iter->second.priceHigh ? price : iter->second.priceHigh;
        bar.priceLow = price < iter->second.priceLow ? price : iter->second.priceLow;
        bar.priceClose = price;
    }
    _m1Bars[pDepthMarketData->InstrumentID] = bar;

    _client->OnRtnMarketData(pDepthMarketData);
    _client->OnTick(instrumentId, price, volume, tickTime);

    if (update1min) {
        _client->On1Min(instrumentId, bar.priceOpen, bar.priceHigh, bar.priceLow, bar.priceClose, bar.volume, bar.time);
    }
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->OnMdError(pRspInfo, nRequestID, bIsLast);
}
