# Linux内核开发 - 快速参考卡

## 📖 学习路线图

```
基础（已完成✓）
  ↓
第一阶段：内核基础深化（1-2周）
  ├─ 内存管理
  └─ 锁和同步
  ↓
第二阶段：设备树深度学习（1-2周）
  ├─ 设备树基础
  ├─ 设备树覆盖
  └─ 设备树与驱动集成
  ↓
第三阶段：高级驱动开发（2-3周）
  ├─ I2C设备驱动
  ├─ 块设备驱动
  └─ USB驱动
  ↓
第四阶段：系统集成（2-3周）
  ├─ 模块管理
  ├─ 内核调试
  └─ 内核配置
```

---

## 🛠️ 常用命令速查

### 模块管理

```bash
# 编译
make

# 加载
sudo insmod module.ko

# 卸载
sudo rmmod module

# 查看已加载
lsmod | grep module

# 查看模块信息
modinfo module.ko

# 加载带参数
sudo insmod module.ko param1=value1

# 导出符号
EXPORT_SYMBOL(symbol_name);
```

### 设备树工具

```bash
# 编译.dts为.dtb
dtc -I dts -O dtb input.dts -o output.dtb

# 查看设备树
dtc -I fs -O dts /sys/firmware/devicetree/base > output.dts

# 查看当前设备树
ls /proc/device-tree/

# 设备树覆盖
sudo dtoverlay overlay.dtbo
sudo dtoverlay -r overlay.dtbo
sudo dtoverlay -l
```

### 内核调试

```bash
# 查看日志
dmesg | tail -20

# 实时监控
dmesg -w

# 过滤日志
dmesg | grep "关键词"

# 清空日志
sudo dmesg -c

# 性能分析
perf top
perf record ./app

# ftrace追踪
echo function > /sys/kernel/debug/tracing/current_tracer
cat /sys/kernel/debug/tracing/trace
```

### 设备管理

```bash
# 字符设备
ls -l /dev/

# Proc文件
ls /proc/

# Sysfs
ls /sys/kernel/

# GPIO
ls /sys/class/gpio/

# SPI
ls /sys/class/spi_master/

# I2C
ls /dev/i2c*
```

---

## 📚 核心API速查

### 内核模块

```c
// 初始化
static int __init my_init(void);
module_init(my_init);

// 退出
static void __exit my_exit(void);
module_exit(my_exit);

// 模块参数
static int param = 0;
module_param(param, int, 0644);

// 元数据
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Name");
MODULE_DESCRIPTION("Description");
MODULE_VERSION("1.0");
```

### 字符设备

```c
// 注册
major = register_chrdev(0, "name", &fops);

// 释放
unregister_chrdev(major, "name");

// file_operations
struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
};

// 数据交换
copy_to_user(dest, src, len);
copy_from_user(dest, src, len);
```

### 设备树

```c
// 读取属性
of_property_read_string(node, "prop", &str);
of_property_read_u32(node, "prop", &val);
of_property_read_bool(node, "prop");
of_property_read_u32_array(node, "prop", array, count);

// 查找节点
node = of_find_compatible_node(NULL, NULL, "compatible");
node = of_find_node_by_path("/path");
```

### GPIO

```c
// 请求
gpio_request(gpio, "label");

// 方向
gpio_direction_output(gpio, value);
gpio_direction_input(gpio);

// 读写
gpio_set_value(gpio, value);
val = gpio_get_value(gpio);

// 中断
irq = gpio_to_irq(gpio);
request_irq(irq, handler, flags, "name", NULL);
free_irq(irq, NULL);

// 释放
gpio_free(gpio);
```

### 中断

```c
// 注册
request_irq(irq, handler, flags, "name", dev_id);

// 处理函数
irqreturn_t handler(int irq, void *dev_id);

// Tasklet
DECLARE_TASKLET(name, func, data);
tasklet_schedule(&tasklet);

// Workqueue
DECLARE_WORK(work, func);
schedule_work(&work);
```

### SPI

```c
// 注册驱动
spi_register_driver(&my_driver);

// 传输
spi_write_then_read(spi, tx, tx_len, rx, rx_len);

// 同步传输
struct spi_transfer xfer;
struct spi_message msg;
spi_sync(spi, &msg);
```

---

## 🔍 调试技巧

### 1. 使用printk

```c
printk(KERN_INFO "信息: %d\n", value);
printk(KERN_WARNING "警告\n");
printk(KERN_ERR "错误\n");
```

### 2. 使用/proc

```c
// 创建proc文件
proc_create("my_file", 0644, NULL, &fops);

// 读取函数
static int my_show(struct seq_file *m, void *v) {
    seq_printf(m, "数据: %d\n", value);
    return 0;
}
```

### 3. 使用sysfs

```c
// 创建kobject
kobj = kobject_create_and_add("my_dir", kernel_kobj);

// 创建属性
static struct kobj_attribute attr = __ATTR(name, 0644, show, store);
sysfs_create_file(kobj, &attr.attr);
```

### 4. 检查返回值

```c
// 总是检查返回值
if (ret < 0) {
    printk(KERN_ERR "失败: %d\n", ret);
    return ret;
}
```

---

## 📖 推荐书籍

### 必读

1. **《Linux设备驱动程序》(LDD3)**
   - 经典教材
   - 适合入门

2. **《深入理解Linux内核》**
   - 深入理解内核机制
   - 适合进阶

3. **《Linux内核设计与实现》**
   - 简洁易懂
   - 适合快速了解

### 设备树

4. **《嵌入式Linux设备树》**
   - 设备树专项
   - 实用性强

### 在线资源

- Linux内核文档：https://www.kernel.org/doc/
- 设备树文档：https://elinux.org/Device_Tree_Usage
- LWN.net：https://lwn.net/Kernel/

---

## 🎯 每日学习清单

### 开始学习前

- [ ] 阅读相关章节
- [ ] 理解核心概念
- [ ] 准备实验环境

### 学习过程中

- [ ] 动手编写代码
- [ ] 编译测试
- [ ] 记录问题

### 学习结束后

- [ ] 复习知识点
- [ ] 整理笔记
- [ ] 总结经验

---

## ⚠️ 常见错误

### 1. 权限问题

```bash
# 内核操作需要root
sudo insmod module.ko

# 或修改权限
chmod 666 /dev/device
```

### 2. 编译失败

```bash
# 检查内核头文件
uname -r
ls /lib/modules/$(uname -r)/build

# 安装头文件
sudo apt-get install linux-headers-$(uname -r)
```

### 3. 模块加载失败

```bash
# 查看错误
dmesg | tail -20

# 检查依赖
modprobe --show-depends module.ko
```

### 4. 设备树问题

```bash
# 检查设备树
ls /proc/device-tree/

# 查看错误
dmesg | grep -i dtb
```

---

## 📞 获取帮助

### 内部资源

1. 查看README文档
2. 阅读内核源码
3. 搜索代码注释

### 外部资源

1. Google搜索
2. Stack Overflow
3. GitHub Issues
4. 邮件列表

---

**保持学习！** 🚀
