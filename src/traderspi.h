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
#pragma once
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiStruct.h"

class CtpClient;
class TraderSpi : public CThostFtdcTraderSpi
{
    CtpClient *_client;
public:
    TraderSpi(CtpClient *client);
    TraderSpi(const TraderSpi&) = delete;
    TraderSpi(TraderSpi&&) = delete;
    TraderSpi& operator=(const TraderSpi&) = delete;
    TraderSpi& operator=(TraderSpi&&) = delete;
    ~TraderSpi();

public:
	void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

};