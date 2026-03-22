#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chang Xiaoyi");
MODULE_DESCRIPTION("A simple Hello World kernel module");
MODULE_VERSION("1.0");

static int __init hello_init(void) {
    printk(KERN_INFO "Hello World! This is a simple kernel module.\n");
    printk(KERN_INFO "Kernel module loaded successfully.\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye World! Kernel module unloaded.\n");
}

module_init(hello_init);
module_exit(hello_exit);