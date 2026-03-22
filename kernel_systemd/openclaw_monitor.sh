#!/bin/bash
#
# OpenClaw 动态路由监控脚本
# 用于监控 OpenClaw 进程状态及其网络路由变化
#

set -e

# 配置参数
SCRIPT_NAME="openclaw-monitor"
LOG_FILE="/var/log/openclaw-monitor.log"
PID_FILE="/var/run/openclaw-monitor.pid"
CHECK_INTERVAL=5
OPENCLAW_PROCESS="openclaw"
ROUTE_TABLE_FILE="/var/lib/openclaw/routes.db"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] [$level] $message" >> "$LOG_FILE"
    
    case "$level" in
        ERROR)
            echo -e "${RED}[$timestamp] [$level] $message${NC}" >&2
            ;;
        WARN)
            echo -e "${YELLOW}[$timestamp] [$level] $message${NC}" >&2
            ;;
        INFO)
            echo -e "${GREEN}[$timestamp] [$level] $message${NC}"
            ;;
    esac
}

# 初始化
init() {
    # 确保 log 目录存在
    mkdir -p "$(dirname "$LOG_FILE")"
    mkdir -p "$(dirname "$ROUTE_TABLE_FILE")"
    
    # 创建 PID 文件
    echo $$ > "$PID_FILE"
    
    log "INFO" "OpenClaw 监控服务启动 (PID: $$)"
}

# 清理函数
cleanup() {
    log "INFO" "OpenClaw 监控服务停止"
    rm -f "$PID_FILE"
    exit 0
}

# 检查 OpenClaw 进程状态
check_process() {
    local pid=$(pgrep -x "$OPENCLAW_PROCESS" | head -1)
    
    if [ -z "$pid" ]; then
        log "WARN" "OpenClaw 进程未运行"
        return 1
    fi
    
    # 获取进程信息
    local cpu_usage=$(ps -p "$pid" -o %cpu --no-headers | tr -d ' ')
    local mem_usage=$(ps -p "$pid" -o %mem --no-headers | tr -d ' ')
    local uptime=$(ps -p "$pid" -o etimes --no-headers | tr -d ' ')
    
    log "INFO" "OpenClaw 进程运行中 [PID: $pid, CPU: ${cpu_usage}%, MEM: ${mem_usage}%, Uptime: ${uptime}s]"
    
    return 0
}

# 监控动态路由表
monitor_routes() {
    # 获取当前路由表
    local current_routes=$(ip route show 2>/dev/null | grep -E "default|openclaw" || true)
    local previous_routes=""
    
    if [ -f "$ROUTE_TABLE_FILE" ]; then
        previous_routes=$(cat "$ROUTE_TABLE_FILE")
    fi
    
    # 检测路由变化
    if [ "$current_routes" != "$previous_routes" ]; then
        log "INFO" "检测到路由表变化:"
        
        # 分析新增的路由
        if [ -n "$current_routes" ]; then
            while IFS= read -r route; do
                if ! echo "$previous_routes" | grep -qF "$route"; then
                    log "INFO" "  新增路由: $route"
                fi
            done <<< "$current_routes"
        fi
        
        # 分析删除的路由
        if [ -n "$previous_routes" ]; then
            while IFS= read -r route; do
                if ! echo "$current_routes" | grep -qF "$route"; then
                    log "INFO" "  删除路由: $route"
                fi
            done <<< "$previous_routes"
        fi
        
        # 更新路由缓存
        echo "$current_routes" > "$ROUTE_TABLE_FILE"
    fi
}

# 监控网络连接状态
monitor_connections() {
    local connections=$(ss -tnp 2>/dev/null | grep "$OPENCLAW_PROCESS" || true)
    local conn_count=$(echo "$connections" | grep -c . || echo "0")
    
    log "INFO" "活跃连接数: $conn_count"
    
    # 检查异常连接状态
    local estab_count=$(echo "$connections" | grep -c "ESTAB" || echo "0")
    local time_wait=$(echo "$connections" | grep -c "TIME-WAIT" || echo "0")
    
    if [ "$time_wait" -gt 50 ]; then
        log "WARN" "TIME-WAIT 连接过多: $time_wait"
    fi
}

# 健康检查
health_check() {
    # 检查 CPU 使用率
    if pgrep -x "$OPENCLAW_PROCESS" > /dev/null; then
        local cpu=$(ps -C "$OPENCLAW_PROCESS" -o %cpu --no-headers | awk '{sum+=$1} END {print sum}')
        
        # 使用 bc 进行浮点数比较，如果 bc 不可用则使用 awk
        if command -v bc &> /dev/null; then
            if [ $(echo "$cpu > 80" | bc -l) -eq 1 ]; then
                log "WARN" "CPU 使用率过高: ${cpu}%"
            fi
        else
            if [ $(awk "BEGIN {print ($cpu > 80) ? 1 : 0}") -eq 1 ]; then
                log "WARN" "CPU 使用率过高: ${cpu}%"
            fi
        fi
    fi
    
    # 检查内存使用
    local mem_available=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
    if [ -n "$mem_available" ] && [ "$mem_available" -lt 1048576 ]; then  # 小于 1GB
        log "WARN" "可用内存不足: $((mem_available / 1024)) MB"
    fi
}

# 主监控循环
main() {
    init
    
    # 设置信号处理
    trap cleanup SIGTERM SIGINT SIGQUIT
    
    log "INFO" "开始监控循环 (检查间隔: ${CHECK_INTERVAL}s)"
    
    while true; do
        # 检查进程状态
        if check_process; then
            # 进程运行中，执行监控任务
            monitor_routes
            monitor_connections
            health_check
        else
            # 进程未运行，尝试重启或通知
            log "WARN" "OpenClaw 进程异常，等待恢复..."
        fi
        
        sleep "$CHECK_INTERVAL"
    done
}

# 显示状态
status() {
    echo "=== OpenClaw 监控状态 ==="
    echo ""
    
    # 进程状态
    if pgrep -x "$OPENCLAW_PROCESS" > /dev/null; then
        echo -e "进程状态: ${GREEN}运行中${NC}"
        ps -C "$OPENCLAW_PROCESS" -o pid,ppid,%cpu,%mem,etime,cmd --no-headers
    else
        echo -e "进程状态: ${RED}未运行${NC}"
    fi
    
    echo ""
    echo "=== 当前路由表 ==="
    ip route show | grep -E "default|openclaw" || echo "无相关路由"
    
    echo ""
    echo "=== 最近日志 ==="
    tail -10 "$LOG_FILE" 2>/dev/null || echo "无日志"
}

# 根据参数执行不同操作
case "${1:-start}" in
    start)
        main
        ;;
    status)
        status
        ;;
    check)
        check_process && monitor_routes && monitor_connections && health_check
        ;;
    *)
        echo "用法: $0 {start|status|check}"
        exit 1
        ;;
esac
