#!/bin/bash

# 安全测试运行脚本
# 用于编译和运行SQL注入防护和输入验证的单元测试

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖项..."
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake 未安装，请先安装 CMake"
        exit 1
    fi
    
    # 检查g++
    if ! command -v g++ &> /dev/null; then
        print_error "g++ 未安装，请先安装 g++"
        exit 1
    fi
    
    # 检查MySQL开发库
    if ! pkg-config --exists mysqlclient; then
        print_warning "MySQL客户端开发库未找到，某些测试可能会跳过"
        print_info "在Ubuntu/Debian上安装: sudo apt-get install libmysqlclient-dev"
        print_info "在CentOS/RHEL上安装: sudo yum install mysql-devel"
    fi
    
    # 检查Google Test
    if ! pkg-config --exists gtest; then
        print_warning "Google Test 未找到，将尝试自动下载"
        print_info "在Ubuntu/Debian上安装: sudo apt-get install libgtest-dev"
        print_info "在CentOS/RHEL上安装: sudo yum install gtest-devel"
    fi
    
    print_success "依赖检查完成"
}

# 创建构建目录
setup_build_dir() {
    print_info "设置构建目录..."
    
    BUILD_DIR="build"
    if [ -d "$BUILD_DIR" ]; then
        print_info "清理现有构建目录"
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_success "构建目录设置完成"
}

# 配置CMake
configure_cmake() {
    print_info "配置CMake..."
    
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug"
    
    # 如果MySQL不可用，添加相应的标志
    if ! pkg-config --exists mysqlclient; then
        CMAKE_ARGS="$CMAKE_ARGS -DSKIP_MYSQL_TESTS=ON"
    fi
    
    if ! cmake $CMAKE_ARGS ..; then
        print_error "CMake配置失败"
        exit 1
    fi
    
    print_success "CMake配置完成"
}

# 编译测试
build_tests() {
    print_info "编译测试程序..."
    
    if ! make -j$(nproc); then
        print_error "编译失败"
        exit 1
    fi
    
    print_success "编译完成"
}

# 运行测试
run_tests() {
    print_info "运行安全测试..."
    
    # 检查测试可执行文件是否存在
    if [ -f "enhanced_security_test" ]; then
        print_info "运行增强安全测试..."
        if ./enhanced_security_test; then
            print_success "增强安全测试通过"
        else
            print_warning "增强安全测试失败或部分跳过"
        fi
    else
        print_error "增强安全测试可执行文件未找到"
    fi
    
    if [ -f "basic_security_test" ]; then
        print_info "运行基础安全测试..."
        if ./basic_security_test; then
            print_success "基础安全测试通过"
        else
            print_warning "基础安全测试失败或部分跳过"
        fi
    fi
    
    # 使用CTest运行所有测试
    print_info "使用CTest运行所有测试..."
    if ctest --verbose; then
        print_success "所有测试完成"
    else
        print_warning "某些测试失败或跳过"
    fi
}

# 生成测试报告
generate_report() {
    print_info "生成测试报告..."
    
    REPORT_FILE="security_test_report.txt"
    
    echo "=== 安全测试报告 ===" > "$REPORT_FILE"
    echo "测试时间: $(date)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    echo "=== 测试结果 ===" >> "$REPORT_FILE"
    if [ -f "Testing/Temporary/LastTest.log" ]; then
        cat "Testing/Temporary/LastTest.log" >> "$REPORT_FILE"
    fi
    
    echo "" >> "$REPORT_FILE"
    echo "=== 系统信息 ===" >> "$REPORT_FILE"
    echo "操作系统: $(uname -a)" >> "$REPORT_FILE"
    echo "编译器: $(g++ --version | head -n1)" >> "$REPORT_FILE"
    echo "CMake: $(cmake --version | head -n1)" >> "$REPORT_FILE"
    
    if pkg-config --exists mysqlclient; then
        echo "MySQL客户端: $(pkg-config --modversion mysqlclient)" >> "$REPORT_FILE"
    else
        echo "MySQL客户端: 未安装" >> "$REPORT_FILE"
    fi
    
    print_success "测试报告已生成: $REPORT_FILE"
}

# 清理函数
cleanup() {
    print_info "清理构建文件..."
    cd ..
    if [ -d "build" ]; then
        rm -rf "build"
    fi
    print_success "清理完成"
}

# 显示帮助信息
show_help() {
    echo "安全测试运行脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help     显示此帮助信息"
    echo "  -c, --clean    仅清理构建文件"
    echo "  -b, --build    仅编译，不运行测试"
    echo "  -r, --run      仅运行测试（假设已编译）"
    echo "  -v, --verbose  详细输出"
    echo ""
    echo "示例:"
    echo "  $0              # 完整的编译和测试流程"
    echo "  $0 --clean     # 清理构建文件"
    echo "  $0 --build     # 仅编译测试"
    echo "  $0 --run       # 仅运行测试"
}

# 主函数
main() {
    print_info "开始安全测试流程..."
    
    # 解析命令行参数
    case "${1:-}" in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            cleanup
            exit 0
            ;;
        -b|--build)
            check_dependencies
            setup_build_dir
            configure_cmake
            build_tests
            print_success "编译完成，使用 '$0 --run' 运行测试"
            exit 0
            ;;
        -r|--run)
            if [ -d "build" ]; then
                cd "build"
                run_tests
                generate_report
            else
                print_error "构建目录不存在，请先运行 '$0 --build'"
                exit 1
            fi
            exit 0
            ;;
        -v|--verbose)
            set -x  # 启用详细输出
            ;;
        "")
            # 默认行为：完整流程
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
    
    # 执行完整流程
    check_dependencies
    setup_build_dir
    configure_cmake
    build_tests
    run_tests
    generate_report
    
    print_success "安全测试流程完成！"
    print_info "查看详细报告: build/security_test_report.txt"
}

# 脚本入口点
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi