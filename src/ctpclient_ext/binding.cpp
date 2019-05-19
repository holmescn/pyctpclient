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
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include "ctpclient.h"
#include "mdspi.h"

using namespace pybind11::literals;
namespace py = pybind11;

#pragma region Getters

const char* InvestorPosition_PositionDate(CThostFtdcInvestorPositionField const *this_)
{
  switch(this_->PositionDate) {
    case THOST_FTDC_PSD_Today: return "today";
    case THOST_FTDC_PSD_History: return "history";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_PositionDirection(T const *this_)
{
  switch(this_->PosiDirection) {
    case THOST_FTDC_PD_Net:   return "net";
    case THOST_FTDC_PD_Long:  return "long";
    case THOST_FTDC_PD_Short: return "short";
    default: return "unknown";
  }
}

template<class T>
Direction toenum_Direction(T const *this_)
{
  switch(this_->Direction) {
    case THOST_FTDC_D_Buy:  return D_Buy;
    case THOST_FTDC_D_Sell: return D_Sell;
    default: throw std::invalid_argument("Invalid direction.");
  }
}

template<class T>
const char* tostr_TradeType(T const *this_)
{
  switch(this_->TradeType) {
    case THOST_FTDC_TRDT_SplitCombination: return "split_combination";
    case THOST_FTDC_TRDT_Common: return "common";
    case THOST_FTDC_TRDT_OptionsExecution: return "options_execution";
    case THOST_FTDC_TRDT_OTC: return "OTC";
    case THOST_FTDC_TRDT_EFPDerived: return "EFP_derived";
    case THOST_FTDC_TRDT_CombinationDerived: return "combination_derived";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_OrderPriceType(T const *this_)
{
  switch(this_->OrderPriceType) {
    case THOST_FTDC_OPT_AnyPrice: 							 return "any_price";
    case THOST_FTDC_OPT_LimitPrice: 						 return "limit_price";
    case THOST_FTDC_OPT_BestPrice: 					 		 return "best_price";
    case THOST_FTDC_OPT_LastPrice:               return "last_price";
    case THOST_FTDC_OPT_LastPricePlusOneTicks:   return "last_price_plus_one_tick";
    case THOST_FTDC_OPT_LastPricePlusTwoTicks:   return "last_price_plus_two_ticks";
    case THOST_FTDC_OPT_LastPricePlusThreeTicks: return "last_price_plus_three_ticks";
    case THOST_FTDC_OPT_AskPrice1: 					     return "Ask_Price1";
    case THOST_FTDC_OPT_AskPrice1PlusOneTicks:   return "ask_price1_plus_one_tick";
    case THOST_FTDC_OPT_AskPrice1PlusTwoTicks:   return "ask_price1_plus_two_ticks";
    case THOST_FTDC_OPT_AskPrice1PlusThreeTicks: return "ask_price1_plus_three_ticks";
    case THOST_FTDC_OPT_BidPrice1: 			  	     return "bid_price1";
    case THOST_FTDC_OPT_BidPrice1PlusOneTicks:   return "bid_price1_plus_one_tick";
    case THOST_FTDC_OPT_BidPrice1PlusTwoTicks:   return "bid_price1_plus_two_ticks";
    case THOST_FTDC_OPT_BidPrice1PlusThreeTicks: return "bid_price1_plus_three_ticks";
    case THOST_FTDC_OPT_FiveLevelPrice:          return "five_level_price";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_OffsetFlag(T const *this_)
{
  switch(this_->OffsetFlag) {
    case THOST_FTDC_OF_Open: return "open";
    case THOST_FTDC_OF_Close: return "close";
    case THOST_FTDC_OF_ForceClose: return "force_close";
    case THOST_FTDC_OF_CloseToday: return "close_today";
    case THOST_FTDC_OF_CloseYesterday: return "close_yesterday";
    case THOST_FTDC_OF_ForceOff: return "force_off";
    case THOST_FTDC_OF_LocalForceClose: return "local_force_close";
    default: return "unknown";
  }
}

template<class T>
std::vector<std::string> tolist_CombOffsetFlag(T const *this_)
{
  std::vector<std::string> v;
  for (size_t i = 0; i < sizeof(TThostFtdcCombOffsetFlagType); ++i) {
    switch(this_->CombOffsetFlag[i]) {
      case 0: break;
      case THOST_FTDC_OF_Open:       v.push_back("open"); break;
      case THOST_FTDC_OF_Close:      v.push_back("close"); break;
      case THOST_FTDC_OF_ForceClose: v.push_back("force_close"); break;
      case THOST_FTDC_OF_CloseToday: v.push_back("close_today"); break;
      case THOST_FTDC_OF_CloseYesterday:  v.push_back("close_yesterday"); break;
      case THOST_FTDC_OF_ForceOff:        v.push_back("force_off"); break;
      case THOST_FTDC_OF_LocalForceClose: v.push_back("local_force_close"); break;
      default: v.push_back("unknown"); break;
    }
  }

  return v;
}

template<class T>
const char* tostr_HedgeFlag(T const *this_)
{
  switch(this_->HedgeFlag) {
    case THOST_FTDC_HF_Speculation: return "speculation";
    case THOST_FTDC_HF_Arbitrage: return "arbitrage";
    case THOST_FTDC_HF_Hedge: return "hedge";
    case THOST_FTDC_HF_MarketMaker: return "market_maker";
    default: return "unknown";
  }
}

template<class T>
std::vector<std::string> tolist_CombHedgeFlag(T const *this_)
{
  std::vector<std::string> v;
  for (size_t i = 0; i < sizeof(TThostFtdcCombHedgeFlagType); ++i) {
    switch(this_->CombHedgeFlag[i]) {
      case 0:
        break;
      case THOST_FTDC_HF_Speculation:
        v.push_back("speculation");
        break;
      case THOST_FTDC_HF_Arbitrage:
        v.push_back("arbitrage");
        break;
      case THOST_FTDC_HF_Hedge:
        v.push_back("hedge");
        break;
      case THOST_FTDC_HF_MarketMaker:
        v.push_back("market_maker");
        break;
      default:
        v.push_back("unknown");
        break;
    }
  }

  return v;
}

template<class T>
const char* tostr_TimeCondition(T const *this_)
{
  switch(this_->TimeCondition) {
    case THOST_FTDC_TC_IOC: return "IOC";
    case THOST_FTDC_TC_GFS: return "GFS";
    case THOST_FTDC_TC_GFD: return "GFD";
    case THOST_FTDC_TC_GTD: return "GTD";
    case THOST_FTDC_TC_GTC: return "GTC";
    case THOST_FTDC_TC_GFA: return "GFA";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_VolumeCondition(T const *this_)
{
  switch(this_->VolumeCondition) {
    case THOST_FTDC_VC_AV: return "any_volume";
    case THOST_FTDC_VC_MV: return "min_volume";
    case THOST_FTDC_VC_CV: return "all";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_ContingentCondition(T const *this_)
{
  switch(this_->ContingentCondition) {
    case THOST_FTDC_CC_Immediately: return "immediately";
    case THOST_FTDC_CC_Touch: return "touch";
    case THOST_FTDC_CC_TouchProfit: return "touch_profit";
    case THOST_FTDC_CC_ParkedOrder: return "parked_order";
    case THOST_FTDC_CC_LastPriceGreaterThanStopPrice: return "last_price_greater_than_stop_price";
    case THOST_FTDC_CC_LastPriceGreaterEqualStopPrice: return "last_price_greater_equal_stop_price";
    case THOST_FTDC_CC_LastPriceLesserThanStopPrice: return "last_price_lesser_than_stop_price";
    case THOST_FTDC_CC_LastPriceLesserEqualStopPrice: return "last_price_lesser_equal_stop_price";
    case THOST_FTDC_CC_AskPriceGreaterThanStopPrice: return "ask_price_greater_than_stop_price";
    case THOST_FTDC_CC_AskPriceGreaterEqualStopPrice: return "ask_price_greater_equal_stop_price";
    case THOST_FTDC_CC_AskPriceLesserThanStopPrice: return "ask_price_lesser_than_stop_price";
    case THOST_FTDC_CC_AskPriceLesserEqualStopPrice: return "ask_price_lesser_equal_stop_price";
    case THOST_FTDC_CC_BidPriceGreaterThanStopPrice: return "bid_price_greater_than_stop_price";
    case THOST_FTDC_CC_BidPriceGreaterEqualStopPrice: return "bid_price_greater_equal_stop_price";
    case THOST_FTDC_CC_BidPriceLesserThanStopPrice: return "bid_price_lesser_than_stop_price";
    case THOST_FTDC_CC_BidPriceLesserEqualStopPrice: return "bid_price_lesser_equal_stop_price";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_ForceCloseReason(T const *this_)
{
  switch(this_->ForceCloseReason) {
    case THOST_FTDC_FCC_NotForceClose: return "not_force_close";
    case THOST_FTDC_FCC_LackDeposit: return "lack_deposit";
    case THOST_FTDC_FCC_ClientOverPositionLimit: return "client_over_position_limit";
    case THOST_FTDC_FCC_MemberOverPositionLimit: return "member_over_position_limit";
    case THOST_FTDC_FCC_NotMultiple: return "not_multiple";
    case THOST_FTDC_FCC_Violation: return "violation";
    case THOST_FTDC_FCC_Other: return "other";
    case THOST_FTDC_FCC_PersonDeliv: return "person_delivery";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_ActionFlag(T const *this_)
{
  switch(this_->ActionFlag) {
    case THOST_FTDC_AF_Delete: return "delete";
    case THOST_FTDC_AF_Modify: return "modify";
    default: return "unknown";
  }
}

template<class T>
OrderSubmitStatus tostr_OrderSubmitStatus(T const *this_)
{
  switch(this_->OrderSubmitStatus) {
    case THOST_FTDC_OSS_InsertSubmitted: return OSS_InsertSubmitted;
    case THOST_FTDC_OSS_CancelSubmitted: return OSS_CancelSubmitted;
    case THOST_FTDC_OSS_Accepted:        return OSS_Accepted;
    case THOST_FTDC_OSS_InsertRejected:  return OSS_InsertRejected;
    case THOST_FTDC_OSS_CancelRejected:  return OSS_CancelRejected;
    case THOST_FTDC_OSS_ModifyRejected:  return OSS_ModifyRejected;
    default: throw std::invalid_argument("unknown order submit status");
  }
}

template<class T>
OrderStatus tostr_OrderStatus(T const *this_)
{
  switch(this_->OrderStatus) {
    case THOST_FTDC_OST_AllTraded:          return OST_AllTraded;
    case THOST_FTDC_OST_PartTradedQueueing: return OST_PartTradedQueueing;
    case THOST_FTDC_OST_PartTradedNotQueueing: return OST_PartTradedNotQueueing;
    case THOST_FTDC_OST_NoTradeQueueing:     return OST_NoTradeQueueing;
    case THOST_FTDC_OST_NoTradeNotQueueing:  return OST_NoTradeNotQueueing;
    case THOST_FTDC_OST_Canceled:    return OST_Canceled;
    case THOST_FTDC_OST_Unknown:     return OST_Unknown;
    case THOST_FTDC_OST_NotTouched:  return OST_NotTouched;
    case THOST_FTDC_OST_Touched:     return OST_Touched;
    default: throw std::invalid_argument("unknown order status");
  }
}

template<class T>
const char* tostr_OrderSource(T const *this_)
{
  switch(this_->OrderSource) {
    case THOST_FTDC_OSRC_Participant: return "participant";
    case THOST_FTDC_OSRC_Administrator: return "administrator";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_OrderType(T const *this_)
{
  switch(this_->OrderType) {
    case THOST_FTDC_ORDT_Normal: return "normal";
    case THOST_FTDC_ORDT_DeriveFromQuote: return "derive_from_quote";
    case THOST_FTDC_ORDT_DeriveFromCombination: return "derive_from_combination";
    case THOST_FTDC_ORDT_Combination: return "combination";
    case THOST_FTDC_ORDT_ConditionalOrder: return "conditional_order";
    case THOST_FTDC_ORDT_Swap: return "swap";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_TradingRole(T const *this_)
{
  switch (this_->TradingRole) {
    case THOST_FTDC_ER_Broker: return "broker";
    case THOST_FTDC_ER_Host: return "host";
    case THOST_FTDC_ER_Maker: return "market_maker";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_PriceSource(T const* this_)
{
  switch(this_->PriceSource) {
    case THOST_FTDC_PSRC_LastPrice: return "last_price";
    case THOST_FTDC_PSRC_Buy: return "buy";
    case THOST_FTDC_PSRC_Sell: return "sell";
    default: return "unknown";
  }
}

template<class T>
const char* tostr_TradeSource(T const* this_)
{
  switch(this_->TradeSource) {
    case THOST_FTDC_TSRC_NORMAL: return "normal";
    case THOST_FTDC_TSRC_QUERY: return "query";
    default: return "unknown";
  }
}

template<class T>
OrderActionStatus tostr_OrderActionStatus(T const* this_)
{
  switch(this_->OrderActionStatus) {
    case THOST_FTDC_OAS_Submitted: return OAS_Submitted;
    case THOST_FTDC_OAS_Accepted: return OAS_Accepted;
    case THOST_FTDC_OAS_Rejected: return OAS_Rejected;
    default: throw std::invalid_argument("unknown order action status");
  }
}

#pragma endregion // Getters

PYBIND11_MODULE(ctpclient, m) {

#pragma region Enums

  py::enum_<Direction>(m, "Direction")
    .value("BUY", D_Buy)
    .value("SELL", D_Sell);

  py::enum_<OffsetFlag>(m, "OffsetFlag")
    .value("OPEN", OF_Open)
    .value("CLOSE", OF_Close)
    .value("FORCE_CLOSE", OF_ForceClose)
    .value("CLOSE_TODAY", OF_CloseToday)
    .value("CLOSE_YESTERDAY", OF_CloseYesterday)
    .value("FORCE_OFF", OF_ForceOff)
    .value("LOCAL_FORCE_CLOSE", OF_LocalForceClose);

  py::enum_<OrderPriceType>(m, "OrderPriceType")
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

  py::enum_<HedgeFlag>(m, "HedgeFlag")
    .value("SPECULATION", HF_Speculation)
    .value("ARBITRAGE", HF_Arbitrage)
    .value("HEDGE", HF_Hedge)
    .value("MARKET_MAKER", HF_MarketMaker);

  py::enum_<TimeCondition>(m, "TimeCondition")
    .value("IOC", TC_IOC)
    .value("GFS", TC_GFS)
    .value("GFD", TC_GFD)
    .value("GTD", TC_GTD)
    .value("GTC", TC_GTC)
    .value("GFA", TC_GFA);
  
  py::enum_<VolumeCondition>(m, "VolumeCondition")
    .value("ANY_VOLUME", VC_AV)
    .value("MIN_VOLUME", VC_MV)
    .value("ALL", VC_CV);

  py::enum_<ContingentCondition>(m, "ContingentCondition")
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

  py::enum_<OrderActionFlag>(m, "OrderActionFlag")
    .value("DELETE", AF_Delete)
    .value("MODIFY", AF_Modify);

  py::enum_<OrderStatus>(m, "OrderStatus")
    .value("ALL_TRADED", OST_AllTraded)
    .value("PART_TRADED_QUEUEING", OST_PartTradedQueueing)
    .value("PART_TRADED_NOT_QUEUEING", OST_PartTradedNotQueueing)
    .value("NO_TRADE_QUEUEING", OST_NoTradeQueueing)
    .value("NO_TRADE_NOT_QUEUEING", OST_NoTradeNotQueueing)
    .value("CANCELED", OST_Canceled)
    .value("UNKNOWN", OST_Unknown)
    .value("NOT_TOUCHED", OST_NotTouched)
    .value("TOUCHED", OST_Touched);

  py::enum_<OrderSubmitStatus>(m, "OrderSubmitStatus")
    .value("INSERT_SUBMITTED", OSS_InsertSubmitted)
    .value("CANCEL_SUBMITTED", OSS_CancelSubmitted)
    .value("MODIFY_SUBMITTED", OSS_ModifySubmitted)
    .value("ACCEPTED", OSS_Accepted)
    .value("INSERT_REJECTED", OSS_InsertRejected)
    .value("CANCEL_REJECTED", OSS_CancelRejected)
    .value("MODIFY_REJECTED", OSS_ModifyRejected);

  py::enum_<OrderActionStatus>(m, "OrderActionStatus")
    .value("SUBMITTED", OAS_Submitted)
    .value("ACCEPTED", OAS_Accepted)
    .value("REJECTED", OAS_Rejected);

#pragma endregion

  py::class_<CThostFtdcRspInfoField>(m, "ResponseInfo")
    .def_readonly("error_id", &CThostFtdcRspInfoField::ErrorID)
    .def_property_readonly("error_msg", [](CThostFtdcRspInfoField const *this_) { return py::bytes(this_->ErrorMsg); })
    ;

  py::class_<CThostFtdcRspUserLoginField>(m, "UserLoginInfo")
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

  py::class_<CThostFtdcUserLogoutField>(m, "UserLogoutInfo")
    .def_readonly("broker_id", &CThostFtdcUserLogoutField::BrokerID)
    .def_readonly("user_id", &CThostFtdcUserLogoutField::UserID)
    ;

  py::class_<CThostFtdcSpecificInstrumentField>(m, "SpecificInstrument")
    .def_readonly("instrument_id", &CThostFtdcSpecificInstrumentField::InstrumentID)
    ;

  py::class_<M1Bar>(m, "M1Bar")
    .def_readonly("instrument_id", &M1Bar::InstrumentID)
    .def_readonly("trading_day", &M1Bar::TradingDay)
    .def_readonly("action_day", &M1Bar::ActionDay)
    .def_readonly("update_time", &M1Bar::UpdateTime)
    .def_readonly("open", &M1Bar::OpenPrice)
    .def_readonly("high", &M1Bar::HighestPrice)
    .def_readonly("low", &M1Bar::LowestPrice)
    .def_readonly("close", &M1Bar::ClosePrice)
    .def_readonly("volume", &M1Bar::Volume)
    .def_readonly("turnover", &M1Bar::Turnover)
    .def_readonly("position", &M1Bar::Position)
    ;

  py::class_<TickBar>(m, "TickBar")
    .def_readonly("instrument_id", &TickBar::InstrumentID)
    .def_readonly("trading_day", &TickBar::TradingDay)
    .def_readonly("action_day", &TickBar::ActionDay)
    .def_readonly("update_time", &TickBar::UpdateTime)
    .def_readonly("price", &TickBar::Price)
    .def_readonly("volume", &TickBar::Volume)
    .def_readonly("turnover", &TickBar::Turnover)
    .def_readonly("position", &TickBar::Position)
    ;

  py::class_<CThostFtdcDepthMarketDataField>(m, "MarketData")
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

  py::class_<CThostFtdcSettlementInfoField>(m, "SettlementInfo")
    .def_readonly("trading_day", &CThostFtdcSettlementInfoField::TradingDay)
    .def_readonly("settlement_id", &CThostFtdcSettlementInfoField::SettlementID)
    .def_readonly("broker_id", &CThostFtdcSettlementInfoField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcSettlementInfoField::InvestorID)
    .def_readonly("sequence_no", &CThostFtdcSettlementInfoField::SequenceNo)
    .def_property_readonly("content", [](CThostFtdcSettlementInfoField const *this_) { return py::bytes(this_->Content); })
    .def_readonly("account_id", &CThostFtdcSettlementInfoField::AccountID)
    .def_readonly("currency_id", &CThostFtdcSettlementInfoField::CurrencyID)
    ;

  py::class_<CThostFtdcSettlementInfoConfirmField>(m, "SettlementInfoConfirm")
    .def_readonly("broker_id", &CThostFtdcSettlementInfoConfirmField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcSettlementInfoConfirmField::InvestorID)
    .def_readonly("confirm_date", &CThostFtdcSettlementInfoConfirmField::ConfirmDate)
    .def_readonly("confirm_time", &CThostFtdcSettlementInfoConfirmField::ConfirmTime)
    .def_readonly("currency_id", &CThostFtdcSettlementInfoConfirmField::CurrencyID)
    ;

  py::class_<CThostFtdcTradingAccountField>(m, "TradingAccount")
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
    .def_readonly("current_margin", &CThostFtdcTradingAccountField::CurrMargin)
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

  py::class_<CThostFtdcInvestorPositionField>(m, "InvestorPosition")
    .def_readonly("instrument_id", &CThostFtdcInvestorPositionField::InstrumentID)
    .def_readonly("broker_id", &CThostFtdcInvestorPositionField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcInvestorPositionField::InvestorID)
    .def_property_readonly("position_direction", tostr_PositionDirection<CThostFtdcInvestorPositionField>)
    .def_property_readonly("hedge_flag", tostr_HedgeFlag<CThostFtdcInvestorPositionField>)
    .def_property_readonly("position_date", InvestorPosition_PositionDate)
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
    .def_readonly("exchange_id", &CThostFtdcInvestorPositionField::ExchangeID)
    .def_readonly("yd_strike_frozen", &CThostFtdcInvestorPositionField::YdStrikeFrozen)
    .def_readonly("invest_unit_id", &CThostFtdcInvestorPositionField::InvestUnitID)
    ;

  py::class_<CThostFtdcInvestorPositionDetailField>(m, "InvestorPositionDetail")
    .def_readonly("instrument_id", &CThostFtdcInvestorPositionDetailField::InstrumentID)
    .def_readonly("broker_id", &CThostFtdcInvestorPositionDetailField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcInvestorPositionDetailField::InvestorID)
    .def_property_readonly("hedge_flag", tostr_HedgeFlag<CThostFtdcInvestorPositionDetailField>)
    .def_property_readonly("direction", toenum_Direction<CThostFtdcInvestorPositionDetailField>)
    .def_readonly("open_date", &CThostFtdcInvestorPositionDetailField::OpenDate)
    .def_readonly("trade_id", &CThostFtdcInvestorPositionDetailField::TradeID)
    .def_readonly("volume", &CThostFtdcInvestorPositionDetailField::Volume)
    .def_readonly("open_price", &CThostFtdcInvestorPositionDetailField::OpenPrice)
    .def_readonly("trading_day", &CThostFtdcInvestorPositionDetailField::TradingDay)
    .def_readonly("settlement_id", &CThostFtdcInvestorPositionDetailField::SettlementID)
    .def_property_readonly("trade_type", tostr_TradeType<CThostFtdcInvestorPositionDetailField>)
    .def_readonly("combine_instrument_id", &CThostFtdcInvestorPositionDetailField::CombInstrumentID)
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

  py::class_<CThostFtdcInputOrderField>(m, "InputOrder")
    .def_readonly("broker_id", &CThostFtdcInputOrderField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcInputOrderField::InvestorID)
    .def_readonly("instrument_id", &CThostFtdcInputOrderField::InstrumentID)
    .def_readonly("order_ref", &CThostFtdcInputOrderField::OrderRef)
    .def_readonly("user_id", &CThostFtdcInputOrderField::UserID)
    .def_property_readonly("order_price_type", tostr_OrderPriceType<CThostFtdcInputOrderField>)
    .def_property_readonly("direction", toenum_Direction<CThostFtdcInputOrderField>)
    .def_property_readonly("combine_offset_flag", tolist_CombOffsetFlag<CThostFtdcInputOrderField>)
    .def_property_readonly("combine_hedge_flag", tolist_CombHedgeFlag<CThostFtdcInputOrderField>)
    .def_readonly("limit_price", &CThostFtdcInputOrderField::LimitPrice)
    .def_readonly("volume_total_original", &CThostFtdcInputOrderField::VolumeTotalOriginal)
    .def_property_readonly("time_condition", tostr_TimeCondition<CThostFtdcInputOrderField>)
    .def_readonly("GTD_date", &CThostFtdcInputOrderField::GTDDate)
    .def_property_readonly("volume_condition", tostr_VolumeCondition<CThostFtdcInputOrderField>)
    .def_readonly("min_volume", &CThostFtdcInputOrderField::MinVolume)
    .def_property_readonly("contingent_condition", tostr_ContingentCondition<CThostFtdcInputOrderField>)
    .def_readonly("stop_price", &CThostFtdcInputOrderField::StopPrice)
    .def_property_readonly("force_close_reason", tostr_ForceCloseReason<CThostFtdcInputOrderField>)
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

  py::class_<CThostFtdcInputOrderActionField>(m, "InputOrderAction")
    .def_readonly("broker_id", &CThostFtdcInputOrderActionField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcInputOrderActionField::InvestorID)
    .def_readonly("order_action_ref", &CThostFtdcInputOrderActionField::OrderActionRef)
    .def_readonly("order_ref", &CThostFtdcInputOrderActionField::OrderActionRef)
    .def_readonly("request_id", &CThostFtdcInputOrderActionField::RequestID)
    .def_readonly("front_id", &CThostFtdcInputOrderActionField::FrontID)
    .def_readonly("session_id", &CThostFtdcInputOrderActionField::SessionID)
    .def_readonly("exchange_id", &CThostFtdcInputOrderActionField::ExchangeID)
    .def_readonly("order_sys_id", &CThostFtdcInputOrderActionField::OrderSysID)
    .def_property_readonly("action_flag", tostr_ActionFlag<CThostFtdcInputOrderActionField>)
    .def_readonly("limit_price", &CThostFtdcInputOrderActionField::LimitPrice)
    .def_readonly("volume_change", &CThostFtdcInputOrderActionField::VolumeChange)
    .def_readonly("user_id", &CThostFtdcInputOrderActionField::UserID)
    .def_readonly("instrument_id", &CThostFtdcInputOrderActionField::InstrumentID)
    .def_readonly("invest_unit_id", &CThostFtdcInputOrderActionField::InvestUnitID)
    .def_readonly("ip_address", &CThostFtdcInputOrderActionField::IPAddress)
    .def_readonly("mac_address", &CThostFtdcInputOrderActionField::MacAddress)
    ;

  py::class_<CThostFtdcOrderField, std::shared_ptr<CThostFtdcOrderField> >(m, "Order")
    .def_readonly("broker_id", &CThostFtdcOrderField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcOrderField::InvestorID)
    .def_readonly("instrument_id", &CThostFtdcOrderField::InstrumentID)
    .def_readonly("order_ref", &CThostFtdcOrderField::OrderRef)
    .def_property_readonly("price_type", tostr_OrderPriceType<CThostFtdcOrderField>)
    .def_property_readonly("direction", toenum_Direction<CThostFtdcOrderField>)
    .def_property_readonly("combine_offset_flag", tolist_CombOffsetFlag<CThostFtdcOrderField>)
    .def_property_readonly("combine_hedge_flag", tolist_CombHedgeFlag<CThostFtdcOrderField>)
    .def_readonly("limit_price", &CThostFtdcOrderField::LimitPrice)
    .def_readonly("volume_total_original", &CThostFtdcOrderField::VolumeTotalOriginal)
    .def_property_readonly("time_condition", tostr_TimeCondition<CThostFtdcOrderField>)
    .def_readonly("GTD_date", &CThostFtdcOrderField::GTDDate)
    .def_property_readonly("volume_condition", tostr_VolumeCondition<CThostFtdcOrderField>)
    .def_property_readonly("contingent_condition", tostr_ContingentCondition<CThostFtdcOrderField>)
    .def_property_readonly("force_close_reason", tostr_ForceCloseReason<CThostFtdcOrderField>)
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
    .def_property_readonly("status", tostr_OrderStatus<CThostFtdcOrderField>)
    .def_property_readonly("submit_status", tostr_OrderSubmitStatus<CThostFtdcOrderField>)
    .def_readonly("notify_sequence", &CThostFtdcOrderField::NotifySequence)
    .def_readonly("trading_day", &CThostFtdcOrderField::TradingDay)
    .def_readonly("settlement_id", &CThostFtdcOrderField::SettlementID)
    .def_readonly("order_sys_id", &CThostFtdcOrderField::OrderSysID)
    .def_property_readonly("source", tostr_OrderSource<CThostFtdcOrderField>)
    .def_property_readonly("type", tostr_OrderType<CThostFtdcOrderField>)
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
    .def_property_readonly("status_msg", [](CThostFtdcOrderField const *this_) { return py::bytes(this_->StatusMsg); })
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

  py::class_<CThostFtdcTradeField>(m, "Trade")
    .def_readonly("broker_id", &CThostFtdcTradeField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcTradeField::InvestorID)
    .def_readonly("instrument_id", &CThostFtdcTradeField::InstrumentID)
    .def_readonly("order_ref", &CThostFtdcTradeField::OrderRef)
    .def_readonly("user_id", &CThostFtdcTradeField::UserID)
    .def_readonly("exchange_id", &CThostFtdcTradeField::ExchangeID)
    .def_readonly("trade_id", &CThostFtdcTradeField::TradeID)
    .def_property_readonly("direction", toenum_Direction<CThostFtdcTradeField>)
    .def_readonly("order_sys_id", &CThostFtdcTradeField::OrderSysID)
    .def_readonly("participant_id", &CThostFtdcTradeField::ParticipantID)
    .def_readonly("client_id", &CThostFtdcTradeField::ClientID)
    .def_property_readonly("trading_role", tostr_TradingRole<CThostFtdcTradeField>)
    .def_readonly("exchange_inst_id", &CThostFtdcTradeField::ExchangeInstID)
    .def_property_readonly("offset_flag", tostr_OffsetFlag<CThostFtdcTradeField>)
    .def_property_readonly("hedge_flag", tostr_HedgeFlag<CThostFtdcTradeField>)
    .def_readonly("price", &CThostFtdcTradeField::Price)
    .def_readonly("volume", &CThostFtdcTradeField::Volume)
    .def_readonly("trade_date", &CThostFtdcTradeField::TradeDate)
    .def_readonly("trade_time", &CThostFtdcTradeField::TradeTime)
    .def_property_readonly("trade_type", tostr_TradeType<CThostFtdcTradeField>)
    .def_property_readonly("price_source", tostr_PriceSource<CThostFtdcTradeField>)
    .def_readonly("trader_id", &CThostFtdcTradeField::TraderID)
    .def_readonly("order_local_id", &CThostFtdcTradeField::OrderLocalID)
    .def_readonly("clearing_part_id", &CThostFtdcTradeField::ClearingPartID)
    .def_readonly("business_unit", &CThostFtdcTradeField::BusinessUnit)
    .def_readonly("sequence_no", &CThostFtdcTradeField::SequenceNo)
    .def_readonly("trading_day", &CThostFtdcTradeField::TradingDay)
    .def_readonly("settlement_id", &CThostFtdcTradeField::SettlementID)
    .def_readonly("broker_order_seq", &CThostFtdcTradeField::BrokerOrderSeq)
    .def_property_readonly("trade_source", tostr_TradeSource<CThostFtdcTradeField>)
    .def_readonly("invest_unit_id", &CThostFtdcTradeField::InvestUnitID)
    ;

  py::class_<CThostFtdcOrderActionField>(m, "OrderAction")
    .def_readonly("broker_id", &CThostFtdcOrderActionField::BrokerID)
    .def_readonly("investor_id", &CThostFtdcOrderActionField::InvestorID)
    .def_readonly("order_action_ref", &CThostFtdcOrderActionField::OrderActionRef)
    .def_readonly("order_ref", &CThostFtdcOrderActionField::OrderRef)
    .def_readonly("front_id", &CThostFtdcOrderActionField::FrontID)
    .def_readonly("session_id", &CThostFtdcOrderActionField::SessionID)
    .def_readonly("exchange_id", &CThostFtdcOrderActionField::ExchangeID)
    .def_readonly("order_sys_id", &CThostFtdcOrderActionField::OrderSysID)
    .def_property_readonly("action_flag", tostr_ActionFlag<CThostFtdcOrderActionField>)
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
    .def_property_readonly("order_action_status", tostr_OrderActionStatus<CThostFtdcOrderActionField>)
    .def_readonly("user_id", &CThostFtdcOrderActionField::UserID)
    .def_property_readonly("status_msg", [](CThostFtdcOrderActionField const *this_) { return py::bytes(this_->StatusMsg); })
    .def_readonly("instrument_id", &CThostFtdcOrderActionField::InstrumentID)
    .def_readonly("branch_id", &CThostFtdcOrderActionField::BranchID)
    .def_readonly("invest_unit_id", &CThostFtdcOrderActionField::InvestUnitID)
    .def_readonly("ip_address", &CThostFtdcOrderActionField::IPAddress)
    .def_readonly("mac_address", &CThostFtdcOrderActionField::MacAddress)
    ;


  py::class_<CtpClient, CtpClientWrap>(m, "CtpClient")
    .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&, const std::string&>(),
         "md_address"_a, "td_address"_a, "broker_id"_a, "user_id"_a, "password"_a)
    .def_property_readonly_static("__version__", &CtpClient::GetApiVersion)
    .def_property("flow_path", &CtpClient::GetFlowPath, &CtpClient::SetFlowPath)
    .def_property("md_address", &CtpClient::GetMdAddr, &CtpClient::SetMdAddr)
    .def_property("td_address", &CtpClient::GetTdAddr, &CtpClient::SetTdAddr)
    .def_property("broker_id", &CtpClient::GetBrokerId, &CtpClient::SetBrokerId)
    .def_property("user_id", &CtpClient::GetUserId, &CtpClient::SetUserId)
    .def_property("password", &CtpClient::GetPassword, &CtpClient::SetPassword)
    .def_property("instrument_ids", &CtpClient::GetInstrumentIds, &CtpClient::SetInstrumentIds)
    .def("init", &CtpClient::Init)
    .def("join", &CtpClient::Join)
    .def("exit", &CtpClient::Exit)

    .def("md_login", &CtpClient::MdLogin)
    .def("subscribe_market_data", &CtpClient::SubscribeMarketData)
    .def("unsubscribe_market_data", &CtpClient::UnsubscribeMarketData)
    .def("on_md_front_connected", &CtpClient::OnMdFrontConnected)
    .def("on_md_front_disconnected", &CtpClient::OnMdFrontDisconnected)
    .def("on_md_user_login", &CtpClient::OnMdUserLogin)
    .def("on_md_user_logout", &CtpClient::OnMdUserLogout)
    .def("on_subscribe_market_data", &CtpClient::OnSubscribeMarketData)
    .def("on_unsubscribe_market_data", &CtpClient::OnUnsubscribeMarketData)
    .def("on_rtn_market_data", &CtpClient::OnRtnMarketData)
    .def("on_tick", &CtpClient::OnTick)
    .def("on_1min", &CtpClient::On1Min)
    .def("on_1min_tick", &CtpClient::On1MinTick)

    .def("td_login", &CtpClient::TdLogin)
    .def("confirm_settlement_info", &CtpClient::ConfirmSettlementInfo)
    .def("query_order", &CtpClient::QueryOrder)
    .def("query_trade", &CtpClient::QueryTrade)
    .def("query_trading_account", &CtpClient::QueryTradingAccount)
    .def("query_investor_position", &CtpClient::QueryInvestorPosition)
    .def("query_investor_position_detail", &CtpClient::QueryInvestorPositionDetail)
    .def("query_market_data", &CtpClient::QueryMarketData, "instrument_id"_a, "request_id"_a=0)
    .def("insert_order", &CtpClient::InsertOrder)
    .def("order_action", &CtpClient::OrderAction)
    .def("delete_order", &CtpClient::DeleteOrder)
    .def("on_td_front_connected", &CtpClient::OnTdFrontConnected)
    .def("on_td_user_login", &CtpClient::OnTdUserLogin)
    .def("on_td_user_logout", &CtpClient::OnTdUserLogout)
    .def("on_settlement_info_confirm", &CtpClient::OnRspSettlementInfoConfirm)
    .def("on_err_order_insert", &CtpClient::OnErrOrderInsert)
    .def("on_err_order_action", &CtpClient::OnErrOrderAction)
    .def("on_rtn_order", &CtpClient::OnRtnOrder)
    .def("on_rtn_trade", &CtpClient::OnRtnTrade)
    .def("on_rsp_order", &CtpClient::OnRspQryOrder)
    .def("on_rsp_trade", &CtpClient::OnRspQryTrade)
    .def("on_rsp_trading_account", &CtpClient::OnRspQryTradingAccount)
    .def("on_rsp_investor_position", &CtpClient::OnRspQryInvestorPosition)
    .def("on_rsp_investor_position_detail", &CtpClient::OnRspQryInvestorPositionDetail)
    .def("on_rsp_market_data", &CtpClient::OnRspQryDepthMarketData)
    .def("on_idle", &CtpClient::OnIdle)
    ;
};
