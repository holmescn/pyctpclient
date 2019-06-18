# -*- coding: utf-8 -*-
from pyctpclient import CtpClient
from pyctpclient import (
    D_BUY, D_SELL, OF_OPEN, OF_CLOSE, OF_CLOSE_TODAY, OF_CLOSE_YESTERDAY,

    OST_ALL_TRADED,
    OST_PART_TRADED_QUEUEING,
    OST_PART_TRADED_NOT_QUEUEING,
    OST_NO_TRADE_QUEUEING,
    OST_NO_TRADE_NOT_QUEUEING,
    OST_CANCELED,
    OST_UNKNOWN,

    OSS_INSERT_SUBMITTED,
    OSS_CANCEL_SUBMITTED,
    OSS_ACCEPTED,
    OSS_INSERT_REJECTED,
    OSS_CANCEL_REJECTED
)
from datetime import datetime


class Client(CtpClient):
    available = 0.0
    move = 0.4
    m1 = []
    position = dict()
    direction = None
    order = None
    trading = False

    def init(self):
        """初始化 CTP API/SPI，切记不要忘了调用 `super` 的 `init`。
        """
        super(Client, self).init()
        # 完成自己的初始化工作

    def on_md_front_disconnected(self, reason):
        """MarketData 前置断开连接时会调用此函数。断开后，系统会尝试重新连接。
        """
        self.exit()

    def on_tick(self, data):
        """MarketData 数据回传函数，这个是简化后的回传数据，去除了不常用的一些数据。

        :type data: pyctpclient.ctpclient.TickBar
        """
        volume = 1
        if self.direction is not None and self.direction == D_BUY and not self.trading:
            price = data.price + self.move
            key = (data.instrument_id, 'short')
            if key in self.position and self.position[key] > 0:
                self.insert_order(data.instrument_id, self.direction, OF_CLOSE_YESTERDAY, price, volume)
                self.log.info("insert order buy close_yesterday %.2f", price)
            else:
                self.insert_order(data.instrument_id, self.direction, OF_OPEN, price, volume)
                self.log.info("insert order buy open %.2f", price)
            self.trading = True

        if self.direction is not None and self.direction == D_SELL and not self.trading:
            price = data.price - self.move
            key = (data.instrument_id, 'long')
            if key in self.position and self.position[key] > 0:
                self.insert_order(data.instrument_id, self.direction, OF_CLOSE_YESTERDAY, price, volume)
                self.log.info("insert order sell close_yesterday %.2f", price)
            else:
                self.insert_order(data.instrument_id, self.direction, OF_OPEN, price, volume)
                self.log.info("insert order sell open %.2f", price)
            self.trading = True

    def on_1min(self, data):
        """MarketData 数据回传函数，由 tick 数据合成的 1 分钟数据，在下一个一分钟开始的时候，传回上一个
        一分钟的数据。

        :type data: pyctpclient.ctpclient.M1Bar
        """
        self.m1.append(data)

    def on_1min_tick(self, data):
        if data.update_time.endswith('00'):
            if self.direction is None:
                self.direction = D_BUY
            else:
                self.direction = D_SELL

    def on_td_front_disconnected(self, reason):
        """Trader 前置断开连接时会调用此函数。断开后，系统会尝试重新连接。reason 和 MarketData 的相同。
        """
        self.exit()

    def on_settlement_info_confirm(self, confirm, rsp_info):
        """Trader 每次登录后回确认上一个交易日的交割单，确认完成后回调用此函数。通常用来执行开机的初始化查询。

        :type confirm: pyctpclient.ctpclient.SettlementInfoConfirm
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        # 查询当前的账户资金情况
        self.query_trading_account()
        # 查询当前的账户持仓情况
        self.query_investor_position()
        # 查询本交易日内所有的报单记录
        self.query_order()

    def on_rsp_trading_account(self, trading_account, rsp_info, is_last):
        """`query_trading_account` 的数据回传函数。请不要保存 `trading_account`，只能保存其中的数据。

        :type trading_account: pyctpclient.ctpclient.TradingAccount
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        # 可用资金
        self.available = trading_account.available

    def on_rsp_investor_position(self, investor_position, rsp_info, is_last):
        """`query_investor_position` 的数据回传函数。请不要保存 `investor_position`，只能保存其中的数据。

        :type investor_position: pyctpclient.ctpclient.InvestorPosition
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if investor_position is not None:
            # 昨持仓 = 现持仓 - 今日持仓
            yd_position = investor_position.position - investor_position.today_position
            key = (investor_position.instrument_id, investor_position.position_direction)
            self.position[key] = yd_position
        elif rsp_info is not None:
            self.log.error("query investor position failed: %d", rsp_info.error_id)
        else:
            self.log.info("No investor position yet.")

    def on_rsp_order(self, order, rsp_info, is_last):
        """`query_order` 的数据回传函数。

        :type order: pyctpclient.ctpclient.Order
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if order is not None and order.status not in (OST_ALL_TRADED, OST_CANCELED):
            self.delete_order(order)
        elif rsp_info is not None:
            self.log.error("query order failed: %d", rsp_info.error_id)
        else:
            self.log.info("No order yet.")

    def on_rtn_order(self, order):
        """报单状态回传函数。当报单状态 `order.status` 或者 `order.submit_status` 变化时，

        :type order: pyctpclient.ctpclient.Order
        """
        self.log.info("%s %s", order.submit_status.name, order.status.name)
        if order.status == OST_ALL_TRADED or order.status == OST_CANCELED:
            self.trading = False

    def on_rtn_trade(self, trade):
        """报单成交回传函数。

        :type trade: pyctpclient.ctpclient.Trade
        """
        self.query_investor_position()

    def on_err_order_insert(self, input_order, rsp_info):
        """报单失败回传函数。这里主要是通过检查 `rsp_info.error_id` 的值来确定错误原因。

        :type input_order: pyctpclient.ctpclient.InputOrder
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        self.log.error("Insert Order failed: %d %s", rsp_info.error_id, rsp_info.error_msg)
        self.trading = False

    def on_err_order_action(self, input_order_action, order_action, rsp_info):
        """撤单/改单失败回传函数。这里主要是通过检查 `rsp_info.error_id` 的值来确定错误原因。

        :type input_order_action: pyctpclient.ctpclient.InputOrderAction
        :type order_action: pyctpclient.ctpclient.OrderAction
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        pass

    def on_idle(self):
        """空闲回传函数。当数据队列中没有数据需要处理，并且延迟大于 `idle_delay` 时调用
        """
        if self.order is not None:
            dt = datetime.strptime('%s %s' % (self.order.insert_date, self.order.insert_time), "%Y-%m-%d %H:%M:%S")
            diff = datetime.now() - dt
            if diff.second > 10:
                self.delete_order(order)


if __name__ == "__main__":
    # 创建 CTP Client 实例
    c = Client(
        # 实盘/电信1
        #md_address="tcp://180.168.146.187:10010",
        #td_address="tcp://180.168.146.187:10000",
        # 实盘/电信2
        #md_address="tcp://180.168.146.187:10011",
        #td_address="tcp://180.168.146.187:10001",
        # 实盘/移动
        #md_address="tcp://218.202.237.33:10012",
        #td_address="tcp://218.202.237.33:10002",
        # 开发
        md_address="tcp://180.168.146.187:10031",
        td_address="tcp://180.168.146.187:10030",
        broker_id="9999",
        user_id="",
        password="",
        # 看穿式管理
        app_id='',
        auth_code='',
    )
    # 订阅要交易的品种, 请在初始化之前指定
    c.instrument_ids = ['IF1906']
    # 设置 on_idle 的最小间隔（毫秒），默认为 1 秒
    c.idle_delay = 1000
    # 初始化 CTP
    c.init()
    # 进入消息循环（必须执行）
    c.join()
    # 善后工作
    c.remove_flow_path()
