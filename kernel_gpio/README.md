# kernel_gpio - GPIO控制示例

这个demo展示了Linux GPIO子系统的使用。

## 学习目标

通过这个demo，你将学到：

- ✅ GPIO的基本概念
- ✅ GPIO请求和释放
- ✅ GPIO方向设置（输入/输出）
- ✅ GPIO读写操作
- ✅ GPIO中断配置
- ✅ GPIO sysfs用户空间接口

## 什么是GPIO？

GPIO（General Purpose I/O）是通用输入/输出引脚：

- **输入**：读取引脚状态（0或1）
- **输出**：控制引脚电平（低电平或高电平）
- **特殊功能**：PWM、SPI、I2C等复用
- **常用场景**：LED、按键、传感器、继电器

## GPIO编号

不同平台GPIO编号方式不同：

- **树莓派**：BCM编号（如GPIO18、GPIO4）
- **Intel**：chip-specific编号
- **查找方法**：`/sys/class/gpio/gpiochip*/base`

## 编译和测试

### 1. 编译

```bash
cd kernel_gpio
make

# 预期输出：
# CC [M]  /path/to/gpio.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/gpio.mod.o
# LD [M]  /path/to/gpio.ko
```

### 2. 方案A：模拟模式测试（推荐用于x86）

```bash
# 使用无效GPIO号，会自动进入模拟模式
sudo insmod gpio.ko gpio_out_num=999 gpio_in_num=888

# 查看内核日志
dmesg | tail -40

# 预期输出：
# [12345.100] gpio_demo: === GPIO驱动初始化 ===
# [12345.101] gpio_demo: 输出GPIO: 999
# [12345.102] gpio_demo: 输入GPIO: 888
# [12345.103] gpio_demo: GPIO 999 无效，使用模拟模式
# [12345.104] gpio_demo: === 演示GPIO输出 ===
# [12345.105] gpio_demo: 模拟 GPIO 999 = HIGH
# [12345.305] gpio_demo: 模拟 GPIO 999 = LOW
# [12345.505] gpio_demo: 模拟 GPIO 999 = HIGH
# ...
```

### 3. 一键测试（模拟模式）

```bash
make test

# 预期输出：
# === 测试GPIO模块 ===
# 1. 加载模块（模拟模式）
#
# 2. 查看内核日志
# [12345.100] gpio_demo: === GPIO驱动初始化 ===
# ...
```

### 4. 方案B：实际硬件测试（树莓派等）

如果你在有GPIO的设备上运行（如树莓派）：

#### 4.1 查看可用GPIO

```bash
make sys-gpio

# 预期输出（树莓派）：
# === 系统GPIO信息 ===
# 1. GPIO控制器数量: 2
#
# 2. GPIO控制器:
# /sys/class/gpio/gpiochip0
# /sys/class/gpio/gpiochip32
#
# 3. 已导出的GPIO:
# （空）
```

#### 4.2 树莓派快速测试

```bash
make rpi-test

# 预期输出（树莓派）：
# ✓ 检测到树莓派
# 加载树莓派GPIO测试...
# [12345.100] gpio_demo: === GPIO驱动初始化 ===
# ...
```

#### 4.3 测试特定GPIO引脚

```bash
# 测试GPIO18（输出）和GPIO4（输入）
make test-pins OUT=18 IN=4

# 或手动
sudo insmod gpio.ko gpio_out_num=18 gpio_in_num=4
dmesg | tail -30
```

### 5. 用户空间GPIO测试（sysfs）

即使没有内核模块，也可以通过sysfs测试GPIO：

#### 5.1 导出GPIO

```bash
# 导出GPIO18
make export-gpio PIN=18

# 或手动
echo 18 | sudo tee /sys/class/gpio/export

# 预期输出：
# 18
```

#### 5.2 配置GPIO方向

```bash
# 设置为输出
echo out | sudo tee /sys/class/gpio/gpio18/direction

# 设置为输入
echo in | sudo tee /sys/class/gpio/gpio18/direction
```

#### 5.3 读写GPIO值

```bash
# 写入高电平（开LED）
echo 1 | sudo tee /sys/class/gpio/gpio18/value

# 写入低电平（关LED）
echo 0 | sudo tee /sys/class/gpio/gpio18/value

# 读取GPIO值
cat /sys/class/gpio/gpio18/value
# 输出：0 或 1
```

#### 5.4 闪烁LED示例

```bash
#!/bin/bash
# blink_led.sh

GPIO=18
echo "闪烁GPIO $GPIO"

for i in {1..10}; do
    echo "第 $i 次：开"
    echo 1 | sudo tee /sys/class/gpio/gpio$GPIO/value > /dev/null
    sleep 0.5

    echo "第 $i 次：关"
    echo 0 | sudo tee /sys/class/gpio/gpio$GPIO/value > /dev/null
    sleep 0.5
done

echo "完成"
```

运行：
```bash
chmod +x blink_led.sh
sudo ./blink_led.sh
```

#### 5.5 读取按键示例

```bash
#!/bin/bash
# read_button.sh

GPIO=4
echo "读取GPIO $GPIO（按键）"
echo "按Ctrl+C退出"

# 配置为输入
echo in | sudo tee /sys/class/gpio/gpio$GPIO/direction > /dev/null

# 持续读取
while true; do
    value=$(cat /sys/class/gpio/gpio$GPIO/value)
    if [ "$value" -eq 1 ]; then
        echo "按键按下！"
    fi
    sleep 0.1
done
```

#### 5.6 清理

```bash
# 取消导出GPIO
make unexport-gpio PIN=18

# 或手动
echo 18 | sudo tee /sys/class/gpio/unexport
```

### 6. 卸载模块

```bash
sudo rmmod gpio

# 查看最终统计
dmesg | tail -10

# 预期输出：
# [12345.200] gpio_demo: === GPIO驱动卸载 ===
# [12345.201] gpio_demo: GPIO事件总数: 1
# [12345.202] gpio_demo: === 卸载完成 ===
```

## 核心API详解

### 请求和释放GPIO

```c
// 请求GPIO
int gpio_request(unsigned gpio, const char *label);

// 释放GPIO
void gpio_free(unsigned gpio);

// 示例
if (gpio_request(18, "my_led") < 0) {
    printk("GPIO请求失败\n");
    return -EINVAL;
}
// 使用GPIO
gpio_free(18);
```

### 设置方向

```c
// 设置为输出
int gpio_direction_output(unsigned gpio, int value);

// 设置为输入
int gpio_direction_input(unsigned gpio);

// 示例
gpio_direction_output(18, 0);  // 输出，初始低电平
gpio_direction_input(4);      // 输入
```

### 读写GPIO

```c
// 写入值
void gpio_set_value(unsigned gpio, int value);

// 读取值
int gpio_get_value(unsigned gpio);

// 示例
gpio_set_value(18, 1);  // 高电平
int val = gpio_get_value(4);  // 读取
```

### 检查GPIO有效性

```c
// 检查GPIO号是否有效
bool gpio_is_valid(unsigned gpio);

// 示例
if (!gpio_is_valid(18)) {
    printk("GPIO 18 无效\n");
}
```

### GPIO中断

```c
// 获取GPIO的中断号
int gpio_to_irq(unsigned gpio);

// 注册中断
int request_irq(unsigned int irq, irq_handler_t handler,
                unsigned long flags, const char *name, void *dev);

// 示例
int irq = gpio_to_irq(4);
request_irq(irq, handler, IRQF_TRIGGER_RISING, "my_irq", NULL);
```

## 常见问题

### Q: 如何找到正确的GPIO号？

树莓派BCM编号：
```bash
# 查看GPIO映射
gpio readall  # 需要wiringPi
```

Linux通用方法：
```bash
# 查看GPIO控制器
cat /sys/class/gpio/gpiochip*/label

# 查看基数
cat /sys/class/gpio/gpiochip*/base
```

### Q: GPIO号和物理引脚号不一样？

是的！不同编号系统：
- **物理引脚**：板子上的物理位置（如40引脚）
- **BCM编号**：Broadcom芯片内部编号（如GPIO18）
- **WiringPi编号**：WiringPi库的编号

**推荐使用BCM编号**（Linux GPIO子系统使用）。

### Q: 权限错误？

GPIO操作需要root权限：
```bash
# 使用sudo
sudo insmod gpio.ko

# 或设置权限
chmod 666 /sys/class/gpio/export
```

### Q: 如何测试GPIO？

1. LED测试（输出）
2. 按键测试（输入）
3. 万用表测量电平
4. 逻辑分析仪观察波形

## 扩展练习

1. **PWM控制**：使用PWM控制LED亮度
2. **I2C设备**：连接I2C传感器
3. **SPI设备**：连接SPI设备
4. **多个GPIO**：控制多个LED
5. **中断优化**：优化GPIO中断响应

## 硬件连接示例

### 树莓派LED测试

```
树莓派GPIO18 ──┬── [330Ω电阻] ── LED ─── GND
```

代码：
```bash
# 导出GPIO
echo 18 | sudo tee /sys/class/gpio/export
echo out | sudo tee /sys/class/gpio/gpio18/direction

# 点亮LED
echo 1 | sudo tee /sys/class/gpio/gpio18/value
```

### 树莓派按键测试

```
树莓派GPIO4 ──┬── [10kΩ上拉电阻] ─── 3.3V
               └── [按键] ─── GND
```

代码：
```bash
# 导出GPIO
echo 4 | sudo tee /sys/class/gpio/export
echo in | sudo tee /sys/class/gpio/gpio4/direction

# 读取按键状态
cat /sys/class/gpio/gpio4/value
```

## GPIO vs sysfs

| 特性 | 内核GPIO API | sysfs接口 |
|------|-------------|-----------|
| 性能 | 高 | 低 |
| 灵活性 | 高 | 低 |
| 中断 | 支持 | 有限 |
| 用户访问 | 间接 | 直接 |
| 适用场景 | 驱动程序 | 快速测试 |

## 下一步

继续学习：

- **kernel_spi** - SPI设备驱动
- **kernel_i2c** - I2C设备驱动

## 参考资料

- GPIO子系统文档：`Documentation/gpio/`
- 设备树GPIO绑定：`Documentation/devicetree/bindings/gpio/`
- libgpiod库：用户空间GPIO现代接口

---

祝你学习愉快！🚀
