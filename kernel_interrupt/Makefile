# Makefile for kernel_interrupt

obj-m += interrupt.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod interrupt.ko

unload:
	rmmod interrupt

log:
	dmesg | tail -30 | grep irq_demo

info:
	modinfo interrupt.ko

test:
	@echo "=== 测试中断模块 ==="
	@echo "1. 加载模块"
	@sudo insmod interrupt.ko
	@echo ""
	@echo "2. 查看统计"
	@cat /proc/irq_demo
	@echo ""
	@echo "3. 查看内核日志"
	@dmesg | tail -20 | grep irq_demo

# 显示系统中断信息
sys-irq:
	@echo "=== 系统中断信息 ==="
	@echo "1. 中断总数: $$(cat /proc/interrupts | wc -l)"
	@echo "2. IRQ数量: $$(cat /proc/irq | wc -l)"
	@echo ""
	@echo "3. 前10个中断:"
	@cat /proc/interrupts | head -10
	@echo ""
	@echo "4. CPU0中断:"
	@cat /proc/interrupts | awk '{print $$1, $$2}'

# 查看特定中断
show-irq:
	@echo "查看特定中断："
	@echo "用法: make show-irq IRQ=<中断号>"
	@if [ -z "$(IRQ)" ]; then \
		echo "示例: make show-irq IRQ=1"; \
		echo "可用的中断:"; \
		ls /proc/irq | grep -E '^[0-9]+' | head -10; \
	else \
		if [ -d /proc/irq/$(IRQ) ]; then \
			echo "=== 中断 $(IRQ) 信息 ==="; \
			echo "中断名称:"; \
			cat /proc/irq/$(IRQ)/* 2>/dev/null | head -10; \
		else \
			echo "中断 $(IRQ) 不存在"; \
		fi \
	fi

.PHONY: all clean load unload log info test sys-irq show-irq
