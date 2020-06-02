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
if(${MSVC})
	add_compile_options(/W4 /MP)
endif()

# 執行文件
add_executable(server "")
add_executable(client "")

# 添加源碼
target_sources(server PRIVATE src/server.cpp)
target_sources(client PRIVATE src/client.cpp)
