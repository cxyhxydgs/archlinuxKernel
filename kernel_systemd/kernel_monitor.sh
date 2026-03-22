#!/bin/bash
#
# 系统内核监控脚本
# 监控 CPU、内存、温度、磁盘、网络、内核状态等
#

set -e

# 配置参数
SCRIPT_NAME="kernel-monitor"
LOG_FILE="/var/log/kernel-monitor.log"
PID_FILE="/var/run/kernel-monitor.pid"
STATUS_FILE="/var/lib/kernel-monitor/status.json"
HISTORY_DIR="/var/lib/kernel-monitor/history"
CHECK_INTERVAL=10
HISTORY_KEEP_DAYS=7

# 告警阈值
CPU_WARN_THRESHOLD=80
CPU_CRIT_THRESHOLD=95
MEM_WARN_THRESHOLD=85
MEM_CRIT_THRESHOLD=95
DISK_WARN_THRESHOLD=85
DISK_CRIT_THRESHOLD=95
TEMP_WARN_THRESHOLD=70
TEMP_CRIT_THRESHOLD=85

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 日志函数
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] [$level] $message" >> "$LOG_FILE"
    
    case "$level" in
        ERROR|CRITICAL)
            echo -e "${RED}[$timestamp] [$level] $message${NC}" >&2
            ;;
        WARN)
            echo -e "${YELLOW}[$timestamp] [$level] $message${NC}" >&2
            ;;
        INFO)
            echo -e "${GREEN}[$timestamp] [$level] $message${NC}"
            ;;
        DEBUG)
            echo -e "${CYAN}[$timestamp] [$level] $message${NC}"
            ;;
    esac
}

# 初始化
init() {
    mkdir -p "$(dirname "$LOG_FILE")"
    mkdir -p "$(dirname "$STATUS_FILE")"
    mkdir -p "$HISTORY_DIR"
    
    echo $$ > "$PID_FILE"
    
    log "INFO" "========================================="
    log "INFO" "系统内核监控服务启动 (PID: $$)"
    log "INFO" "内核版本: $(uname -r)"
    log "INFO" "系统: $(uname -s) $(uname -m)"
    log "INFO" "========================================="
}

# 清理函数
cleanup() {
    log "INFO" "系统内核监控服务停止"
    rm -f "$PID_FILE"
    exit 0
}

# ==================== 监控函数 ====================

# CPU 监控
monitor_cpu() {
    # CPU 使用率
    local cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1 2>/dev/null || echo "0")
    
    # 如果 top 输出格式不同，尝试其他方法
    if [ "$cpu_usage" = "0" ] || [ -z "$cpu_usage" ]; then
        cpu_usage=$(awk '/cpu /{u=$2; n=$3; s=$4; i=$5; total=u+n+s+i; if(total>0) printf "%.1f", (u+n+s)*100/total}' /proc/stat 2>/dev/null || echo "0")
    fi
    
    # 各核心使用率
    local core_usage=$(mpstat -P ALL 1 1 2>/dev/null | awk '/Average/ && $2 !~ /CPU/ {printf "%s:%.1f%% ", $2, 100-$NF}' || echo "N/A")
    
    # 负载
    local load_avg=$(awk '{print $1, $2, $3}' /proc/loadavg)
    local load_1=$(echo "$load_avg" | awk '{print $1}')
    local cpu_cores=$(nproc)
    
    # 检查告警
    if [ $(awk "BEGIN {print ($cpu_usage > $CPU_CRIT_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "CRITICAL" "CPU 使用率过高: ${cpu_usage}%"
    elif [ $(awk "BEGIN {print ($cpu_usage > $CPU_WARN_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "WARN" "CPU 使用率警告: ${cpu_usage}%"
    fi
    
    # 负载告警
    if [ $(awk "BEGIN {print ($load_1 > $cpu_cores) ? 1 : 0}") -eq 1 ]; then
        log "WARN" "系统负载过高: $load_avg (核心数: $cpu_cores)"
    fi
    
    echo "cpu_usage=$cpu_usage"
    echo "load_avg=$load_avg"
    echo "cpu_cores=$cpu_cores"
}

# 温度监控
monitor_temperature() {
    local temps=""
    local max_temp=0
    
    # 使用 sensors 命令（需要 lm_sensors）
    if command -v sensors &>/dev/null; then
        while IFS= read -r line; do
            if [[ $line =~ ([A-Za-z0-9]+):[[:space:]]*\+([0-9.]+) ]]; then
                local label="${BASH_REMATCH[1]}"
                local temp="${BASH_REMATCH[2]}"
                temps+="${label}:${temp}°C "
                
                if [ $(awk "BEGIN {print ($temp > $max_temp) ? $temp : $max_temp}"} ) -gt 0 ]; then
                    max_temp=$(awk "BEGIN {print ($temp > $max_temp) ? $temp : $max_temp}")
                fi
            fi
        done < <(sensors 2>/dev/null | grep -E "Core|Package|temp[0-9]")
    fi
    
    # 直接读取 thermal zone
    for zone in /sys/class/thermal/thermal_zone*; do
        if [ -d "$zone" ]; then
            local temp=$(cat "$zone/temp" 2>/dev/null || echo "0")
            if [ "$temp" != "0" ]; then
                local temp_c=$(awk "BEGIN {printf \"%.1f\", $temp/1000}")
                local type=$(cat "$zone/type" 2>/dev/null || echo "unknown")
                temps+="${type}:${temp_c}°C "
                
                if [ $(awk "BEGIN {print ($temp_c > $max_temp) ? 1 : 0}") -eq 1 ]; then
                    max_temp=$temp_c
                fi
            fi
        fi
    done
    
    # 温度告警
    if [ $(awk "BEGIN {print ($max_temp > $TEMP_CRIT_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "CRITICAL" "温度过高: ${max_temp}°C"
    elif [ $(awk "BEGIN {print ($max_temp > $TEMP_WARN_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "WARN" "温度警告: ${max_temp}°C"
    fi
    
    echo "temperatures=${temps:-N/A}"
    echo "max_temp=$max_temp"
}

# 内存监控
monitor_memory() {
    local mem_info=$(free -b | awk '/Mem:/ {printf "total=%d used=%d free=%d available=%d", $2, $3, $4, $7}')
    local swap_info=$(free -b | awk '/Swap:/ {printf "total=%d used=%d free=%d", $2, $3, $4}')
    
    local mem_total=$(echo "$mem_info" | awk -F'[= ]' '{print $2}')
    local mem_used=$(echo "$mem_info" | awk -F'[= ]' '{print $4}')
    local mem_available=$(echo "$mem_info" | awk -F'[= ]' '{print $8}')
    local mem_percent=$(awk "BEGIN {printf \"%.1f\", ($mem_used/$mem_total)*100}")
    
    local swap_total=$(echo "$swap_info" | awk -F'[= ]' '{print $2}')
    local swap_used=$(echo "$swap_info" | awk -F'[= ]' '{print $4}')
    local swap_percent=0
    if [ "$swap_total" -gt 0 ]; then
        swap_percent=$(awk "BEGIN {printf \"%.1f\", ($swap_used/$swap_total)*100}")
    fi
    
    # 内存告警
    if [ $(awk "BEGIN {print ($mem_percent > $MEM_CRIT_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "CRITICAL" "内存使用率过高: ${mem_percent}%"
    elif [ $(awk "BEGIN {print ($mem_percent > $MEM_WARN_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
        log "WARN" "内存使用率警告: ${mem_percent}%"
    fi
    
    # 转换为人类可读格式
    local mem_total_hr=$(numfmt --to=iec-i --suffix=B "$mem_total" 2>/dev/null || echo "$mem_total")
    local mem_used_hr=$(numfmt --to=iec-i --suffix=B "$mem_used" 2>/dev/null || echo "$mem_used")
    local mem_available_hr=$(numfmt --to=iec-i --suffix=B "$mem_available" 2>/dev/null || echo "$mem_available")
    
    echo "mem_total=$mem_total_hr"
    echo "mem_used=$mem_used_hr"
    echo "mem_available=$mem_available_hr"
    echo "mem_percent=$mem_percent%"
    echo "swap_percent=$swap_percent%"
}

# 磁盘监控
monitor_disk() {
    local disk_info=""
    local disk_io=""
    
    # 磁盘空间
    while IFS= read -r line; do
        local fs=$(echo "$line" | awk '{print $1}')
        local size=$(echo "$line" | awk '{print $2}')
        local used=$(echo "$line" | awk '{print $3}')
        local avail=$(echo "$line" | awk '{print $4}')
        local pcent=$(echo "$line" | awk '{print $5}' | tr -d '%')
        local mount=$(echo "$line" | awk '{print $6}')
        
        disk_info+="${mount}:${pcent}% "
        
        if [ -n "$pcent" ] && [ $(awk "BEGIN {print ($pcent > $DISK_WARN_THRESHOLD) ? 1 : 0}") -eq 1 ]; then
            log "WARN" "磁盘空间不足: $mount 使用 ${pcent}%"
        fi
    done < <(df -h | grep -E "^/dev" | grep -v tmpfs)
    
    # 磁盘 I/O
    if [ -f /proc/diskstats ]; then
        disk_io=$(awk '{
            if($3 ~ /^(sd|nvme|vd|hd)/) {
                read = $6
                write = $10
                printf "%s:R%dW%d ", $3, read, write
            }
        }' /proc/diskstats)
    fi
    
    echo "disk_usage=$disk_info"
    echo "disk_io=$disk_io"
}

# 网络监控
monitor_network() {
    local net_info=""
    local net_connections=0
    
    # 网络流量
    for iface in /sys/class/net/*; do
        if [ -d "$iface" ]; then
            local name=$(basename "$iface")
            local rx=$(cat "$iface/statistics/rx_bytes" 2>/dev/null || echo "0")
            local tx=$(cat "$iface/statistics/tx_bytes" 2>/dev/null || echo "0")
            local rx_hr=$(numfmt --to=iec-i --suffix=B "$rx" 2>/dev/null || echo "$rx")
            local tx_hr=$(numfmt --to=iec-i --suffix=B "$tx" 2>/dev/null || echo "$tx")
            net_info+="${name}:↓${rx_hr} ↑${tx_hr} "
        fi
    done
    
    # 连接统计
    net_connections=$(ss -tn 2>/dev/null | wc -l)
    local established=$(ss -tn state established 2>/dev/null | wc -l)
    local time_wait=$(ss -tn state time-wait 2>/dev/null | wc -l)
    
    if [ "$time_wait" -gt 100 ]; then
        log "WARN" "TIME-WAIT 连接过多: $time_wait"
    fi
    
    echo "interfaces=$net_info"
    echo "connections=$net_connections"
    echo "established=$established"
    echo "time_wait=$time_wait"
}

# 内核状态监控
monitor_kernel() {
    # 内核版本
    local kernel_version=$(uname -r)
    
    # 运行时间
    local uptime=$(awk '{print int($1)}' /proc/uptime)
    local uptime_hr=$(printf '%dd %dh %dm\n' $((uptime/86400)) $((uptime%86400/3600)) $((uptime%3600/60)))
    
    # 进程数
    local process_count=$(ls -1 /proc | grep -E '^[0-9]+$' | wc -l)
    
    # 文件描述符
    local fd_total=$(cat /proc/sys/fs/file-nr 2>/dev/null | awk '{print $1}')
    local fd_limit=$(cat /proc/sys/fs/file-nr 2>/dev/null | awk '{print $3}')
    local fd_percent=$(awk "BEGIN {printf \"%.1f\", ($fd_total/$fd_limit)*100}")
    
    # 内核模块数
    local module_count=$(lsmod | wc -l)
    
    # 内核错误日志
    local kernel_errors=$(dmesg -l err,crit,alert,emerg 2>/dev/null | tail -5 | head -1 | cut -d']' -f2- || echo "")
    if [ -n "$kernel_errors" ]; then
        log "WARN" "内核错误: $kernel_errors"
    fi
    
    # 中断统计
    local interrupts=$(awk 'NR>1 {sum+=$2} END {print sum}' /proc/interrupts 2>/dev/null || echo "0")
    
    # 上下文切换
    local ctxt=$(awk '{print $1}' /proc/stat | grep -A1 ctxt | tail -1 || echo "0")
    
    echo "kernel_version=$kernel_version"
    echo "uptime=$uptime_hr"
    echo "processes=$process_count"
    echo "fd_used=$fd_total/$fd_limit ($fd_percent%)"
    echo "modules=$module_count"
    echo "interrupts=$interrupts"
}

# GPU 监控（如果可用）
monitor_gpu() {
    local gpu_info=""
    
    # NVIDIA GPU
    if command -v nvidia-smi &>/dev/null; then
        local nvidia_temp=$(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits 2>/dev/null | head -1)
        local nvidia_util=$(nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null | head -1)
        local nvidia_mem=$(nvidia-smi --query-gpu=memory.used,memory.total --format=csv,noheader,nounits 2>/dev/null | head -1)
        
        if [ -n "$nvidia_temp" ]; then
            gpu_info+="NVIDIA: ${nvidia_temp}°C ${nvidia_util}% util"
        fi
    fi
    
    # AMD GPU
    if [ -f /sys/class/drm/card0/device/hwmon/hwmon*/temp1_input ]; then
        local amd_temp=$(cat /sys/class/drm/card0/device/hwmon/hwmon*/temp1_input 2>/dev/null)
        if [ -n "$amd_temp" ]; then
            local amd_temp_c=$(awk "BEGIN {printf \"%.0f\", $amd_temp/1000}")
            gpu_info+="AMD: ${amd_temp_c}°C "
        fi
    fi
    
    # Intel GPU
    if [ -f /sys/class/drm/card0/gt_cur_freq ]; then
        local intel_freq=$(cat /sys/class/drm/card0/gt_cur_freq 2>/dev/null)
        gpu_info+="Intel: ${intel_freq}MHz "
    fi
    
    echo "gpu_info=${gpu_info:-N/A}"
}

# 系统服务状态
monitor_services() {
    local failed_services=$(systemctl --failed --no-legend --plain 2>/dev/null | awk '{print $1}')
    
    if [ -n "$failed_services" ]; then
        log "WARN" "发现失败的服务: $failed_services"
    fi
    
    echo "failed_services=${failed_services:-无}"
}

# 生成状态报告
generate_status() {
    local timestamp=$(date -Iseconds)
    
    log "INFO" "---------- 系统状态报告 ----------"
    
    # CPU
    local cpu_data=$(monitor_cpu)
    log "INFO" "CPU: $(echo "$cpu_data" | grep cpu_usage | cut -d= -f2)% | 负载: $(echo "$cpu_data" | grep load_avg | cut -d= -f2)"
    
    # 温度
    local temp_data=$(monitor_temperature)
    log "INFO" "温度: $(echo "$temp_data" | grep temperatures | cut -d= -f2)"
    
    # 内存
    local mem_data=$(monitor_memory)
    log "INFO" "内存: $(echo "$mem_data" | grep mem_percent | cut -d= -f2) | Swap: $(echo "$mem_data" | grep swap_percent | cut -d= -f2)"
    
    # 磁盘
    local disk_data=$(monitor_disk)
    log "INFO" "磁盘: $(echo "$disk_data" | grep disk_usage | cut -d= -f2)"
    
    # 网络
    local net_data=$(monitor_network)
    log "INFO" "网络: $(echo "$net_data" | grep connections | cut -d= -f2) 连接"
    
    # 内核
    local kernel_data=$(monitor_kernel)
    log "INFO" "内核: $(echo "$kernel_data" | grep kernel_version | cut -d= -f2) | 运行时间: $(echo "$kernel_data" | grep uptime | cut -d= -f2)"
    
    # GPU
    local gpu_data=$(monitor_gpu)
    if [ "$(echo "$gpu_data" | cut -d= -f2)" != "N/A" ]; then
        log "INFO" "GPU: $(echo "$gpu_data" | cut -d= -f2)"
    fi
    
    log "INFO" "----------------------------------"
    
    # 保存历史数据
    local history_file="$HISTORY_DIR/$(date +%Y%m%d).log"
    echo "[$timestamp] CPU:$(echo "$cpu_data" | grep cpu_usage | cut -d= -f2)% MEM:$(echo "$mem_data" | grep mem_percent | cut -d= -f2)" >> "$history_file"
    
    # 清理旧历史
    find "$HISTORY_DIR" -name "*.log" -mtime +$HISTORY_KEEP_DAYS -delete 2>/dev/null
}

# 显示完整状态
show_status() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       系统内核监控状态报告${NC}"
    echo -e "${CYAN}=========================================${NC}"
    echo ""
    
    echo -e "${BLUE}[CPU]${NC}"
    monitor_cpu | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[温度]${NC}"
    monitor_temperature | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[内存]${NC}"
    monitor_memory | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[磁盘]${NC}"
    monitor_disk | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[网络]${NC}"
    monitor_network | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[内核]${NC}"
    monitor_kernel | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[GPU]${NC}"
    monitor_gpu | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${BLUE}[服务]${NC}"
    monitor_services | while read line; do echo "  $line"; done
    echo ""
    
    echo -e "${CYAN}=========================================${NC}"
}

# 主监控循环
main() {
    init
    trap cleanup SIGTERM SIGINT SIGQUIT
    
    log "INFO" "监控间隔: ${CHECK_INTERVAL}秒"
    
    while true; do
        generate_status
        sleep "$CHECK_INTERVAL"
    done
}

# 根据参数执行
case "${1:-start}" in
    start)
        main
        ;;
    status)
        show_status
        ;;
    cpu)
        monitor_cpu | while read line; do echo "$line"; done
        ;;
    memory|mem)
        monitor_memory | while read line; do echo "$line"; done
        ;;
    disk)
        monitor_disk | while read line; do echo "$line"; done
        ;;
    network|net)
        monitor_network | while read line; do echo "$line"; done
        ;;
    temp)
        monitor_temperature | while read line; do echo "$line"; done
        ;;
    kernel)
        monitor_kernel | while read line; do echo "$line"; done
        ;;
    gpu)
        monitor_gpu | while read line; do echo "$line"; done
        ;;
    *)
        echo "用法: $0 {start|status|cpu|memory|disk|network|temp|kernel|gpu}"
        exit 1
        ;;
esac
