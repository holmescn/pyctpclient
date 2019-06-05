# -*- coding: utf-8 -*-
from pyctpclient import CtpClient
from pyctpclient import RequestError
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


class Client(CtpClient):
    tick = None
    m1 = None
    m1_tick = None
    counter = 0

    def init(self):
        """初始化 CTP API/SPI，切记不要忘了调用 `super` 的 `init`。
        """
        super(Client, self).init()
        # 完成自己的初始化工作

    def on_md_front_disconnected(self, reason):
        """MarketData 前置断开连接时会调用此函数。断开后，系统会尝试重新连接。

        :param nReason:
	        0x1001 网络读失败(4097)
	        0x1002 网络写失败(4098)
	        0x2001 接收心跳超时(8193)
	        0x2002 发送心跳失败(8194)
	        0x2003 收到错误报文(8195)
        """
        pass

    def on_rtn_market_data(self, data):
        """MarketData 数据回传函数，包含完全的原始数据信息，

        :type data: pyctpclient.ctpclient.MarketData
        """
        pass

    def on_tick(self, data):
        """MarketData 数据回传函数，这个是简化后的回传数据，去除了不常用的一些数据。

        :type data: pyctpclient.ctpclient.TickBar
        """
        # 报单
        # self.insert_order(data.instrument_id, D_BUY, OF_OPEN, data.price + 1, 1, request_id=10)
        self.tick = data

    def on_1min(self, data):
        """MarketData 数据回传函数，由 tick 数据合成的 1 分钟数据，在下一个一分钟开始的时候，传回上一个
        一分钟的数据。

        :type data: pyctpclient.ctpclient.M1Bar
        """
        self.m1 = data

    def on_1min_tick(self, data):
        """MarketData 数据回传函数，在每一个 tick 都会得到当前 1 分钟数据。

        :type data: pyctpclient.ctpclient.M1Bar
        """
        self.m1_tick = data

    def on_td_front_disconnected(self, reason):
        """Trader 前置断开连接时会调用此函数。断开后，系统会尝试重新连接。reason 和 MarketData 的相同。
        """
        pass

    def on_settlement_info_confirm(self, confirm, rsp_info):
        """Trader 每次登录后回确认上一个交易日的交割单，确认完成后回调用此函数。通常用来执行开机的初始化查询。

        :type confirm: pyctpclient.ctpclient.SettlementInfoConfirm
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        # 目前主要有以下几个查询函数
        # 查询当前的账户资金情况
        self.query_trading_account()
        # 查询当前的账户持仓情况
        self.query_investor_position()
        # 查询当前的账户持仓明细
        self.query_investor_position_detail()
        # 查询本交易日内所有的报单记录
        self.query_order()
        # 查询本交易日内所有的成交记录
        self.query_trade()

        # 这个函数一般不会在交易的时候使用
        # 查询制定合约的市场信息
        self.query_market_data("IF1906")

    def on_rsp_trading_account(self, trading_account, rsp_info, is_last):
        """`query_trading_account` 的数据回传函数。请不要保存 `trading_account`，只能保存其中的数据。

        :type trading_account: pyctpclient.ctpclient.TradingAccount
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        self.log.info("on_rsp_trading_account")
        self.log.info("available: %.2f wan" % (trading_account.available / 10000))
        self.log.info("position profit: %.2f" % trading_account.position_profit)
        self.log.info("close profit: %.2f" % trading_account.close_profit)

    def on_rsp_investor_position(self, investor_position, rsp_info, is_last):
        """`query_investor_position` 的数据回传函数。请不要保存 `investor_position`，只能保存其中的数据。

        :type investor_position: pyctpclient.ctpclient.InvestorPosition
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if investor_position is not None:
            self.log.info("direction %s", str(investor_position.position_direction))
            self.log.info("position: %.2f", investor_position.position)
            self.log.info("today position: %.2f", investor_position.today_position)
            self.log.info("yd position: %.2f", investor_position.position - investor_position.today_position)
        elif rsp_info is not None:
            self.log.error("query investor position failed: %d", rsp_info.error_id)
        else:
            self.log.info("No investor position yet.")

    def on_rsp_investor_position_detail(self, investor_position_detail, rsp_info, is_last):
        """`query_investor_position_detail` 的数据回传函数。请不要保存 `investor_position_detail`，只能保存其中的数据。

        :type investor_position_detail: pyctpclient.ctpclient.InvestorPositionDetail
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        self.log.info("on_rsp_investor_position_detail")

    def on_rsp_order(self, order, rsp_info, is_last):
        """`query_order` 的数据回传函数。

        :type order: pyctpclient.ctpclient.Order
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        self.log.info('on_rsp_order')
        if order is not None:
            # 获取 Order 的信息，或者保存这个 Order
            self.log.info("direction: %s", order.direction)
            self.log.info("order status: %s", order.status)
            self.log.info("order submit status: %s", order.submit_status)
        elif rsp_info is not None:
            self.log.error("query order failed: %d", rsp_info.error_id)
        else:
            self.log.info("No order yet.")

    def on_rsp_trade(self, trade, rsp_info, is_last):
        """`query_trade` 的数据回传函数。请不要保存 `trade`，只能保存其中的数据。

        :type trade: pyctpclient.ctpclient.Trade
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if trade is not None:
            # 在当前交易日没有成效记录的时候，trade 是 None
            self.log.info('traded: %s', trade.instrument_id)
        elif rsp_info is not None:
            self.log.error("query trade failed: %d", rsp_info.error_id)
        else:
            self.log.error("No trade today.")

    def on_rsp_market_data(self, data, rsp_info, request_id, is_last):
        """`query_market_data` 的数据回传函数。请不要保存 `data`，只能保存其中的数据。

        :type data: pyctpclient.ctpclient.MarketData
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        self.log.info("rsp_market_data: %s", data.instrument_id)

    def on_rtn_order(self, order):
        """报单状态回传函数。当报单状态 `order.status` 或者 `order.submit_status` 变化时，

        :type order: pyctpclient.ctpclient.Order
        """
        self.log.info("direction: %s", order.direction)
        self.log.info("order status: %s", order.status)
        self.log.info("order submit status: %s", order.submit_status)
        self.log.info("request_id: %d", order.request_id)

    def on_rtn_trade(self, trade):
        """报单成交回传函数。

        :type trade: pyctpclient.ctpclient.Trade
        """
        self.log.info("Traded: %d", trade.instrument_id)

    def on_err_order_insert(self, input_order, rsp_info):
        """报单失败回传函数。这里主要是通过检查 `rsp_info.error_id` 的值来确定错误原因。

        :type input_order: pyctpclient.ctpclient.InputOrder
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if rsp_info is not None:
            self.log.error("Insert order failed: %d", rsp_info.error_id)

    def on_err_order_action(self, input_order_action, order_action, rsp_info):
        """撤单/改单失败回传函数。这里主要是通过检查 `rsp_info.error_id` 的值来确定错误原因。

        :type input_order_action: pyctpclient.ctpclient.InputOrderAction
        :type order_action: pyctpclient.ctpclient.OrderAction
        :type rsp_info: pyctpclient.ctpclient.ResponseInfo
        """
        if rsp_info is not None:
            self.log.error("Request order action failed: %d", rsp_info.error_id)

    def on_idle(self):
        """空闲回传函数。当数据队列中没有数据需要处理，并且延迟大于 `idle_delay` 时调用
        """
        self.counter += 1
        if self.counter > 10:
            try:
                self.insert_order("IF1906", "buy", "open", 3700, 1, request_id=10)
                self.counter = 0
                self.log.info("Insert Order")
            except RequestError:
                self.log.error("insert order failed")

    def on_exception(self, message):
        pass


if __name__ == "__main__":
    # 创建 CTP Client 实例
    c = Client(
        # 实盘/电信1
        md_address="tcp://180.168.146.187:10010",
        td_address="tcp://180.168.146.187:10000",
        # 实盘/电信2
        #md_address="tcp://180.168.146.187:10011",
        #td_address="tcp://180.168.146.187:10001",
        # 实盘/移动
        #md_address="tcp://218.202.237.33:10012",
        #td_address="tcp://218.202.237.33:10002",
        # 开发
        #md_address="tcp://180.168.146.187:10031",
        #td_address="tcp://180.168.146.187:10030",
        broker_id="9999",
        user_id="",
        password=""
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
