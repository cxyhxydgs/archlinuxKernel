/*
 * kernel_test - 简单的内核测试模块
 * 用于验证 Git 流程
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cxyhxydgs");
MODULE_DESCRIPTION("Git 流程验证测试模块");
MODULE_VERSION("1.0");

static int __init test_init(void)
{
    pr_info("kernel_test: 模块加载成功！\n");
    pr_info("kernel_test: Git 流程验证通过！\n");
    return 0;
}

static void __exit test_exit(void)
{
    pr_info("kernel_test: 模块卸载成功！\n");
}

module_init(test_init);
module_exit(test_exit);
