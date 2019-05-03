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
		.def("confirm_settlement_info", &CtpClient::ConfirmSettlementInfo)
		.def("query_order", &CtpClient::QueryOrder)
		.def("query_trading_account", &CtpClient::QueryTradingAccount)
		.def("query_investor_position", &CtpClient::QueryInvestorPosition)
		.def("on_td_front_connected", pure_virtual(&CtpClient::OnTdFrontConnected))
		.def("on_settlement_info_confirm", pure_virtual(&CtpClient::OnRspSettlementInfoConfirm))
		.def("on_rsp_order", pure_virtual(&CtpClient::OnRspQryOrder))
		.def("on_rsp_trading_account", pure_virtual(&CtpClient::OnRspQryTradingAccount))
		.def("on_rsp_investor_position", pure_virtual(&CtpClient::OnRspQryInvestorPosition))
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

	class_<CThostFtdcOrderField>("Order")
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


};
