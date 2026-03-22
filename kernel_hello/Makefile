# Makefile for kernel_hello
#
# 内核模块的Makefile有特定的格式
# obj-m += 告诉内核构建系统需要编译哪些模块

# 模块名（对应源文件名去掉.c）
obj-m += hello.o

# 内核源码路径 - 通常自动检测
# 如果编译失败，取消下面的注释并设置正确的路径
# KDIR := /lib/modules/$(shell uname -r)/build

# 当前目录
PWD := $(shell pwd)

# 编译目标
all:
	make -C $(KDIR) M=$(PWD) modules

# 清理编译文件
clean:
	make -C $(KDIR) M=$(PWD) clean

# 加载模块（需要root权限）
load:
	insmod hello.ko

# 卸载模块（需要root权限）
unload:
	rmmod hello

# 查看内核日志
log:
	dmesg | tail -20

# 显示模块信息
info:
	modinfo hello.ko

# 查看模块参数
params:
	cat /sys/module/hello/parameters/count 2>/dev/null || echo "模块未加载"

# 帮助信息
help:
	@echo "可用的make目标："
	@echo "  make all    - 编译模块"
	@echo "  make clean  - 清理编译文件"
	@echo "  make load   - 加载模块 (需要sudo)"
	@echo "  make unload - 卸载模块 (需要sudo)"
	@echo "  make log    - 查看内核日志"
	@echo "  make info   - 显示模块信息"
	@echo "  make params - 查看模块参数"
	@echo ""
	@echo "高级用法："
	@echo "  sudo insmod hello.ko count=10  # 加载时设置参数"
	@echo "  dmesg -w                        # 实时查看内核日志"

.PHONY: all clean load unload log info params help
