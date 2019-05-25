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
    CtpClient::Response r;
    memset(&r, 0, sizeof r);
    r.type = CtpClient::ResponseType::OnMdFrontConnected;
    _client->Enqueue(r);
}

void MdSpi::OnFrontDisconnected(int nReason)
{
    CtpClient::Response r;
    memset(&r, 0, sizeof r);
    r.type = CtpClient::ResponseType::OnMdFrontDisconnected;
    r.nReason = nReason;
    _client->Enqueue(r);
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnMdUserLogin, pRspUserLogin, pRspInfo, nRequestID, bIsLast);
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnMdUserLogout, pUserLogout, pRspInfo, nRequestID, bIsLast);
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnSubMarketData, pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnUnSubMarketData, pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    _client->Enqueue(CtpClient::ResponseType::OnRtnMarketData, pDepthMarketData, nullptr, 0, true);

    TickBar tickBar;
    memset(&tickBar, 0, sizeof tickBar);
    snprintf(tickBar.UpdateTime, 16, "%s.%03d", pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
    strncpy(tickBar.TradingDay, pDepthMarketData->TradingDay, sizeof tickBar.TradingDay);
    strncpy(tickBar.ActionDay, pDepthMarketData->ActionDay, sizeof tickBar.ActionDay);
    strncpy(tickBar.InstrumentID, pDepthMarketData->InstrumentID, sizeof tickBar.InstrumentID);
    tickBar.Price = pDepthMarketData->LastPrice;
    tickBar.Volume = pDepthMarketData->Volume;
    tickBar.Turnover = pDepthMarketData->Turnover;
    tickBar.Position = pDepthMarketData->OpenInterest;
    _client->Enqueue(CtpClient::ResponseType::OnTick, &tickBar);

    M1Bar m1Bar;
    memset(&m1Bar, 0, sizeof m1Bar);
    memcpy(m1Bar.InstrumentID, pDepthMarketData->InstrumentID, sizeof m1Bar.InstrumentID);
    memcpy(m1Bar.TradingDay, pDepthMarketData->TradingDay, sizeof m1Bar.TradingDay);
    memcpy(m1Bar.ActionDay, pDepthMarketData->ActionDay, sizeof m1Bar.ActionDay);
    memcpy(m1Bar.UpdateTime, pDepthMarketData->UpdateTime, 5);

    std::string m1Now(m1Bar.UpdateTime);
    std::string instrumentId(pDepthMarketData->InstrumentID);
    auto price = pDepthMarketData->LastPrice;

    auto iter = _m1Bars.find(instrumentId);
    if (iter == _m1Bars.end()) {
        m1Bar.OpenPrice = m1Bar.HighestPrice = m1Bar.LowestPrice = m1Bar.ClosePrice = price;
        m1Bar.BaseVolume = pDepthMarketData->Volume;
        m1Bar.BaseTurnover = pDepthMarketData->Turnover;
        m1Bar.TickVolume = pDepthMarketData->Volume;
        m1Bar.TickTurnover = pDepthMarketData->Turnover;
        m1Bar.Volume = pDepthMarketData->Volume;
        m1Bar.Turnover = pDepthMarketData->Turnover;
        m1Bar.Position = pDepthMarketData->OpenInterest;
        _m1Bars[instrumentId] = m1Bar;
    } else {
        auto &prev = iter->second;
        if (m1Now == prev.UpdateTime) {
            m1Bar.OpenPrice = prev.OpenPrice;
            m1Bar.HighestPrice = price > prev.HighestPrice ? price : prev.HighestPrice;
            m1Bar.LowestPrice = price < prev.LowestPrice ? price : prev.LowestPrice;
            m1Bar.ClosePrice = price;
            m1Bar.BaseVolume = prev.BaseVolume;
            m1Bar.BaseTurnover = prev.BaseTurnover;
        } else {
            m1Bar.OpenPrice = m1Bar.HighestPrice = m1Bar.LowestPrice = m1Bar.ClosePrice = price;
            m1Bar.BaseVolume = prev.TickVolume;
            m1Bar.BaseTurnover = prev.TickTurnover;

            _client->Enqueue(CtpClient::ResponseType::On1Min, &prev);
        }

        m1Bar.TickVolume = pDepthMarketData->Volume;
        m1Bar.TickTurnover = pDepthMarketData->Turnover;
        m1Bar.Position = pDepthMarketData->OpenInterest;
        m1Bar.Volume = m1Bar.TickVolume >= m1Bar.BaseVolume ? m1Bar.TickVolume - m1Bar.BaseVolume : m1Bar.TickVolume;
        m1Bar.Turnover = m1Bar.TickTurnover >= m1Bar.BaseTurnover ? m1Bar.TickTurnover - m1Bar.BaseTurnover : m1Bar.TickTurnover;

        memcpy(&prev, &m1Bar, sizeof m1Bar);
    }

    memcpy(m1Bar.UpdateTime, pDepthMarketData->UpdateTime, sizeof m1Bar.UpdateTime);
    _client->Enqueue(CtpClient::ResponseType::On1MinTick, &m1Bar);
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _client->Enqueue(CtpClient::ResponseType::OnMdError, pRspInfo, nRequestID, bIsLast);    
}
