# -*- coding: utf-8 -*-
from pyctpclient import CtpClient, Direction, OffsetFlag

class Client(CtpClient):
    def on_idle(self):
        print("idle")

    def on_tick(self, bar):
        print(bar.instrument_id, bar.price, bar.volume)

    def on_1min(self, bar):
        print("1min", bar.instrument_id, bar.close, bar.volume, bar.turnover)

    def on_1min_tick(self, bar):
        print("1min tick", bar.instrument_id, bar.close, bar.volume, bar.turnover)

    def on_settlement_info_confirm(self, settlement_info_confirm, rsp_info):
        self.query_trading_account()
        self.query_investor_position()

    def on_rsp_trading_account(self, trading_account, rsp_info, is_last):
        print("trading_account")

    def on_rsp_investor_position(self, investor_position, rsp_info, is_last):
        print("investor_position")
        if investor_position is None:
            print("No position")

    def on_rtn_order(self, order):
        print(order.submit_status, order.status)

    def on_rtn_trade(self, trade):
        print("trade")

    def on_err_order_insert(self, input_order, rsp_info):
        print("error insert order")

    def on_err_order_action(self, input_order_action, order_action, rsp_info):
        print("error order action")

    def on_md_error(self, rsp_info):
        print("market data error")

    def on_td_error(self, rsp_info):
        print("trader error")


if __name__ == "__main__":
    # c = Client("tcp://180.168.146.187:10011", "tcp://180.168.146.187:10001", "9999", "", "")
    c = Client("tcp://180.168.146.187:10031", "tcp://180.168.146.187:10030", "9999", "", "")
    c.instrument_ids = ['IF1905', 'rb1910']
    c.init()
    c.join()
