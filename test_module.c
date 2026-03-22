#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#define SYSCALL_HELLO 436  // 假设的系统调用号，实际使用时需要确认

int main() {
    printf("=== Linux 内核模块测试程序 ===\n\n");
    
    // 检查内核模块是否加载
    printf("1. 检查内核模块是否加载...\n");
    system("lsmod | grep hello_module");
    
    // 尝试加载模块
    printf("\n2. 尝试加载内核模块...\n");
    int load_result = system("sudo insmod hello_module.ko");
    if (load_result == 0) {
        printf("✓ 模块加载成功！\n");
    } else {
        printf("✗ 模块加载失败！\n");
    }
    
    // 再次检查模块状态
    printf("\n3. 再次检查模块状态...\n");
    system("lsmod | grep hello_module");
    
    // 查看内核日志
    printf("\n4. 查看内核日志...\n");
    system("dmesg | tail -5");
    
    // 卸载模块
    printf("\n5. 卸载内核模块...\n");
    int unload_result = system("sudo rmmod hello_module");
    if (unload_result == 0) {
        printf("✓ 模块卸载成功！\n");
    } else {
        printf("✗ 模块卸载失败！\n");
    }
    
    // 最终检查
    printf("\n6. 最终检查模块状态...\n");
    system("lsmod | grep hello_module");
    
    // 最终查看日志
    printf("\n7. 最终查看内核日志...\n");
    system("dmesg | tail -3");
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}