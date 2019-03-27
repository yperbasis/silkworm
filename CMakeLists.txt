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

cmake_minimum_required(VERSION 3.5)
project(Silkworm)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
# https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if(MSVC)
  add_compile_options(/W4 /WX)
else(MSVC)
  add_compile_options(-Wall -Wextra -pedantic -Werror)
  # add_compile_options(-gline-tables-only) # for profiling
  add_compile_options(-march=native)
endif(MSVC)

add_subdirectory(core)
add_subdirectory(lab)
add_subdirectory(test)

enable_testing()
