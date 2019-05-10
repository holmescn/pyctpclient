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

struct M1Bar {
	TThostFtdcInstrumentIDType InstrumentID;
	TThostFtdcTimeType  UpdateTime;
	TThostFtdcPriceType OpenPrice;
	TThostFtdcPriceType HighestPrice;
	TThostFtdcPriceType LowestPrice;
	TThostFtdcPriceType ClosePrice;
	TThostFtdcVolumeType TickVolume;
	TThostFtdcVolumeType BaseVolume;
	TThostFtdcVolumeType Volume;
	TThostFtdcMoneyType	TickTurnover;
	TThostFtdcMoneyType	BaseTurnover;
	TThostFtdcMoneyType	Turnover;
	TThostFtdcLargeVolumeType Position;
};

struct TickBar {
	TThostFtdcInstrumentIDType InstrumentID;
	char UpdateTime[16];
	TThostFtdcPriceType Price;
	TThostFtdcMoneyType	Turnover;
	TThostFtdcVolumeType Volume;
	TThostFtdcLargeVolumeType Position;
};
