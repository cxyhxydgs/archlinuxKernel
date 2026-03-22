# kernel_procfs - Proc文件系统接口

这个demo展示了如何在 `/proc` 文件系统中创建内核接口。

## 学习目标

通过这个demo，你将学到：

- ✅ Proc文件系统的作用和用途
- ✅ 使用 `seq_file` 接口显示数据
- ✅ 创建 /proc 文件和目录
- ✅ 实现 /proc 文件的读写操作
- ✅ 内核统计数据展示

## 什么是 /proc？

`/proc` 是一个虚拟文件系统，提供内核和进程信息的接口：

- **实时数据**：每次读取都是最新的
- **虚拟文件**：不占用磁盘空间
- **系统信息**：CPU、内存、进程、设备等
- **内核调试**：查看内核状态、配置参数

## 常见的 /proc 文件

```bash
/proc/cpuinfo      # CPU信息
/proc/meminfo      # 内存信息
/proc/cmdline      # 内核启动参数
/proc/modules      # 已加载模块
/proc/interrupts   # 中断统计
/proc/devices      # 设备号分配
/proc/version      # 内核版本
```

## 核心概念

### 1. proc_create

创建 /proc 文件：

```c
struct proc_dir_entry *proc_create(
    const char *name,        // 文件名
    umode_t mode,           // 权限（0666, 0444 等）
    struct proc_dir_entry *parent,  // 父目录（NULL = /proc/）
    const struct file_operations *proc_fops  // 操作函数
);
```

### 2. seq_file 接口

`seq_file` 是一种简化的接口，用于生成序列化输出：

```c
// 显示函数（在读取时调用）
static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "数据: %d\n", some_value);
    return 0;
}

// 打开函数
static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

// file_operations
static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = proc_open,
    .read = seq_read,        // 标准读取函数
    .write = my_write,       // 自定义写入函数
    .llseek = seq_lseek,     // 标准 seek 函数
    .release = single_release,
};
```

### 3. seq_printf

类似 `printf`，但输出到 seq_file：

```c
seq_printf(m, "格式字符串", 参数...);

// 支持标准格式化：
// %d, %u, %ld, %lu, %x, %p, %s 等
```

## 编译和测试

### 1. 编译

```bash
cd kernel_procfs
make

# 预期输出：
# CC [M]  /path/to/procfs.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/procfs.mod.o
# LD [M]  /path/to/procfs.ko
```

### 2. 加载模块

```bash
sudo insmod procfs.ko

# 查看内核日志
dmesg | tail -10

# 预期输出：
# [12345.100] === proc_demo: 初始化开始 ===
# [12345.101] proc_demo: 创建 /proc/kernel_proc_demo
# [12345.102] proc_demo: 创建 /proc/demo_proc/
# [12345.103] proc_demo: 创建 /proc/demo_proc/info
# [12345.104] === proc_demo: 初始化完成 ===
```

### 3. 验证创建的文件

```bash
# 查看主文件
ls -l /proc/kernel_proc_demo

# 查看目录
ls -l /proc/demo_proc/

# 查看目录内容
ls -l /proc/demo_proc/info

# 预期输出：
# -rw-rw-rw- 1 root root 0 Mar 21 04:50 /proc/kernel_proc_demo
# drwxr-xr-x 2 root root 0 Mar 21 04:50 /proc/demo_proc/
# -r--r--r-- 1 root root 0 Mar 21 04:50 /proc/demo_proc/info
```

### 4. 读取 /proc/kernel_proc_demo

```bash
cat /proc/kernel_proc_demo

# 预期输出：
# === Proc文件系统示例 ===
#
# 调用次数: 1
# 最后消息: 等待输入...
# 最后更新: 0 秒前
#
# 当前Jiffies: 4294967295
# HZ (时钟频率): 250
#
# === 系统信息 ===
# 当前进程: cat (PID: 12345)
# UID: 1000, GID: 1000
```

### 5. 写入数据

```bash
# 写入消息
echo "Hello, ProcFS!" | sudo tee /proc/kernel_proc_demo > /dev/null

# 查看内核日志
dmesg | tail -5

# 预期输出：
# [12345.200] proc_demo: 写入请求 16 字节
# [12345.201] proc_demo: 接收到消息: Hello, ProcFS!
```

### 6. 再次读取（查看更新）

```bash
cat /proc/kernel_proc_demo

# 预期输出（消息已更新）：
# === Proc文件系统示例 ===
#
# 调用次数: 3
# 最后消息: Hello, ProcFS!
# 最后更新: 0 秒前
# ...
```

### 7. 访问目录文件

```bash
# 读取 /proc/demo_proc/info
cat /proc/demo_proc/info

# 预期输出：
# 这是 /proc/demo_proc/info 文件
# 演示在 proc 目录中创建多个文件
#
# 数据统计:
#   总调用次数: 4
#   消息: Hello, ProcFS!
```

### 8. 多次读取测试

```bash
# 连续读取3次
for i in {1..3}; do
    echo "=== 第 $i 次读取 ==="
    cat /proc/kernel_proc_demo | grep "调用次数"
done

# 预期输出：
# === 第 1 次读取 ===
# 调用次数: 5
# === 第 2 次读取 ===
# 调用次数: 6
# === 第 3 次读取 ===
# 调用次数: 7
```

### 9. 卸载模块

```bash
sudo rmmod procfs

# 文件会被自动删除
ls /proc/kernel_proc_demo
# 输出：ls: cannot access '/proc/kernel_proc_demo': No such file or directory

# 查看日志
dmesg | tail -3
# 输出：
# [12345.300] === proc_demo: 模块已卸载 ===
```

## 监控脚本

创建一个监控脚本 `monitor_proc.sh`：

```bash
#!/bin/bash

echo "=== 实时监控 /proc/kernel_proc_demo ==="
echo "按 Ctrl+C 退出"
echo ""

while true; do
    clear
    date
    echo ""
    cat /proc/kernel_proc_demo
    sleep 2
done
```

运行：
```bash
chmod +x monitor_proc.sh
./monitor_proc.sh
```

## 对比其他 /proc 文件

```bash
# 对比标准 /proc 文件
echo "=== CPU信息 ==="
cat /proc/cpuinfo | head -20

echo ""
echo "=== 内存信息 ==="
cat /proc/meminfo | head -10

echo ""
echo "=== 内核版本 ==="
cat /proc/version

echo ""
echo "=== 我们的接口 ==="
cat /proc/kernel_proc_demo
```

## Python示例

```python
#!/usr/bin/env python3
"""读取 /proc 文件的Python示例"""

# 读取我们的接口
with open('/proc/kernel_proc_demo', 'r') as f:
    print("=== 我们的接口 ===")
    print(f.read())

# 读取系统信息
print("\n=== 内存信息 ===")
with open('/proc/meminfo', 'r') as f:
    lines = f.readlines()
    for line in lines[:5]:
        print(line.strip())
```

## 调试技巧

### 1. 实时监控

```bash
# 监控内核日志
dmesg -w | grep proc_demo

# 在另一个终端操作
cat /proc/kernel_proc_demo
echo "test" | sudo tee /proc/kernel_proc_demo > /dev/null
```

### 2. 查看进程信息

```bash
# 查看当前进程的 /proc 信息
cat /proc/$$/cmdline
cat /proc/$$/status
```

### 3. 权限测试

```bash
# 测试不同权限
# 0666: 读写
# 0444: 只读
# 0600: 仅root

# 尝试写入只读文件
cat /proc/demo_proc/info
echo "test" | sudo tee /proc/demo_proc/info  # 应该失败
```

## 常见问题

### Q: 读取时数据不变？

/proc 文件每次读取都是实时生成的，但你看到的可能是缓存：

```bash
# 强制重新读取
cat /proc/kernel_proc_demo
cat /proc/kernel_proc_demo  # 每次都是重新生成的
```

### Q: 写入失败？

检查权限：
```bash
ls -l /proc/kernel_proc_demo
# 应该是 -rw-rw-rw- (0666)

# 如果权限不够，修改源码中的权限值
```

### Q: 卸载后文件还存在？

模块卸载后会自动清理：
```bash
sudo rmmod procfs
ls /proc/kernel_proc_demo  # 应该不存在
```

## 扩展练习

1. **添加更多接口**：创建多个 /proc 文件
2. **二进制数据**：支持读取二进制数据
3. **目录操作**：创建嵌套目录结构
4. **动态创建**：根据条件创建/删除文件
5. **使用标准库**：探索 `include/linux/seq_file.h` 的其他函数

## 下一步

继续学习：

- **kernel_sysfs** - Sysfs 接口（/sys/）
- **kernel_device_tree** - 设备树解析
- **kernel_interrupt** - 中断处理

## Proc vs Sysfs

| 特性 | /proc | /sys |
|------|-------|------|
| 用途 | 内核信息、进程信息 | 设备模型、驱动参数 |
| 数据格式 | 文本为主 | 结构化（每个文件一个值）|
| 标准 | 遵循约定 | 严格规范 |
| 主要用途 | 信息展示 | 配置和控制 |

---

祝学习愉快！🚀
