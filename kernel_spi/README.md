# kernel_spi - SPI设备驱动示例

这个demo展示了Linux SPI子系统的使用。

## 学习目标

通过这个demo，你将学到：

- ✅ SPI的基本概念
- ✅ SPI设备注册和探测
- ✅ SPI消息和传输
- ✅ SPI同步和异步操作
- ✅ SPI设备树配置
- ✅ SPI调试技巧

## 什么是SPI？

SPI（Serial Peripheral Interface）是串行外设接口：

- **全双工**：同时发送和接收
- **高速**：可达数十MHz
- **主从模式**：通常1个主控，多个从设备
- **4线**：SCLK（时钟）、MOSI（主出从入）、MISO（主入从出）、CS（片选）

## SPI信号线

```
        SPI Master                    SPI Slave
    ┌───────────────┐              ┌──────────────┐
    │               │              │              │
    │          SCLK ├─────────────>│ SCLK         │
    │          MOSI ├─────────────>│ MOSI         │
    │          MISO │<─────────────┤ MISO         │
    │           CS  ├─────────────>│ CS           │
    │               │              │              │
    └───────────────┘              └──────────────┘
```

## SPI模式

SPI有4种模式，由时钟极性（CPOL）和相位（CPHA）决定：

| 模式 | CPOL | CPHA | 描述 |
|------|------|------|------|
| 0    | 0    | 0    | 空闲低电平，第一边沿采样 |
| 1    | 0    | 1    | 空闲低电平，第二边沿采样 |
| 2    | 1    | 0    | 空闲高电平，第一边沿采样 |
| 3    | 1    | 1    | 空闲高电平，第二边沿采样 |

## 编译和测试

### 1. 编译

```bash
cd kernel_spi
make

# 预期输出：
# CC [M]  /path/to/spi.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/spi.mod.o
# LD [M]  /path/to/spi.ko
```

### 2. 加载模块

```bash
sudo insmod spi.ko

# 查看内核日志
dmesg | tail -30

# 预期输出：
# [12345.100] spi_demo: === SPI驱动初始化 ===
# [12345.101] spi_demo: 驱动名称: demo_spi
# [12345.102] spi_demo: 检查SPI子系统...
# [12345.103] spi_demo: SPI控制器数量（模拟）
# [12345.104] spi_demo: SPI驱动已注册
# [12345.105] spi_demo: 兼容字符串: demo,spi-device
# [12345.106] spi_demo: 等待SPI设备匹配...
# [12345.107] spi_demo: === 初始化完成 ===
```

### 3. 一键测试

```bash
make test

# 预期输出：
# === 测试SPI模块 ===
# 1. 加载模块
#
# 2. 查看内核日志
# [12345.100] spi_demo: === SPI驱动初始化 ===
# ...
```

### 4. 查看系统SPI信息

```bash
make sys-spi

# 预期输出：
# === 系统SPI信息 ===
# 1. SPI控制器:
# spi0
# spi1
#
# 2. SPI设备:
# （如果有设备）
#
# 3. SPI驱动:
# demo_spi
# ...
```

### 5. 方案A：树莓派实际硬件测试

如果你有树莓派，可以测试实际的SPI：

#### 5.1 树莓派快速测试

```bash
make rpi-test

# 预期输出（树莓派）：
# ✓ 检测到树莓派
# 加载SPI驱动...
# ...
```

#### 5.2 检查SPI总线

```bash
# 查看SPI0总线
make show-spi BUS=0

# 或手动
ls /sys/class/spi_master/spi0/
```

#### 5.3 启用树莓派SPI

在树莓派上，需要先启用SPI：

```bash
# 启用SPI
sudo raspi-config
# 选择 Interface Options -> SPI -> Enable

# 或使用命令行
sudo dtoverlay spi-bcm2835

# 验证SPI设备
ls /dev/spidev0.*
# 应该看到 /dev/spidev0.0, /dev/spidev0.1
```

#### 5.4 设备树配置（树莓派）

创建设备树覆盖：

```bash
cat > spi-overlay.dts << 'EOF'
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target = <&spi0>;
        __overlay__ {
            status = "okay";

            demo_spi: demo@0 {
                compatible = "demo,spi-device";
                reg = <0>;
                spi-max-frequency = <1000000>;
                spi-cpol;
                spi-cpha;
            };
        };
    };
};
EOF

# 编译
dtc -@ -I dts -O dtb -o spi-overlay.dtbo spi-overlay.dts

# 加载
sudo dtoverlay spi-overlay.dtbo
```

### 6. 方案B：用户空间SPI测试（spidev）

即使没有内核驱动，也可以通过spidev测试SPI：

#### 6.1 安装工具

```bash
# Ubuntu/Debian
sudo apt-get install spi-tools

# Arch Linux
sudo pacman -S spi-tools
```

#### 6.2 测试SPI传输

```bash
# 使用spidev_test（如果存在）
spidev_test -D /dev/spidev0.0

# 或使用spi-config
spi-config -d /dev/spidev0.0
```

#### 6.3 Python SPI测试

```python
#!/usr/bin/env python3
"""SPI测试脚本"""

import spidev
import time

# 创建SPI设备
spi = spidev.SpiDev()
spi.open(0, 0)  # 总线0，片选0
spi.max_speed_hz = 1000000  # 1 MHz
spi.mode = 0  # SPI模式0

# 读取数据
data = [0x01, 0x02, 0x03]
result = spi.xfer2(data)
print(f"发送: {data}")
print(f"接收: {result}")

# 闭合连接
spi.close()
```

### 7. 卸载模块

```bash
sudo rmmod spi

# 查看日志
dmesg | tail -5

# 预期输出：
# [12345.200] spi_demo: === SPI驱动卸载 ===
# [12345.201] spi_demo: SPI驱动已注销
# [12345.202] spi_demo: === 卸载完成 ===
```

## 核心API详解

### 注册SPI驱动

```c
static struct spi_driver my_driver = {
    .driver = {
        .name = "my_spi_driver",
        .owner = THIS_MODULE,
    },
    .probe = my_probe,
    .remove = my_remove,
};

module_spi_driver(my_driver);
```

### 探测函数

```c
static int my_probe(struct spi_device *spi)
{
    // 配置SPI参数
    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi_setup(spi);

    // 初始化设备
    // ...

    return 0;
}
```

### SPI传输

#### 方法1: spi_write_then_read

```c
// 简单的写后读
u8 cmd[2] = {0x01, 0x02};
u8 data[4];
spi_write_then_read(spi, cmd, 2, data, 4);
```

#### 方法2: spi_sync

```c
struct spi_transfer xfer;
struct spi_message msg;

memset(&xfer, 0, sizeof(xfer));
xfer.tx_buf = tx_data;
xfer.rx_buf = rx_data;
xfer.len = 4;

spi_message_init(&msg);
spi_message_add_tail(&xfer, &msg);
spi_sync(spi, &msg);
```

#### 方法3: 多段传输

```c
struct spi_transfer xfers[2];
struct spi_message msg;

// 第一段：命令
memset(&xfers[0], 0, sizeof(xfers[0]));
xfers[0].tx_buf = cmd;
xfers[0].len = 2;
xfers[0].cs_change = 1;  // 保持CS

// 第二段：数据
memset(&xfers[1], 0, sizeof(xfers[1]));
xfers[1].tx_buf = data;
xfers[1].len = 4;
xfers[1].cs_change = 0;  // 释放CS

spi_message_init(&msg);
spi_message_add_tail(&xfers[0], &msg);
spi_message_add_tail(&xfers[1], &msg);
spi_sync(spi, &msg);
```

## 常见问题

### Q: 如何知道SPI设备是否存在？

检查sysfs：
```bash
ls /sys/class/spi_master/
ls /dev/spidev*
```

### Q: SPI传输失败？

检查：
1. 物理连接是否正确
2. SPI参数（速度、模式）是否匹配
3. 设备是否正确启用

### Q: 如何选择SPI模式？

查看从设备数据手册：
- 大多数传感器使用模式0或模式3
- Flash存储通常使用模式0

### Q: SPI速度如何设置？

```c
// 在设备树中设置
spi-max-frequency = <1000000>;  // 1 MHz

// 或在代码中设置
spi->max_speed_hz = 1000000;
spi_setup(spi);
```

## 扩展练习

1. **实际传感器**：连接真实SPI传感器
2. **异步传输**：使用spi_async实现异步操作
3. **DMA传输**：使用DMA加速SPI传输
4. **多从设备**：控制多个SPI设备
5. **错误处理**：完善的错误处理机制

## 硬件连接示例

### 树莓派SPI连接

```
树莓派          SPI设备
GPIO11 (SCLK) ── SCLK
GPIO10 (MOSI) ── MOSI
GPIO9  (MISO) ── MISO
GPIO8  (CE0)  ── CS
3.3V          ── VCC
GND           ── GND
```

### SPI Flash示例

```bash
# 读取Flash ID
spidev_test -D /dev/spidev0.0

# 或使用Python
python3 read_flash_id.py
```

## SPI vs I2C

| 特性 | SPI | I2C |
|------|-----|-----|
| 线数 | 4线 | 2线 |
| 速度 | 高（MHz级） | 中（百kHz级） |
| 设备数 | 单独CS，灵活 | 地址寻址，有限 |
| 全双工 | 是 | 否 |
| 复杂度 | 简单 | 复杂 |

## 下一步

继续学习：

- **kernel_i2c** - I2C设备驱动
- **kernel_dma** - DMA操作

## 参考资料

- SPI子系统文档：`Documentation/spi/`
- 设备树SPI绑定：`Documentation/devicetree/bindings/spi/`
- spidev文档：`Documentation/spi/spidev`

---

祝你学习愉快！🚀
