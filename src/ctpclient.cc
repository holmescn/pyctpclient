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
#include <sstream>
#include <iostream>
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/filesystem.hpp>

#include "ctpclient.h"

using namespace boost::python;

#define assert_request(request) _assertRequest(__FILE__, __LINE__, (request), #request)

void _assertRequest(const char* file, int line, int rcRequest, const char *request)
{
	std::stringstream ss;
	switch (rcRequest) {
	case 0:
		// 发送成功
		break;
	case -1:
		// 因网络原因发送失败
		ss << "Request " << request
			<< " failed because of network." << std::endl;
		throw std::runtime_error(ss.str());
	case -2:
		// 未处理请求队列总数量超限
		ss << "Request " << request
			<< " failed because of request queue is full." << std::endl;
		throw std::runtime_error(ss.str());
	case -3:
		// 每秒发送请求数量超限
		ss << "Request " << request
			<< " failed because of request too frequently." << std::endl;
		throw std::runtime_error(ss.str());
	default:
		ss << "Request " << request
			<< " failed because of unhandled error - " << rcRequest << std::endl;
		throw std::runtime_error(ss.str());
	}
}

CtpClient::CtpClient(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password)
: _mdAddr(mdAddr), _tdAddr(tdAddr), _brokerId(brokerId), _userId(userId), _password(password)
{
	//
}

CtpClient::~CtpClient()
{
	if (_mdSpi) {
		delete _mdSpi;
	}

	if (_tdSpi) {
		delete _tdSpi;
	}
}

tuple CtpClient::GetApiVersion()
{
	std::string v1 = CThostFtdcMdApi::GetApiVersion();
	std::string v2 = CThostFtdcTraderApi::GetApiVersion();
	return boost::python::make_tuple(v1, v2);
}


void CtpClient::Run()
{
	using namespace boost::filesystem;

	std::string mdFlowPath, tdFlowPath;
	if (_flowPath == "") {
		auto tmpPath = temp_directory_path();
		auto mdPath = tmpPath / "md";
		auto tdPath = tmpPath / "td";
		create_directory(mdPath);
		create_directory(tdPath);

		mdFlowPath = mdPath.string();
		tdFlowPath = tdPath.string();
	} else {
		path p(_flowPath);
		if (!exists(p)) {
			create_directory(p);
		}
		auto mdPath = p / "md";
		if (!exists(mdPath)) {
			create_directory(mdPath);
		}
		auto tdPath = p / "td";
		if (!exists(tdPath)) {
			create_directory(tdPath);
		}

		mdFlowPath = mdPath.string();
		tdFlowPath = tdPath.string();
	}

	_mdApi = CThostFtdcMdApi::CreateFtdcMdApi(mdFlowPath.c_str(), /*using udp*/false, /*multicast*/false);
	_mdSpi = new MdSpi(this);
	_mdApi->RegisterSpi(_mdSpi);
	_mdApi->RegisterFront(const_cast<char*>(_mdAddr.c_str()));

	_tdApi = CThostFtdcTraderApi::CreateFtdcTraderApi(tdFlowPath.c_str());
	_tdSpi = new TraderSpi(this);
	_tdApi->RegisterSpi(_tdSpi);
	_tdApi->RegisterFront(const_cast<char*>(_tdAddr.c_str()));

	_mdApi->Init();
	_tdApi->Init();

	_mdApi->Join();
	_tdApi->Join();
}

void CtpClient::MdLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.UserID, _userId.c_str(), sizeof req.UserID);
	strncpy(req.Password, _password.c_str(), sizeof req.Password);
	// strncpy(req.UserProductInfo, _userProductInfo.c_str(), sizeof req.UserProductInfo);

	assert_request(_mdApi->ReqUserLogin(&req, 0));
}

void CtpClient::TdLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof req);
	strncpy(req.BrokerID, _brokerId.c_str(), sizeof req.BrokerID);
	strncpy(req.UserID, _userId.c_str(), sizeof req.UserID);
	strncpy(req.Password, _password.c_str(), sizeof req.Password);
	// strncpy(req.UserProductInfo, _userProductInfo.c_str(), sizeof req.UserProductInfo);

	assert_request(_tdApi->ReqUserLogin(&req, 0));
}

class CtpClientWrap : public CtpClient, public boost::python::wrapper<CtpClient>
{
public:
	CtpClientWrap(std::string mdAddr, std::string tdAddr, std::string brokerId, std::string userId, std::string password)
	: CtpClient(mdAddr, tdAddr, brokerId, userId, password)
	{
		//
	}

	~CtpClientWrap()
	{
		//
	}

	void OnMdFrontConnected() override
	{
		if (override fn = this->get_override("on_md_front_connected")) {
			fn();
		} else {
			std::cerr << "Market Data Front Connected" << std::endl;
			MdLogin();
		}
	}

	void OnMdFrontDisconnected(int nReason) override
	{
		if (override fn = this->get_override("on_md_front_disconnected")) {
			fn();
		} else {
			std::cerr << "Market Data Front Disconnected with reason = " << nReason << std::endl;
		}
	}

	void OnMdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
	{
		if (override fn = this->get_override("on_md_user_login")) {
			fn();
		} else {
			std::cerr << "Market Data User Login" << std::endl;
		}
	}

	void OnMdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
	{
		if (override fn = this->get_override("on_md_user_logout")) {
			fn();
		} else {
			std::cerr << "Market Data User Logout" << std::endl;
		}
	}

	void OnTdFrontConnected() override
	{
		std::cerr << "Trader Front Connected" << std::endl;

		if (override fn = this->get_override("on_td_front_connected")) {
			fn();
		} else {
			TdLogin();
		}
	}

	void OnTdFrontDisconnected(int nReason) override
	{
		if (override fn = this->get_override("on_md_front_disconnected")) {
			fn();
		} else {
			std::cerr << "Trader Front Disconnected with reason = " << nReason << std::endl;
		}
	}

	void OnTdUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
	{
		if (override fn = this->get_override("on_md_user_login")) {
			fn();
		} else {
			std::cerr << "Trader User Login" << std::endl;
		}
	}

	void OnTdUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
	{
		if (override fn = this->get_override("on_md_user_logout")) {
			fn();
		} else {
			std::cerr << "Trader User Logout" << std::endl;
		}
	}

};

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
