#[[Copyright 2019 Ethereum Foundation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
]]

find_package(Catch2 REQUIRED)
include_directories(${Silkworm_SOURCE_DIR}/src
                    ${Catch2_INTERFACE_INCLUDE_DIRECTORIES})

file(GLOB Silkworm_TEST_SRC "*.cpp")
add_executable(tests ${Silkworm_TEST_SRC})
target_link_libraries(tests silkworm Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(tests)
