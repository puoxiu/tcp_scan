cmake_minimum_required(VERSION 3.10)

# 项目名称
project(TCP_Scan)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加源文件
set(SOURCES
    main.cpp
)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加可执行文件
add_executable(tcp_scan ${SOURCES})

# 如果需要链接额外的库，可以在这里添加
# target_link_libraries(tcp_scan PRIVATE some_library)
