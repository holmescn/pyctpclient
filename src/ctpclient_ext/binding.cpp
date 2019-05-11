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
#include "mdspi.h"

using namespace boost::python;

#pragma region Exception translators

static void trans_exception(int rc, std::string request)
{
  auto msg = "[%d] %s" % boost::python::make_tuple(rc, request);
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
  auto msg = "InvalidArgument %s." % boost::python::make_tuple(e.what());
  std::string msg_s = extract<std::string>(msg);
  PyErr_SetString(PyExc_Exception, msg_s.c_str());
}

#pragma endregion // Exception translators

#pragma region Getters

template<class T1, class T2>
inline boost::python::object tostr(T2 T1::*member_var)
{
  return make_function(
        [member_var](T1 const* this_) { return str(this_->*member_var); },
        default_call_policies(),
        boost::mpl::vector<str, T1 const*>());
}

template<class T1, class T2>
inline boost::python::object tobytes(T2 T1::*member_var)
{
  return make_function(
        [member_var](T1 const* this_) {
          char *s = const_cast<char*>(this_->*member_var);
          auto pyobj = PyBytes_FromString(s);
          return boost::python::object(handle<>(pyobj));
        },
        default_call_policies(),
        boost::mpl::vector<boost::python::object, T1 const*>());
}

str InvestorPosition_PositionDate(CThostFtdcInvestorPositionField const *this_)
{
  switch(this_->PositionDate) {
    case THOST_FTDC_PSD_Today: return str("today");
    case THOST_FTDC_PSD_History: return str("history");
    default: return str("unknown");
  }
}

template<class T>
str tostr_PositionDirection(T const *this_)
{
  switch(this_->PosiDirection) {
    case THOST_FTDC_PD_Net:   return str("net");
    case THOST_FTDC_PD_Long:  return str("long");
    case THOST_FTDC_PD_Short: return str("short");
    default: return str("unknown");
  }
}

template<class T>
Direction toenum_Direction(T const *this_)
{
  switch(this_->Direction) {
    case THOST_FTDC_D_Buy:  return D_Buy;
    case THOST_FTDC_D_Sell: return D_Sell;
    default: return D_Unknown;
  }
}

template<class T>
str tostr_TradeType(T const *this_)
{
  switch(this_->TradeType) {
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
str tostr_OrderPriceType(T const *this_)
{
  switch(this_->OrderPriceType) {
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
str tostr_OffsetFlag(T const *this_)
{
  switch(this_->OffsetFlag) {
    case THOST_FTDC_OF_Open: return str("open");
    case THOST_FTDC_OF_Close: return str("close");
    case THOST_FTDC_OF_ForceClose: return str("force_close");
    case THOST_FTDC_OF_CloseToday: return str("close_today");
    case THOST_FTDC_OF_CloseYesterday: return str("close_yesterday");
    case THOST_FTDC_OF_ForceOff: return str("force_off");
    case THOST_FTDC_OF_LocalForceClose: return str("local_force_close");
    default: return str("unknown");
  }
}

template<class T>
list tolist_CombOffsetFlag(T const *this_)
{
  list L;
  for (size_t i = 0; i < sizeof(TThostFtdcCombOffsetFlagType); ++i) {
    switch(this_->CombOffsetFlag[i]) {
      case THOST_FTDC_OF_Open:
        L.append("open");
        break;
      case THOST_FTDC_OF_Close:
        L.append("close");
        break;
      case THOST_FTDC_OF_ForceClose:
        L.append("force_close");
        break;
      case THOST_FTDC_OF_CloseToday:
        L.append("close_today");
        break;
      case THOST_FTDC_OF_CloseYesterday:
        L.append("close_yesterday");
        break;
      case THOST_FTDC_OF_ForceOff:
        L.append("force_off");
        break;
      case THOST_FTDC_OF_LocalForceClose:
        L.append("local_force_close");
        break;
      default:
        L.append("unknown");
        break;
    }
  }

  return L;
}

template<class T>
str tostr_HedgeFlag(T const *this_)
{
  switch(this_->HedgeFlag) {
    case THOST_FTDC_HF_Speculation: return str("speculation");
    case THOST_FTDC_HF_Arbitrage: return str("arbitrage");
    case THOST_FTDC_HF_Hedge: return str("hedge");
    case THOST_FTDC_HF_MarketMaker: return str("market_maker");
    default: return str("unknown");
  }
}

template<class T>
list tolist_CombHedgeFlag(T const *this_)
{
  list L;
  for (size_t i = 0; i < sizeof(TThostFtdcCombHedgeFlagType); ++i) {
    switch(this_->CombHedgeFlag[i]) {
      case THOST_FTDC_HF_Speculation:
        L.append("speculation");
        break;
      case THOST_FTDC_HF_Arbitrage:
        L.append("arbitrage");
        break;
      case THOST_FTDC_HF_Hedge:
        L.append("hedge");
        break;
      case THOST_FTDC_HF_MarketMaker:
        L.append("market_maker");
        break;
      default:
        L.append("unknown");
        break;
    }
  }

  return L;
}

template<class T>
str tostr_TimeCondition(T const *this_)
{
  switch(this_->TimeCondition) {
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
str tostr_VolumeCondition(T const *this_)
{
  switch(this_->VolumeCondition) {
    case THOST_FTDC_VC_AV: return str("any_volume");
    case THOST_FTDC_VC_MV: return str("min_volume");
    case THOST_FTDC_VC_CV: return str("all");
    default: return str("unknown");
  }
}

template<class T>
str tostr_ContingentCondition(T const *this_)
{
  switch(this_->ContingentCondition) {
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
str tostr_ForceCloseReason(T const *this_)
{
  switch(this_->ForceCloseReason) {
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

template<class T>
str tostr_ActionFlag(T const *this_)
{
  switch(this_->ActionFlag) {
    case THOST_FTDC_AF_Delete: return str("delete");
    case THOST_FTDC_AF_Modify: return str("modify");
    default: return str("unknown");
  }
}

template<class T>
str tostr_OrderSubmitStatus(T const *this_)
{
  switch(this_->OrderSubmitStatus) {
    case THOST_FTDC_OSS_InsertSubmitted: return str("insert_submitted");
    case THOST_FTDC_OSS_CancelSubmitted: return str("cancel_submitted");
    case THOST_FTDC_OSS_Accepted:        return str("accepted");
    case THOST_FTDC_OSS_InsertRejected:  return str("insert_rejected");
    case THOST_FTDC_OSS_CancelRejected:  return str("cancel_rejected");
    case THOST_FTDC_OSS_ModifyRejected:  return str("modify_rejected");
    default: return str("unknown");
  }
}

template<class T>
str tostr_OrderStatus(T const *this_)
{
  switch(this_->OrderStatus) {
    case THOST_FTDC_OST_AllTraded: return str("all_traded");
    case THOST_FTDC_OST_PartTradedQueueing: return str("part_traded_queueing");
    case THOST_FTDC_OST_PartTradedNotQueueing: return str("part_traded_not_queueing");
    case THOST_FTDC_OST_NoTradeQueueing:  return str("no_trade_queueing");
    case THOST_FTDC_OST_NoTradeNotQueueing:  return str("no_trade_not_queueing");
    case THOST_FTDC_OST_Canceled:  return str("canceled");
    case THOST_FTDC_OST_Unknown:  return str("unknown");
    case THOST_FTDC_OST_NotTouched:  return str("not_touched");
    case THOST_FTDC_OST_Touched:  return str("touched");
    default: return str("error");
  }
}

template<class T>
str tostr_OrderSource(T const *this_)
{
  switch(this_->OrderSource) {
    case THOST_FTDC_OSRC_Participant: return str("participant");
    case THOST_FTDC_OSRC_Administrator: return str("administrator");
    default: return str("unknown");
  }
}

template<class T>
str tostr_OrderType(T const *this_)
{
  switch(this_->OrderType) {
    case THOST_FTDC_ORDT_Normal: return str("normal");
    case THOST_FTDC_ORDT_DeriveFromQuote: return str("derive_from_quote");
    case THOST_FTDC_ORDT_DeriveFromCombination: return str("derive_from_combination");
    case THOST_FTDC_ORDT_Combination: return str("combination");
    case THOST_FTDC_ORDT_ConditionalOrder: return str("conditional_order");
    case THOST_FTDC_ORDT_Swap: return str("swap");
    default: return str("unknown");
  }
}

template<class T>
str tostr_TradingRole(T const *this_)
{
  switch (this_->TradingRole) {
    case THOST_FTDC_ER_Broker: return str("broker");
    case THOST_FTDC_ER_Host: return str("host");
    case THOST_FTDC_ER_Maker: return str("market_maker");
    default: return str("unknown");
  }
}

template<class T>
str tostr_PriceSource(T const* this_)
{
  switch(this_->PriceSource) {
    case THOST_FTDC_PSRC_LastPrice: return str("last_price");
    case THOST_FTDC_PSRC_Buy: return str("buy");
    case THOST_FTDC_PSRC_Sell: return str("sell");
    default: return str("unknown");
  }
}

template<class T>
str tostr_TradeSource(T const* this_)
{
  switch(this_->TradeSource) {
    case THOST_FTDC_TSRC_NORMAL: return str("normal");
    case THOST_FTDC_TSRC_QUERY: return str("query");
    default: return str("unknown");
  }
}

template<class T>
str tostr_OrderActionStatus(T const* this_)
{
  switch(this_->OrderActionStatus) {
    case THOST_FTDC_OAS_Submitted: return str("submitted");
    case THOST_FTDC_OAS_Accepted: return str("accepted");
    case THOST_FTDC_OAS_Rejected: return str("rejected");
    default: return str("unknown");
  }
}

#pragma endregion // Getters

BOOST_PYTHON_MODULE(_ctpclient)
{
  PyEval_InitThreads();

  register_exception_translator<RequestNetworkException>(RequestNetworkException_translator);
  register_exception_translator<FullRequestQueueException>(FullRequestQueueException_translator);
  register_exception_translator<RequestTooFrequentlyException>(RequestTooFrequentlyException_translator);
  register_exception_translator<UnknownRequestException>(UnknownRequestException_translator);
  register_exception_translator<std::invalid_argument>(invalid_argument_translator);

  enum_<Direction>("Direction")
    .value("UNKNOWN", D_Unknown)
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

  class_<M1Bar>("M1Bar")
    .add_property("instrument_id", tostr(&M1Bar::InstrumentID))
    .add_property("update_time", tostr(&M1Bar::UpdateTime))
    .def_readonly("open", &M1Bar::OpenPrice)
    .def_readonly("high", &M1Bar::HighestPrice)
    .def_readonly("low", &M1Bar::LowestPrice)
    .def_readonly("close", &M1Bar::ClosePrice)
    .def_readonly("volume", &M1Bar::Volume)
    .def_readonly("turnover", &M1Bar::Turnover)
    .def_readonly("position", &M1Bar::Position)
    ;

  class_<TickBar>("TickBar")
    .add_property("instrument_id", tostr(&TickBar::InstrumentID))
    .add_property("update_time", tostr(&TickBar::UpdateTime))
    .def_readonly("price", &TickBar::Price)
    .def_readonly("volume", &TickBar::Volume)
    .def_readonly("turnover", &TickBar::Turnover)
    .def_readonly("position", &TickBar::Position)
    ;

  class_<CThostFtdcRspUserLoginField>("UserLoginInfo")
    .add_property("trading_day", tostr(&CThostFtdcRspUserLoginField::TradingDay))
    .add_property("login_time", tostr(&CThostFtdcRspUserLoginField::LoginTime))
    .add_property("broker_id", tostr(&CThostFtdcRspUserLoginField::BrokerID))
    .add_property("user_id", tostr(&CThostFtdcRspUserLoginField::UserID))
    .add_property("system_name", tostr(&CThostFtdcRspUserLoginField::SystemName))
    .def_readonly("front_id", &CThostFtdcRspUserLoginField::FrontID)
    .def_readonly("session_id", &CThostFtdcRspUserLoginField::SessionID)
    .add_property("max_order_ref", tostr(&CThostFtdcRspUserLoginField::MaxOrderRef))
    .add_property("SHFE_time", tostr(&CThostFtdcRspUserLoginField::SHFETime))
    .add_property("DCE_time", tostr(&CThostFtdcRspUserLoginField::DCETime))
    .add_property("CZCE_time", tostr(&CThostFtdcRspUserLoginField::CZCETime))
    .add_property("FFEX_time", tostr(&CThostFtdcRspUserLoginField::FFEXTime))
    .add_property("INE_time", tostr(&CThostFtdcRspUserLoginField::INETime))
    ;

  class_<CThostFtdcUserLogoutField>("UserLogoutInfo")
    .add_property("broker_id", tostr(&CThostFtdcUserLogoutField::BrokerID))
    .add_property("user_id", tostr(&CThostFtdcUserLogoutField::UserID))
    ;

  class_<CThostFtdcRspInfoField>("ResponseInfo")
    .def_readonly("error_id", &CThostFtdcRspInfoField::ErrorID)
    .add_property("error_msg", tobytes(&CThostFtdcRspInfoField::ErrorMsg))
    ;

  class_<CThostFtdcSpecificInstrumentField>("SpecificInstrument")
    .add_property("instrument_id", tostr(&CThostFtdcSpecificInstrumentField::InstrumentID))
    ;

  class_<CThostFtdcDepthMarketDataField>("MarketData")
    .add_property("trading_day", tostr(&CThostFtdcDepthMarketDataField::TradingDay))
    .add_property("instrument_id", tostr(&CThostFtdcDepthMarketDataField::InstrumentID))
    .add_property("exchange_id", tostr(&CThostFtdcDepthMarketDataField::ExchangeID))
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
    .add_property("update_time", tostr(&CThostFtdcDepthMarketDataField::UpdateTime))
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
    .add_property("action_day", tostr(&CThostFtdcDepthMarketDataField::ActionDay))
    ;

  class_<CThostFtdcSettlementInfoField>("SettlementInfo")
    .add_property("trading_day", tostr(&CThostFtdcSettlementInfoField::TradingDay))
    .def_readonly("settlement_id", &CThostFtdcSettlementInfoField::SettlementID)
    .add_property("broker_id", tostr(&CThostFtdcSettlementInfoField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcSettlementInfoField::InvestorID))
    .def_readonly("sequence_no", &CThostFtdcSettlementInfoField::SequenceNo)
    .add_property("content", tobytes(&CThostFtdcSettlementInfoField::Content))
    .add_property("account_id", tostr(&CThostFtdcSettlementInfoField::AccountID))
    .add_property("currency_id", tostr(&CThostFtdcSettlementInfoField::CurrencyID))
    ;

  class_<CThostFtdcSettlementInfoConfirmField>("SettlementInfoConfirm")
    .add_property("broker_id", tostr(&CThostFtdcSettlementInfoConfirmField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcSettlementInfoConfirmField::InvestorID))
    .add_property("confirm_date", tostr(&CThostFtdcSettlementInfoConfirmField::ConfirmDate))
    .add_property("confirm_time", tostr(&CThostFtdcSettlementInfoConfirmField::ConfirmTime))
    .add_property("currency_id", tostr(&CThostFtdcSettlementInfoConfirmField::CurrencyID))
    ;

  class_<CThostFtdcTradingAccountField>("TradingAccount")
    .add_property("broker_id", tostr(&CThostFtdcTradingAccountField::BrokerID))
    .add_property("account_id", tostr(&CThostFtdcTradingAccountField::AccountID))
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
    .add_property("trading_day", tostr(&CThostFtdcTradingAccountField::TradingDay))
    .def_readonly("settlement_id", &CThostFtdcTradingAccountField::SettlementID)
    .def_readonly("credit", &CThostFtdcTradingAccountField::Credit)
    .def_readonly("mortgage", &CThostFtdcTradingAccountField::Mortgage)
    .def_readonly("exchange_margin", &CThostFtdcTradingAccountField::ExchangeMargin)
    .def_readonly("delivery_margin", &CThostFtdcTradingAccountField::DeliveryMargin)
    .def_readonly("exchange_delivery_margin", &CThostFtdcTradingAccountField::ExchangeDeliveryMargin)
    .def_readonly("reserve_balance", &CThostFtdcTradingAccountField::ReserveBalance)
    .add_property("currency_id", tostr(&CThostFtdcTradingAccountField::CurrencyID))
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
    .add_property("instrument_id", tostr(&CThostFtdcInvestorPositionField::InstrumentID))
    .add_property("broker_id", tostr(&CThostFtdcInvestorPositionField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcInvestorPositionField::InvestorID))
    .add_property("position_direction", tostr_PositionDirection<CThostFtdcInvestorPositionField>)
    .add_property("hedge_flag", tostr_HedgeFlag<CThostFtdcInvestorPositionField>)
    .add_property("position_date", InvestorPosition_PositionDate)
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
    .add_property("trading_day", tostr(&CThostFtdcInvestorPositionField::TradingDay))
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
    .add_property("exchange_id", tostr(&CThostFtdcInvestorPositionField::ExchangeID))
    .def_readonly("yd_strike_frozen", &CThostFtdcInvestorPositionField::YdStrikeFrozen)
    // .def_readonly("invest_unit_id", &CThostFtdcInvestorPositionField::InvestUnitID)
    ;

  class_<CThostFtdcInvestorPositionDetailField>("InvestorPositionDetail")
    .add_property("instrument_id", tostr(&CThostFtdcInvestorPositionDetailField::InstrumentID))
    .add_property("broker_id", tostr(&CThostFtdcInvestorPositionDetailField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcInvestorPositionDetailField::InvestorID))
    .add_property("hedge_flag", tostr_HedgeFlag<CThostFtdcInvestorPositionDetailField>)
    .add_property("direction", toenum_Direction<CThostFtdcInvestorPositionDetailField>)
    .add_property("open_date", tostr(&CThostFtdcInvestorPositionDetailField::OpenDate))
    .add_property("trade_id", tostr(&CThostFtdcInvestorPositionDetailField::TradeID))
    .def_readonly("volume", &CThostFtdcInvestorPositionDetailField::Volume)
    .def_readonly("open_price", &CThostFtdcInvestorPositionDetailField::OpenPrice)
    .add_property("trading_day", tostr(&CThostFtdcInvestorPositionDetailField::TradingDay))
    .def_readonly("settlement_id", &CThostFtdcInvestorPositionDetailField::SettlementID)
    .add_property("trade_type", tostr_TradeType<CThostFtdcInvestorPositionDetailField>)
    .add_property("combine_instrument_id", tostr(&CThostFtdcInvestorPositionDetailField::CombInstrumentID))
    .add_property("exchange_id", tostr(&CThostFtdcInvestorPositionDetailField::ExchangeID))
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
    .add_property("broker_id", tostr(&CThostFtdcInputOrderField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcInputOrderField::InvestorID))
    .add_property("instrument_id", tostr(&CThostFtdcInputOrderField::InstrumentID))
    .add_property("order_ref", tostr(&CThostFtdcInputOrderField::OrderRef))
    .add_property("user_id", tostr(&CThostFtdcInputOrderField::UserID))
    .add_property("order_price_type", tostr_OrderPriceType<CThostFtdcInputOrderField>)
    .add_property("direction", toenum_Direction<CThostFtdcInputOrderField>)
    .add_property("combine_offset_flag", tolist_CombOffsetFlag<CThostFtdcInputOrderField>)
    .add_property("combine_hedge_flag", tolist_CombHedgeFlag<CThostFtdcInputOrderField>)
    .def_readonly("limit_price", &CThostFtdcInputOrderField::LimitPrice)
    .def_readonly("volume_total_original", &CThostFtdcInputOrderField::VolumeTotalOriginal)
    .add_property("time_condition", tostr_TimeCondition<CThostFtdcInputOrderField>)
    .add_property("GTD_date", tostr(&CThostFtdcInputOrderField::GTDDate))
    .add_property("volume_condition", tostr_VolumeCondition<CThostFtdcInputOrderField>)
    .def_readonly("min_volume", &CThostFtdcInputOrderField::MinVolume)
    .add_property("contingent_condition", tostr_ContingentCondition<CThostFtdcInputOrderField>)
    .def_readonly("stop_price", &CThostFtdcInputOrderField::StopPrice)
    .add_property("force_close_reason", tostr_ForceCloseReason<CThostFtdcInputOrderField>)
    .def_readonly("is_auto_suspend", &CThostFtdcInputOrderField::IsAutoSuspend)
    // .def_readonly("business_unit", &CThostFtdcInputOrderField::BusinessUnit)
    .def_readonly("request_id", &CThostFtdcInputOrderField::RequestID)
    .def_readonly("user_force_close", &CThostFtdcInputOrderField::UserForceClose)
    .def_readonly("is_swap_order", &CThostFtdcInputOrderField::IsSwapOrder)
    .add_property("exchange_id", tostr(&CThostFtdcInputOrderField::ExchangeID))
    // .def_readonly("invest_unit_id", &CThostFtdcInputOrderField::InvestUnitID)
    .add_property("account_id", tostr(&CThostFtdcInputOrderField::AccountID))
    .add_property("currency_id", tostr(&CThostFtdcInputOrderField::CurrencyID))
    // .def_readonly("client_id", &CThostFtdcInputOrderField::ClientID)
    // .def_readonly("ip_address", &CThostFtdcInputOrderField::IPAddress)
    // .def_readonly("mac_address", &CThostFtdcInputOrderField::MacAddress)
    ;

  class_<CThostFtdcInputOrderActionField>("InputOrderAction")
    .add_property("broker_id", tostr(&CThostFtdcInputOrderActionField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcInputOrderActionField::InvestorID))
    .add_property("order_action_ref", tostr(&CThostFtdcInputOrderActionField::OrderActionRef))
    .add_property("order_ref", tostr(&CThostFtdcInputOrderActionField::OrderActionRef))
    .def_readonly("request_id", &CThostFtdcInputOrderActionField::RequestID)
    .def_readonly("front_id", &CThostFtdcInputOrderActionField::FrontID)
    .def_readonly("session_id", &CThostFtdcInputOrderActionField::SessionID)
    .add_property("exchange_id", tostr(&CThostFtdcInputOrderActionField::ExchangeID))
    .add_property("order_sys_id", tostr(&CThostFtdcInputOrderActionField::OrderSysID))
    .add_property("action_flag", tostr_ActionFlag<CThostFtdcInputOrderActionField>)
    .def_readonly("limit_price", &CThostFtdcInputOrderActionField::LimitPrice)
    .def_readonly("volume_change", &CThostFtdcInputOrderActionField::VolumeChange)
    .add_property("user_id", tostr(&CThostFtdcInputOrderActionField::UserID))
    .add_property("instrument_id", tostr(&CThostFtdcInputOrderActionField::InstrumentID))
    // .def_readonly("invest_unit_id", &CThostFtdcInputOrderActionField::InvestUnitID)
    // .def_readonly("ip_address", &CThostFtdcInputOrderActionField::IPAddress)
    // .def_readonly("mac_address", &CThostFtdcInputOrderActionField::MacAddress)
    ;

  class_<CThostFtdcOrderField, boost::shared_ptr<CThostFtdcOrderField> >("Order")
    .add_property("broker_id", tostr(&CThostFtdcOrderField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcOrderField::InvestorID))
    .add_property("instrument_id", tostr(&CThostFtdcOrderField::InstrumentID))
    .add_property("order_ref", tostr(&CThostFtdcOrderField::OrderRef))
    .add_property("price_type", tostr_OrderPriceType<CThostFtdcOrderField>)
    .add_property("direction", toenum_Direction<CThostFtdcOrderField>)
    .add_property("combine_offset_flag", tolist_CombOffsetFlag<CThostFtdcOrderField>)
    .add_property("combine_hedge_flag", tolist_CombHedgeFlag<CThostFtdcOrderField>)
    .def_readonly("limit_price", &CThostFtdcOrderField::LimitPrice)
    .def_readonly("volume_total_original", &CThostFtdcOrderField::VolumeTotalOriginal)
    .add_property("time_condition", tostr_TimeCondition<CThostFtdcOrderField>)
    .add_property("GTD_date", tostr(&CThostFtdcOrderField::GTDDate))
    .add_property("volume_condition", tostr_VolumeCondition<CThostFtdcOrderField>)
    .add_property("contingent_condition", tostr_ContingentCondition<CThostFtdcOrderField>)
    .add_property("force_close_reason", tostr_ForceCloseReason<CThostFtdcOrderField>)
    .def_readonly("is_auto_suspend", &CThostFtdcOrderField::IsAutoSuspend)
    // .def_readonly("business_unit", &CThostFtdcOrderField::BusinessUnit)
    .def_readonly("request_id", &CThostFtdcOrderField::RequestID)
    .add_property("order_local_id", tostr(&CThostFtdcOrderField::OrderLocalID))
    .add_property("exchange_id", tostr(&CThostFtdcOrderField::ExchangeID))
    // .def_readonly("participant_id", &CThostFtdcOrderField::ParticipantID)
    // .def_readonly("client_id", &CThostFtdcOrderField::ClientID)
    // .def_readonly("exchange_inst_id", &CThostFtdcOrderField::ExchangeInstID)
    .add_property("trader_id", tostr(&CThostFtdcOrderField::TraderID))
    // .def_readonly("install_id", &CThostFtdcOrderField::InstallID)
    .add_property("status", tostr_OrderStatus<CThostFtdcOrderField>)
    .add_property("submit_status", tostr_OrderSubmitStatus<CThostFtdcOrderField>)
    .def_readonly("notify_sequence", &CThostFtdcOrderField::NotifySequence)
    .add_property("trading_day", tostr(&CThostFtdcOrderField::TradingDay))
    .def_readonly("settlement_id", &CThostFtdcOrderField::SettlementID)
    .add_property("order_sys_id", tostr(&CThostFtdcOrderField::OrderSysID))
    .add_property("source", tostr_OrderSource<CThostFtdcOrderField>)
    .add_property("type", tostr_OrderType<CThostFtdcOrderField>)
    .def_readonly("volume_traded", &CThostFtdcOrderField::VolumeTraded)
    .def_readonly("volume_total", &CThostFtdcOrderField::VolumeTotal)
    .add_property("insert_date", tostr(&CThostFtdcOrderField::InsertDate))
    .add_property("insert_time", tostr(&CThostFtdcOrderField::InsertTime))
    .add_property("active_time", tostr(&CThostFtdcOrderField::ActiveTime))
    .add_property("suspend_time", tostr(&CThostFtdcOrderField::SuspendTime))
    .add_property("cancel_time", tostr(&CThostFtdcOrderField::CancelTime))
    // .add_property("active_trader_id", &CThostFtdcOrderField::ActiveTraderID)
    // .def_readonly("clearing_part_id", &CThostFtdcOrderField::ClearingPartID)
    .def_readonly("sequence_no", &CThostFtdcOrderField::SequenceNo)
    .def_readonly("front_id", &CThostFtdcOrderField::FrontID)
    .def_readonly("session_id", &CThostFtdcOrderField::SessionID)
    .add_property("user_product_info", tostr(&CThostFtdcOrderField::UserProductInfo))
    .add_property("status_msg", tobytes(&CThostFtdcOrderField::StatusMsg))
    .def_readonly("user_force_close", &CThostFtdcOrderField::UserForceClose)
    // .def_readonly("active_user_id", &CThostFtdcOrderField::ActiveUserID)
    .add_property("broker_order_seq", tostr(&CThostFtdcOrderField::BrokerOrderSeq))
    .add_property("relative_order_sys_id", tostr(&CThostFtdcOrderField::RelativeOrderSysID))
    .def_readonly("ZCE_total_traded_volume", &CThostFtdcOrderField::ZCETotalTradedVolume)
    .def_readonly("is_swap_order", &CThostFtdcOrderField::IsSwapOrder)
    .add_property("branch_id", tostr(&CThostFtdcOrderField::BranchID))
    // .def_readonly("invest_unit_id", &CThostFtdcOrderField::InvestUnitID)
    .add_property("account_id", tostr(&CThostFtdcOrderField::AccountID))
    .add_property("currency_id", tostr(&CThostFtdcOrderField::CurrencyID))
    // .def_readonly("ip_address", &CThostFtdcOrderField::IPAddress)
    // .def_readonly("mac_address", &CThostFtdcOrderField::MacAddress)
    ;

  class_<CThostFtdcTradeField>("Trade")
    .add_property("broker_id", tostr(&CThostFtdcTradeField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcTradeField::InvestorID))
    .add_property("instrument_id", tostr(&CThostFtdcTradeField::InstrumentID))
    .add_property("order_ref", tostr(&CThostFtdcTradeField::OrderRef))
    .add_property("user_id", tostr(&CThostFtdcTradeField::UserID))
    .add_property("exchange_id", tostr(&CThostFtdcTradeField::ExchangeID))
    .add_property("trade_id", tostr(&CThostFtdcTradeField::TradeID))
    .add_property("direction", toenum_Direction<CThostFtdcTradeField>)
    .add_property("order_sys_id", tostr(&CThostFtdcTradeField::OrderSysID))
    // .def_readonly("participant_id", tostr(&CThostFtdcTradeField::ParticipantID))
    // .def_readonly("client_id", tostr(&CThostFtdcTradeField::ClientID))
    .add_property("trading_role", tostr_TradingRole<CThostFtdcTradeField>)
    // .add_property("exchange_inst_id", tostr(&CThostFtdcTradeField::ExchangeInstID))
    .add_property("offset_flag", tostr_OffsetFlag<CThostFtdcTradeField>)
    .add_property("hedge_flag", tostr_HedgeFlag<CThostFtdcTradeField>)
    .def_readonly("price", &CThostFtdcTradeField::Price)
    .def_readonly("volume", &CThostFtdcTradeField::Volume)
    .add_property("trade_date", tostr(&CThostFtdcTradeField::TradeDate))
    .add_property("trade_time", tostr(&CThostFtdcTradeField::TradeTime))
    .add_property("trade_type", tostr_TradeType<CThostFtdcTradeField>)
    .add_property("price_source", tostr_PriceSource<CThostFtdcTradeField>)
    .add_property("trader_id", tostr(&CThostFtdcTradeField::TraderID))
    .add_property("order_local_id", tostr(&CThostFtdcTradeField::OrderLocalID))
    // .add_property("clearing_part_id", &CThostFtdcTradeField::ClearingPartID)
    // .add_property("business_unit", &CThostFtdcTradeField::BusinessUnit)
    .def_readonly("sequence_no", &CThostFtdcTradeField::SequenceNo)
    .add_property("trading_day", tostr(&CThostFtdcTradeField::TradingDay))
    .def_readonly("settlement_id", &CThostFtdcTradeField::SettlementID)
    .def_readonly("broker_order_seq", &CThostFtdcTradeField::BrokerOrderSeq)
    .add_property("trade_source", tostr_TradeSource<CThostFtdcTradeField>)
    // .add_property("invest_unit_id", &CThostFtdcTradeField::InvestUnitID)
    ;

  class_<CThostFtdcOrderActionField>("OrderAction")
    .add_property("broker_id", tostr(&CThostFtdcOrderActionField::BrokerID))
    .add_property("investor_id", tostr(&CThostFtdcOrderActionField::InvestorID))
    .add_property("order_action_ref", tostr(&CThostFtdcOrderActionField::OrderActionRef))
    .add_property("order_ref", tostr(&CThostFtdcOrderActionField::OrderRef))
    .def_readonly("front_id", &CThostFtdcOrderActionField::FrontID)
    .def_readonly("session_id", &CThostFtdcOrderActionField::SessionID)
    .add_property("exchange_id", tostr(&CThostFtdcOrderActionField::ExchangeID))
    .add_property("order_sys_id", tostr(&CThostFtdcOrderActionField::OrderSysID))
    .add_property("action_flag", tostr_ActionFlag<CThostFtdcOrderActionField>)
    .def_readonly("limit_price", &CThostFtdcOrderActionField::LimitPrice)
    .def_readonly("volume_change", &CThostFtdcOrderActionField::VolumeChange)
    .add_property("action_date", tostr(&CThostFtdcOrderActionField::ActionDate))
    .add_property("action_time", tostr(&CThostFtdcOrderActionField::ActionTime))
    .add_property("trader_id", tostr(&CThostFtdcOrderActionField::TraderID))
    // .add_property("install_id", &CThostFtdcOrderActionField::InstallID)
    .add_property("order_local_id", tostr(&CThostFtdcOrderActionField::OrderLocalID))
    .add_property("action_local_id", tostr(&CThostFtdcOrderActionField::ActionLocalID))
    // .add_property("participant_id", &CThostFtdcOrderActionField::ParticipantID)
    // .add_property("client_id", &CThostFtdcOrderActionField::ClientID)
    // .add_property("business_unit", &CThostFtdcOrderActionField::BusinessUnit)
    .add_property("order_action_status", tostr_OrderActionStatus<CThostFtdcOrderActionField>)
    .add_property("user_id", tostr(&CThostFtdcOrderActionField::UserID))
    .add_property("status_msg", tobytes(&CThostFtdcOrderActionField::StatusMsg))
    .add_property("instrument_id", tostr(&CThostFtdcOrderActionField::InstrumentID))
    .add_property("branch_id", tostr(&CThostFtdcOrderActionField::BranchID))
    // .add_property("invest_unit_id", &CThostFtdcOrderActionField::InvestUnitID)
    // .add_property("ip_address", &CThostFtdcOrderActionField::IPAddress)
    // .add_property("mac_address", &CThostFtdcOrderActionField::MacAddress)
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
    .def("init", &CtpClient::Init)
    .def("join", &CtpClient::Join)
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
    .def("query_market_data", &CtpClient::QueryMarketData, arg("instrument_id"))
    .def("query_settlement_info", &CtpClient::QuerySettlementInfo)
    .def("insert_order", &CtpClient::InsertOrder,
      (arg("instrument_id"), arg("direction"), arg("offset_flag"), arg("limit_price"),
       arg("volume"), arg("order_price_type")=OPT_LimitPrice,
       arg("hedge_flag")=HF_Speculation, arg("time_condition")=TC_GFD,
       arg("volume_condition")=VC_AV, arg("contingent_condition")=CC_Immediately,
       arg("min_volume")=1, arg("stop_price")=0.0,
       arg("is_auto_suspend")=false, arg("user_force_close")=false,
       arg("request_id")=0))
    .def("order_action", &CtpClient::InsertOrder,
      (arg("order"), arg("action_flag"), arg("limit_price")=0.0,
       arg("volume_change")=0, arg("request_id")=0))
    .def("delete_order", &CtpClient::DeleteOrder,
      (arg("order"), arg("request_id")=0))
    .def("modify_order", &CtpClient::ModifyOrder,
      (arg("order"), arg("limit_price")=0.0, arg("volume_change")=0, arg("request_id")=0))			
    .def("on_td_front_connected", pure_virtual(&CtpClient::OnTdFrontConnected))
    .def("on_td_user_login", pure_virtual(&CtpClient::OnTdUserLogin))
    .def("on_td_user_logout", pure_virtual(&CtpClient::OnTdUserLogout))
    .def("on_settlement_info_confirm", pure_virtual(&CtpClient::OnRspSettlementInfoConfirm))
    .def("on_err_order_insert", pure_virtual(&CtpClient::OnErrOrderInsert))
    .def("on_err_order_action", pure_virtual(&CtpClient::OnErrOrderAction))
    .def("on_rtn_order", pure_virtual(&CtpClient::OnRtnOrder))
    .def("on_rtn_trade", pure_virtual(&CtpClient::OnRtnTrade))
    .def("on_rsp_order", pure_virtual(&CtpClient::OnRspQryOrder))
    .def("on_rsp_trade", pure_virtual(&CtpClient::OnRspQryTrade))
    .def("on_rsp_trading_account", pure_virtual(&CtpClient::OnRspQryTradingAccount))
    .def("on_rsp_investor_position", pure_virtual(&CtpClient::OnRspQryInvestorPosition))
    .def("on_rsp_investor_position_detail", pure_virtual(&CtpClient::OnRspQryInvestorPositionDetail))
    .def("on_rsp_market_data", pure_virtual(&CtpClient::OnRspQryDepthMarketData))
    .def("on_rsp_settlement_info", pure_virtual(&CtpClient::OnRspQrySettlementInfo))
    .def("on_idle", pure_virtual(&CtpClient::OnIdle))
    ;

};
