
# 定义一个SRC_LIST变量，包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(./common COMMON_LIST)
aux_source_directory(./security SECURITY_LIST)

# 制定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST} ${COMMON_LIST} ${SECURITY_LIST})  
# 制定生成可执行文件连接时所需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread hiredis crypto ssl)
 