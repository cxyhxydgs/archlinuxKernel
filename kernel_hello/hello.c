/**
 * kernel_hello - 最简单的内核模块
 *
 * 这个模块演示了内核模块的基本结构：
 * - 模块初始化函数
 * - 模块退出函数
 * - 模块元数据（作者、描述、许可证）
 * - 模块参数
 * - 内核日志输出 (printk)
 */

#include <linux/module.h>     // 必需：所有模块都要包含
#include <linux/kernel.h>     // 用于 printk()、KERN_* 宏
#include <linux/init.h>       // 用于 __init 和 __exit 宏

/**
 * 模块参数 - 可以在加载时动态传递
 *
 * 用法：insmod hello.ko count=5
 * 查看：cat /sys/module/hello/parameters/count
 */
static int count = 3;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "打印Hello的次数 (默认: 3)");

/**
 * 模块初始化函数
 *
 * 当模块被加载时调用（insmod）
 * 返回0表示成功，非0表示失败
 */
static int __init hello_init(void)
{
    int i;

    printk(KERN_INFO "=== Hello模块开始初始化 ===\n");
    printk(KERN_INFO "参数 count = %d\n", count);

    // 演示循环打印
    for (i = 0; i < count; i++) {
        printk(KERN_INFO "Hello, Linux Kernel! [%d/%d]\n", i + 1, count);
    }

    printk(KERN_INFO "=== Hello模块初始化完成 ===\n");

    return 0;
}

/**
 * 模块退出函数
 *
 * 当模块被卸载时调用（rmmod）
 */
static void __exit hello_exit(void)
{
    printk(KERN_INFO "=== Hello模块已卸载 ===\n");
    printk(KERN_INFO "再见！\n");
}

/**
 * 模块元数据
 *
 * MODULE_LICENSE("GPL") 是必需的，否则内核会警告
 * 其他元数据是可选的，但建议添加
 */

// 许可证 - 必需！GPL表示开源兼容
MODULE_LICENSE("GPL");

// 作者信息
MODULE_AUTHOR("Your Name <your.email@example.com>");

// 模块描述
MODULE_DESCRIPTION("一个简单的Hello World内核模块");

// 模块版本
MODULE_VERSION("1.0");

/**
 * 模块注册
 *
 * 告诉内核哪个函数用于初始化，哪个用于退出
 */
module_init(hello_init);
module_exit(hello_exit);
