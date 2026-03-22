# 监控服务使用指南

## 一、安装

```bash
cd /home/changxiaoyi/CXY_2026/Kernel_Mode/kernel_systemd
sudo ./install.sh
```

选择要安装的服务：
- `1` - 内核监控
- `2` - OpenClaw 监控  
- `3` - 全部安装

## 二、服务管理命令

### 内核监控

| 操作 | 命令 |
|------|------|
| 启动 | `sudo systemctl start kernel-monitor` |
| 停止 | `sudo systemctl stop kernel-monitor` |
| 重启 | `sudo systemctl restart kernel-monitor` |
| 状态 | `sudo systemctl status kernel-monitor` |
| 开机自启 | `sudo systemctl enable kernel-monitor` |
| 禁用自启 | `sudo systemctl disable kernel-monitor` |
| 查看日志 | `sudo journalctl -u kernel-monitor -f` |

### OpenClaw 监控

| 操作 | 命令 |
|------|------|
| 启动 | `sudo systemctl start openclaw-monitor` |
| 停止 | `sudo systemctl stop openclaw-monitor` |
| 重启 | `sudo systemctl restart openclaw-monitor` |
| 状态 | `sudo systemctl status openclaw-monitor` |
| 查看日志 | `sudo journalctl -u openclaw-monitor -f` |

## 三、手动查看状态

```bash
# 查看完整系统状态
sudo /usr/local/bin/kernel-monitor.sh status

# 单独查看各项
sudo /usr/local/bin/kernel-monitor.sh cpu      # CPU 状态
sudo /usr/local/bin/kernel-monitor.sh memory   # 内存状态
sudo /usr/local/bin/kernel-monitor.sh disk     # 磁盘状态
sudo /usr/local/bin/kernel-monitor.sh network  # 网络状态
sudo /usr/local/bin/kernel-monitor.sh temp     # 温度
sudo /usr/local/bin/kernel-monitor.sh gpu      # GPU 状态
sudo /usr/local/bin/kernel-monitor.sh kernel   # 内核信息
```

## 四、卸载

```bash
sudo ./uninstall.sh
```

## 五、文件位置

| 内容 | 路径 |
|------|------|
| 内核监控脚本 | `/usr/local/bin/kernel-monitor.sh` |
| OpenClaw 监控脚本 | `/usr/local/bin/openclaw-monitor.sh` |
| 内核监控日志 | `/var/log/kernel-monitor.log` |
| OpenClaw 监控日志 | `/var/log/openclaw-monitor.log` |
| 历史数据 | `/var/lib/kernel-monitor/history/` |

## 六、修改配置

编辑监控脚本修改阈值和检查间隔：

```bash
sudo nano /usr/local/bin/kernel-monitor.sh
```

可修改参数：
```bash
CHECK_INTERVAL=10          # 检查间隔（秒）
CPU_WARN_THRESHOLD=80      # CPU 警告阈值 %
MEM_WARN_THRESHOLD=85      # 内存警告阈值 %
DISK_WARN_THRESHOLD=85     # 磁盘警告阈值 %
TEMP_WARN_THRESHOLD=70     # 温度警告阈值 °C
```

修改后重启服务：
```bash
sudo systemctl restart kernel-monitor
```

## 七、依赖安装（可选）

```bash
# 温度监控
sudo pacman -S lm_sensors
sudo sensors-detect

# CPU 统计
sudo pacman -S sysstat
```

## 八、故障排查

```bash
# 服务是否运行
systemctl is-active kernel-monitor

# 服务是否自启
systemctl is-enabled kernel-monitor

# 查看最近 50 条日志
sudo journalctl -u kernel-monitor -n 50

# 实时日志
sudo journalctl -u kernel-monitor -f
```
