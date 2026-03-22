# Makefile for kernel_gpio

obj-m += gpio.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod gpio.ko

unload:
	rmmod gpio

log:
	dmesg | tail -40 | grep gpio_demo

info:
	modinfo gpio.ko

test:
	@echo "=== 测试GPIO模块 ==="
	@echo "1. 加载模块（模拟模式）"
	@sudo insmod gpio.ko gpio_out_num=999 gpio_in_num=888
	@echo ""
	@echo "2. 查看内核日志"
	@dmesg | tail -35 | grep gpio_demo

# 测试特定GPIO
test-pins:
	@echo "测试特定GPIO引脚..."
	@echo "用法: make test-pins OUT=<输出引脚> IN=<输入引脚>"
	@if [ -z "$(OUT)" ] || [ -z "$(IN)" ]; then \
		echo "示例: make test-pins OUT=18 IN=4"; \
	else \
		echo "使用 GPIO: 输出=$(OUT), 输入=$(IN)"; \
		sudo insmod gpio.ko gpio_out_num=$(OUT) gpio_in_num=$(IN); \
		echo ""; \
		echo "查看日志:"; \
		dmesg | tail -30 | grep gpio_demo; \
	fi

# 查看系统GPIO信息
sys-gpio:
	@echo "=== 系统GPIO信息 ==="
	@echo "1. GPIO控制器数量:"
	@ls -d /sys/class/gpio/gpiochip* 2>/dev/null | wc -l
	@echo ""
	@echo "2. GPIO控制器:"
	@ls -d /sys/class/gpio/gpiochip* 2>/dev/null | head -5
	@echo ""
	@echo "3. 已导出的GPIO:"
	@ls /sys/class/gpio/ 2>/dev/null | grep -v gpiochip | head -10 || echo "无"

# 导出GPIO用于用户空间测试
export-gpio:
	@echo "导出GPIO用于测试"
	@echo "用法: make export-gpio PIN=<引脚号>"
	@if [ -z "$(PIN)" ]; then \
		echo "示例: make export-gpio PIN=18"; \
	else \
		echo "导出 GPIO $(PIN)..."; \
		if [ -d /sys/class/gpio/gpio$(PIN) ]; then \
			echo "GPIO $(PIN) 已导出"; \
		else \
			echo $(PIN) | sudo tee /sys/class/gpio/export; \
		fi; \
		echo ""; \
		echo "设置方向: out"; \
		echo out | sudo tee /sys/class/gpio/gpio$(PIN)/direction; \
		echo ""; \
		echo "设置值: 1 (高电平)"; \
		echo 1 | sudo tee /sys/class/gpio/gpio$(PIN)/value; \
		sleep 1; \
		echo "设置值: 0 (低电平)"; \
		echo 0 | sudo tee /sys/class/gpio/gpio$(PIN)/value; \
	fi

# 取消导出GPIO
unexport-gpio:
	@echo "取消导出GPIO"
	@echo "用法: make unexport-gpio PIN=<引脚号>"
	@if [ -n "$(PIN)" ]; then \
		echo $(PIN) | sudo tee /sys/class/gpio/unexport; \
		echo "GPIO $(PIN) 已取消导出"; \
	fi

# 树莓派GPIO快速测试（如果是树莓派）
rpi-test:
	@echo "树莓派GPIO快速测试"
	@if [ -f /proc/device-tree/model ] && grep -q "Raspberry Pi" /proc/device-tree/model; then \
		echo "✓ 检测到树莓派"; \
		echo "加载树莓派GPIO测试..."; \
		sudo insmod gpio.ko gpio_out_num=18 gpio_in_num=4; \
		dmesg | tail -30 | grep gpio_demo; \
	else \
		echo "✗ 不是树莓派"; \
		echo "使用: make test (模拟模式)"; \
	fi

.PHONY: all clean load unload log info test test-pins sys-gpio export-gpio unexport-gpio rpi-test
