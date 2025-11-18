# cmake最低版本
cmake_minimum_required(VERSION 4.1)

# 包含模塊
include(CMakePrintHelpers)

# 項目名
project(windows_socket)

find_package(spdlog REQUIRED)
find_package(boost_config REQUIRED)

# MSVC下以UTF-8格式編譯
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

add_subdirectory(socket_util)

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
    socket_util
)

## 客戶端
add_executable(client)
target_sources(client
  PRIVATE
    src/client_decl.hpp
    src/client_impl.hpp
    src/client.cpp
)
target_link_libraries(client
  PRIVATE
    socket_comm
    socket_util
)

add_executable(client_header)
target_sources(client_header
  PRIVATE
    src/client_decl.hpp
    src/client_impl.hpp
    src/client_header.cpp
)
target_include_directories(client_header
  PRIVATE
    ${PROJECT_SOURCE_DIR}
)
target_compile_definitions(client_header
  PRIVATE
    SOCKET_UTIL_HEADER_ONLY
)
target_link_libraries(client_header
  PRIVATE
    socket_comm
)

set(custom "${CMAKE_SOURCE_DIR}/custom") 

if(EXISTS ${custom} AND IS_DIRECTORY ${custom})
  add_subdirectory(${custom})
endif()
