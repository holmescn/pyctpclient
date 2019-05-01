# -*- coding: utf-8 -*-
from _ctpclient import CtpClient


class Client(CtpClient):
    pass


if __name__ == "__main__":
    c = Client("tcp://180.168.146.187:10031", "tcp://180.168.146.187:10030", "9999", "", "")
    print(c.get_api_version())
    c.run()
