#[[
   Copyright 2019 Ethereum Foundation

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

find_package(Boost REQUIRED COMPONENTS date_time)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(${Silkworm_SOURCE_DIR}/core)

file(GLOB Silkworm_LAB_SRC "*.hpp" "*.cpp")
add_executable(sync_emulator ${Silkworm_LAB_SRC})
target_link_libraries(sync_emulator silkworm ${Boost_LIBRARIES})
