# cmake最低版本
cmake_minimum_required(VERSION 4.1)

# 包含模塊
include(CMakePrintHelpers)

# 項目名
project(windows_socket)

find_package(spdlog REQUIRED)
find_package(boost_config REQUIRED)

add_library(socket_util)
target_sources(socket_util
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src # 模塊根目錄
    FILES src/socket_util.ixx                 # 模塊接口單元
  PRIVATE
    src/socket_util.cpp                       # 模塊實現單元
)
target_compile_features(socket_util PRIVATE cxx_std_23)

add_library(socket_comm INTERFACE)
target_link_libraries(socket_comm
  INTERFACE
    "$<$<CXX_COMPILER_ID:MSVC>:ws2_32>"
    spdlog::spdlog
    Boost::config
    socket_util
)
target_compile_features(socket_comm INTERFACE cxx_std_23)

# 服務端
add_executable(server)
target_sources(server
  PRIVATE
    src/server.cpp
)
target_link_libraries(server
  PRIVATE
    socket_comm
)

## 客戶端
add_executable(client)
target_sources(client
  PRIVATE src/client.cpp
)
target_link_libraries(client
  PRIVATE
    socket_comm
)
