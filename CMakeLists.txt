# cmake最低版本
cmake_minimum_required(VERSION 3.17)

# 啟用IDE目錄功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 包含模塊
include(CMakePrintHelpers)

# 輸出目錄
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 項目名
project(WindowsSocket)

# 設置編譯選項
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP>"
    "$<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/Od;/Ob0>"
    "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2;/Ob2>"
)

# 執行文件
add_executable(server "")
add_executable(client "")

# 引用目錄
target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)

# 引用庫
target_link_libraries(server PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:ws2_32>")
target_link_libraries(client PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:ws2_32>")

# 添加源碼
target_sources(server PRIVATE src/server.cpp include/socket.h)
target_sources(client PRIVATE src/client.cpp include/socket.h)
