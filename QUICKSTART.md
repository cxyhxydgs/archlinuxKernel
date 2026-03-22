# Linux内核开发 - 完整学习指南

这是一套完整的Linux内核开发Demo集合，从基础到进阶，共8个Demo。

## 📚 Demo列表

| 序号 | Demo名称 | 难度 | 预计时间 | 核心概念 |
|------|----------|------|----------|----------|
| 1 | kernel_hello | ⭐ | 30分钟 | 模块初始化、printk、模块参数 |
| 2 | kernel_char_device | ⭐⭐ | 2-3小时 | 字符设备、file_operations、/dev/节点 |
| 3 | kernel_procfs | ⭐⭐ | 1-2小时 | Proc文件系统、seq_file接口 |
| 4 | kernel_sysfs | ⭐⭐ | 1-2小时 | Sysfs接口、kobject、属性 |
| 5 | kernel_device_tree | ⭐⭐⭐ | 2-3小时 | 设备树解析、platform驱动 |
| 6 | kernel_interrupt | ⭐⭐⭐ | 2-3小时 | 中断处理、tasklet、workqueue |
| 7 | kernel_gpio | ⭐⭐⭐ | 2-3小时 | GPIO控制、GPIO中断、sysfs |
| 8 | kernel_spi | ⭐⭐⭐⭐ | 3-4小时 | SPI驱动、SPI传输、设备树 |

## 🚀 快速开始

### 环境准备

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)

# Arch Linux
sudo pacman -S base-devel linux-headers

# CentOS/RHEL
sudo yum install gcc make kernel-devel
```

### 通用编译步骤

每个demo都遵循相同的模式：

```bash
# 1. 进入demo目录
cd kernel_<demo_name>

# 2. 编译
make

# 3. 加载模块
sudo insmod <demo_name>.ko

# 4. 查看日志
dmesg | tail -20

# 5. 卸载模块
sudo rmmod <demo_name>
```

---

## 📖 各Demo详细指南

### Demo 1: kernel_hello

**目标：** 学习最基础的内核模块

```bash
cd kernel_hello

# 编译
make

# 加载（带参数）
sudo insmod hello.ko count=5

# 查看日志
dmesg | tail -10

# 卸载
sudo rmmod hello

# 预期输出：
# [12345.100] === Hello模块开始初始化 ===
# [12345.101] 参数 count = 5
# [12345.102] Hello, Linux Kernel! [1/5]
# [12345.102] Hello, Linux Kernel! [2/5]
# ...
```

**学习重点：**
- `module_init()` 和 `module_exit()`
- `printk()` 内核日志
- `module_param()` 模块参数
- 模块元数据（MODULE_LICENSE等）

---

### Demo 2: kernel_char_device

**目标：** 创建字符设备驱动

```bash
cd kernel_char_device

# 编译
make

# 加载
sudo insmod chardev.ko

# 验证设备节点
ls -l /dev/chardev

# 测试写入
echo "Hello, Kernel!" | sudo tee /dev/chardev > /dev/null

# 测试读取
sudo cat /dev/chardev

# 卸载
sudo rmmod chardev
```

**学习重点：**
- `register_chrdev()` 注册字符设备
- `file_operations` 结构
- `copy_to_user()` 和 `copy_from_user()`
- 自动创建 /dev/ 节点

---

### Demo 3: kernel_procfs

**目标：** 在 /proc 创建接口

```bash
cd kernel_procfs

# 编译
make

# 加载
sudo insmod procfs.ko

# 读取接口
cat /proc/kernel_proc_demo

# 写入数据
echo "测试消息" | sudo tee /proc/kernel_proc_demo > /dev/null

# 再次读取
cat /proc/kernel_proc_demo

# 卸载
sudo rmmod procfs
```

**学习重点：**
- `proc_create()` 创建 /proc 文件
- `seq_file` 接口
- `single_open()` 和 `single_release()`
- 实现 `show` 和 `store` 函数

---

### Demo 4: kernel_sysfs

**目标：** 在 /sys 创建接口

```bash
cd kernel_sysfs

# 编译
make

# 加载
sudo insmod sysfs.ko

# 查看属性
ls -l /sys/kernel/demo_sysfs/

# 读取属性
cat /sys/kernel/demo_sysfs/int_value
cat /sys/kernel/demo_sysfs/str_value

# 写入属性
echo 100 | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null
echo "Hello" | sudo tee /sys/kernel/demo_sysfs/str_value > /dev/null

# 卸载
sudo rmmod sysfs
```

**学习重点：**
- `kobject` 结构
- `kobj_attribute` 属性
- `sysfs_create_group()` 批量创建
- `show` 和 `store` 函数

---

### Demo 5: kernel_device_tree

**目标：** 解析设备树

```bash
cd kernel_device_tree

# 编译
make

# 方案A：模拟模式（推荐x86用户）
sudo insmod devtree.ko dt_string_param="测试" dt_int_param=999
dmesg | tail -20

# 方案B：实际设备树（树莓派等）
# 1. 创建设备树覆盖
# 2. 编译为 .dtbo
# 3. 加载覆盖
# 4. 加载内核模块

# 卸载
sudo rmmod devtree
```

**学习重点：**
- `of_property_read_*()` 读取属性
- `platform_driver` 框架
- Compatible 匹配机制
- 设备树节点遍历

---

### Demo 6: kernel_interrupt

**目标：** 学习中断处理

```bash
cd kernel_interrupt

# 编译
make

# 加载
sudo insmod interrupt.ko

# 查看统计
cat /proc/irq_demo

# 查看系统中断
cat /proc/interrupts | head -20

# 卸载
sudo rmmod interrupt
```

**学习重点：**
- `request_irq()` 注册中断
- `tasklet` 软中断
- `workqueue` 工作队列
- 顶半部和底半部分离

---

### Demo 7: kernel_gpio

**目标：** GPIO控制

```bash
cd kernel_gpio

# 编译
make

# 方案A：模拟模式（x86）
sudo insmod gpio.ko gpio_out_num=999 gpio_in_num=888
dmesg | tail -30

# 方案B：树莓派实际GPIO
make rpi-test

# 方案C：用户空间GPIO（sysfs）
# 导出GPIO
echo 18 | sudo tee /sys/class/gpio/export
echo out | sudo tee /sys/class/gpio/gpio18/direction
echo 1 | sudo tee /sys/class/gpio/gpio18/value  # 高电平
echo 0 | sudo tee /sys/class/gpio/gpio18/value  # 低电平

# 卸载内核模块
sudo rmmod gpio
```

**学习重点：**
- `gpio_request()` 请求GPIO
- `gpio_direction_*()` 设置方向
- `gpio_get_value()` 和 `gpio_set_value()`
- `gpio_to_irq()` GPIO中断

---

### Demo 8: kernel_spi

**目标：** SPI设备驱动

```bash
cd kernel_spi

# 编译
make

# 加载
sudo insmod spi.ko
dmesg | tail -30

# 查看系统SPI
ls /sys/class/spi_master/
ls /dev/spidev*

# 树莓派测试
make rpi-test

# 卸载
sudo rmmod spi
```

**学习重点：**
- `spi_register_driver()` 注册驱动
- `spi_sync()` 同步传输
- `spi_write_then_read()` 简单传输
- SPI模式配置

---

## 🔧 常用命令速查

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

# 加载带参数的模块
sudo insmod module.ko param1=value1 param2=value2
```

### 日志查看

```bash
# 查看最近的内核日志
dmesg | tail -20

# 实时监控日志
dmesg -w

# 过滤特定内容
dmesg | grep "关键词"

# 清空日志
sudo dmesg -c
```

### 设备管理

```bash
# 查看字符设备
ls -l /dev/

# 查看proc文件
ls /proc/

# 查看sysfs
ls /sys/kernel/

# 查看GPIO
ls /sys/class/gpio/

# 查看SPI
ls /sys/class/spi_master/
```

## ⚠️ 重要提示

1. **安全第一**
   - 始终在虚拟机或开发板上测试
   - 不要在生产环境加载未验证的模块
   - 内核代码bug可能导致系统崩溃

2. **权限问题**
   - 大多数操作需要root权限
   - 使用 `sudo` 执行关键操作
   - 注意设备文件权限

3. **内核版本**
   - 不同内核版本API可能变化
   - 使用 `uname -r` 查看内核版本
   - 参考对应版本的文档

4. **调试技巧**
   - 使用 `dmesg` 查看内核日志
   - 使用 `strace` 追踪系统调用
   - 检查返回值和错误码

## 📝 学习建议

1. **按顺序学习**：从 Demo 1 开始，循序渐进
2. **动手实践**：每个demo都要编译、加载、测试
3. **理解原理**：不要只记API，要理解背后的机制
4. **修改实验**：尝试修改代码，观察效果
5. **查阅文档**：遇到问题查阅内核源码和文档

## 🎯 学习路径建议

### 初学者（1-2周）

1. **Week 1**
   - Demo 1: kernel_hello
   - Demo 2: kernel_char_device
   - Demo 3: kernel_procfs

2. **Week 2**
   - Demo 4: kernel_sysfs
   - Demo 5: kernel_device_tree

### 进阶（2-3周）

3. **Week 3**
   - Demo 6: kernel_interrupt
   - Demo 7: kernel_gpio

4. **Week 4**
   - Demo 8: kernel_spi
   - 实际项目练习

## 📚 推荐资源

### 书籍

- 《Linux设备驱动程序》(LDD3)
- 《深入理解Linux内核》
- 《Linux内核设计与实现》

### 在线资源

- Linux内核文档：`Documentation/` 目录
- Kernel Newbies：https://kernelnewbies.org/
- LWN.net：https://lwn.net/Kernel/

### 源码参考

- 实际驱动：`/usr/src/linux*/drivers/`
- 示例代码：`/usr/src/linux*/samples/`
- 内核头文件：`/usr/src/linux*/include/linux/`

## ❓ 常见问题

### Q1: 编译失败，提示找不到内核源码？

```bash
# 安装内核头文件
sudo apt-get install linux-headers-$(uname -r)

# 或手动设置路径
export KDIR=/lib/modules/$(uname -r)/build
```

### Q2: insmod失败，提示"Invalid module format"？

检查内核版本匹配：
```bash
uname -r
modinfo module.ko | grep vermagic
```

### Q3: 系统不支持设备树怎么办？

使用模拟模式，通过模块参数传递值。

### Q4: 没有硬件设备怎么测试？

大部分demo都支持模拟模式，可以在没有硬件的情况下测试。

## 🤝 贡献和反馈

如果你有改进建议或新想法，欢迎反馈！

---

**祝你学习愉快！** 🚀

有问题随时询问！
