# Makefile for kernel_device_tree

obj-m += devtree.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod devtree.ko

unload:
	rmmod devtree

log:
	dmesg | tail -40 | grep device_tree_demo

info:
	modinfo devtree.ko

# 测试模拟模式（没有实际设备树）
test-sim:
	@echo "=== 测试模拟模式（无设备树）==="
	@echo "1. 加载模块（使用默认参数）"
	@sudo insmod devtree.ko dt_string_param="测试字符串" dt_int_param=999
	@echo ""
	@echo "2. 查看内核日志"
	@dmesg | tail -25 | grep device_tree_demo

# 查找设备树节点
find-node:
	@echo "=== 查找设备树节点 ==="
	@echo "1. 检查设备树支持"
	@if [ -d /proc/device-tree ]; then \
		echo "✓ 设备树支持: 是"; \
	else \
		echo "✗ 设备树支持: 否"; \
	fi
	@echo ""
	@echo "2. 搜索 demo 节点"
	@find /proc/device-tree -name "*demo*" 2>/dev/null || echo "未找到 demo 节点"
	@echo ""
	@echo "3. 查看设备树根节点"
	@ls /proc/device-tree/ 2>/dev/null | head -10 || echo "设备树不可用"

# 显示设备树节点信息
show-dt:
	@echo "=== 设备树节点信息 ==="
	@if [ -f /proc/device-tree/model ]; then \
		echo "设备模型: $$(cat /proc/device-tree/model)"; \
	fi
	@if [ -f /proc/device-tree/compatible ]; then \
		echo "兼容字符串: $$(cat /proc/device-tree/compatible)"; \
	fi

.PHONY: all clean load unload log info test-sim find-node show-dt
