# -*- coding: utf-8 -*-
from ctpclient import CtpClient


class Client(CtpClient):
    def on_rtn_market_data(self, market_data):
        pass

    def on_tick(instrument_id, price, volume, time):
        pass

    def on_1min(instrument_id, price_open, price_high, price_low, price_close, volume, time):
        pass


if __name__ == "__main__":
    c = Client("tcp://180.168.146.187:10031", "tcp://180.168.146.187:10030", "9999", "", "")
    c.instrument_ids = ['IF1905', 'rb1910']
    c.run()
