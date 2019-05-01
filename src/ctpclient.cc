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

boost::python::tuple CtpClient::GetApiVersion()
{
	std::string v1 = CThostFtdcMdApi::GetApiVersion();
	std::string v2 = CThostFtdcTraderApi::GetApiVersion();
	return boost::python::make_tuple(v1, v2);
}

class CtpClientWrap : public CtpClient, public wrapper<CtpClient>
{
	void OnMdFrontConnected() override {
		if (override fn = this->get_override("OnMdFrontConnected")) {
			fn();
		} else {

		}
	}

	void OnTraderFrontConnected() override {
		if (override fn = this->get_override("OnTraderFrontConnected")) {
			fn();
		} else {

		}
	}
};

BOOST_PYTHON_MODULE(_ctpclient)
{
	class_<CtpClientWrap, boost::noncopyable>("CtpClient")
		.def("GetApiVersion", &CtpClient::GetApiVersion)
        .staticmethod("GetApiVersion")
    	.def("OnMdFrontConnected", pure_virtual(&CtpClient::OnMdFrontConnected))
    	.def("OnTraderFrontConnected", pure_virtual(&CtpClient::OnTraderFrontConnected))
    	;
};
