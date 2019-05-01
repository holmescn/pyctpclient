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
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include "ctpclient.h"

using namespace boost::python;

BOOST_PYTHON_MODULE(_ctpclient)
{
	class_<CtpClientWrap, boost::noncopyable>("CtpClient", init<std::string, std::string, std::string, std::string, std::string>())
	 	.add_property("flow_path", &CtpClient::GetFlowPath, &CtpClient::SetFlowPath)
	 	.add_property("md_address", &CtpClient::GetMdAddr, &CtpClient::SetMdAddr)
	 	.add_property("td_address", &CtpClient::GetTdAddr, &CtpClient::SetTdAddr)
	 	.add_property("broker_id", &CtpClient::GetBrokerId, &CtpClient::SetBrokerId)
	 	.add_property("user_id", &CtpClient::GetUserId, &CtpClient::SetUserId)
		.def("get_api_version", &CtpClient::GetApiVersion)
        .staticmethod("get_api_version")
		.def("run", &CtpClient::Run)

    	.def("md_login", &CtpClient::MdLogin)
		.def("on_md_front_connected", pure_virtual(&CtpClient::OnMdFrontConnected))
    	.def("on_md_front_disconnected", pure_virtual(&CtpClient::OnMdFrontDisconnected))
    	.def("on_md_user_login", pure_virtual(&CtpClient::OnMdUserLogin))
    	.def("on_md_user_logout", pure_virtual(&CtpClient::OnMdUserLogout))

    	.def("td_login", &CtpClient::TdLogin)
    	.def("on_td_front_connected", pure_virtual(&CtpClient::OnTdFrontConnected))
    	;
};
