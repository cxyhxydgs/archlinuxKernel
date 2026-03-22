#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_DEBUG 1

// 模拟内核日志函数
void kernel_log(int level, const char *message) {
    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0'; // 移除换行符
    
    if (level == LOG_LEVEL_INFO) {
        printf("[INFO] [%s] %s\n", time_str, message);
    } else {
        printf("[DEBUG] [%s] %s\n", time_str, message);
    }
}

// 模拟内核模块结构
typedef struct {
    char name[64];
    int loaded;
    void (*init)(void);
    void (*exit)(void);
} kernel_module_t;

// 初始化函数
static void hello_init(void) {
    kernel_log(LOG_LEVEL_INFO, "Hello World! This is a simple kernel module.");
    kernel_log(LOG_LEVEL_INFO, "Kernel module loaded successfully.");
}

// 清理函数
static void hello_exit(void) {
    kernel_log(LOG_LEVEL_INFO, "Goodbye World! Kernel module unloaded.");
}

// 模块实例
static kernel_module_t hello_module = {
    .name = "hello_module",
    .loaded = 0,
    .init = hello_init,
    .exit = hello_exit
};

// 模拟模块加载
int module_load(kernel_module_t *module) {
    if (module->loaded) {
        printf("Module %s is already loaded.\n", module->name);
        return -1;
    }
    
    if (module->init) {
        module->init();
    }
    
    module->loaded = 1;
    printf("Module %s loaded successfully.\n", module->name);
    return 0;
}

// 模拟模块卸载
int module_unload(kernel_module_t *module) {
    if (!module->loaded) {
        printf("Module %s is not loaded.\n", module->name);
        return -1;
    }
    
    if (module->exit) {
        module->exit();
    }
    
    module->loaded = 0;
    printf("Module %s unloaded successfully.\n", module->name);
    return 0;
}

// 模拟列出已加载模块
void list_modules(void) {
    printf("Currently loaded modules:\n");
    printf("  %s [%s]\n", hello_module.name, 
           hello_module.loaded ? "LOADED" : "NOT LOADED");
}

int main() {
    printf("=== 用户空间内核模块模拟器 ===\n\n");
    
    // 显示初始状态
    printf("1. 初始状态:\n");
    list_modules();
    
    // 加载模块
    printf("\n2. 加载模块...\n");
    if (module_load(&hello_module) == 0) {
        printf("✓ 模块加载成功！\n");
    }
    
    // 再次显示状态
    printf("\n3. 加载后的状态:\n");
    list_modules();
    
    // 再次加载测试
    printf("\n4. 重复加载测试...\n");
    if (module_load(&hello_module) != 0) {
        printf("✓ 重复加载被正确拒绝！\n");
    }
    
    // 卸载模块
    printf("\n5. 卸载模块...\n");
    if (module_unload(&hello_module) == 0) {
        printf("✓ 模块卸载成功！\n");
    }
    
    // 最终状态
    printf("\n6. 最终状态:\n");
    list_modules();
    
    // 再次卸载测试
    printf("\n7. 重复卸载测试...\n");
    if (module_unload(&hello_module) != 0) {
        printf("✓ 重复卸载被正确拒绝！\n");
    }
    
    printf("\n=== 模拟测试完成 ===\n");
    return 0;
}