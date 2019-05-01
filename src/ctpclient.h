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
#include <boost/python/tuple.hpp>
#include "mdspi.h"
#include "traderspi.h"

class CtpClient
{
    MdSpi *_mdSpi;
    CThostFtdcMdApi *_mdApi;
    TraderSpi *_traderSpi;
    CThostFtdcTraderApi *_traderApi;
public:
    CtpClient();
    CtpClient(const CtpClient&) = delete;
    CtpClient(CtpClient&&) = delete;
    CtpClient& operator=(const CtpClient&) = delete;
    CtpClient& operator=(CtpClient&&) = delete;
    virtual ~CtpClient() {}

public:
    static boost::python::tuple GetApiVersion();

public:
    // MdApi

public:
    // MdSpi
	virtual void OnMdFrontConnected() = 0;

public:
    // TraderApi

public:
    // TraderSpi
	virtual void OnTraderFrontConnected() = 0;

};