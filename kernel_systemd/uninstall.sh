#!/bin/bash
#
# 监控服务卸载脚本
#

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 显示帮助
show_help() {
    echo -e "${CYAN}用法:${NC} $0 [选项]"
    echo ""
    echo "选项:"
    echo "  --all         卸载所有监控服务"
    echo "  --kernel      仅卸载内核监控服务"
    echo "  --openclaw    仅卸载 OpenClaw 监控服务"
    echo "  --help        显示此帮助信息"
    echo ""
    exit 0
}

# 卸载内核监控服务
uninstall_kernel_monitor() {
    echo -e "${BLUE}>>> 卸载内核监控服务...${NC}"
    
    systemctl stop kernel-monitor.service 2>/dev/null || true
    systemctl disable kernel-monitor.service 2>/dev/null || true
    
    rm -f /etc/systemd/system/kernel-monitor.service
    rm -f /usr/local/bin/kernel-monitor.sh
    rm -f /var/run/kernel-monitor.pid
    
    echo -e "${GREEN}    -> 内核监控服务已卸载${NC}"
}

# 卸载 OpenClaw 监控服务
uninstall_openclaw_monitor() {
    echo -e "${BLUE}>>> 卸载 OpenClaw 监控服务...${NC}"
    
    systemctl stop openclaw-monitor.service 2>/dev/null || true
    systemctl disable openclaw-monitor.service 2>/dev/null || true
    
    rm -f /etc/systemd/system/openclaw-monitor.service
    rm -f /usr/local/bin/openclaw-monitor.sh
    rm -f /var/run/openclaw-monitor.pid
    
    echo -e "${GREEN}    -> OpenClaw 监控服务已卸载${NC}"
}

# 主卸载流程
main() {
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}     监控服务卸载程序${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo ""
    
    # 检查 root 权限
    if [ "$EUID" -ne 0 ]; then
        echo -e "${RED}请使用 root 权限运行此脚本${NC}"
        echo "使用: sudo $0"
        exit 1
    fi
    
    local uninstall_kernel=false
    local uninstall_openclaw=false
    
    # 解析参数
    case "${1:-}" in
        --all)
            uninstall_kernel=true
            uninstall_openclaw=true
            ;;
        --kernel)
            uninstall_kernel=true
            ;;
        --openclaw)
            uninstall_openclaw=true
            ;;
        --help|-h)
            show_help
            ;;
        "")
            # 无参数时显示选择菜单
            echo -e "${YELLOW}请选择要卸载的服务:${NC}"
            echo "  1) 内核监控服务"
            echo "  2) OpenClaw 监控服务"
            echo "  3) 全部卸载"
            echo "  q) 退出"
            echo ""
            read -p "请输入选项 [1/2/3/q]: " choice
            
            case "$choice" in
                1) uninstall_kernel=true ;;
                2) uninstall_openclaw=true ;;
                3) uninstall_kernel=true; uninstall_openclaw=true ;;
                q|Q) echo "已取消"; exit 0 ;;
                *) echo -e "${RED}无效选项${NC}"; exit 1 ;;
            esac
            ;;
        *)
            echo -e "${RED}未知选项: $1${NC}"
            show_help
            ;;
    esac
    
    # 确认卸载
    read -p "确定要卸载选中的服务？[y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "已取消卸载"
        exit 0
    fi
    
    # 执行卸载
    if [ "$uninstall_kernel" = true ]; then
        uninstall_kernel_monitor
        echo ""
    fi
    
    if [ "$uninstall_openclaw" = true ]; then
        uninstall_openclaw_monitor
        echo ""
    fi
    
    # 重新加载 systemd
    echo -e "${YELLOW}>>> 重新加载 systemd...${NC}"
    systemctl daemon-reload
    
    # 询问是否删除日志和数据
    read -p "是否同时删除日志和缓存数据？[y/N] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if [ "$uninstall_kernel" = true ]; then
            rm -f /var/log/kernel-monitor.log
            rm -rf /var/lib/kernel-monitor
            echo -e "${GREEN}    -> 内核监控日志和数据已删除${NC}"
        fi
        if [ "$uninstall_openclaw" = true ]; then
            rm -f /var/log/openclaw-monitor.log
            rm -rf /var/lib/openclaw
            echo -e "${GREEN}    -> OpenClaw 监控日志和数据已删除${NC}"
        fi
    fi
    
    echo ""
    echo -e "${GREEN}卸载完成！${NC}"
}

main "$@"