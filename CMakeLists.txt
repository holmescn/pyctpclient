##
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
#
cmake_minimum_required (VERSION 3.8)

project ("ctp_python")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR}/python3.7)

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
	string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(CTP_LIBS ${CMAKE_SOURCE_DIR}/lib/thostmduserapi.lib ${CMAKE_SOURCE_DIR}/lib/thosttraderapi.lib)

set(PYTHON_LIBS ${Boost_LIBRARY_DIRS}/python37.lib)
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(BOOST_LIBS ${Boost_LIBRARY_DIRS}/../debug/lib/boost_python37-vc140-mt-gd.lib)
else()
	set(BOOST_LIBS ${Boost_LIBRARY_DIRS}/boost_python37-vc140-mt.lib)
endif()
