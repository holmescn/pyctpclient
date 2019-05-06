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

str UserLoginInfo_SystemName(CThostFtdcRspUserLoginField const *obj)
{
  return str(obj->SystemName);
}

str UserLoginInfo_MaxOrderRef(CThostFtdcRspUserLoginField const *obj)
{
  return str(obj->MaxOrderRef);
}

object SettlementInfo_Content(CThostFtdcSettlementInfoField const *obj)
{
  char *s = const_cast<char*>(obj->Content);
  auto pyobj = PyBytes_FromString(s);
  return object(handle<>(pyobj));
}

str InvestorPositionDetail_CombInstrumentID(CThostFtdcInvestorPositionDetailField const *obj)
{
  return str(obj->CombInstrumentID);
}

template<class T, TThostFtdcDateType T::*date>
str Str_DateType(T const *o)
{
  return str(o->*date);
}

template<class T, TThostFtdcTimeType T::*time>
str Str_TimeType(T const *o)
{
  return str(o->*time);
}

template<class T>
str Str_InstrumentID(T const *obj)
{
  return str(obj->InstrumentID);
}

template<class T>
str Str_TradingDay(T const *obj)
{
  return str(obj->TradingDay);
}

template<class T>
str Str_BrokerID(T const *obj)
{
  return str(obj->BrokerID);
}

template<class T>
str Str_UserID(T const *obj)
{
  return str(obj->UserID);
}

template<class T>
str Str_InvestorID(T const *obj)
{
  return str(obj->InvestorID);
}

template<class T>
str Str_AccountID(T const *obj)
{
  return str(obj->AccountID);
}

template<class T>
str Str_ExchangeID(T const *obj)
{
  return str(obj->ExchangeID);
}

template<class T>
str Str_CurrencyID(T const *obj)
{
  return str(obj->CurrencyID);
}

template<class T>
str Str_TradeID(T const *obj)
{
  return str(obj->TradeID);
}

template<class T>
str Str_OrderRef(T const *obj)
{
  return str(obj->OrderRef);
}

template<class T>
str Str_PositionDirection(T const *obj)
{
  switch(obj->PosiDirection) {
    case THOST_FTDC_PD_Net:   return str("net");
    case THOST_FTDC_PD_Long:  return str("long");
    case THOST_FTDC_PD_Short: return str("short");
    default: return str("unknown");
  }
}

template<class T>
str Str_Direction(T const *obj)
{
  switch(obj->Direction) {
    case THOST_FTDC_D_Buy:  return str("buy");
    case THOST_FTDC_D_Sell: return str("sell");
    default: return str("unknown");
  }
}

template<class T>
str Str_HedgeFlag(T const *obj)
{
  switch(obj->HedgeFlag) {
    case THOST_FTDC_HF_Speculation: return str("speculation");
    case THOST_FTDC_HF_Arbitrage:   return str("arbitrage");
    case THOST_FTDC_HF_Hedge:       return str("hedge");
    case THOST_FTDC_HF_MarketMaker: return str("market_maker");
    default: return str("unknown");
  }
}

template<class T>
str Str_TradeType(T const *obj)
{
  switch(obj->TradeType) {
    case THOST_FTDC_TRDT_SplitCombination: return str("split_combination");
    case THOST_FTDC_TRDT_Common: return str("common");
    case THOST_FTDC_TRDT_OptionsExecution: return str("options_execution");
    case THOST_FTDC_TRDT_OTC: return str("OTC");
    case THOST_FTDC_TRDT_EFPDerived: return str("EFP_derived");
    case THOST_FTDC_TRDT_CombinationDerived: return str("combination_derived");
    default: return str("unknown");
  }
}

template<class T>
str Str_OrderPriceType(T const *obj)
{
  switch(obj->OrderPriceType) {
    case THOST_FTDC_OPT_AnyPrice: 							 return str("any_price");
    case THOST_FTDC_OPT_LimitPrice: 						 return str("limit_price");
    case THOST_FTDC_OPT_BestPrice: 					 		 return str("best_price");
    case THOST_FTDC_OPT_LastPrice:               return str("last_price");
    case THOST_FTDC_OPT_LastPricePlusOneTicks:   return str("last_price_plus_one_tick");
    case THOST_FTDC_OPT_LastPricePlusTwoTicks:   return str("last_price_plus_two_ticks");
    case THOST_FTDC_OPT_LastPricePlusThreeTicks: return str("last_price_plus_three_ticks");
    case THOST_FTDC_OPT_AskPrice1: 					     return str("Ask_Price1");
    case THOST_FTDC_OPT_AskPrice1PlusOneTicks:   return str("ask_price1_plus_one_tick");
    case THOST_FTDC_OPT_AskPrice1PlusTwoTicks:   return str("ask_price1_plus_two_ticks");
    case THOST_FTDC_OPT_AskPrice1PlusThreeTicks: return str("ask_price1_plus_three_ticks");
    case THOST_FTDC_OPT_BidPrice1: 			  	     return str("bid_price1");
    case THOST_FTDC_OPT_BidPrice1PlusOneTicks:   return str("bid_price1_plus_one_tick");
    case THOST_FTDC_OPT_BidPrice1PlusTwoTicks:   return str("bid_price1_plus_two_ticks");
    case THOST_FTDC_OPT_BidPrice1PlusThreeTicks: return str("bid_price1_plus_three_ticks");
    case THOST_FTDC_OPT_FiveLevelPrice:          return str("five_level_price");
    default: return str("unknown");
  }
}

template<class T>
list List_CombOffsetFlag(T const *obj)
{
  list lst;
  for (size_t i = 0; i < sizeof(TThostFtdcCombOffsetFlagType); ++i) {
    switch(obj->CombOffsetFlag[i]) {
      case THOST_FTDC_OF_Open: lst.append("open");
      case THOST_FTDC_OF_Close: lst.append("close");
      case THOST_FTDC_OF_ForceClose: lst.append("force_close");
      case THOST_FTDC_OF_CloseToday: lst.append("close_today");
      case THOST_FTDC_OF_CloseYesterday: lst.append("close_yesterday");
      case THOST_FTDC_OF_ForceOff: lst.append("force_off");
      case THOST_FTDC_OF_LocalForceClose: lst.append("local_force_close");
      default: lst.append("unknown");
    }
  }

  return lst;
}

template<class T>
list List_CombHedgeFlag(T const *obj)
{
  list lst;
  for (size_t i = 0; i < sizeof(TThostFtdcCombHedgeFlagType); ++i) {
    switch(obj->CombHedgeFlag[i]) {
      case THOST_FTDC_HF_Speculation: lst.append("speculation");
      case THOST_FTDC_HF_Arbitrage: lst.append("arbitrage");
      case THOST_FTDC_HF_Hedge: lst.append("hedge");
      case THOST_FTDC_HF_MarketMaker: lst.append("market_maker");
      default: lst.append("unknown");
    }
  }

  return lst;
}

template<class T>
str Str_TimeCondition(T const *obj)
{
  switch(obj->TimeCondition) {
    case THOST_FTDC_TC_IOC: return str("IOC");
    case THOST_FTDC_TC_GFS: return str("GFS");
    case THOST_FTDC_TC_GFD: return str("GFD");
    case THOST_FTDC_TC_GTD: return str("GTD");
    case THOST_FTDC_TC_GTC: return str("GTC");
    case THOST_FTDC_TC_GFA: return str("GFA");
    default: return str("unknown");
  }
}

template<class T>
str Str_VolumeCondition(T const *obj)
{
  switch(obj->VolumeCondition) {
    case THOST_FTDC_VC_AV: return str("any_volume");
    case THOST_FTDC_VC_MV: return str("min_volume");
    case THOST_FTDC_VC_CV: return str("all");
    default: return str("unknown");
  }
}

template<class T>
str Str_ContingentCondition(T const *obj)
{
  switch(obj->ContingentCondition) {
    case THOST_FTDC_CC_Immediately: return str("immediately");
    case THOST_FTDC_CC_Touch: return str("touch");
    case THOST_FTDC_CC_TouchProfit: return str("touch_profit");
    case THOST_FTDC_CC_ParkedOrder: return str("parked_order");
    case THOST_FTDC_CC_LastPriceGreaterThanStopPrice: return str("last_price_greater_than_stop_price");
    case THOST_FTDC_CC_LastPriceGreaterEqualStopPrice: return str("last_price_greater_equal_stop_price");
    case THOST_FTDC_CC_LastPriceLesserThanStopPrice: return str("last_price_lesser_than_stop_price");
    case THOST_FTDC_CC_LastPriceLesserEqualStopPrice: return str("last_price_lesser_equal_stop_price");
    case THOST_FTDC_CC_AskPriceGreaterThanStopPrice: return str("ask_price_greater_than_stop_price");
    case THOST_FTDC_CC_AskPriceGreaterEqualStopPrice: return str("ask_price_greater_equal_stop_price");
    case THOST_FTDC_CC_AskPriceLesserThanStopPrice: return str("ask_price_lesser_than_stop_price");
    case THOST_FTDC_CC_AskPriceLesserEqualStopPrice: return str("ask_price_lesser_equal_stop_price");
    case THOST_FTDC_CC_BidPriceGreaterThanStopPrice: return str("bid_price_greater_than_stop_price");
    case THOST_FTDC_CC_BidPriceGreaterEqualStopPrice: return str("bid_price_greater_equal_stop_price");
    case THOST_FTDC_CC_BidPriceLesserThanStopPrice: return str("bid_price_lesser_than_stop_price");
    case THOST_FTDC_CC_BidPriceLesserEqualStopPrice: return str("bid_price_lesser_equal_stop_price");
    default: return str("unknown");
  }
}

template<class T>
str Str_ForceCloseReason(T const *obj)
{
  switch(obj->ForceCloseReason) {
    case THOST_FTDC_FCC_NotForceClose: return str("not_force_close");
    case THOST_FTDC_FCC_LackDeposit: return str("lack_deposit");
    case THOST_FTDC_FCC_ClientOverPositionLimit: return str("client_over_position_limit");
    case THOST_FTDC_FCC_MemberOverPositionLimit: return str("member_over_position_limit");
    case THOST_FTDC_FCC_NotMultiple: return str("not_multiple");
    case THOST_FTDC_FCC_Violation: return str("violation");
    case THOST_FTDC_FCC_Other: return str("other");
    case THOST_FTDC_FCC_PersonDeliv: return str("person_delivery");
    default: return str("unknown");
  }
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

  class_<CThostFtdcRspUserLoginField>("UserLoginInfo")
    .add_property("trading_day", Str_TradingDay<CThostFtdcRspUserLoginField>)
    .add_property("login_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::LoginTime>)
    .add_property("broker_id", Str_BrokerID<CThostFtdcRspUserLoginField>)
    .add_property("user_id", Str_UserID<CThostFtdcRspUserLoginField>)
    .add_property("system_name", UserLoginInfo_SystemName)
    .def_readonly("front_id", &CThostFtdcRspUserLoginField::FrontID)
    .def_readonly("session_id", &CThostFtdcRspUserLoginField::SessionID)
    .add_property("max_order_ref", UserLoginInfo_MaxOrderRef)
    .add_property("SHFE_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::SHFETime>)
    .add_property("DCE_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::DCETime>)
    .add_property("CZCE_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::CZCETime>)
    .add_property("FFEX_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::FFEXTime>)
    .add_property("INE_time", Str_TimeType<CThostFtdcRspUserLoginField, &CThostFtdcRspUserLoginField::INETime>)
    ;

  class_<CThostFtdcUserLogoutField>("UserLogoutInfo")
    .add_property("broker_id", Str_BrokerID<CThostFtdcUserLogoutField>)
    .add_property("user_id", Str_UserID<CThostFtdcUserLogoutField>)
    ;

  class_<CThostFtdcRspInfoField>("ResponseInfo")
    .def_readonly("error_id", &CThostFtdcRspInfoField::ErrorID)
    .add_property("error_msg", ResponseInfo_ErrorMsg)
    ;

  class_<CThostFtdcSpecificInstrumentField>("SpecificInstrument")
    .add_property("instrument_id", Str_InstrumentID<CThostFtdcSpecificInstrumentField>)
    ;

  class_<CThostFtdcDepthMarketDataField>("MarketData")
    .add_property("trading_day", Str_TradingDay<CThostFtdcDepthMarketDataField>)
    .add_property("instrument_id", Str_InstrumentID<CThostFtdcDepthMarketDataField>)
    .add_property("exchange_id", Str_ExchangeID<CThostFtdcDepthMarketDataField>)
    // .add_property("exchange_inst_id", &CThostFtdcDepthMarketDataField::ExchangeInstID)
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
    .add_property("update_time", Str_TimeType<CThostFtdcDepthMarketDataField, &CThostFtdcDepthMarketDataField::UpdateTime>)
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
    .add_property("action_day", Str_DateType<CThostFtdcDepthMarketDataField, &CThostFtdcDepthMarketDataField::ActionDay>)
    ;

  class_<CThostFtdcSettlementInfoField>("SettlementInfo")
    .add_property("trading_day", Str_TradingDay<CThostFtdcSettlementInfoField>)
    .def_readonly("settlement_id", &CThostFtdcSettlementInfoField::SettlementID)
    .add_property("broker_id", Str_BrokerID<CThostFtdcSettlementInfoField>)
    .add_property("investor_id", Str_InvestorID<CThostFtdcSettlementInfoField>)
    .def_readonly("sequence_no", &CThostFtdcSettlementInfoField::SequenceNo)
    .add_property("content", SettlementInfo_Content)
    .add_property("account_id", Str_AccountID<CThostFtdcSettlementInfoField>)
    .add_property("currency_id", Str_CurrencyID<CThostFtdcSettlementInfoField>)
    ;

  class_<CThostFtdcSettlementInfoConfirmField>("SettlementInfoConfirm")
    .add_property("broker_id", Str_BrokerID<CThostFtdcSettlementInfoConfirmField>)
    .add_property("investor_id", Str_InvestorID<CThostFtdcSettlementInfoConfirmField>)
    .add_property("confirm_date", Str_DateType<CThostFtdcSettlementInfoConfirmField, &CThostFtdcSettlementInfoConfirmField::ConfirmDate>)
    .add_property("confirm_time", Str_TimeType<CThostFtdcSettlementInfoConfirmField, &CThostFtdcSettlementInfoConfirmField::ConfirmTime>)
    .add_property("currency_id", Str_CurrencyID<CThostFtdcSettlementInfoConfirmField>)
    ;

  class_<CThostFtdcTradingAccountField>("TradingAccount")
    .add_property("broker_id", Str_BrokerID<CThostFtdcTradingAccountField>)
    .add_property("account_id", Str_AccountID<CThostFtdcTradingAccountField>)
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
    .def_readonly("current_margin", &CThostFtdcTradingAccountField::CurrMargin)
    .def_readonly("cash_in", &CThostFtdcTradingAccountField::CashIn)
    .def_readonly("commission", &CThostFtdcTradingAccountField::Commission)
    .def_readonly("close_profit", &CThostFtdcTradingAccountField::CloseProfit)
    .def_readonly("position_profit", &CThostFtdcTradingAccountField::PositionProfit)
    .def_readonly("balance", &CThostFtdcTradingAccountField::Balance)
    .def_readonly("available", &CThostFtdcTradingAccountField::Available)
    .def_readonly("withdraw_quota", &CThostFtdcTradingAccountField::WithdrawQuota)
    .def_readonly("reserve", &CThostFtdcTradingAccountField::Reserve)
    .add_property("trading_day", Str_TradingDay<CThostFtdcTradingAccountField>)
    .def_readonly("settlement_id", &CThostFtdcTradingAccountField::SettlementID)
    .def_readonly("credit", &CThostFtdcTradingAccountField::Credit)
    .def_readonly("mortgage", &CThostFtdcTradingAccountField::Mortgage)
    .def_readonly("exchange_margin", &CThostFtdcTradingAccountField::ExchangeMargin)
    .def_readonly("delivery_margin", &CThostFtdcTradingAccountField::DeliveryMargin)
    .def_readonly("exchange_delivery_margin", &CThostFtdcTradingAccountField::ExchangeDeliveryMargin)
    .def_readonly("reserve_balance", &CThostFtdcTradingAccountField::ReserveBalance)
    .add_property("currency_id", Str_CurrencyID<CThostFtdcTradingAccountField>)
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
    // .def_readonly("biz_type", &CThostFtdcTradingAccountField::BizType)
    .def_readonly("frozen_swap", &CThostFtdcTradingAccountField::FrozenSwap)
    .def_readonly("remain_swap", &CThostFtdcTradingAccountField::RemainSwap)
    ;

  class_<CThostFtdcInvestorPositionField>("InvestorPosition")
    .add_property("instrument_id", Str_InstrumentID<CThostFtdcInvestorPositionField>)
    .add_property("broker_id", Str_BrokerID<CThostFtdcInvestorPositionField>)
    .add_property("investor_id", Str_InvestorID<CThostFtdcInvestorPositionField>)
    .add_property("position_direction", Str_PositionDirection<CThostFtdcInvestorPositionField>)
    .add_property("hedge_flag", Str_HedgeFlag<CThostFtdcInvestorPositionField>)
    .add_property("position_date", Str_DateType<CThostFtdcInvestorPositionField, &CThostFtdcInvestorPositionField::PositionDate>)
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
    .add_property("trading_day", Str_TradingDay<CThostFtdcInvestorPositionField>)
    .def_readonly("settlement_id", &CThostFtdcInvestorPositionField::SettlementID)
    .def_readonly("open_cost", &CThostFtdcInvestorPositionField::OpenCost)
    .def_readonly("exchange_margin", &CThostFtdcInvestorPositionField::ExchangeMargin)
    .def_readonly("combine_position", &CThostFtdcInvestorPositionField::CombPosition)
    .def_readonly("combine_long_frozen", &CThostFtdcInvestorPositionField::CombLongFrozen)
    .def_readonly("combine_short_frozen", &CThostFtdcInvestorPositionField::CombShortFrozen)
    .def_readonly("close_profit_by_date", &CThostFtdcInvestorPositionField::CloseProfitByDate)
    .def_readonly("close_profit_by_trade", &CThostFtdcInvestorPositionField::CloseProfitByTrade)
    .def_readonly("today_position", &CThostFtdcInvestorPositionField::TodayPosition)
    .def_readonly("margin_rate_by_money", &CThostFtdcInvestorPositionField::MarginRateByMoney)
    .def_readonly("margin_rate_by_volume", &CThostFtdcInvestorPositionField::MarginRateByVolume)
    .def_readonly("strike_frozen", &CThostFtdcInvestorPositionField::StrikeFrozen)
    .def_readonly("strike_frozen_amount", &CThostFtdcInvestorPositionField::StrikeFrozenAmount)
    .def_readonly("abandon_frozen", &CThostFtdcInvestorPositionField::AbandonFrozen)
    .add_property("exchange_id", Str_ExchangeID<CThostFtdcInvestorPositionField>)
    .def_readonly("yd_strike_frozen", &CThostFtdcInvestorPositionField::YdStrikeFrozen)
    // .def_readonly("invest_unit_id", &CThostFtdcInvestorPositionField::InvestUnitID)
    ;

  class_<CThostFtdcInvestorPositionDetailField>("InvestorPositionDetail")
    .add_property("instrument_id", Str_InstrumentID<CThostFtdcInvestorPositionDetailField>)
    .add_property("broker_id", Str_BrokerID<CThostFtdcInvestorPositionDetailField>)
    .add_property("investor_id", Str_InvestorID<CThostFtdcInvestorPositionDetailField>)
    .add_property("hedge_flag", Str_HedgeFlag<CThostFtdcInvestorPositionDetailField>)
    .add_property("direction", Str_Direction<CThostFtdcInvestorPositionDetailField>)
    .add_property("open_date", Str_DateType<CThostFtdcInvestorPositionDetailField, &CThostFtdcInvestorPositionDetailField::OpenDate>)
    .add_property("trade_id", Str_TradeID<CThostFtdcInvestorPositionDetailField>)
    .def_readonly("volume", &CThostFtdcInvestorPositionDetailField::Volume)
    .def_readonly("open_price", &CThostFtdcInvestorPositionDetailField::OpenPrice)
    .add_property("trading_day", Str_TradingDay<CThostFtdcInvestorPositionDetailField>)
    .def_readonly("settlement_id", &CThostFtdcInvestorPositionDetailField::SettlementID)
    .add_property("trade_type", Str_TradeType<CThostFtdcInvestorPositionDetailField>)
    .add_property("combine_instrument_id", InvestorPositionDetail_CombInstrumentID)
    .add_property("exchange_id", Str_ExchangeID<CThostFtdcInvestorPositionDetailField>)
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
    // .def_readonly("invest_unit_id", &CThostFtdcInvestorPositionDetailField::InvestUnitID)
    ;

  class_<CThostFtdcInputOrderField>("InputOrder")
    .add_property("broker_id", Str_BrokerID<CThostFtdcInputOrderField>)
    .add_property("investor_id", Str_InvestorID<CThostFtdcInputOrderField>)
    .add_property("instrument_id", Str_InstrumentID<CThostFtdcInputOrderField>)
    .add_property("order_ref", Str_OrderRef<CThostFtdcInputOrderField>)
    .add_property("user_id", Str_UserID<CThostFtdcInputOrderField>)
    .add_property("order_price_type", Str_OrderPriceType<CThostFtdcInputOrderField>)
    .add_property("direction", Str_Direction<CThostFtdcInputOrderField>)
    .add_property("combine_offset_flag", List_CombOffsetFlag<CThostFtdcInputOrderField>)
    .add_property("combine_hedge_flag", List_CombHedgeFlag<CThostFtdcInputOrderField>)
    .def_readonly("limit_price", &CThostFtdcInputOrderField::LimitPrice)
    .def_readonly("volume_total_original", &CThostFtdcInputOrderField::VolumeTotalOriginal)
    .add_property("time_condition", Str_TimeCondition<CThostFtdcInputOrderField>)
    .add_property("GTD_date", Str_DateType<CThostFtdcInputOrderField, &CThostFtdcInputOrderField::GTDDate>)
    .add_property("volume_condition", Str_VolumeCondition<CThostFtdcInputOrderField>)
    .def_readonly("min_volume", &CThostFtdcInputOrderField::MinVolume)
    .add_property("contingent_condition", Str_ContingentCondition<CThostFtdcInputOrderField>)
    .def_readonly("stop_price", &CThostFtdcInputOrderField::StopPrice)
    .add_property("force_close_reason", Str_ForceCloseReason<CThostFtdcInputOrderField>)
    .def_readonly("is_auto_suspend", &CThostFtdcInputOrderField::IsAutoSuspend)
    // .def_readonly("business_unit", &CThostFtdcInputOrderField::BusinessUnit)
    .def_readonly("request_id", &CThostFtdcInputOrderField::RequestID)
    .def_readonly("user_force_close", &CThostFtdcInputOrderField::UserForceClose)
    .def_readonly("is_swap_order", &CThostFtdcInputOrderField::IsSwapOrder)
    .add_property("exchange_id", Str_ExchangeID<CThostFtdcInputOrderField>)
    // .def_readonly("invest_unit_id", &CThostFtdcInputOrderField::InvestUnitID)
    .add_property("account_id", Str_AccountID<CThostFtdcInputOrderField>)
    .add_property("currency_id", Str_CurrencyID<CThostFtdcInputOrderField>)
    // .def_readonly("client_id", &CThostFtdcInputOrderField::ClientID)
    // .def_readonly("ip_address", &CThostFtdcInputOrderField::IPAddress)
    // .def_readonly("mac_address", &CThostFtdcInputOrderField::MacAddress)
    ;

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
    .def("exit", &CtpClient::Exit)

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
    .def("on_td_user_login", pure_virtual(&CtpClient::OnTdUserLogin))
    .def("on_td_user_logout", pure_virtual(&CtpClient::OnTdUserLogout))
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


  class_<CThostFtdcOrderField, boost::shared_ptr<CThostFtdcOrderField> >("Order")
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
