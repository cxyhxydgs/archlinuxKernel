# Makefile for kernel_spi

obj-m += spi.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod spi.ko

unload:
	rmmod spi

log:
	dmesg | tail -50 | grep spi_demo

info:
	modinfo spi.ko

test:
	@echo "=== 测试SPI模块 ==="
	@echo "1. 加载模块"
	@sudo insmod spi.ko
	@echo ""
	@echo "2. 查看内核日志"
	@dmesg | tail -30 | grep spi_demo

# 查看系统SPI信息
sys-spi:
	@echo "=== 系统SPI信息 ==="
	@echo "1. SPI控制器:"
	@ls /sys/bus/spi/devices/ 2>/dev/null | grep master | head -5 || echo "无SPI控制器"
	@echo ""
	@echo "2. SPI设备:"
	@ls /sys/bus/spi/devices/ 2>/dev/null | grep -v master | head -10 || echo "无SPI设备"
	@echo ""
	@echo "3. SPI驱动:"
	@ls /sys/bus/spi/drivers/ 2>/dev/null | head -10 || echo "无SPI驱动"

# 查看特定SPI总线
show-spi:
	@echo "查看SPI总线信息"
	@echo "用法: make show-spi BUS=<总线号>"
	@if [ -z "$(BUS)" ]; then \
		echo "示例: make show-spi BUS=0"; \
		echo ""; \
		echo "可用的SPI总线:"; \
		ls /sys/class/spi_master/ 2>/dev/null || echo "无"; \
	else \
		if [ -d /sys/class/spi_master/spi$(BUS) ]; then \
			echo "=== SPI总线 $(BUS) 信息 ==="; \
			echo "设备:"; \
			ls /sys/class/spi_master/spi$(BUS)/ 2>/dev/null | grep -E '^[0-9]' | head -5; \
		else \
			echo "SPI总线 $(BUS) 不存在"; \
		fi \
	fi

# 树莓派SPI测试
rpi-test:
	@echo "树莓派SPI测试"
	@if [ -f /proc/device-tree/model ] && grep -q "Raspberry Pi" /proc/device-tree/model; then \
		echo "✓ 检测到树莓派"; \
		echo "加载SPI驱动..."; \
		sudo insmod spi.ko; \
		dmesg | tail -30 | grep spi_demo; \
		echo ""; \
		echo "检查SPI设备:"; \
		ls /sys/bus/spi/devices/ 2>/dev/null | head -10; \
	else \
		echo "✗ 不是树莓派"; \
		echo "使用: make test (演示模式)"; \
	fi

# 模拟SPI设备（演示用）
mock-device:
	@echo "模拟SPI设备注册"
	@echo "注意: 这只是演示，实际使用需要硬件"
	@echo "创建模拟设备节点..."
	@sudo mkdir -p /sys/bus/spi/devices/spi0.0 2>/dev/null && \
		echo "mode" | sudo tee /sys/bus/spi/devices/spi0.0/modalias > /dev/null && \
		echo "模拟设备已创建" || \
		echo "模拟设备创建失败（可能已存在）"
	@ls /sys/bus/spi/devices/spi0.0 2>/dev/null || echo "设备不存在"

.PHONY: all clean load unload log info test sys-spi show-spi rpi-test mock-device
