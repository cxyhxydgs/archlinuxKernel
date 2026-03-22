#!/bin/bash
#
# 监控服务安装脚本
# 适用于 Arch Linux
#

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 显示帮助
show_help() {
    echo -e "${CYAN}用法:${NC} $0 [选项]"
    echo ""
    echo "选项:"
    echo "  --all         安装所有监控服务"
    echo "  --kernel      仅安装内核监控服务"
    echo "  --openclaw    仅安装 OpenClaw 监控服务"
    echo "  --help        显示此帮助信息"
    echo ""
    exit 0
}

# 安装内核监控服务
install_kernel_monitor() {
    echo -e "${BLUE}>>> 安装内核监控服务...${NC}"
    
    # 创建目录
    mkdir -p /var/lib/kernel-monitor/history
    mkdir -p /var/log
    mkdir -p /var/run
    
    # 安装脚本
    install -m 755 "$SCRIPT_DIR/kernel_monitor.sh" /usr/local/bin/kernel-monitor.sh
    echo -e "${GREEN}    -> 脚本已安装到 /usr/local/bin/kernel-monitor.sh${NC}"
    
    # 安装服务文件
    install -m 644 "$SCRIPT_DIR/kernel-monitor.service" /etc/systemd/system/kernel-monitor.service
    echo -e "${GREEN}    -> 服务文件已安装到 /etc/systemd/system/kernel-monitor.service${NC}"
    
    # 启用服务
    systemctl enable kernel-monitor.service
    echo -e "${GREEN}    -> 内核监控服务已设置为开机自启${NC}"
}

# 安装 OpenClaw 监控服务
install_openclaw_monitor() {
    echo -e "${BLUE}>>> 安装 OpenClaw 监控服务...${NC}"
    
    # 创建目录
    mkdir -p /var/lib/openclaw
    mkdir -p /var/log
    mkdir -p /var/run
    
    # 安装脚本
    if [ -f "$SCRIPT_DIR/openclaw_monitor.sh" ]; then
        install -m 755 "$SCRIPT_DIR/openclaw_monitor.sh" /usr/local/bin/openclaw-monitor.sh
        echo -e "${GREEN}    -> 脚本已安装到 /usr/local/bin/openclaw-monitor.sh${NC}"
        
        # 安装服务文件
        install -m 644 "$SCRIPT_DIR/openclaw-monitor.service" /etc/systemd/system/openclaw-monitor.service
        echo -e "${GREEN}    -> 服务文件已安装到 /etc/systemd/system/openclaw-monitor.service${NC}"
        
        # 启用服务
        systemctl enable openclaw-monitor.service
        echo -e "${GREEN}    -> OpenClaw 监控服务已设置为开机自启${NC}"
    else
        echo -e "${YELLOW}    -> 未找到 openclaw_monitor.sh，跳过${NC}"
    fi
}

# 主安装流程
main() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}     监控服务安装程序${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    
    # 检查 root 权限
    if [ "$EUID" -ne 0 ]; then
        echo -e "${RED}请使用 root 权限运行此脚本${NC}"
        echo "使用: sudo $0"
        exit 1
    fi
    
    local install_kernel=false
    local install_openclaw=false
    
    # 解析参数
    case "${1:-}" in
        --all)
            install_kernel=true
            install_openclaw=true
            ;;
        --kernel)
            install_kernel=true
            ;;
        --openclaw)
            install_openclaw=true
            ;;
        --help|-h)
            show_help
            ;;
        "")
            # 无参数时显示选择菜单
            echo -e "${YELLOW}请选择要安装的服务:${NC}"
            echo "  1) 内核监控服务"
            echo "  2) OpenClaw 监控服务"
            echo "  3) 全部安装"
            echo "  q) 退出"
            echo ""
            read -p "请输入选项 [1/2/3/q]: " choice
            
            case "$choice" in
                1) install_kernel=true ;;
                2) install_openclaw=true ;;
                3) install_kernel=true; install_openclaw=true ;;
                q|Q) echo "已取消"; exit 0 ;;
                *) echo -e "${RED}无效选项${NC}"; exit 1 ;;
            esac
            ;;
        *)
            echo -e "${RED}未知选项: $1${NC}"
            show_help
            ;;
    esac
    
    # 执行安装
    if [ "$install_kernel" = true ]; then
        install_kernel_monitor
        echo ""
    fi
    
    if [ "$install_openclaw" = true ]; then
        install_openclaw_monitor
        echo ""
    fi
    
    # 重新加载 systemd
    echo -e "${YELLOW}>>> 重新加载 systemd 守护进程...${NC}"
    systemctl daemon-reload
    echo -e "${GREEN}    -> systemd 已重新加载${NC}"
    
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  安装完成！${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    
    # 显示使用说明
    echo -e "${CYAN}服务管理命令:${NC}"
    echo ""
    
    if [ "$install_kernel" = true ]; then
        echo -e "${YELLOW}[内核监控服务]${NC}"
        echo -e "  ${BLUE}状态:${NC}     sudo systemctl status kernel-monitor"
        echo -e "  ${BLUE}启动:${NC}     sudo systemctl start kernel-monitor"
        echo -e "  ${BLUE}停止:${NC}     sudo systemctl stop kernel-monitor"
        echo -e "  ${BLUE}日志:${NC}     sudo journalctl -u kernel-monitor -f"
        echo -e "  ${BLUE}实时状态:${NC} sudo /usr/local/bin/kernel-monitor.sh status"
        echo ""
    fi
    
    if [ "$install_openclaw" = true ]; then
        echo -e "${YELLOW}[OpenClaw 监控服务]${NC}"
        echo -e "  ${BLUE}状态:${NC}     sudo systemctl status openclaw-monitor"
        echo -e "  ${BLUE}启动:${NC}     sudo systemctl start openclaw-monitor"
        echo -e "  ${BLUE}停止:${NC}     sudo systemctl stop openclaw-monitor"
        echo -e "  ${BLUE}日志:${NC}     sudo journalctl -u openclaw-monitor -f"
        echo ""
    fi
    
    # 询问是否立即启动
    read -p "是否立即启动已安装的服务？[Y/n] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Nn]$ ]]; then
        if [ "$install_kernel" = true ]; then
            systemctl start kernel-monitor.service
            echo -e "${GREEN}内核监控服务已启动${NC}"
        fi
        if [ "$install_openclaw" = true ]; then
            systemctl start openclaw-monitor.service
            echo -e "${GREEN}OpenClaw 监控服务已启动${NC}"
        fi
    fi
}

main "$@"