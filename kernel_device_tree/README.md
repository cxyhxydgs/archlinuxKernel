# kernel_device_tree - 设备树解析示例

这个demo展示了如何在Linux内核中解析设备树（Device Tree）。

## 学习目标

通过这个demo，你将学到：

- ✅ 设备树的基本概念
- ✅ Platform Device 驱动框架
- ✅ 读取设备树属性（字符串、整数、布尔、数组）
- ✅ 遍历设备树节点
- ✅ Compatible 匹配机制
- ✅ Phandle 引用解析

## 什么是设备树？

设备树是一种数据结构，用于描述硬件配置：

- **独立于内核**：硬件信息在设备树中，不硬编码
- **嵌入式常用**：ARM、PowerPC等架构标准配置
- **可移植性**：同一内核可用于不同硬件

## 设备树节点示例

```dts
/ {
    demo-device {
        compatible = "demo,device-tree";
        demo-string = "Hello from Device Tree";
        demo-int = <42>;
        demo-bool;
        demo-array = <1 2 3 4>;
        demo-u32 = <0x12345678>;
        demo-u64 = /bits/ 64 <0x123456789ABCDEF0>;
        demo-phandle = <&some_other_node>;
    };
};
```

## 编译和测试

### 1. 编译

```bash
cd kernel_device_tree
make

# 预期输出：
# CC [M]  /path/to/devtree.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/devtree.mod.o
# LD [M]  /path/to/devtree.ko
```

### 2. 检查设备树支持

```bash
# 检查系统是否支持设备树
ls -la /proc/device-tree

# 或者
make find-node
```

**输出解释：**

- 如果存在 `/proc/device-tree/` - 系统支持设备树
- 如果不存在 - 系统不使用设备树（x86桌面通常不支持）

### 3. 方案A：模拟模式测试（推荐用于x86）

即使系统不支持设备树，也可以测试这个驱动：

```bash
# 使用模拟参数加载模块
sudo insmod devtree.ko dt_string_param="我的测试字符串" dt_int_param=888

# 查看内核日志
dmesg | tail -30

# 预期输出：
# [12345.100] device_tree_demo: === 模块初始化 ===
# [12345.101] device_tree_demo: platform driver 已注册
# [12345.102] device_tree_demo: 兼容字符串: demo,device-tree
# [12345.103] device_tree_demo: 未找到匹配的设备树节点
# [12345.104] device_tree_demo: 将使用模拟参数进行测试
# [12345.105] device_tree_demo: === 初始化完成 ===
```

### 4. 一键测试（模拟模式）

```bash
make test-sim

# 预期输出：
# === 测试模拟模式（无设备树）===
# 1. 加载模块（使用默认参数）
#
# 2. 查看内核日志
# [12345.100] device_tree_demo: === 设备树解析开始 ===
# [12345.101] device_tree_demo: 没有设备树节点，使用模拟参数
# [12345.102] device_tree_demo: 字符串: 测试字符串
# [12345.103] device_tree_demo: 整数: 999
```

### 5. 方案B：实际设备树测试（嵌入式设备/树莓派等）

如果你在支持设备树的设备上运行（如ARM开发板），可以：

#### 5.1 创建测试设备树覆盖

```bash
# 创建设备树覆盖文件
cat > test-dt-overlay.dts << 'EOF'
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target-path = "/";
        __overlay__ {
            demo-device {
                compatible = "demo,device-tree";
                status = "okay";
                demo-string = "Hello from Device Tree!";
                demo-int = <123>;
                demo-bool;
                demo-array = <10 20 30 40>;
                demo-u32 = <0xABCDEF>;
            };
        };
    };
};
EOF
```

#### 5.2 编译设备树覆盖

```bash
# 需要设备树编译器
dtc -@ -I dts -O dtb -o test-dt-overlay.dtbo test-dt-overlay.dts
```

#### 5.3 加载设备树覆盖

```bash
# 应用设备树覆盖
sudo dtoverlay test-dt-overlay.dtbo

# 验证节点是否创建
ls /proc/device-tree/demo-device

# 加载内核模块
sudo insmod devtree.ko

# 查看日志 - 应该看到从设备树读取的值
dmesg | tail -30
```

#### 5.4 卸载

```bash
sudo rmmod devtree
sudo dtoverlay -r test-dt-overlay
```

### 6. 查看设备树信息

```bash
# 查看设备模型
make show-dt

# 或手动
cat /proc/device-tree/model
cat /proc/device-tree/compatible

# 遍历设备树
find /proc/device-tree -type f | head -20
```

### 7. 卸载模块

```bash
sudo rmmod devtree

# 查看日志
dmesg | tail -5

# 预期输出：
# [12345.200] device_tree_demo: === 模块卸载 ===
# [12345.201] device_tree_demo: platform driver 已注销
```

## 测试不同的参数值

```bash
# 测试不同的字符串
sudo insmod devtree.ko dt_string_param="测试中文" dt_int_param=777
dmesg | tail -15

# 测试长字符串
sudo insmod devtree.ko dt_string_param="这是一个很长的测试字符串，用于测试缓冲区"
dmesg | tail -15

# 测试大整数
sudo insmod devtree.ko dt_int_param=2147483647
dmesg | tail -15
```

## Python 测试脚本

```python
#!/usr/bin/env python3
"""设备树模块参数测试"""

import subprocess
import sys

def load_module(string_val, int_val):
    """加载模块并设置参数"""
    cmd = f"sudo insmod devtree.ko dt_string_param=\"{string_val}\" dt_int_param={int_val}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"加载失败: {result.stderr}")
        return False
    return True

def unload_module():
    """卸载模块"""
    subprocess.run("sudo rmmod devtree", shell=True)

def get_logs():
    """获取内核日志"""
    result = subprocess.run("dmesg | tail -20 | grep device_tree_demo",
                           shell=True, capture_output=True, text=True)
    return result.stdout

# 测试用例
test_cases = [
    ("Hello", 42),
    ("测试", 100),
    ("Long string with spaces", 999),
    ("Special!@#$%", 123),
]

for i, (string_val, int_val) in enumerate(test_cases, 1):
    print(f"\n=== 测试 {i}: {string_val}, {int_val} ===")

    if not load_module(string_val, int_val):
        continue

    print(get_logs())
    unload_module()
```

## 设备树调试技巧

### 1. 查看系统设备树

```bash
# 查看完整设备树
dtc -I fs -O dts /sys/firmware/devicetree/base

# 或查看特定节点
ls /proc/device-tree/

# 查看节点属性
hexdump -C /proc/device-tree/demo-device/demo-string 2>/dev/null
```

### 2. 检查已加载的设备树覆盖

```bash
# 查看所有覆盖
ls /sys/kernel/config/device-tree/overlays/

# 查看特定覆盖的状态
cat /sys/kernel/config/device-tree/overlays/*/status
```

### 3. 解码设备树二进制文件

```bash
# 解码设备树 blob
dtc -I dtb -O dts /boot/dtb-$(uname -r)

# 或
dtc -I dtb -O dts /boot/dtb
```

## 核心API说明

### 读取属性

```c
// 读取字符串
const char *str;
of_property_read_string(node, "property-name", &str);

// 读取32位整数
u32 value;
of_property_read_u32(node, "property-name", &value);

// 读取64位整数
u64 value;
of_property_read_u64(node, "property-name", &value);

// 读取布尔值（存在即为true）
bool flag;
flag = of_property_read_bool(node, "property-name");

// 读取数组
u32 array[10];
of_property_read_u32_array(node, "array-name", array, 10);
```

### 查找节点

```c
// 按路径查找
struct device_node *node;
node = of_find_node_by_path("/path/to/node");

// 按compatible查找
node = of_find_compatible_node(NULL, NULL, "vendor,device");

// 遍历子节点
struct device_node *child;
for_each_child_of_node(parent, child) {
    printk("Child: %s\n", child->name);
}
```

### Phandle 引用

```c
// 获取phandle指向的节点
struct device_node *phandle_node;
phandle_node = of_parse_phandle(node, "phandle-property", 0);
if (phandle_node) {
    // 使用 phandle_node
    of_node_put(phandle_node);  // 释放引用
}
```

## 常见问题

### Q: 系统不支持设备树怎么办？

使用模拟模式，通过模块参数传递值：
```bash
sudo insmod devtree.ko dt_string_param="测试" dt_int_param=100
```

### Q: 如何知道系统是否支持设备树？

```bash
ls /proc/device-tree
```
如果目录存在，说明支持。

### Q: 模块加载失败，提示 "Invalid parameters"

检查参数格式：
```bash
# 正确
sudo insmod devtree.ko dt_string_param="test" dt_int_param=123

# 错误（缺少引号）
sudo insmod devtree.ko dt_string_param=test dt_int_param=123
```

### Q: 如何在嵌入式设备上测试？

1. 创建设备树覆盖（.dts）
2. 编译为 .dtbo
3. 使用 dtoverlay 加载
4. 加载内核模块

## 扩展练习

1. **添加更多属性解析**：支持更多数据类型
2. **子设备创建**：在probe中创建子设备
3. **资源获取**：解析IRQ、内存、GPIO资源
4. **时序配置**：解析时钟、pinctrl配置
5. **多实例支持**：支持多个设备树节点

## 相关文件

- 设备树规范：`Documentation/devicetree/`
- API文档：`include/linux/of.h`
- 示例驱动：`drivers/` 下的实际驱动

## 下一步

继续学习：

- **kernel_interrupt** - 中断处理
- **kernel_gpio** - GPIO控制
- **kernel_spi** - SPI设备驱动

---

祝你学习愉快！🚀
