# Linux内核学习Demo集合

这是一套完整的Linux内核开发学习Demo，从基础到进阶，涵盖内核模块、设备驱动和硬件交互。

## 📁 项目结构

```
workspace/
├── kernel_learning_guide.md      # 完整学习路径
├── kernel_hello/                 # Demo 1: Hello World模块
│   ├── hello.c
│   ├── Makefile
│   └── README.md
├── kernel_char_device/           # Demo 2: 字符设备驱动
│   ├── chardev.c
│   ├── Makefile
│   └── README.md
├── kernel_procfs/               # Demo 3: Proc文件系统
│   ├── procfs.c
│   ├── Makefile
│   └── README.md
└── kernel_sysfs/                # Demo 4: Sysfs接口
    ├── sysfs.c
    ├── Makefile
    └── README.md
```

## 🚀 快速开始

### 环境要求

- Linux 内核头文件
- GCC编译器
- Make
- sudo权限（用于加载模块）

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)

# Arch Linux
sudo pacman -S base-devel linux-headers

# CentOS/RHEL
sudo yum install gcc make kernel-devel
```

### 编译和运行

每个demo都有独立的目录和Makefile：

```bash
# 进入任意demo目录
cd kernel_hello

# 编译
make

# 加载模块
sudo insmod hello.ko

# 查看日志
dmesg | tail -10

# 卸载模块
sudo rmmod hello
```

## 📚 学习路径

### 第一阶段：基础模块（1-3天）

1. **kernel_hello** - Hello World模块
   - 最简单的内核模块
   - 学习：模块初始化、printk、模块参数
   - ⏱️ 预计时间：30分钟

2. **kernel_char_device** - 字符设备驱动
   - 创建/dev节点
   - 学习：file_operations、cdev、设备号
   - ⏱️ 预计时间：2-3小时

3. **kernel_procfs** - Proc文件系统
   - 在/proc创建接口
   - 学习：seq_file、proc_create
   - ⏱️ 预计时间：1-2小时

### 第二阶段：设备模型（3-5天）

4. **kernel_sysfs** - Sysfs接口
   - 在/sys创建接口
   - 学习：kobject、attribute、kset
   - ⏱️ 预计时间：1-2小时

**（更多demo正在开发中...）**

- kernel_device_tree - 设备树解析
- kernel_interrupt - 中断处理
- kernel_gpio - GPIO控制
- kernel_spi - SPI驱动

## 🔧 常用命令

### 模块管理

```bash
# 加载模块
sudo insmod module.ko

# 卸载模块
sudo rmmod module

# 查看已加载模块
lsmod | grep module

# 查看模块信息
modinfo module.ko
```

### 日志查看

```bash
# 查看内核日志
dmesg | tail -20

# 实时监控
dmesg -w

# 过滤特定内容
dmesg | grep "关键词"
```

### 设备管理

```bash
# 查看字符设备
ls -l /dev/

# 查看proc文件
ls /proc/

# 查看sysfs
ls /sys/kernel/
```

## ⚠️ 注意事项

1. **安全第一**
   - 始终在虚拟机或开发板上测试
   - 不要在生产环境加载未验证的模块
   - 内核代码bug可能导致系统崩溃

2. **内核版本**
   - 不同内核版本API可能变化
   - 使用 `uname -r` 查看内核版本
   - 参考对应版本的文档

3. **编译问题**
   - 确保内核头文件版本匹配
   - 检查Makefile中的路径配置
   - 查看完整编译错误信息

4. **权限管理**
   - 大多数操作需要root权限
   - 谨慎使用sudo
   - 注意设备文件权限

## 📖 推荐资源

### 书籍

- 《Linux设备驱动程序》(LDD3) - 经典入门
- 《深入理解Linux内核》 - 深入理解内核机制
- 《Linux内核设计与实现》 - 设计思想

### 在线资源

- Linux内核文档：`Documentation/` 目录
- Linux Kernel Mailing List (LKML)
- Kernel Newbies：https://kernelnewbies.org/
- LWN.net：https://lwn.net/Kernel/

### 源码参考

- 实际驱动：`/usr/src/linux*/drivers/`
- 示例代码：`/usr/src/linux*/samples/`
- 内核头文件：`/usr/src/linux*/include/linux/`

## 🛠️ 调试技巧

### 1. 使用 dmesg

```bash
# 查看最近的日志
dmesg | tail -20

# 实时监控
dmesg -w

# 过滤模块日志
dmesg | grep "模块名"
```

### 2. 使用 strace

```bash
# 跟踪系统调用
strace -e open,read,write cat /dev/chardev
```

### 3. 使用 /proc 接口

```bash
# 查看模块信息
cat /proc/modules

# 查看设备号分配
cat /proc/devices

# 查看中断统计
cat /proc/interrupts
```

### 4. 使用 gdb (高级)

```bash
# 调试内核（需要特殊配置）
sudo gdb vmlinux /proc/kcore
```

## ❓ 常见问题

### Q1: 编译失败，提示找不到内核源码

```bash
# 安装内核头文件
sudo apt-get install linux-headers-$(uname -r)

# 或手动设置路径
export KDIR=/lib/modules/$(uname -r)/build
```

### Q2: insmod失败，提示"Invalid module format"

检查内核版本匹配：
```bash
uname -r
modinfo module.ko | grep vermagic
```

### Q3: 无法访问 /dev/ 设备

权限问题：
```bash
# 修改权限
sudo chmod 666 /dev/chardev

# 或添加到相关用户组
```

### Q4: 系统崩溃怎么办

重启并查看日志：
```bash
journalctl -k | tail -100
```

## 🎯 学习建议

1. **按顺序学习**：从简单到复杂，不要跳跃
2. **动手实践**：每个demo都要编译、加载、测试
3. **阅读源码**：理解代码背后的原理
4. **修改实验**：尝试修改代码，观察效果
5. **记录笔记**：记录遇到的问题和解决方案

## 📝 下一步

完成基础demo后，可以探索：

- 高级驱动：网络设备、块设备、USB驱动
- 内核子系统：调度器、内存管理、文件系统
- 实时内核：RT-Linux、抢占式调度
- 设备树：嵌入式系统常用
- 内核调试：ftrace、perf、kprobe

## 🤝 贡献

如果你有改进建议或新想法，欢迎反馈！

## 📄 许可证

所有demo代码均为教育目的，可自由使用和修改。

---

**祝你学习愉快！** 🚀

有问题或建议？欢迎反馈！
