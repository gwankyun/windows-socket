add_library(socket_util_common INTERFACE)
target_sources(socket_util_common
  INTERFACE
    src/socket_util_inc.h    # 引入頭文件
    src/socket_util_decl.h   # 聲明
    src/socket_util_impl.hpp # 實現
)

add_library(socket_util)
target_sources(socket_util
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src # 模塊根目錄
    FILES src/socket_util.ixx                 # 模塊接口單元
  PRIVATE
    src/socket_util_module.cpp                # 模塊實現單元
)
target_link_libraries(socket_util PRIVATE socket_util_common)
target_compile_features(socket_util PRIVATE cxx_std_23)

add_library(socket_util_lib)
target_sources(socket_util_lib
  PUBLIC
    src/socket_util.h
  PRIVATE
    src/socket_util_lib.cpp
)
target_link_libraries(socket_util_lib PRIVATE socket_util_common)
