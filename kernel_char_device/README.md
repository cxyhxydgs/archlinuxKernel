# kernel_char_device - 字符设备驱动

这个demo展示了如何创建一个简单的字符设备驱动程序。

## 学习目标

通过这个demo，你将学到：

- ✅ 字符设备的概念和作用
- ✅ 如何申请主设备号（动态/静态）
- ✅ `file_operations` 结构 - 设备的操作接口
- ✅ 用户态和内核态的数据交换
- ✅ 自动创建设备节点（/dev/chardev）
- ✅ open/read/write/close 的内核实现

## 什么是字符设备？

字符设备是Linux中最基本的设备类型之一，特点是：

- 以字节流方式访问（像文件一样）
- 没有缓冲区，直接读写
- 典型例子：键盘、鼠标、串口、声卡

与之相对的是**块设备**（如硬盘），以块为单位访问。

## 核心概念

### 1. 设备号

设备号由**主设备号**和**次设备号**组成：

- **主设备号**：标识驱动程序（所有同类设备共享）
- **次设备号**：标识具体设备（同一驱动的不同实例）

```c
MKDEV(major, minor)  // 合成设备号
MAJOR(dev)           // 提取主设备号
MINOR(dev)           // 提取次设备号
```

### 2. file_operations

这是驱动程序的核心，定义了用户空间如何操作设备：

```c
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    int (*open) (struct inode *, struct file *);
    int (*release) (struct inode *, struct file *);
    // ... 更多操作
};
```

### 3. 数据交换

**绝对不能**直接使用 `memcpy` 在用户态和内核态之间复制数据！

```c
// ❌ 错误！会导致段错误或安全漏洞
memcpy(user_buffer, kernel_buffer, len);

// ✅ 正确！
copy_to_user(user_buffer, kernel_buffer, len);
copy_from_user(kernel_buffer, user_buffer, len);
```

**为什么？**
- 用户空间地址可能无效
- 防止内核被恶意利用
- 处理页面错误

## 编译和测试

### 1. 编译

```bash
cd kernel_char_device
make

# 预期输出：
# Building modules, stage 2.
# MODPOST 1 modules
# CC      /path/to/chardev.mod.o
# LD [M]  /path/to/chardev.ko
```

### 2. 加载模块

```bash
sudo insmod chardev.ko

# 查看内核日志
dmesg | tail -10

# 预期输出：
# [12345.123] === chardev: 初始化开始 ===
# [12345.124] chardev: 注册成功，主设备号 = 237
# [12345.125] chardev: 设备节点 /dev/chardev 创建成功
# [12345.126] === chardev: 初始化完成 ===
```

### 3. 验证设备节点

```bash
# 查看设备节点
ls -l /dev/chardev

# 预期输出：
# crw-rw---- 1 root root 237, 0 Mar 21 04:50 /dev/chardev
#           ^ ^           ^    ^
#           | |           |    |
#           | |           |    +-- 次设备号 (0)
#           | |           +------- 主设备号 (237)
#           | +------------------- 字符设备 (c = character)
#           +--------------------- 权限: crw-rw----
```

### 4. 测试写入

```bash
# 方法1：使用 echo
echo "Hello from shell!" | sudo tee /dev/chardev > /dev/null

# 查看内核日志
dmesg | tail -5

# 预期输出：
# [12345.200] chardev: 写入请求 18 字节
# [12345.200] chardev: 写入了 18 字节 (总共 18 字节)

# 方法2：使用 printf
printf "Hello, Kernel!" | sudo tee /dev/chardev > /dev/null
```

### 5. 测试读取

```bash
# 读取设备内容（会清空缓冲区）
sudo cat /dev/chardev

# 预期输出：
# Hello from shell!Hello, Kernel!

# 查看日志
dmesg | tail -5

# 预期输出：
# [12345.250] chardev: 读取请求 32768 字节
# [12345.250] chardev: 读取了 32 字节
# [12345.250] chardev: 设备被打开
# [12345.250] chardev: 设备被关闭
```

### 6. 完整测试脚本

```bash
#!/bin/bash
# test_chardev.sh

# 清空缓冲区（通过读取）
sudo cat /dev/chardev > /dev/null 2>&1

# 写入测试
echo "=== 写入测试 ==="
echo -n "Test data 1" | sudo tee /dev/chardev > /dev/null
echo -n "Test data 2" | sudo tee /dev/chardev > /dev/null

# 读取测试
echo "=== 读取测试 ==="
RESULT=$(sudo cat /dev/chardev)
echo "从设备读取: $RESULT"

# 再次读取（应该为空）
echo "=== 再次读取（应该为空）==="
RESULT=$(sudo cat /dev/chardev)
echo "从设备读取: '$RESULT'"

# 查看日志
echo ""
echo "=== 内核日志 ==="
dmesg | tail -20 | grep chardev
```

### 7. 卸载模块

```bash
sudo rmmod chardev

# 设备节点会被自动删除
ls /dev/chardev
# 输出：ls: cannot access '/dev/chardev': No such file or directory

# 查看日志
dmesg | tail -3
# 输出：
# [12345.300] chardev: 设备被关闭
# [12345.301] === chardev: 模块已卸载 ===
```

## Python测试脚本

创建一个更友好的测试脚本 `test_chardev.py`：

```python
#!/usr/bin/env python3
import os
import time

DEVICE = "/dev/chardev"

print("=== 字符设备测试 ===\n")

# 写入测试
print("1. 写入数据")
data = "Hello from Python!"
with open(DEVICE, 'w') as f:
    f.write(data)
print(f"   写入: {data}")

# 读取测试
print("\n2. 读取数据")
with open(DEVICE, 'r') as f:
    result = f.read()
print(f"   读取: {result}")

# 多次写入
print("\n3. 多次写入")
for i in range(3):
    msg = f"Message {i+1}"
    with open(DEVICE, 'w') as f:
        f.write(msg + "\n")
    print(f"   写入: {msg}")

# 读取所有
print("\n4. 读取所有数据")
with open(DEVICE, 'r') as f:
    result = f.read()
print(f"   读取:\n{result}")

print("\n=== 测试完成 ===")
```

运行：
```bash
sudo python3 test_chardev.py
```

## 查看设备信息

```bash
# 查看字符设备列表
ls -l /dev/ | head -20

# 查看设备信息
sudo file /dev/chardev
# 输出：/dev/chardev: character special (237/0)

# 查看 /proc/devices 中的主设备号
cat /proc/devices | grep chardev
```

## 调试技巧

### 1. 实时监控日志

```bash
# 打开两个终端
# 终端1：实时监控
dmesg -w | grep chardev

# 终端2：操作设备
echo "test" | sudo tee /dev/chardev > /dev/null
```

### 2. 使用 strace 跟踪系统调用

```bash
# 跟踪 cat 命令
strace -e open,read,write cat /dev/chardev

# 会看到类似输出：
# open("/dev/chardev", O_RDONLY) = 3
# read(3, "Test data", 4096)         = 9
# read(3, "", 4096)                   = 0
```

### 3. 查看模块依赖

```bash
cat /proc/modules | grep chardev
```

## 常见问题

### Q: insmod失败，提示 "Device or resource busy"

设备号被占用：
```bash
# 查看已使用的设备号
cat /proc/devices

# 查看哪个模块占用了
lsmod
```

### Q: 无法访问 /dev/chardev

权限问题：
```bash
# 修改权限
sudo chmod 666 /dev/chardev

# 或者将当前用户添加到相关组
# 具体组取决于系统配置
```

### Q: 读取时没有数据

缓冲区是 FIFO（先进先出）的，读取后会清空：
```bash
# 写入
echo "test1" | sudo tee /dev/chardev > /dev/null
# 读取（正确）
sudo cat /dev/chardev
# 再次读取（为空，因为已经读过了）
sudo cat /dev/chardev
```

## 扩展练习

尝试改进这个驱动：

1. **添加 ioctl 支持**：实现自定义控制命令
2. **支持非阻塞 I/O**：添加 O_NONBLOCK 支持
3. **实现 seek**：支持文件偏移量
4. **多实例支持**：支持多个次设备号
5. **并发控制**：使用互斥锁保护共享资源

## 下一步

继续学习：

- **kernel_procfs** - 在 /proc 创建接口
- **kernel_sysfs** - 在 /sys 创建接口
- **kernel_interrupt** - 中断处理

## 相关概念对比

| 概念 | 字符设备 | 块设备 |
|------|---------|--------|
| 访问方式 | 字节流 | 数据块 |
| 缓冲 | 无缓冲 | 有缓冲 |
| 典型例子 | 键盘、串口 | 硬盘、SSD |
| 随机访问 | 通常顺序 | 随机支持 |

---

祝学习愉快！🚀
