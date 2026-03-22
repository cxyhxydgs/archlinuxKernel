# Makefile for kernel_char_device

obj-m += chardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod chardev.ko

unload:
	rmmod chardev

log:
	dmesg | tail -30

info:
	modinfo chardev.ko

test:
	@echo "=== 测试字符设备 ==="
	@echo "1. 写入数据到设备"
	@echo -n "Hello, Char Device!" | sudo tee /dev/chardev > /dev/null
	@echo ""
	@echo "2. 从设备读取数据"
	@sudo cat /dev/chardev
	@echo ""
	@echo "3. 查看内核日志"
	@dmesg | tail -10 | grep chardev

help:
	@echo "可用的make目标："
	@echo "  make all    - 编译模块"
	@echo "  make clean  - 清理编译文件"
	@echo "  make load   - 加载模块"
	@echo "  make unload - 卸载模块"
	@echo "  make log    - 查看内核日志"
	@echo "  make info   - 显示模块信息"
	@echo "  make test   - 测试设备读写"

.PHONY: all clean load unload log info test help
