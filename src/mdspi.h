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
#include "ThostFtdcMdApi.h"
#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcUserApiDataType.h"

class CtpClient;
class MdSpi : public CThostFtdcMdSpi
{
    CtpClient *_client;
public:
    MdSpi(CtpClient *client);
    MdSpi(const MdSpi&) = delete;
    MdSpi(MdSpi&&) = delete;
    MdSpi& operator=(const MdSpi&) = delete;
    MdSpi& operator=(MdSpi&&) = delete;
    ~MdSpi();

public:
	void OnFrontConnected() override;
};