# Makefile for kernel_procfs

obj-m += procfs.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod procfs.ko

unload:
	rmmod procfs

log:
	dmesg | tail -20 | grep proc_demo

info:
	modinfo procfs.ko

test:
	@echo "=== 测试 /proc/kernel_proc_demo ==="
	@echo "1. 读取数据"
	@cat /proc/kernel_proc_demo
	@echo ""
	@echo "2. 写入数据"
	@echo "Hello from shell!" | sudo tee /proc/kernel_proc_demo > /dev/null
	@echo ""
	@echo "3. 再次读取"
	@cat /proc/kernel_proc_demo
	@echo ""
	@echo "4. 查看 /proc/demo_proc/info"
	@cat /proc/demo_proc/info

.PHONY: all clean load unload log info test
