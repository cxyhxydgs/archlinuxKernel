# Makefile for kernel_user_comm

obj-m += user_comm.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# 加载模块
load:
	insmod user_comm.ko

# 卸载模块
unload:
	rmmod user_comm

# 查看内核日志
log:
	dmesg | tail -50 | grep user_comm

# 查看模块信息
info:
	modinfo user_comm.ko

# 测试字符设备
test-char:
	@echo "========================================"
	@echo "测试1: 字符设备通信"
	@echo "========================================"
	@echo "1. 写入数据"
	@echo "Hello from shell!" | sudo tee /dev/user_comm > /dev/null
	@echo ""
	@echo "2. 读取数据"
	@sudo cat /dev/user_comm
	@echo ""
	@echo "3. 查看内核日志"
	@dmesg | tail -10 | grep user_comm
	@echo ""

# 测试Proc文件
test-proc:
	@echo "========================================"
	@echo "测试2: Proc文件通信"
	@echo "========================================"
	@echo "1. 写入数据"
	@echo "Test via proc" | sudo tee /proc/user_comm_proc > /dev/null
	@echo ""
	@echo "2. 读取数据"
	@sudo cat /proc/user_comm_proc
	@echo ""
	@echo "3. 查看内核日志"
	@dmesg | tail -10 | grep user_comm
	@echo ""

# 测试共享内存
test-shm:
	@echo "========================================"
	@echo "测试3: 共享内存通信"
	@echo "========================================"
	@echo "1. 获取共享内存ID"
	@sudo ipcs -m | grep user_comm
	@echo ""
	@echo "2. 在内核写入数据"
	@echo "Hello from kernel!" | sudo tee /dev/user_comm > /dev/null
	@echo ""
	@echo "3. 在用户空间读取共享内存"
	@sudo cat /dev/user_comm
	@echo ""
	@echo "4. 查看内核日志"
	@dmesg | tail -10 | grep user_comm
	@echo ""

# 全部测试
test-all:
	@echo "========================================"
	@echo "=== 开始全部测试 ==="
	@echo "========================================"
	@make test-char
	@make test-proc
	@make test-shm
	@echo "========================================"
	@echo "=== 测试完成 ==="
	@echo "========================================"

# 创建用户测试程序
create-user-test:
	@echo "创建用户测试程序..."
	@cat > user_test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_SIZE 4096

int main() {
    int fd;
    char buf[256];
    int shm_id;

    printf("=== 用户测试程序 ===\n\n");

    // 测试1: 字符设备
    printf("【测试1】字符设备通信\n");
    printf("写入: Hello from user!\n");
    fd = open("/dev/user_comm", O_WRONLY);
    if (fd >= 0) {
        write(fd, "Hello from user!", 16);
        close(fd);
    }

    sleep(1);

    printf("读取: ");
    fd = open("/dev/user_comm", O_RDONLY);
    if (fd >= 0) {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
        close(fd);
    }
    printf("\n");

    // 测试2: Proc文件
    printf("【测试2】Proc文件通信\n");
    printf("写入: Test via proc\n");
    fd = open("/proc/user_comm_proc", O_WRONLY);
    if (fd >= 0) {
        write(fd, "Test via proc", 13);
        close(fd);
    }

    sleep(1);

    printf("读取: ");
    fd = open("/proc/user_comm_proc", O_RDONLY);
    if (fd >= 0) {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
        close(fd);
    }
    printf("\n");

    // 测试3: 共享内存
    printf("【测试3】共享内存通信\n");
    shm_id = shmget(0, SHM_SIZE, 0666);
    if (shm_id >= 0) {
        char *shm_addr = shmat(shm_id, NULL, 0);
        if (shm_addr != (char *)-1) {
            printf("读取共享内存: %s\n", shm_addr);
            shmdt(shm_addr);
        }
    }
    printf("\n");

    printf("=== 测试完成 ===\n");
    return 0;
}
EOF
	@echo "用户测试程序已创建: user_test.c"
	@echo "编译: gcc -o user_test user_test.c"
	@echo "运行: sudo ./user_test"

# 编译并运行用户测试程序
run-user-test:
	@gcc -o user_test user_test.c
	@echo "=== 运行用户测试程序 ==="
	@sudo ./user_test

.PHONY: all clean load unload log info test-char test-proc test-shm test-all create-user-test run-user-test
