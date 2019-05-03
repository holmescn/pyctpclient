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
#include <boost/python.hpp>
#include "ctpclient.h"

using namespace boost::python;

static void trans_exception(int rc, std::string request)
{
	object msg = "[%d] %s" % boost::python::make_tuple(rc, request);
	std::string msg_s = extract<std::string>(msg);
    PyErr_SetString(PyExc_Exception, msg_s.c_str());
}

void RequestNetworkException_translator(RequestNetworkException const& e)
{
	trans_exception(-1, e.request);
}

void FullRequestQueueException_translator(FullRequestQueueException const& e)
{
	trans_exception(-2, e.request);
}

void RequestTooFrequentlyException_translator(RequestTooFrequentlyException const& e)
{
	trans_exception(-3, e.request);
}

void UnknownRequestException_translator(UnknownRequestException const& e)
{
	trans_exception(e.rc, e.request);
}

object ResponseInfo_ErrorMsg(CThostFtdcRspInfoField const *pRspInfo)
{
	char *s = const_cast<char*>(pRspInfo->ErrorMsg);
	auto pyobj = PyBytes_FromString(s);
	return object(handle<>(pyobj));
}

template<class T>
str _InstrumentID(T const *obj)
{
	return str(obj->InstrumentID);
}

BOOST_PYTHON_MODULE(_ctpclient)
{
	register_exception_translator<RequestNetworkException>(RequestNetworkException_translator);
	register_exception_translator<FullRequestQueueException>(FullRequestQueueException_translator);
	register_exception_translator<RequestTooFrequentlyException>(RequestTooFrequentlyException_translator);
	register_exception_translator<UnknownRequestException>(UnknownRequestException_translator);
 
	class_<CtpClientWrap, boost::noncopyable>("CtpClient", init<std::string, std::string, std::string, std::string, std::string>())
	 	.add_property("flow_path", &CtpClient::GetFlowPath, &CtpClient::SetFlowPath)
	 	.add_property("md_address", &CtpClient::GetMdAddr, &CtpClient::SetMdAddr)
	 	.add_property("td_address", &CtpClient::GetTdAddr, &CtpClient::SetTdAddr)
	 	.add_property("broker_id", &CtpClient::GetBrokerId, &CtpClient::SetBrokerId)
	 	.add_property("user_id", &CtpClient::GetUserId, &CtpClient::SetUserId)
	 	.add_property("password", &CtpClient::GetPassword, &CtpClient::SetPassword)
	 	.add_property("instrument_ids", &CtpClient::GetInstrumentIds, &CtpClient::SetInstrumentIds)
		.def("get_api_version", &CtpClient::GetApiVersion)
        .staticmethod("get_api_version")
		.def("run", &CtpClient::Run)

		.def("md_login", &CtpClient::MdLogin)
		.def("subscribe_market_data", &CtpClient::SubscribeMarketData)
		.def("unsubscribe_market_data", &CtpClient::UnsubscribeMarketData)
		.def("on_md_front_connected", pure_virtual(&CtpClient::OnMdFrontConnected))
		.def("on_md_front_disconnected", pure_virtual(&CtpClient::OnMdFrontDisconnected))
		.def("on_md_user_login", pure_virtual(&CtpClient::OnMdUserLogin))
		.def("on_md_user_logout", pure_virtual(&CtpClient::OnMdUserLogout))
		.def("on_subscribe_market_data", pure_virtual(&CtpClient::OnSubscribeMarketData))
		.def("on_unsubscribe_market_data", pure_virtual(&CtpClient::OnUnsubscribeMarketData))
		.def("on_rtn_market_data", pure_virtual(&CtpClient::OnRtnMarketData))
		.def("on_tick", pure_virtual(&CtpClient::OnTick))
		.def("on_1min", pure_virtual(&CtpClient::On1Min))

		.def("td_login", &CtpClient::TdLogin)
		.def("on_td_front_connected", pure_virtual(&CtpClient::OnTdFrontConnected))
		;

	class_<CThostFtdcRspInfoField>("ResponseInfo")
		.def_readonly("error_id", &CThostFtdcRspInfoField::ErrorID)
		.add_property("error_msg", ResponseInfo_ErrorMsg)
	  ;

	class_<CThostFtdcRspUserLoginField>("UserLoginInfo")
		.def_readonly("trading_day", &CThostFtdcRspUserLoginField::TradingDay)
		.def_readonly("login_time", &CThostFtdcRspUserLoginField::LoginTime)
		.def_readonly("broker_id", &CThostFtdcRspUserLoginField::BrokerID)
		.def_readonly("user_id", &CThostFtdcRspUserLoginField::UserID)
		.def_readonly("system_name", &CThostFtdcRspUserLoginField::SystemName)
		.def_readonly("front_id", &CThostFtdcRspUserLoginField::FrontID)
		.def_readonly("session_id", &CThostFtdcRspUserLoginField::SessionID)
		.def_readonly("max_order_ref", &CThostFtdcRspUserLoginField::MaxOrderRef)
		.def_readonly("SHFE_time", &CThostFtdcRspUserLoginField::SHFETime)
		.def_readonly("DCE_time", &CThostFtdcRspUserLoginField::DCETime)
		.def_readonly("CZCE_time", &CThostFtdcRspUserLoginField::CZCETime)
		.def_readonly("FFEX_time", &CThostFtdcRspUserLoginField::FFEXTime)
		.def_readonly("INE_time", &CThostFtdcRspUserLoginField::INETime)
	  ;

	class_<CThostFtdcUserLogoutField>("UserLogoutInfo")
		.def_readonly("broker_id", &CThostFtdcUserLogoutField::BrokerID)
		.def_readonly("user_id", &CThostFtdcUserLogoutField::UserID)
	  ;

	class_<CThostFtdcSpecificInstrumentField>("SpecificInstrument")
    .add_property("instrument_id", _InstrumentID<CThostFtdcSpecificInstrumentField>)
		;

	class_<CThostFtdcDepthMarketDataField>("MarketData")
		.def_readonly("trading_day", &CThostFtdcDepthMarketDataField::TradingDay)
		.def_readonly("instrument_id", &CThostFtdcDepthMarketDataField::InstrumentID)
		.def_readonly("exchange_id", &CThostFtdcDepthMarketDataField::ExchangeID)
		.def_readonly("exchange_inst_id", &CThostFtdcDepthMarketDataField::ExchangeInstID)
		.def_readonly("last_price", &CThostFtdcDepthMarketDataField::LastPrice)
		.def_readonly("pre_settlement_price", &CThostFtdcDepthMarketDataField::PreSettlementPrice)
		.def_readonly("pre_close_price", &CThostFtdcDepthMarketDataField::PreClosePrice)
		.def_readonly("pre_open_interest", &CThostFtdcDepthMarketDataField::PreOpenInterest)
		.def_readonly("open_price", &CThostFtdcDepthMarketDataField::OpenPrice)
		.def_readonly("highest_price", &CThostFtdcDepthMarketDataField::HighestPrice)
		.def_readonly("lowest_price", &CThostFtdcDepthMarketDataField::LowestPrice)
		.def_readonly("volume", &CThostFtdcDepthMarketDataField::Volume)
		.def_readonly("turnover", &CThostFtdcDepthMarketDataField::Turnover)
		.def_readonly("open_interest", &CThostFtdcDepthMarketDataField::OpenInterest)
		.def_readonly("close_price", &CThostFtdcDepthMarketDataField::ClosePrice)
		.def_readonly("settlement_price", &CThostFtdcDepthMarketDataField::SettlementPrice)
		.def_readonly("upper_limit_price", &CThostFtdcDepthMarketDataField::UpperLimitPrice)
		.def_readonly("lower_limit_price", &CThostFtdcDepthMarketDataField::LowerLimitPrice)
		.def_readonly("pre_delta", &CThostFtdcDepthMarketDataField::PreDelta)
		.def_readonly("curr_delta", &CThostFtdcDepthMarketDataField::CurrDelta)
		.def_readonly("update_time", &CThostFtdcDepthMarketDataField::UpdateTime)
		.def_readonly("update_millisec", &CThostFtdcDepthMarketDataField::UpdateMillisec)
		.def_readonly("bid_price1", &CThostFtdcDepthMarketDataField::BidPrice1)
		.def_readonly("bid_volume1", &CThostFtdcDepthMarketDataField::BidVolume1)
		.def_readonly("ask_price1", &CThostFtdcDepthMarketDataField::AskPrice1)
		.def_readonly("ask_volume1", &CThostFtdcDepthMarketDataField::AskVolume1)
		.def_readonly("bid_price2", &CThostFtdcDepthMarketDataField::BidPrice2)
		.def_readonly("bid_volume2", &CThostFtdcDepthMarketDataField::BidVolume2)
		.def_readonly("ask_price2", &CThostFtdcDepthMarketDataField::AskPrice2)
		.def_readonly("ask_volume2", &CThostFtdcDepthMarketDataField::AskVolume2)
		.def_readonly("bid_price3", &CThostFtdcDepthMarketDataField::BidPrice3)
		.def_readonly("bid_volume3", &CThostFtdcDepthMarketDataField::BidVolume3)
		.def_readonly("ask_price3", &CThostFtdcDepthMarketDataField::AskPrice3)
		.def_readonly("ask_volume3", &CThostFtdcDepthMarketDataField::AskVolume3)
		.def_readonly("bid_price4", &CThostFtdcDepthMarketDataField::BidPrice4)
		.def_readonly("bid_volume4", &CThostFtdcDepthMarketDataField::BidVolume4)
		.def_readonly("ask_price4", &CThostFtdcDepthMarketDataField::AskPrice4)
		.def_readonly("ask_volume4", &CThostFtdcDepthMarketDataField::AskVolume4)
		.def_readonly("bid_price5", &CThostFtdcDepthMarketDataField::BidPrice5)
		.def_readonly("bid_volume5", &CThostFtdcDepthMarketDataField::BidVolume5)
		.def_readonly("ask_price5", &CThostFtdcDepthMarketDataField::AskPrice5)
		.def_readonly("ask_volume5", &CThostFtdcDepthMarketDataField::AskVolume5)
		.def_readonly("average_price", &CThostFtdcDepthMarketDataField::AveragePrice)
		.def_readonly("action_day", &CThostFtdcDepthMarketDataField::ActionDay)
		;

};
