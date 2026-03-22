/**
 * user_test.c - 用户空间测试程序
 *
 * 这个程序测试内核模块和用户空间的通信
 */

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

    printf("========================================\n");
    printf("  用户测试程序 - 内核态和用户态通信\n");
    printf("========================================\n\n");

    // 测试1: 字符设备
    printf("【测试1】字符设备通信\n");
    printf("写入: Hello from user!\n");
    fd = open("/dev/user_comm", O_WRONLY);
    if (fd >= 0) {
        write(fd, "Hello from user!", 16);
        close(fd);
    } else {
        printf("错误: 无法打开 /dev/user_comm\n");
    }

    sleep(1);

    printf("读取: ");
    fd = open("/dev/user_comm", O_RDONLY);
    if (fd >= 0) {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
        close(fd);
    } else {
        printf("错误: 无法打开 /dev/user_comm\n");
    }
    printf("\n");

    // 测试2: Proc文件
    printf("【测试2】Proc文件通信\n");
    printf("写入: Test via proc\n");
    fd = open("/proc/user_comm_proc", O_WRONLY);
    if (fd >= 0) {
        write(fd, "Test via proc", 13);
        close(fd);
    } else {
        printf("错误: 无法打开 /proc/user_comm_proc\n");
    }

    sleep(1);

    printf("读取: ");
    fd = open("/proc/user_comm_proc", O_RDONLY);
    if (fd >= 0) {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
        close(fd);
    } else {
        printf("错误: 无法打开 /proc/user_comm_proc\n");
    }
    printf("\n");

    // 测试3: 共享内存
    printf("【测试3】共享内存通信\n");
    shm_id = shmget(0, SHM_SIZE, 0666);
    if (shm_id < 0) {
        printf("错误: 无法获取共享内存\n");
    } else {
        char *shm_addr = shmat(shm_id, NULL, 0);
        if (shm_addr == (char *)-1) {
            printf("错误: 无法映射共享内存\n");
        } else {
            printf("读取共享内存: %s\n", shm_addr);
            shmdt(shm_addr);
        }
    }
    printf("\n");

    printf("========================================\n");
    printf("  测试完成！\n");
    printf("========================================\n");

    return 0;
}
