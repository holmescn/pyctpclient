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
    TickBar tickBar;
    memset(&tickBar, 0, sizeof tickBar);
    snprintf(tickBar.UpdateTime, 16, "%s.%03d", pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
    strncpy(tickBar.InstrumentID, pDepthMarketData->InstrumentID, sizeof tickBar.InstrumentID);
    tickBar.Price = pDepthMarketData->LastPrice;
    tickBar.Volume = pDepthMarketData->Volume;
    tickBar.Turnover = pDepthMarketData->Turnover;
    tickBar.Position = pDepthMarketData->OpenInterest;

    M1Bar m1Bar;
    memset(&m1Bar, 0, sizeof m1Bar);
    memcpy(m1Bar.InstrumentID, pDepthMarketData->InstrumentID, sizeof m1Bar.InstrumentID);
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
        _m1Bars.emplace(std::make_pair(instrumentId, m1Bar));
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

            auto r = new CtpClient::Response(CtpClient::ResponseType::On1Min);
            r->SetRsp(&prev);
            _client->Push(r);
        }

        m1Bar.TickVolume = pDepthMarketData->Volume;
        m1Bar.TickTurnover = pDepthMarketData->Turnover;
        m1Bar.Position = pDepthMarketData->OpenInterest;
        m1Bar.Volume = m1Bar.TickVolume > m1Bar.BaseVolume ? m1Bar.TickVolume - m1Bar.BaseVolume : m1Bar.TickVolume;
        m1Bar.Turnover = m1Bar.TickTurnover > m1Bar.BaseTurnover ? m1Bar.TickTurnover - m1Bar.BaseTurnover : m1Bar.TickTurnover;

        memcpy(&prev, &m1Bar, sizeof m1Bar);
    }

    auto r = new CtpClient::Response(CtpClient::ResponseType::OnRtnMarketData);
    r->SetRsp(pDepthMarketData);
    _client->Push(r);

    r = new CtpClient::Response(CtpClient::ResponseType::OnTick);
    r->SetRsp(&tickBar);
    _client->Push(r);

    r = new CtpClient::Response(CtpClient::ResponseType::On1MinTick);
    r->SetRsp(&m1Bar);
    _client->Push(r);
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    auto r = new CtpClient::Response(CtpClient::ResponseType::On1MinTick, pRspInfo, nRequestID, bIsLast);
    _client->Push(r);
}
