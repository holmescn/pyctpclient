# -*- coding: utf-8 -*-
import re
from pyctpclient import CtpClient
from datetime import datetime


class Client(CtpClient):
    leading_contracts = dict()
    re_split = re.compile(r"([a-zA-Z]+)([0-9]+)")
    counter = 0

    def on_settlement_info_confirm(self, confirm, rsp_info):
        contract_prefix = [
            "IF", "IC", "IH",
            "a",  "ag", "al", "au",  "b", "bu",  "c", "cu", "cs",
            "fu", "fb", "hc",  "i",  "j", "jd", "jm",  "l",  "m", "ni",
             "p", "pp", "pb", "ru", "rb", "sn", "sp", "sc", "wr", "zn",
        ]
        for c in contract_prefix:
            today = datetime.today()
            year = today.year % 2000
            for m in range(12):
                month = today.month + m
                instrument_id = "%s%02d%02d" % (c, year, month % 12 if month % 12 > 0 else 12)
                self.log.info("query %s" % instrument_id)
                self.query_market_data(instrument_id)
                self.counter += 1
                if month % 12 == 0:
                    year += 1

    def on_rsp_market_data(self, data, rsp_info, request_id, is_last):
        if data is not None:
            matched = self.re_split.match(data.instrument_id)
            instrument = matched.group(1)
            t = (data.instrument_id, data.volume, data.open_interest)
            if instrument not in self.leading_contracts:
                self.leading_contracts[instrument] = [t]
            else:
                self.leading_contracts[instrument].append(t)

        self.counter -= 1
        if self.counter == 0:
            print('')

            for v in self.leading_contracts.values():
                v0 = None
                for vv in sorted(v, key=lambda x: -x[1]):
                    print("%s %d %d" % vv)
                    if v0 is not None and vv[1] < v0[1]:
                        break
                    v0 = vv

            self.exit()
        else:
            print('.', end='', flush=True)


if __name__ == "__main__":
    # 创建 CTP Client 实例
    c = Client(
        # 实盘/电信1
        md_address="tcp://180.168.146.187:10111",
        td_address="tcp://180.168.146.187:10101",
        # 实盘/电信2
        #md_address="tcp://218.202.237.33:10112",
        #td_address="tcp://218.202.237.33:10102",
        broker_id="9999",
        user_id="",
        password="",
        # 看穿式管理
        app_id='simnow_client_test',
        auth_code='0' * 16,
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
