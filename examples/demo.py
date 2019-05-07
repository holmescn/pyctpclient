# -*- coding: utf-8 -*-
from pyctpclient import CtpClient, Direction, OffsetFlag


class Client(CtpClient):
    def on_tick(self, instrument_id, price, volume, time):
        print(instrument_id, price, volume)

    def on_1min(self, bar):
        if bar.instrument_id == "rb1910":
            self.insert_order(bar.instrument_id, Direction.BUY, OffsetFlag.OPEN, bar.close, volume=1)
            print("Insert Order")

    def on_1min_tick(self, bar):
        print(bar.instrument_id, "1 min tick")

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

    def on_rsp_order_action(self, input_order_action, rsp_info):
        print("order action")

    def on_err_order_action(self, order_action, rsp_info):
        print("error order action")

    def on_md_error(self, rsp_info):
        print("market data error")

    def on_td_error(self, rsp_info):
        print("trader error")


if __name__ == "__main__":
    c = Client("tcp://180.168.146.187:10011", "tcp://180.168.146.187:10001", "9999", "", "")
    c.instrument_ids = ['IF1905', 'rb1910']
    c.run()
