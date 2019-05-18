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
from tempfile import mkdtemp
from ._ctpclient import CtpClient as _CtpClient
from ._ctpclient import ResponseInfo, UserLoginInfo, UserLogoutInfo
from ._ctpclient import Direction, OffsetFlag

__version__ = "0.3.0a0"
__author__ = "Holmes Conan"


class CtpClient(_CtpClient):

    def __init__(self, md_address, td_address, broker_id, user_id, password):
        _CtpClient.__init__(self, md_address, td_address, broker_id, user_id, password)

    def init(self):
        if self.flow_path == "":
            self.flow_path = mkdtemp(prefix="ctp")
        
        if not os.path.exists(self.flow_path):
            os.makedirs(self.flow_path)

        _CtpClient.init(self)

    def on_md_front_connected(self):
        self.md_login()

    def on_md_user_login(self, user_login_info, rsp_info):
        print("??")
        if rsp_info.error_id == 0 and len(self.instrument_ids) > 0:
            self.subscribe_market_data(self.instrument_ids)
