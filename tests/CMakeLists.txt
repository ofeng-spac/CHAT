cmake_minimum_required(VERSION 3.10)
project(ChatServerSecurityTests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# 查找必要的包
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# 查找MySQL (可选)
pkg_check_modules(MYSQL mysqlclient)
if(MYSQL_FOUND)
    message(STATUS "Found MySQL: ${MYSQL_LIBRARIES}")
    add_definitions(-DHAVE_MYSQL)
else()
    message(WARNING "MySQL not found. Some tests will be disabled.")
endif()

# 查找Google Test
find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
    # 如果系统没有安装GTest，尝试下载和编译
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188ec8e5902e4ec0908.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty
    ${MYSQL_INCLUDE_DIRS}
)

# 源文件
set(COMMON_SOURCES
    ../src/server/common/InputValidator.cpp
    ../src/server/common/EnhancedInputValidator.cpp
    ../src/server/common/ErrorCodes.cpp
    ../src/server/common/Logger.cpp
    ../src/server/db/SecureDB.cpp
    ../src/server/model/SecureUserModel.cpp
    ../src/server/security/PasswordUtils.cpp
)

# 检查源文件是否存在
foreach(source_file ${COMMON_SOURCES})
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${source_file})
        message(WARNING "Source file ${source_file} not found, removing from build")
        list(REMOVE_ITEM COMMON_SOURCES ${source_file})
    endif()
endforeach()

# 创建增强安全测试可执行文件
add_executable(enhanced_security_test
    enhanced_security_test.cpp
    ${COMMON_SOURCES}
)

# 链接库
target_link_libraries(enhanced_security_test
    ${GTEST_LIBRARIES}
    Threads::Threads
    pthread
    crypto
    ssl
    muduo_base
    muduo_net
    mysqlclient
)

# 条件性链接MySQL
if(MYSQL_FOUND)
    target_link_libraries(enhanced_security_test ${MYSQL_LIBRARIES})
    target_link_options(enhanced_security_test PRIVATE ${MYSQL_LDFLAGS})
endif()

# 如果使用FetchContent下载的GTest
if(TARGET gtest)
    target_link_libraries(enhanced_security_test gtest gtest_main)
else()
    target_link_libraries(enhanced_security_test ${GTEST_LIBRARIES})
endif()

# 添加编译定义
if(MYSQL_FOUND)
    target_compile_definitions(enhanced_security_test PRIVATE MYSQL_CLIENT)
endif()

# 创建基础安全测试（如果原始测试文件存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/security_test.cpp)
    add_executable(basic_security_test
        security_test.cpp
        ${COMMON_SOURCES}
    )
    
    target_link_libraries(basic_security_test
        ${GTEST_LIBRARIES}
        Threads::Threads
        pthread
        crypto
        ssl
    )
    
    # 条件性链接MySQL
    if(MYSQL_FOUND)
        target_link_libraries(basic_security_test ${MYSQL_LIBRARIES})
        target_compile_definitions(basic_security_test PRIVATE MYSQL_CLIENT)
    endif()
    
    if(TARGET gtest)
        target_link_libraries(basic_security_test gtest gtest_main)
    endif()
endif()

# 添加测试
enable_testing()
add_test(NAME EnhancedSecurityTest COMMAND enhanced_security_test)

if(TARGET basic_security_test)
    add_test(NAME BasicSecurityTest COMMAND basic_security_test)
endif()

# 设置测试属性
set_tests_properties(EnhancedSecurityTest PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 创建一个运行所有测试的目标
add_custom_target(run_security_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS enhanced_security_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all security tests"
)

# 打印配置信息
message(STATUS "MySQL include dirs: ${MYSQL_INCLUDE_DIRS}")
message(STATUS "MySQL libraries: ${MYSQL_LIBRARIES}")
message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 安装目标（可选）
install(TARGETS enhanced_security_test
    RUNTIME DESTINATION bin
)

if(TARGET basic_security_test)
    install(TARGETS basic_security_test
        RUNTIME DESTINATION bin
    )
endif()