# Makefile for kernel_sysfs

obj-m += sysfs.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod sysfs.ko

unload:
	rmmod sysfs

log:
	dmesg | tail -20 | grep sysfs_demo

info:
	modinfo sysfs.ko

test:
	@echo "=== Sysfs 测试 ==="
	@echo ""
	@echo "1. 读取所有属性"
	@echo "   int_value: $$(cat /sys/kernel/demo_sysfs/int_value)"
	@echo "   str_value: $$(cat /sys/kernel/demo_sysfs/str_value)"
	@echo "   bool_value: $$(cat /sys/kernel/demo_sysfs/bool_value)"
	@echo ""
	@echo "2. 写入属性"
	@echo "100" | sudo tee /sys/kernel/demo_sysfs/int_value > /dev/null
	@echo "hello" | sudo tee /sys/kernel/demo_sysfs/str_value > /dev/null
	@echo "0" | sudo tee /sys/kernel/demo_sysfs/bool_value > /dev/null
	@echo ""
	@echo "3. 再次读取"
	@echo "   int_value: $$(cat /sys/kernel/demo_sysfs/int_value)"
	@echo "   str_value: $$(cat /sys/kernel/demo_sysfs/str_value)"
	@echo "   bool_value: $$(cat /sys/kernel/demo_sysfs/bool_value)"

tree:
	@echo "=== Sysfs 目录结构 ==="
	@tree /sys/kernel/demo_sysfs 2>/dev/null || find /sys/kernel/demo_sysfs -type f

.PHONY: all clean load unload log info test tree
