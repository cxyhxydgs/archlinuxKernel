# 监控服务集合

用于 Arch Linux 的系统监控服务，包含内核监控和 OpenClaw 监控。

## 快速开始

### 安装

```bash
cd /home/changxiaoyi/CXY_2026/Kernel_Mode/kernel_systemd
sudo ./install.sh
```

安装时可选择：
- 内核监控服务
- OpenClaw 监控服务
- 全部安装

**命令行参数：**
```bash
sudo ./install.sh --all       # 安装全部
sudo ./install.sh --kernel    # 仅安装内核监控
sudo ./install.sh --openclaw  # 仅安装 OpenClaw 监控
```

### 卸载

```bash
sudo ./uninstall.sh
```

---

## 内核监控服务

全面监控系统状态，包括 CPU、内存、温度、磁盘、网络、内核状态等。

### 监控项目

| 类别 | 监控内容 |
|------|----------|
| **CPU** | 使用率、各核心负载、系统负载 (Load Average) |
| **温度** | CPU 核心、GPU、thermal zone 温度 |
| **内存** | 内存使用率、Swap 使用率、可用内存 |
| **磁盘** | 空间使用率、I/O 统计 |
| **网络** | 流量统计、连接数、TIME-WAIT 检测 |
| **内核** | 版本、运行时间、进程数、文件描述符、中断统计 |
| **GPU** | NVIDIA/AMD/Intel GPU 温度和使用率 |
| **服务** | 失败的服务检测 |

### 告警阈值

| 项目 | 警告阈值 | 严重阈值 |
|------|---------|---------|
| CPU | 80% | 95% |
| 内存 | 85% | 95% |
| 磁盘 | 85% | 95% |
| 温度 | 70°C | 85°C |

### 服务管理

```bash
# 启动/停止/重启
sudo systemctl start kernel-monitor
sudo systemctl stop kernel-monitor
sudo systemctl restart kernel-monitor

# 查看状态
sudo systemctl status kernel-monitor

# 开机自启
sudo systemctl enable kernel-monitor
sudo systemctl disable kernel-monitor

# 查看日志
sudo journalctl -u kernel-monitor -f
```

### 手动运行

```bash
# 查看完整状态
sudo /usr/local/bin/kernel-monitor.sh status

# 单独查看各项
sudo /usr/local/bin/kernel-monitor.sh cpu
sudo /usr/local/bin/kernel-monitor.sh memory
sudo /usr/local/bin/kernel-monitor.sh disk
sudo /usr/local/bin/kernel-monitor.sh network
sudo /usr/local/bin/kernel-monitor.sh temp
sudo /usr/local/bin/kernel-monitor.sh kernel
sudo /usr/local/bin/kernel-monitor.sh gpu
```

### 配置参数

编辑 `/usr/local/bin/kernel-monitor.sh` 修改：

```bash
CHECK_INTERVAL=10          # 检查间隔（秒）
HISTORY_KEEP_DAYS=7        # 历史数据保留天数
CPU_WARN_THRESHOLD=80      # CPU 警告阈值
MEM_WARN_THRESHOLD=85      # 内存警告阈值
TEMP_WARN_THRESHOLD=70     # 温度警告阈值
```

---

## OpenClaw 监控服务

监控 OpenClaw 进程及其动态路由变化。

### 监控功能

- 进程状态监控（CPU/内存/运行时间）
- 动态路由表变化检测
- 网络连接状态统计
- 系统健康检查

### 服务管理

```bash
sudo systemctl start openclaw-monitor
sudo systemctl stop openclaw-monitor
sudo systemctl status openclaw-monitor
sudo journalctl -u openclaw-monitor -f
```

---

## 文件位置

| 文件 | 内核监控 | OpenClaw 监控 |
|------|---------|--------------|
| 脚本 | `/usr/local/bin/kernel-monitor.sh` | `/usr/local/bin/openclaw-monitor.sh` |
| 服务 | `/etc/systemd/system/kernel-monitor.service` | `/etc/systemd/system/openclaw-monitor.service` |
| 日志 | `/var/log/kernel-monitor.log` | `/var/log/openclaw-monitor.log` |
| 数据 | `/var/lib/kernel-monitor/` | `/var/lib/openclaw/` |

---

## 故障排查

```bash
# 检查服务状态
systemctl is-active kernel-monitor
systemctl is-enabled kernel-monitor

# 检查脚本语法
bash -n /usr/local/bin/kernel-monitor.sh

# 查看最近日志
sudo journalctl -u kernel-monitor -n 50

# 手动测试脚本
sudo /usr/local/bin/kernel-monitor.sh status
```

## 依赖

推荐安装以下工具以获得完整监控功能：

```bash
# 温度监控
sudo pacman -S lm_sensors
sudo sensors-detect

# CPU 统计
sudo pacman -S sysstat

# NVIDIA GPU（如有）
sudo pacman -S nvidia-utils
```