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
#include <boost/smart_ptr.hpp>
#include "ctpclient.h"

using namespace boost::python;

#pragma region Exception translators

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

void invalid_argument_translator(std::invalid_argument const& e)
{
	object msg = "InvalidArgument %s." % boost::python::make_tuple(e.what());
	std::string msg_s = extract<std::string>(msg);
  PyErr_SetString(PyExc_Exception, msg_s.c_str());
}

#pragma endregion // Exception translators

#pragma region Getters

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

#pragma endregion // Getters

BOOST_PYTHON_MODULE(_ctpclient)
{
	register_exception_translator<RequestNetworkException>(RequestNetworkException_translator);
	register_exception_translator<FullRequestQueueException>(FullRequestQueueException_translator);
	register_exception_translator<RequestTooFrequentlyException>(RequestTooFrequentlyException_translator);
	register_exception_translator<UnknownRequestException>(UnknownRequestException_translator);
	register_exception_translator<std::invalid_argument>(invalid_argument_translator);

	enum_<Direction>("Direction")
		.value("BUY", D_Buy)
		.value("SELL", D_Sell);

	enum_<OffsetFlag>("OffsetFlag")
		.value("OPEN", OF_Open)
		.value("CLOSE", OF_Close)
		.value("FORCE_CLOSE", OF_ForceClose)
		.value("CLOSE_TODAY", OF_CloseToday)
		.value("CLOSE_YESTERDAY", OF_CloseYesterday)
		.value("FORCE_OFF", OF_ForceOff)
		.value("LOCAL_FORCE_CLOSE", OF_LocalForceClose);

	enum_<OrderPriceType>("OrderPriceType")
		.value("ANY_PRICE", OPT_AnyPrice)
    .value("LIMIT_PRICE", OPT_LimitPrice)
		.value("BEST_PRICE", OPT_BestPrice)
		.value("LAST_PRICE", OPT_LastPrice)
		.value("LAST_PRICE_PLUS_ONE_TICK", OPT_LastPricePlusOneTick)
		.value("LAST_PRICE_PLUS_TWO_TICKS", OPT_LastPricePlusTwoTicks)
		.value("LAST_PRICE_PLUS_THREE_TICKS", OPT_LastPricePlusThreeTicks)
		.value("ASK_PRICE1", OPT_AskPrice1)
		.value("ASK_PRICE1_PLUS_ONE_TICK", OPT_AskPrice1PlusOneTick)
		.value("ASK_PRICE1_PLUS_TWO_TICKS", OPT_AskPrice1PlusTwoTicks)
		.value("ASK_PRICE1_PLUS_THREE_TICKS", OPT_AskPrice1PlusThreeTicks)
    .value("BID_PRICE1", OPT_BidPrice1)
		.value("BID_PRICE1_PLUS_ONE_TICK", OPT_BidPrice1PlusOneTick)
		.value("BID_PRICE1_PLUS_TWO_TICKS", OPT_BidPrice1PlusTwoTicks)
		.value("BID_PRICE1_PLUS_THREE_TICKS", OPT_BidPrice1PlusThreeTicks)
		.value("FIVE_LEVEL_PRICE", OPT_FiveLevelPrice);

	enum_<HedgeFlag>("HedgeFlag")
		.value("SPECULATION", HF_Speculation)
		.value("ARBITRAGE", HF_Arbitrage)
		.value("HEDGE", HF_Hedge)
		.value("MARKET_MAKER", HF_MarketMaker);

	enum_<TimeCondition>("TimeCondition")
		.value("IOC", TC_IOC)
		.value("GFS", TC_GFS)
		.value("GFD", TC_GFD)
		.value("GTD", TC_GTD)
		.value("GTC", TC_GTC)
		.value("GFA", TC_GFA);
	
	enum_<VolumeCondition>("VolumeCondition")
		.value("ANY_VOLUME", VC_AV)
		.value("MIN_VOLUME", VC_MV)
		.value("ALL", VC_CV);

	enum_<ContingentCondition>("ContingentCondition")
    .value("IMMEDIATELY", CC_Immediately)
		.value("TOUCH", CC_Touch)
		.value("TOUCH_PROFIT", CC_TouchProfit)
		.value("PARKED_ORDER", CC_ParkedOrder)
    .value("LAST_PRICE_GREATER_THAN_STOP_PRICE", CC_LastPriceGreaterThanStopPrice)
		.value("LAST_PRICE_GREATER_EQUAL_STOP_PRICE", CC_LastPriceGreaterEqualStopPrice)
    .value("LAST_PRICE_LESSER_THAN_STOP_PRICE", CC_LastPriceLesserThanStopPrice)
		.value("LAST_PRICE_LESSER_EQUAL_STOP_PRICE", CC_LastPriceLesserEqualStopPrice)
    .value("ASK_PRICE_GREATER_THAN_STOP_PRICE", CC_AskPriceGreaterThanStopPrice)
		.value("ASK_PRICE_GREATER_EQUAL_STOP_PRICE", CC_AskPriceGreaterEqualStopPrice)
    .value("ASK_PRICE_LESSER_THAN_STOP_PRICE", CC_AskPriceLesserThanStopPrice)
		.value("ASK_PRICE_LESSER_EQUAL_STOP_PRICE", CC_AskPriceLesserEqualStopPrice)
    .value("BID_PRICE_GREATER_THAN_STOP_PRICE", CC_BidPriceGreaterThanStopPrice)
		.value("BID_PRICE_GREATER_EQUAL_STOP_PRICE", CC_BidPriceGreaterEqualStopPrice)
    .value("BID_PRICE_LESSER_THAN_STOP_PRICE", CC_BidPriceLesserThanStopPrice)
		.value("BID_PRICE_LESSER_EQUAL_STOP_PRICE", CC_BidPriceLesserEqualStopPrice);
	enum_<OrderActionFlag>("OrderActionFlag")
		.value("Delete", AF_Delete)
		.value("Modify", AF_Modify);

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
		.def("confirm_settlement_info", &CtpClient::ConfirmSettlementInfo)
		.def("query_order", &CtpClient::QueryOrder)
		.def("query_trading_account", &CtpClient::QueryTradingAccount)
		.def("query_investor_position", &CtpClient::QueryInvestorPosition)
		.def("query_investor_position_detail", &CtpClient::QueryInvestorPositionDetail)
		.def("query_market_dara", &CtpClient::QueryMarketData)
		.def("query_settlement_info", &CtpClient::QuerySettlementInfo)
		.def("insert_order", &CtpClient::InsertOrder,
			(arg("instrument_id"), arg("direction"), arg("offset_flag"), arg("limit_price"),
			 arg("volume"), arg("order_price_type")=OPT_LimitPrice,
			 arg("hedge_flag")=HF_Speculation, arg("time_condition")=TC_GFS,
			 arg("volume_condition")=VC_AV, arg("contingent_condition")=CC_Immediately,
			 arg("min_volume")=1, arg("stop_price")=0.0,
			 arg("is_auto_suspend")=false, arg("user_force_close")=false,
			 arg("request_id")=0))
		.def("order_action", &CtpClient::InsertOrder,
			(arg("order"), arg("action_flag"), arg("limit_price")=0.0,
			 arg("volume_change")=0, arg("request_id")=0))
		.def("delete_order", &CtpClient::InsertOrder,
			(arg("order"), arg("request_id")=0))
		.def("modify_order", &CtpClient::InsertOrder,
			(arg("order"), arg("limit_price")=0.0, arg("volume_change")=0, arg("request_id")=0))			
		.def("on_td_front_connected", pure_virtual(&CtpClient::OnTdFrontConnected))
		.def("on_settlement_info_confirm", pure_virtual(&CtpClient::OnRspSettlementInfoConfirm))
		.def("on_rsp_order_insert", pure_virtual(&CtpClient::OnRspOrderInsert))
		.def("on_rsp_order_action", pure_virtual(&CtpClient::OnRspOrderAction))
		.def("on_err_order_insert", pure_virtual(&CtpClient::OnErrRtnOrderInsert))
		.def("on_err_order_action", pure_virtual(&CtpClient::OnErrRtnOrderAction))
		.def("on_rtn_order", pure_virtual(&CtpClient::OnRtnOrder))
		.def("on_rtn_trade", pure_virtual(&CtpClient::OnRtnTrade))
		.def("on_rsp_order", pure_virtual(&CtpClient::OnRspQryOrder))
		.def("on_rsp_trading_account", pure_virtual(&CtpClient::OnRspQryTradingAccount))
		.def("on_rsp_investor_position", pure_virtual(&CtpClient::OnRspQryInvestorPosition))
		.def("on_rsp_investor_position_detail", pure_virtual(&CtpClient::OnRspQryInvestorPositionDetail))
		.def("on_rsp_market_data", pure_virtual(&CtpClient::OnRspQryDepthMarketData))
		.def("on_rsp_settlement_info", pure_virtual(&CtpClient::OnRspQrySettlementInfo))
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

	class_<CThostFtdcSettlementInfoConfirmField>("SettlementInfoConfirm")
		.def_readonly("broker_id", &CThostFtdcSettlementInfoConfirmField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcSettlementInfoConfirmField::InvestorID)
		.def_readonly("confirm_date", &CThostFtdcSettlementInfoConfirmField::ConfirmDate)
		.def_readonly("confirm_time", &CThostFtdcSettlementInfoConfirmField::ConfirmTime)
		.def_readonly("currency_id", &CThostFtdcSettlementInfoConfirmField::CurrencyID)
		;

	class_<CThostFtdcOrderField, boost::shared_ptr<CThostFtdcOrderField>>("Order")
		.def_readonly("broker_id", &CThostFtdcOrderField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcOrderField::InvestorID)
		.def_readonly("order_ref", &CThostFtdcOrderField::OrderRef)
		.def_readonly("order_price_type", &CThostFtdcOrderField::OrderPriceType)
		.def_readonly("direction", &CThostFtdcOrderField::Direction)
		.def_readonly("comb_offset_flag", &CThostFtdcOrderField::CombOffsetFlag)
		.def_readonly("comb_hedge_flag", &CThostFtdcOrderField::CombHedgeFlag)
		.def_readonly("limit_price", &CThostFtdcOrderField::LimitPrice)
		.def_readonly("volume_total_original", &CThostFtdcOrderField::VolumeTotalOriginal)
		.def_readonly("time_condition", &CThostFtdcOrderField::TimeCondition)
		.def_readonly("GTD_date", &CThostFtdcOrderField::GTDDate)
		.def_readonly("volume_condition", &CThostFtdcOrderField::VolumeCondition)
		.def_readonly("contingent_condition", &CThostFtdcOrderField::ContingentCondition)
		.def_readonly("force_close_reason", &CThostFtdcOrderField::ForceCloseReason)
		.def_readonly("is_auto_suspend", &CThostFtdcOrderField::IsAutoSuspend)
		.def_readonly("business_unit", &CThostFtdcOrderField::BusinessUnit)
		.def_readonly("request_id", &CThostFtdcOrderField::RequestID)
		.def_readonly("order_local_id", &CThostFtdcOrderField::OrderLocalID)
		.def_readonly("exchange_id", &CThostFtdcOrderField::ExchangeID)
		.def_readonly("participant_id", &CThostFtdcOrderField::ParticipantID)
		.def_readonly("client_id", &CThostFtdcOrderField::ClientID)
		.def_readonly("exchange_inst_id", &CThostFtdcOrderField::ExchangeInstID)
		.def_readonly("trader_id", &CThostFtdcOrderField::TraderID)
		.def_readonly("install_id", &CThostFtdcOrderField::InstallID)
		.def_readonly("order_submit_status", &CThostFtdcOrderField::OrderSubmitStatus)
		.def_readonly("notify_sequence", &CThostFtdcOrderField::NotifySequence)
		.def_readonly("trading_day", &CThostFtdcOrderField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcOrderField::SettlementID)
		.def_readonly("order_sys_id", &CThostFtdcOrderField::OrderSysID)
		.def_readonly("order_source", &CThostFtdcOrderField::OrderSource)
		.def_readonly("order_type", &CThostFtdcOrderField::OrderType)
		.def_readonly("volume_traded", &CThostFtdcOrderField::VolumeTraded)
		.def_readonly("volume_total", &CThostFtdcOrderField::VolumeTotal)
		.def_readonly("insert_date", &CThostFtdcOrderField::InsertDate)
		.def_readonly("insert_time", &CThostFtdcOrderField::InsertTime)
		.def_readonly("active_time", &CThostFtdcOrderField::ActiveTime)
		.def_readonly("suspend_time", &CThostFtdcOrderField::SuspendTime)
		.def_readonly("cancel_time", &CThostFtdcOrderField::CancelTime)
		.def_readonly("active_trader_id", &CThostFtdcOrderField::ActiveTraderID)
		.def_readonly("clearing_part_id", &CThostFtdcOrderField::ClearingPartID)
		.def_readonly("sequence_no", &CThostFtdcOrderField::SequenceNo)
		.def_readonly("front_id", &CThostFtdcOrderField::FrontID)
		.def_readonly("session_id", &CThostFtdcOrderField::SessionID)
		.def_readonly("user_product_info", &CThostFtdcOrderField::UserProductInfo)
		.def_readonly("status_msg", &CThostFtdcOrderField::StatusMsg)
		.def_readonly("user_force_close", &CThostFtdcOrderField::UserForceClose)
		.def_readonly("active_user_id", &CThostFtdcOrderField::ActiveUserID)
		.def_readonly("broker_order_seq", &CThostFtdcOrderField::BrokerOrderSeq)
		.def_readonly("relative_order_sys_id", &CThostFtdcOrderField::RelativeOrderSysID)
		.def_readonly("ZCE_total_traded_volume", &CThostFtdcOrderField::ZCETotalTradedVolume)
		.def_readonly("is_swap_order", &CThostFtdcOrderField::IsSwapOrder)
		.def_readonly("branch_id", &CThostFtdcOrderField::BranchID)
		.def_readonly("invest_unit_id", &CThostFtdcOrderField::InvestUnitID)
		.def_readonly("account_id", &CThostFtdcOrderField::AccountID)
		.def_readonly("currency_id", &CThostFtdcOrderField::CurrencyID)
		.def_readonly("ip_address", &CThostFtdcOrderField::IPAddress)
		.def_readonly("mac_address", &CThostFtdcOrderField::MacAddress)
		;

	class_<CThostFtdcTradeField>("Trade")
		.def_readonly("broker_id", &CThostFtdcTradeField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcTradeField::InvestorID)
		.def_readonly("instrument_id", &CThostFtdcTradeField::InstrumentID)
		.def_readonly("order_ref", &CThostFtdcTradeField::OrderRef)
		.def_readonly("user_id", &CThostFtdcTradeField::UserID)
		.def_readonly("exchange_id", &CThostFtdcTradeField::ExchangeID)
		.def_readonly("trade_id", &CThostFtdcTradeField::TradeID)
		.def_readonly("direction", &CThostFtdcTradeField::Direction)
		.def_readonly("order_sys_id", &CThostFtdcTradeField::OrderSysID)
		.def_readonly("participant_id", &CThostFtdcTradeField::ParticipantID)
		.def_readonly("client_id", &CThostFtdcTradeField::ClientID)
		.def_readonly("trading_role", &CThostFtdcTradeField::TradingRole)
		.def_readonly("exchange_inst_id", &CThostFtdcTradeField::ExchangeInstID)
		.def_readonly("offset_flag", &CThostFtdcTradeField::OffsetFlag)
		.def_readonly("hedge_flag", &CThostFtdcTradeField::HedgeFlag)
		.def_readonly("price", &CThostFtdcTradeField::Price)
		.def_readonly("volume", &CThostFtdcTradeField::Volume)
		.def_readonly("trade_date", &CThostFtdcTradeField::TradeDate)
		.def_readonly("trade_time", &CThostFtdcTradeField::TradeTime)
		.def_readonly("trade_type", &CThostFtdcTradeField::TradeType)
		.def_readonly("price_source", &CThostFtdcTradeField::PriceSource)
		.def_readonly("trader_id", &CThostFtdcTradeField::TraderID)
		.def_readonly("order_local_id", &CThostFtdcTradeField::OrderLocalID)
		.def_readonly("clearing_part_id", &CThostFtdcTradeField::ClearingPartID)
		.def_readonly("business_unit", &CThostFtdcTradeField::BusinessUnit)
		.def_readonly("sequence_no", &CThostFtdcTradeField::SequenceNo)
		.def_readonly("trading_day", &CThostFtdcTradeField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcTradeField::SettlementID)
		.def_readonly("broker_order_seq", &CThostFtdcTradeField::BrokerOrderSeq)
		.def_readonly("trade_source", &CThostFtdcTradeField::TradeSource)
		.def_readonly("invest_unit_id", &CThostFtdcTradeField::InvestUnitID)
		;

	class_<CThostFtdcTradingAccountField>("TradingAccount")
		.def_readonly("broker_id", &CThostFtdcTradingAccountField::BrokerID)
		.def_readonly("account_id", &CThostFtdcTradingAccountField::AccountID)
		.def_readonly("pre_mortgage", &CThostFtdcTradingAccountField::PreMortgage)
		.def_readonly("pre_credit", &CThostFtdcTradingAccountField::PreCredit)
		.def_readonly("pre_deposit", &CThostFtdcTradingAccountField::PreDeposit)
		.def_readonly("pre_balance", &CThostFtdcTradingAccountField::PreBalance)
		.def_readonly("pre_margin", &CThostFtdcTradingAccountField::PreMargin)
		.def_readonly("interest_base", &CThostFtdcTradingAccountField::InterestBase)
		.def_readonly("interest", &CThostFtdcTradingAccountField::Interest)
		.def_readonly("deposit", &CThostFtdcTradingAccountField::Deposit)
		.def_readonly("withdraw", &CThostFtdcTradingAccountField::Withdraw)
		.def_readonly("frozen_margin", &CThostFtdcTradingAccountField::FrozenMargin)
		.def_readonly("frozen_cash", &CThostFtdcTradingAccountField::FrozenCash)
		.def_readonly("frozen_commission", &CThostFtdcTradingAccountField::FrozenCommission)
		.def_readonly("curr_margin", &CThostFtdcTradingAccountField::CurrMargin)
		.def_readonly("cash_in", &CThostFtdcTradingAccountField::CashIn)
		.def_readonly("commission", &CThostFtdcTradingAccountField::Commission)
		.def_readonly("close_profit", &CThostFtdcTradingAccountField::CloseProfit)
		.def_readonly("position_profit", &CThostFtdcTradingAccountField::PositionProfit)
		.def_readonly("balance", &CThostFtdcTradingAccountField::Balance)
		.def_readonly("available", &CThostFtdcTradingAccountField::Available)
		.def_readonly("withdraw_quota", &CThostFtdcTradingAccountField::WithdrawQuota)
		.def_readonly("reserve", &CThostFtdcTradingAccountField::Reserve)
		.def_readonly("trading_day", &CThostFtdcTradingAccountField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcTradingAccountField::SettlementID)
		.def_readonly("credit", &CThostFtdcTradingAccountField::Credit)
		.def_readonly("mortgage", &CThostFtdcTradingAccountField::Mortgage)
		.def_readonly("exchange_margin", &CThostFtdcTradingAccountField::ExchangeMargin)
		.def_readonly("delivery_margin", &CThostFtdcTradingAccountField::DeliveryMargin)
		.def_readonly("exchange_delivery_margin", &CThostFtdcTradingAccountField::ExchangeDeliveryMargin)
		.def_readonly("reserve_balance", &CThostFtdcTradingAccountField::ReserveBalance)
		.def_readonly("currency_id", &CThostFtdcTradingAccountField::CurrencyID)
		.def_readonly("pre_fund_mortgage_in", &CThostFtdcTradingAccountField::PreFundMortgageIn)
		.def_readonly("pre_fund_mortgage_out", &CThostFtdcTradingAccountField::PreFundMortgageOut)
		.def_readonly("fund_mortgage_in", &CThostFtdcTradingAccountField::FundMortgageIn)
		.def_readonly("fund_mortgage_out", &CThostFtdcTradingAccountField::FundMortgageOut)
		.def_readonly("fund_mortgage_available", &CThostFtdcTradingAccountField::FundMortgageAvailable)
		.def_readonly("mortgageable_fund", &CThostFtdcTradingAccountField::MortgageableFund)
		.def_readonly("spec_product_margin", &CThostFtdcTradingAccountField::SpecProductMargin)
		.def_readonly("spec_product_frozen_margin", &CThostFtdcTradingAccountField::SpecProductFrozenMargin)
		.def_readonly("spec_product_commission", &CThostFtdcTradingAccountField::SpecProductCommission)
		.def_readonly("spec_product_frozen_commission", &CThostFtdcTradingAccountField::SpecProductFrozenCommission)
		.def_readonly("spec_product_position_profit", &CThostFtdcTradingAccountField::SpecProductPositionProfit)
		.def_readonly("spec_product_close_profit", &CThostFtdcTradingAccountField::SpecProductCloseProfit)
		.def_readonly("spec_product_position_profit_by_alg", &CThostFtdcTradingAccountField::SpecProductPositionProfitByAlg)
		.def_readonly("spec_product_exchange_margin", &CThostFtdcTradingAccountField::SpecProductExchangeMargin)
		.def_readonly("biz_type", &CThostFtdcTradingAccountField::BizType)
		.def_readonly("frozen_swap", &CThostFtdcTradingAccountField::FrozenSwap)
		.def_readonly("remain_swap", &CThostFtdcTradingAccountField::RemainSwap)
		;

	class_<CThostFtdcInvestorPositionField>("CThostFtdcInvestorPositionField")
		.def_readonly("instrument_id", &CThostFtdcInvestorPositionField::InstrumentID)
		.def_readonly("broker_id", &CThostFtdcInvestorPositionField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcInvestorPositionField::InvestorID)
		.def_readonly("position_direction", &CThostFtdcInvestorPositionField::PosiDirection)
		.def_readonly("hedge_flag", &CThostFtdcInvestorPositionField::HedgeFlag)
		.def_readonly("position_date", &CThostFtdcInvestorPositionField::PositionDate)
		.def_readonly("yd_position", &CThostFtdcInvestorPositionField::YdPosition)
		.def_readonly("position", &CThostFtdcInvestorPositionField::Position)
		.def_readonly("long_frozen", &CThostFtdcInvestorPositionField::LongFrozen)
		.def_readonly("short_frozen", &CThostFtdcInvestorPositionField::ShortFrozen)
		.def_readonly("long_frozen_amount", &CThostFtdcInvestorPositionField::LongFrozenAmount)
		.def_readonly("short_frozen_amount", &CThostFtdcInvestorPositionField::ShortFrozenAmount)
		.def_readonly("open_volume", &CThostFtdcInvestorPositionField::OpenVolume)
		.def_readonly("close_volume", &CThostFtdcInvestorPositionField::CloseVolume)
		.def_readonly("open_amount", &CThostFtdcInvestorPositionField::OpenAmount)
		.def_readonly("close_amount", &CThostFtdcInvestorPositionField::CloseAmount)
		.def_readonly("position_cost", &CThostFtdcInvestorPositionField::PositionCost)
		.def_readonly("pre_margin", &CThostFtdcInvestorPositionField::PreMargin)
		.def_readonly("use_margin", &CThostFtdcInvestorPositionField::UseMargin)
		.def_readonly("frozen_margin", &CThostFtdcInvestorPositionField::FrozenMargin)
		.def_readonly("frozen_cash", &CThostFtdcInvestorPositionField::FrozenCash)
		.def_readonly("frozen_commission", &CThostFtdcInvestorPositionField::FrozenCommission)
		.def_readonly("cash_in", &CThostFtdcInvestorPositionField::CashIn)
		.def_readonly("commission", &CThostFtdcInvestorPositionField::Commission)
		.def_readonly("close_profit", &CThostFtdcInvestorPositionField::CloseProfit)
		.def_readonly("position_profit", &CThostFtdcInvestorPositionField::PositionProfit)
		.def_readonly("pre_settlement_price", &CThostFtdcInvestorPositionField::PreSettlementPrice)
		.def_readonly("settlement_price", &CThostFtdcInvestorPositionField::SettlementPrice)
		.def_readonly("trading_day", &CThostFtdcInvestorPositionField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcInvestorPositionField::SettlementID)
		.def_readonly("open_cost", &CThostFtdcInvestorPositionField::OpenCost)
		.def_readonly("exchange_margin", &CThostFtdcInvestorPositionField::ExchangeMargin)
		.def_readonly("comb_position", &CThostFtdcInvestorPositionField::CombPosition)
		.def_readonly("comb_long_frozen", &CThostFtdcInvestorPositionField::CombLongFrozen)
		.def_readonly("comb_short_frozen", &CThostFtdcInvestorPositionField::CombShortFrozen)
		.def_readonly("close_profit_by_date", &CThostFtdcInvestorPositionField::CloseProfitByDate)
		.def_readonly("close_profit_by_trade", &CThostFtdcInvestorPositionField::CloseProfitByTrade)
		.def_readonly("today_position", &CThostFtdcInvestorPositionField::TodayPosition)
		.def_readonly("margin_rate_by_money", &CThostFtdcInvestorPositionField::MarginRateByMoney)
		.def_readonly("margin_rate_by_volume", &CThostFtdcInvestorPositionField::MarginRateByVolume)
		.def_readonly("strike_frozen", &CThostFtdcInvestorPositionField::StrikeFrozen)
		.def_readonly("strike_frozen_amount", &CThostFtdcInvestorPositionField::StrikeFrozenAmount)
		.def_readonly("abandon_frozen", &CThostFtdcInvestorPositionField::AbandonFrozen)
		.def_readonly("exchange_id", &CThostFtdcInvestorPositionField::ExchangeID)
		.def_readonly("yd_strike_frozen", &CThostFtdcInvestorPositionField::YdStrikeFrozen)
		.def_readonly("invest_unit_id", &CThostFtdcInvestorPositionField::InvestUnitID)
	  ;

	class_<CThostFtdcSettlementInfoField>("SettlementInfo")
		.def_readonly("trading_day", &CThostFtdcSettlementInfoField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcSettlementInfoField::SettlementID)
		.def_readonly("broker_id", &CThostFtdcSettlementInfoField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcSettlementInfoField::InvestorID)
		.def_readonly("sequence_no", &CThostFtdcSettlementInfoField::SequenceNo)
		.def_readonly("content", &CThostFtdcSettlementInfoField::Content)
		.def_readonly("account_id", &CThostFtdcSettlementInfoField::AccountID)
		.def_readonly("currency_id", &CThostFtdcSettlementInfoField::CurrencyID)
		;

	class_<CThostFtdcInvestorPositionDetailField>("InvestorPositionDetail")
		.def_readonly("instrument_id", &CThostFtdcInvestorPositionDetailField::InstrumentID)
		.def_readonly("broker_id", &CThostFtdcInvestorPositionDetailField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcInvestorPositionDetailField::InvestorID)
		.def_readonly("hedge_flag", &CThostFtdcInvestorPositionDetailField::HedgeFlag)
		.def_readonly("direction", &CThostFtdcInvestorPositionDetailField::Direction)
		.def_readonly("open_date", &CThostFtdcInvestorPositionDetailField::OpenDate)
		.def_readonly("trade_id", &CThostFtdcInvestorPositionDetailField::TradeID)
		.def_readonly("volume", &CThostFtdcInvestorPositionDetailField::Volume)
		.def_readonly("open_price", &CThostFtdcInvestorPositionDetailField::OpenPrice)
		.def_readonly("trading_day", &CThostFtdcInvestorPositionDetailField::TradingDay)
		.def_readonly("settlement_id", &CThostFtdcInvestorPositionDetailField::SettlementID)
		.def_readonly("trade_type", &CThostFtdcInvestorPositionDetailField::TradeType)
		.def_readonly("comb_instrument_id", &CThostFtdcInvestorPositionDetailField::CombInstrumentID)
		.def_readonly("exchange_id", &CThostFtdcInvestorPositionDetailField::ExchangeID)
		.def_readonly("close_profit_by_date", &CThostFtdcInvestorPositionDetailField::CloseProfitByDate)
		.def_readonly("close_profit_by_trade", &CThostFtdcInvestorPositionDetailField::CloseProfitByTrade)
		.def_readonly("position_profit_by_date", &CThostFtdcInvestorPositionDetailField::PositionProfitByDate)
		.def_readonly("position_profit_by_trade", &CThostFtdcInvestorPositionDetailField::PositionProfitByTrade)
		.def_readonly("margin", &CThostFtdcInvestorPositionDetailField::Margin)
		.def_readonly("exchange_margin", &CThostFtdcInvestorPositionDetailField::ExchMargin)
		.def_readonly("margin_rate_by_money", &CThostFtdcInvestorPositionDetailField::MarginRateByMoney)
		.def_readonly("margin_rate_by_volume", &CThostFtdcInvestorPositionDetailField::MarginRateByVolume)
		.def_readonly("last_settlement_price", &CThostFtdcInvestorPositionDetailField::LastSettlementPrice)
		.def_readonly("settlement_price", &CThostFtdcInvestorPositionDetailField::SettlementPrice)
		.def_readonly("close_volume", &CThostFtdcInvestorPositionDetailField::CloseVolume)
		.def_readonly("close_amount", &CThostFtdcInvestorPositionDetailField::CloseAmount)
		.def_readonly("invest_unit_id", &CThostFtdcInvestorPositionDetailField::InvestUnitID)
		;

	class_<CThostFtdcInputOrderField>("InputOrder")
		.def_readonly("broker_id", &CThostFtdcInputOrderField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcInputOrderField::InvestorID)
		.def_readonly("instrument_id", &CThostFtdcInputOrderField::InstrumentID)
		.def_readonly("order_ref", &CThostFtdcInputOrderField::OrderRef)
		.def_readonly("user_id", &CThostFtdcInputOrderField::UserID)
		.def_readonly("order_price_type", &CThostFtdcInputOrderField::OrderPriceType)
		.def_readonly("direction", &CThostFtdcInputOrderField::Direction)
		.def_readonly("comb_offset_flag", &CThostFtdcInputOrderField::CombOffsetFlag)
		.def_readonly("comb_hedge_flag", &CThostFtdcInputOrderField::CombHedgeFlag)
		.def_readonly("limit_price", &CThostFtdcInputOrderField::LimitPrice)
		.def_readonly("volume_total_original", &CThostFtdcInputOrderField::VolumeTotalOriginal)
		.def_readonly("time_condition", &CThostFtdcInputOrderField::TimeCondition)
		.def_readonly("GTD_date", &CThostFtdcInputOrderField::GTDDate)
		.def_readonly("volume_condition", &CThostFtdcInputOrderField::VolumeCondition)
		.def_readonly("min_volume", &CThostFtdcInputOrderField::MinVolume)
		.def_readonly("contingent_condition", &CThostFtdcInputOrderField::ContingentCondition)
		.def_readonly("stop_price", &CThostFtdcInputOrderField::StopPrice)
		.def_readonly("force_close_reason", &CThostFtdcInputOrderField::ForceCloseReason)
		.def_readonly("is_auto_suspend", &CThostFtdcInputOrderField::IsAutoSuspend)
		.def_readonly("business_unit", &CThostFtdcInputOrderField::BusinessUnit)
		.def_readonly("request_id", &CThostFtdcInputOrderField::RequestID)
		.def_readonly("user_force_close", &CThostFtdcInputOrderField::UserForceClose)
		.def_readonly("is_swap_order", &CThostFtdcInputOrderField::IsSwapOrder)
		.def_readonly("exchange_id", &CThostFtdcInputOrderField::ExchangeID)
		.def_readonly("invest_unit_id", &CThostFtdcInputOrderField::InvestUnitID)
		.def_readonly("account_id", &CThostFtdcInputOrderField::AccountID)
		.def_readonly("currency_id", &CThostFtdcInputOrderField::CurrencyID)
		.def_readonly("client_id", &CThostFtdcInputOrderField::ClientID)
		.def_readonly("ip_address", &CThostFtdcInputOrderField::IPAddress)
		.def_readonly("mac_address", &CThostFtdcInputOrderField::MacAddress)
		;

	class_<CThostFtdcInputOrderActionField>("InputOrderAction")
		.def_readonly("broker_id", &CThostFtdcInputOrderActionField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcInputOrderActionField::InvestorID)
		.def_readonly("order_action_ref", &CThostFtdcInputOrderActionField::OrderActionRef)
		.def_readonly("order_ref", &CThostFtdcInputOrderActionField::OrderRef)
		.def_readonly("request_id", &CThostFtdcInputOrderActionField::RequestID)
		.def_readonly("front_id", &CThostFtdcInputOrderActionField::FrontID)
		.def_readonly("session_id", &CThostFtdcInputOrderActionField::SessionID)
		.def_readonly("exchange_id", &CThostFtdcInputOrderActionField::ExchangeID)
		.def_readonly("order_sys_id", &CThostFtdcInputOrderActionField::OrderSysID)
		.def_readonly("action_flag", &CThostFtdcInputOrderActionField::ActionFlag)
		.def_readonly("limit_price", &CThostFtdcInputOrderActionField::LimitPrice)
		.def_readonly("volume_change", &CThostFtdcInputOrderActionField::VolumeChange)
		.def_readonly("user_id", &CThostFtdcInputOrderActionField::UserID)
		.def_readonly("instrument_id", &CThostFtdcInputOrderActionField::InstrumentID)
		.def_readonly("invest_unit_id", &CThostFtdcInputOrderActionField::InvestUnitID)
		.def_readonly("ip_address", &CThostFtdcInputOrderActionField::IPAddress)
		.def_readonly("mac_address", &CThostFtdcInputOrderActionField::MacAddress)
		;

	class_<CThostFtdcOrderActionField>("OrderAction")
		.def_readonly("broker_id", &CThostFtdcOrderActionField::BrokerID)
		.def_readonly("investor_id", &CThostFtdcOrderActionField::InvestorID)
		.def_readonly("order_action_ref", &CThostFtdcOrderActionField::OrderActionRef)
		.def_readonly("order_ref", &CThostFtdcOrderActionField::OrderRef)
		.def_readonly("front_id", &CThostFtdcOrderActionField::FrontID)
		.def_readonly("session_id", &CThostFtdcOrderActionField::SessionID)
		.def_readonly("exchange_id", &CThostFtdcOrderActionField::ExchangeID)
		.def_readonly("order_sys_id", &CThostFtdcOrderActionField::OrderSysID)
		.def_readonly("action_flag", &CThostFtdcOrderActionField::ActionFlag)
		.def_readonly("limit_price", &CThostFtdcOrderActionField::LimitPrice)
		.def_readonly("volume_change", &CThostFtdcOrderActionField::VolumeChange)
		.def_readonly("action_date", &CThostFtdcOrderActionField::ActionDate)
		.def_readonly("action_time", &CThostFtdcOrderActionField::ActionTime)
		.def_readonly("trader_id", &CThostFtdcOrderActionField::TraderID)
		.def_readonly("install_id", &CThostFtdcOrderActionField::InstallID)
		.def_readonly("order_local_id", &CThostFtdcOrderActionField::OrderLocalID)
		.def_readonly("action_local_id", &CThostFtdcOrderActionField::ActionLocalID)
		.def_readonly("participant_id", &CThostFtdcOrderActionField::ParticipantID)
		.def_readonly("client_id", &CThostFtdcOrderActionField::ClientID)
		.def_readonly("business_unit", &CThostFtdcOrderActionField::BusinessUnit)
		.def_readonly("order_action_status", &CThostFtdcOrderActionField::OrderActionStatus)
		.def_readonly("user_id", &CThostFtdcOrderActionField::UserID)
		.def_readonly("status_msg", &CThostFtdcOrderActionField::StatusMsg)
		.def_readonly("instrument_id", &CThostFtdcOrderActionField::InstrumentID)
		.def_readonly("branch_id", &CThostFtdcOrderActionField::BranchID)
		.def_readonly("invest_unit_id", &CThostFtdcOrderActionField::InvestUnitID)
		.def_readonly("ip_address", &CThostFtdcOrderActionField::IPAddress)
		.def_readonly("mac_address", &CThostFtdcOrderActionField::MacAddress)
		;


};
