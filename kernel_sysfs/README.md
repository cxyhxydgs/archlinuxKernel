# kernel_sysfs - Sysfs接口示例

这个demo展示了如何在 `/sys` 文件系统中创建内核接口。

## 学习目标

通过这个demo，你将学到：

- ✅ Sysfs 的作用和结构
- ✅ `kobject` 的概念和使用
- ✅ 创建 sysfs 属性（单个和批量）
- ✅ 实现 `show` 和 `store` 函数
- ✅ 属性组（attribute groups）
- ✅ /proc 和 /sys 的区别

## 什么是 /sys？

`/sys` 是 Linux 2.6 引入的虚拟文件系统，用于展示内核设备模型：

- **设备模型**：表示设备和驱动的关系
- **一个文件一个值**：每个文件只包含一个值
- **严格规范**：文件格式有明确定义
- **配置接口**：配置设备参数和驱动

## 目录结构

```
/sys/
├── block/          # 块设备
├── bus/            # 系统总线（pci, usb, i2c 等）
├── class/          # 设备类（net, sound, video 等）
├── devices/        # 所有设备
├── dev/            # 设备号映射（char, block）
├── firmware/       # 固件
├── fs/             # 文件系统参数
├── kernel/         # 内核参数
└── module/         # 内核模块参数
```

## 核心概念

### 1. kobject

kobject 是 sysfs 的基础结构：

```c
struct kobject {
    const char      *name;
    struct list_head entry;
    struct kobject  *parent;
    struct kset     *kset;
    struct kobj_type *ktype;
    struct kernfs_node *sd;
    // ...
};
```

创建 kobject：
```c
// 在 /sys/kernel/ 下创建目录
struct kobject *kobj = kobject_create_and_add("demo_sysfs", kernel_kobj);

// 清理
kobject_put(kobj);
```

### 2. sysfs 属性

定义属性：
```c
// 使用 __ATTR 宏
static struct kobj_attribute attr = __ATTR(name, mode, show, store);
```

权限模式：
- `0644` - 读写
- `0444` - 只读
- `0200` - 只写

### 3. show 和 store

```c
// show: 读取属性
static ssize_t attr_show(struct kobject *kobj,
                       struct kobj_attribute *attr,
                       char *buf)
{
    return sprintf(buf, "%d\n", value);
}

// store: 写入属性
static ssize_t attr_store(struct kobject *kobj,
                        struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    int ret = kstrtoint(buf, 10, &value);
    if (ret)
        return ret;
    return count;
}
```

### 4. 属性组

批量创建属性：
```c
static struct attribute *attrs[] = {
    &attr1.attr,
    &attr2.attr,
    NULL,  // 必须以 NULL 结尾
};

static struct attribute_group group = {
    .attrs = attrs,
};

// 创建
sysfs_create_group(kobj, &group);

// 删除
sysfs_remove_group(kobj, &group);
```

## 编译和测试

### 1. 编译

```bash
cd kernel_sysfs
make

# 预期输出：
# CC [M]  /path/to/sysfs.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/sysfs.mod.o
# LD [M]  /path/to/sysfs.ko
```

### 2. 加载模块

```bash
sudo insmod sysfs.ko

# 查看内核日志
dmesg | tail -10

# 预期输出：
# [12345.100] === sysfs_demo: 初始化开始 ===
# [12345.101] sysfs_demo: 创建 /sys/kernel/demo_sysfs
# [12345.102] sysfs_demo: 创建属性组成功
# [12345.103] sysfs_demo: 创建动态属性成功
# [12345.104] === sysfs_demo: 初始化完成 ===
```

### 3. 查看目录结构

```bash
# 查看创建的目录
ls -l /sys/kernel/demo_sysfs/

# 预期输出：
# total 0
# -rw-r--r-- 1 root root 4096 Mar 21 04:50 bool_value
# -r--r--r-- 1 root root 4096 Mar 21 04:50 dynamic
# -r--r--r-- 1 root root 4096 Mar 21 04:50 info
# -rw-r--r-- 1 root root 4096 Mar 21 04:50 int_value
# -rw-r--r-- 1 root root 4096 Mar 21 04:50 str_value
```

### 4. 使用 tree 查看结构

```bash
make tree

# 或手动
tree /sys/kernel/demo_sysfs

# 预期输出：
# /sys/kernel/demo_sysfs/
# ├── bool_value
# ├── dynamic
# ├── info
# ├── int_value
# └── str_value
```

### 5. 读取属性

```bash
# 读取整数属性
cat /sys/kernel/demo_sysfs/int_value
# 输出：42

# 读取字符串属性
cat /sys/kernel/demo_sysfs/str_value
# 输出：default

# 读取布尔属性
cat /sys/kernel/demo_sysfs/bool_value
# 输出：1

# 读取只读信息
cat /sys/kernel/demo_sysfs/info
# 输出：
# Sysfs Demo Module
# Built-in: sysfs
# Kernel: 6.18.13
```

### 6. 写入属性

```bash
# 写入整数
echo 100 | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null
cat /sys/kernel/demo_sysfs/int_value
# 输出：100

# 查看内核日志
dmesg | tail -5
# 输出：
# [12345.200] sysfs_demo: int_value 更新为 100

# 写入字符串
echo "Hello, SysFS!" | sudo tee /sys/kernel/demo_sysfs/str_value > /dev/null
cat /sys/kernel/demo_sysfs/str_value
# 输出：Hello, SysFS!

# 写入布尔值
echo 0 | sudo tee /sys/kernel/demo_sysfs/bool_value > /dev/null
cat /sys/kernel/demo_sysfs/bool_value
# 输出：0
```

### 7. 批量测试

```bash
make test

# 预期输出：
# === Sysfs 测试 ===
#
# 1. 读取所有属性
#    int_value: 42
#    str_value: default
#    bool_value: 1
#
# 2. 写入属性
#
# 3. 再次读取
#    int_value: 100
#    str_value: hello
#    bool_value: 0
```

### 8. 运行时监控

创建监控脚本 `monitor_sysfs.sh`：

```bash
#!/bin/bash

echo "=== 实时监控 sysfs 属性 ==="
echo "按 Ctrl+C 退出"
echo ""

SYSFS_DIR="/sys/kernel/demo_sysfs"

while true; do
    clear
    date
    echo ""
    echo "int_value: $(cat $SYSFS_DIR/int_value)"
    echo "str_value: $(cat $SYSFS_DIR/str_value)"
    echo "bool_value: $(cat $SYSFS_DIR/bool_value)"
    sleep 1
done
```

### 9. 卸载模块

```bash
sudo rmmod sysfs

# 目录被删除
ls /sys/kernel/demo_sysfs
# 输出：ls: cannot access '/sys/kernel/demo_sysfs': No such file or directory
```

## 对比标准 sysfs 文件

```bash
# 对比其他 sysfs 文件
echo "=== 模块参数 ==="
ls /sys/module/sysfs/parameters/

echo ""
echo "=== 模块信息 ==="
cat /sys/module/sysfs/refcnt

echo ""
echo "=== 我们的属性 ==="
ls /sys/kernel/demo_sysfs/
```

## Python 示例

```python
#!/usr/bin/env python3
"""与 sysfs 交互的 Python 示例"""

import os

SYSFS_DIR = "/sys/kernel/demo_sysfs"

def read_attr(name):
    """读取属性"""
    path = os.path.join(SYSFS_DIR, name)
    with open(path, 'r') as f:
        return f.read().strip()

def write_attr(name, value):
    """写入属性"""
    path = os.path.join(SYSFS_DIR, name)
    with open(path, 'w') as f:
        f.write(str(value))

print("=== 读取初始值 ===")
print(f"int_value: {read_attr('int_value')}")
print(f"str_value: {read_attr('str_value')}")
print(f"bool_value: {read_attr('bool_value')}")

print("\n=== 写入新值 ===")
write_attr('int_value', 999)
write_attr('str_value', 'Python')
write_attr('bool_value', 1)

print("=== 验证新值 ===")
print(f"int_value: {read_attr('int_value')}")
print(f"str_value: {read_attr('str_value')}")
print(f"bool_value: {read_attr('bool_value')}")
```

## 调试技巧

### 1. 实时监控日志

```bash
dmesg -w | grep sysfs_demo
```

### 2. 查看属性变化

```bash
# 使用 watch 命令
watch -n 1 'cat /sys/kernel/demo_sysfs/int_value'

# 在另一个终端写入
echo 123 | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null
```

### 3. 验证权限

```bash
# 查看所有属性权限
ls -l /sys/kernel/demo_sysfs/

# 预期：
# -rw-r--r-- (0644) - 读写
# -r--r--r-- (0444) - 只读
```

## 常见问题

### Q: 写入失败 "Invalid argument"

检查数据格式：
```bash
# 正确
echo 42 | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null

# 错误（字符串写入整数属性）
echo "hello" | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null  # 会失败
```

### Q: 卸载后文件还存在？

文件会自动删除：
```bash
sudo rmmod sysfs
ls /sys/kernel/demo_sysfs  # 应该不存在
```

如果存在，可能是残留：
```bash
# 手动检查
ls -la /sys/kernel/ | grep demo
```

### Q: 权限错误

使用 sudo 写入：
```bash
# 大多数 sysfs 属性需要 root 权限
echo 100 | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null
```

## 扩展练习

1. **添加更多类型**：支持十六进制输入、数组等
2. **嵌套结构**：创建子目录
3. **权限控制**：根据值设置不同权限
4. **回调函数**：在属性变化时执行特定操作
5. **与设备集成**：将 sysfs 与真实设备驱动结合

## 下一步

继续学习：

- **kernel_device_tree** - 设备树解析
- **kernel_interrupt** - 中断处理
- **kernel_gpio** - GPIO控制

## /proc vs /sys

| 特性 | /proc | /sys |
|------|-------|------|
| 用途 | 信息展示 | 设备模型 |
| 文件格式 | 自由 | 一个文件一个值 |
| 标准 | 惯例 | 严格规范 |
| 主要用途 | 调试、信息 | 配置、控制 |
| 示例 | /proc/cpuinfo | /sys/class/net |

## sysfs 设计原则

1. **一个值一个文件**：不要在一个文件中放多个值
2. **文本格式**：使用人类可读的文本
3. **简单的验证**：在 store 函数中验证输入
4. **原子操作**：确保操作是原子的
5. **权限控制**：合理设置文件权限

---

祝学习愉快！🚀
