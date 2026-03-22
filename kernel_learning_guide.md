# Linux内核学习路径

## 概述

这是一个从零开始学习Linux内核开发的路径，通过实际动手编写内核模块和驱动来掌握核心技术。每个demo对应一个核心概念，按难度递增。

## 学习顺序

### 第一阶段：基础模块 (1-3天)

**目标：** 理解内核模块的基本结构、编译和加载

1. `kernel_hello` - Hello World模块
   - 最简单的内核模块
   - 学习：模块初始化/退出、printk、模块参数
   - 核心概念：`module_init()`, `module_exit()`, `MODULE_LICENSE()`

2. `kernel_char_device` - 字符设备驱动
   - 创建/dev节点
   - 学习：file_operations、cdev、设备号申请
   - 核心概念：`register_chrdev()`, `file_operations`, `copy_to/from_user()`

3. `kernel_procfs` - Proc文件系统
   - 在/proc创建接口
   - 学习：proc_create、seq_file
   - 核心概念：`proc_create()`, `proc_remove()`, `seq_file`

### 第二阶段：设备模型 (3-5天)

**目标：** 理解Linux设备模型和设备树

4. `kernel_sysfs` - Sysfs接口
   - 在/sys创建接口
   - 学习：kobject、attribute、kset
   - 核心概念：`kobject_create_and_add()`, `sysfs_create_group()`

5. `kernel_device_tree` - 设备树解析
   - 从设备树读取属性
   - 学习：of_node、of_property_read_*、platform device
   - 核心概念：`of_find_node_by_path()`, `of_property_read_*()`, `platform_driver`

### 第三阶段：硬件交互 (5-7天)

**目标：** 学习与硬件设备交互

6. `kernel_interrupt` - 中断处理
   - 注册和处理中断
   - 学习：request_irq、tasklet、workqueue
   - 核心概念：`request_irq()`, `free_irq()`, `tasklet_schedule()`

7. `kernel_gpio` - GPIO控制
   - GPIO操作
   - 学习：gpio_request、gpio_direction_*、gpio_set_value
   - 核心概念：`gpio_request()`, `gpio_set_value()`, `gpio_get_value()`

8. `kernel_spi` - SPI设备驱动
   - SPI总线驱动
   - 学习：spi_transfer、spi_message、spi_device
   - 核心概念：`spi_sync()`, `spi_write_then_read()`, `spi_driver`

### 第四阶段：高级主题 (可选)

9. `kernel_dma` - DMA操作
10. `kernel_netdev` - 网络设备驱动
11. `kernel_i2c` - I2C设备驱动

## 编译和测试

每个demo都包含：
- `.c` 源文件
- `Makefile` - 编译脚本
- `README.md` - 详细说明和测试步骤

### 编译通用步骤

```bash
cd kernel_<demo_name>
make              # 编译生成.ko文件
sudo insmod demo.ko   # 加载模块
dmesg | tail     # 查看内核日志
sudo rmmod demo      # 卸载模块
```

### 内核源码路径

如果编译失败，需要设置内核源码路径：

```bash
export KDIR=/lib/modules/$(uname -r)/build
export KERNELDIR=/lib/modules/$(uname -r)/build
```

## 学习建议

1. **每个demo都要动手编译和测试**
2. **查看内核日志**：`dmesg -w` 实时查看
3. **阅读相关源码**：`/usr/src/linux-*/include/linux/`
4. **理解核心数据结构**：不要只背API
5. **设备树相关**：需要实际的设备树文件（.dts/dtsb）

## 推荐资源

- **书籍：** 《Linux设备驱动程序》(LDD3)、《深入理解Linux内核》
- **源码：** `/usr/src/linux*/drivers/` 参考实际驱动
- **文档：** `man 2 syscall`、内核文档 `Documentation/`

## 注意事项

- ⚠️ 内核代码运行在最高权限，bug可能崩溃系统
- ⚠️ 使用虚拟机或开发板练习，不要在生产环境测试
- ⚠️ 注意内核版本差异，API可能变化
- ⚠️ 总是检查返回值，处理错误情况

---

开始你的内核之旅吧！从 `kernel_hello` 开始。🚀
