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

add_library(_ctpclient SHARED ctpclient.cc mdspi.cc traderspi.cc binding.cc)
target_link_libraries(_ctpclient ${BOOST_LIBS} ${PYTHON_LIBS} ${CTP_LIBS})
set_target_properties(_ctpclient PROPERTIES PREFIX "")
install(TARGETS _ctpclient DESTINATION "${CMAKE_SOURCE_DIR}/lib")
