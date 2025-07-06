cmake_minimum_required(VERSION 3.10)
project(CHAT)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置最终可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

# 添加连接池相关头文件路径
include_directories(${PROJECT_SOURCE_DIR}/../CommonConnectionPool/include)

# 查找MySQL库 - 使用传统方式
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    PATHS
    /usr/include
    /usr/local/include
    /usr/include/mysql
    /usr/local/include/mysql
    /usr/local/mysql/include
    /opt/mysql/include
)

find_library(MYSQL_LIBRARY
    NAMES mysqlclient mysql
    PATHS
    /usr/lib
    /usr/local/lib
    /usr/lib/mysql
    /usr/local/lib/mysql
    /usr/local/mysql/lib
    /opt/mysql/lib
    /usr/lib64
    /usr/lib64/mysql
)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
    include_directories(${MYSQL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "MySQL not found!")
endif()

# 加载子目录
add_subdirectory(src)

# 添加性能测试可执行文件
aux_source_directory(./src/server/db DB_LIST)
aux_source_directory(./src/server/model MODEL_LIST)
aux_source_directory(./src/server/common COMMON_LIST)
aux_source_directory(./src/server/security SECURITY_LIST)

add_executable(performance_test 
    test/performance_test.cpp
    ${DB_LIST}
    ${MODEL_LIST}
    ${COMMON_LIST}
    ${SECURITY_LIST}
)

# 链接必要的库 - 添加 muduo 库
target_link_libraries(performance_test muduo_net muduo_base mysqlclient pthread crypto ssl)