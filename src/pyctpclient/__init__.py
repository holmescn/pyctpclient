# -*- coding: utf-8 -*-
# Copyright 2019 Holmes Conan
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os
import logging
from tempfile import mkdtemp
from shutil import rmtree
from .ctpclient import CtpClient as _CtpClient

# Data Structs
from .ctpclient import (
    ResponseInfo, UserLoginInfo, UserLogoutInfo,
    MarketData, TickBar, M1Bar,
    SpecificInstrument,
    SettlementInfo, SettlementInfoConfirm,
    TradingAccount, InvestorPosition, InvestorPositionDetail,
    InputOrder, InputOrderAction, Order, Trade, OrderAction
)

# Enums
from .ctpclient import (
    D_BUY,
    D_SELL,

    OF_OPEN,
    OF_CLOSE,
    OF_FORCE_CLOSE,
    OF_CLOSE_TODAY,
    OF_CLOSE_YESTERDAY,
    OF_FORCE_OFF,
    OF_LOCAL_FORCE_CLOSE,

    OST_ALL_TRADED,
    OST_PART_TRADED_QUEUEING,
    OST_PART_TRADED_NOT_QUEUEING,
    OST_NO_TRADE_QUEUEING,
    OST_NO_TRADE_NOT_QUEUEING,
    OST_CANCELED,
    OST_UNKNOWN,
    OST_NOT_TOUCHED,
    OST_TOUCHED,

    OSS_INSERT_SUBMITTED,
    OSS_CANCEL_SUBMITTED,
    OSS_MODIFY_SUBMITTED,
    OSS_ACCEPTED,
    OSS_INSERT_REJECTED,
    OSS_CANCEL_REJECTED,
    OSS_MODIFY_REJECTED,

    OAS_SUBMITTED,
    OAS_ACCEPTED,
    OAS_REJECTED
)
from .ctpclient import Direction, OffsetFlag

__version__ = "0.3.0a0"
__author__ = "Holmes Conan"

class CtpClient(_CtpClient):

    def __init__(self, md_address, td_address, broker_id, user_id, password):
        _CtpClient.__init__(self, md_address, td_address, broker_id, user_id, password)
        self.log = logging.getLogger('CTP')
        self.log.setLevel(logging.INFO)
        # create console handler with a higher log level
        ch = logging.StreamHandler()
        ch.setLevel(logging.INFO)
        # create formatter and add it to the handlers
        formatter = logging.Formatter('[%(asctime)s] [%(name)s] [%(levelname)s] %(message)s')
        ch.setFormatter(formatter)
        # add the handlers to the logger
        self.log.addHandler(ch)

    def init(self):
        if self.flow_path == "":
            self.flow_path = mkdtemp(prefix="ctp-")

        if not os.path.exists(self.flow_path):
            os.makedirs(self.flow_path)

        _CtpClient.init(self)

    def remove_flow_path(self):
        rmtree(self.flow_path)

    def on_md_front_connected(self):
        self.log.info("MarketData front connected")
        self.md_login()

    def on_md_front_disconnected(self, reason: int):
        self.log.info("MarketData front disconnected: %d" % reason)

    def on_md_user_login(self, user_login_info: UserLoginInfo, rsp_info: ResponseInfo):
        if rsp_info.error_id == 0 and len(self.instrument_ids) > 0:
            self.log.info("MarketData user logged in")
            self.subscribe_market_data(self.instrument_ids)
        else:
            self.log.error("MarketData login failed: %d" % rsp_info.error_id)

    def on_md_user_logout(self, user_logout, rsp_info):
        self.log.info("MarketData logged out.")

    def on_subscribe_market_data(self, info, rsp_info, is_last):
        if rsp_info.error_id == 0:
            self.log.info("Market data of %s is subscribed" % info.instrument_id)

    def on_unsubscribe_market_data(self, info, rsp_info, is_last):
        if rsp_info.error_id == 0:
            self.log.info("Market data of %s market data is unsubscribed" % info.instrument_id)

    def on_rtn_market_data(self, data: MarketData):
        pass

    def on_tick(self, data: TickBar):
        pass

    def on_1min(self, data: M1Bar):
        pass

    def on_1min_tick(self, data: M1Bar):
        pass

    def on_td_front_connected(self):
        self.log.info("Trader front connected")
        self.td_login()

    def on_td_front_disconnected(self, reason):
        self.log.info("Trader front disconnected")

    def on_td_user_login(self, user_login_info, rsp_info):
        if rsp_info.error_id == 0:
            self.log.info("Trader user logged in.")
            self.confirm_settlement_info()
        else:
            self.log.info("Trader user login failed.")

    def on_td_user_logout(self, user_logout, rsp_info):
        if rsp_info.error_id == 0:
            self.log.info("Trader user logged out.")
        else:
            self.log.info("Trader user logout failed.")

    def on_settlement_info_confirm(self, confirm, rsp_info):
        if rsp_info.error_id == 0:
            self.log.info("Trader settlement info confirmed.")
        else:
            self.log.info("Trader settlement info confirm failed %d." % rsp_info.error_id)

    def on_rtn_order(self, order):
        pass

    def on_rtn_trade(self, trade):
        pass

    def on_err_order_insert(self, input_order, rsp_info):
        pass

    def on_err_order_action(self, input_order_action, order_action, rsp_info):
        pass

    def on_rsp_order(self, order, rsp_info, is_last):
        pass

    def on_rsp_trade(self, trade, rsp_info, is_last):
        pass

    def on_rsp_trading_account(self, trading_account, rsp_info, is_last):
        pass

    def on_rsp_investor_position(self, investor_position, rsp_info, is_last):
        pass

    def on_rsp_investor_position_detail(self, investor_position_detail, rsp_info, is_last):
        pass

    def on_rsp_market_data(self, data, rsp_info, request_id, is_last):
        pass

    def on_idle(self):
        pass

    def insert_order(self, instrument_id: str, direction: Direction, offset_flag: OffsetFlag, price: float, volume: int, **kwargs):
        _CtpClient.insert_order(self, instrument_id, direction, offset_flag, price, volume, **kwargs)
