/**
 * kernel_char_device - 简单的字符设备驱动
 *
 * 这个demo演示了如何创建一个字符设备：
 * - 申请设备号
 * - 注册字符设备
 * - 实现file_operations（open, read, write, release）
 * - 用户态和内核态数据交换（copy_to_user, copy_from_user）
 * - 创建设备节点（/dev/chardev）
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/init.h>

#define DEVICE_NAME "chardev"
#define CLASS_NAME "chardev_class"
#define BUF_LEN 1024  // 缓冲区大小

/**
 * 全局变量
 */
static int majorNumber;                  // 主设备号
static struct class *chardevClass = NULL; // 设备类（用于自动创建设备节点）
static struct device *chardevDevice = NULL; // 设备结构
static struct cdev my_cdev;               // 字符设备结构
static char device_buffer[BUF_LEN];      // 设备缓冲区
static int data_size = 0;                // 当前数据大小

/**
 * 函数声明
 */
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

/**
 * file_operations 结构
 *
 * 这告诉内核如何操作这个设备
 */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

/**
 * dev_open - 打开设备
 *
 * 当用户调用 open() 时触发
 */
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "chardev: 设备被打开\n");
    return 0;  // 成功
}

/**
 * dev_read - 从设备读取数据
 *
 * @filep: 文件指针
 * @buffer: 用户空间缓冲区
 * @len: 请求读取的字节数
 * @offp: 文件偏移量
 */
static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offp)
{
    int bytes_to_read;

    printk(KERN_INFO "chardev: 读取请求 %zu 字节\n", len);

    // 如果没有数据，返回EOF
    if (data_size == 0) {
        printk(KERN_INFO "chardev: 没有数据可读\n");
        return 0;
    }

    // 计算实际要读取的字节数
    bytes_to_read = len < data_size ? len : data_size;

    // 将数据从内核空间复制到用户空间
    // 这是关键！不能直接 memcpy，必须使用 copy_to_user
    if (copy_to_user(buffer, device_buffer, bytes_to_read) != 0) {
        printk(KERN_ERR "chardev: copy_to_user 失败\n");
        return -EFAULT;
    }

    // 更新数据大小
    data_size -= bytes_to_read;

    printk(KERN_INFO "chardev: 读取了 %d 字节\n", bytes_to_read);

    return bytes_to_read;
}

/**
 * dev_write - 向设备写入数据
 *
 * @filep: 文件指针
 * @buffer: 用户空间缓冲区
 * @len: 要写入的字节数
 * @offp: 文件偏移量
 */
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offp)
{
    int bytes_to_write;

    printk(KERN_INFO "chardev: 写入请求 %zu 字节\n", len);

    // 检查缓冲区空间
    if (data_size + len > BUF_LEN) {
        printk(KERN_WARNING "chardev: 缓冲区空间不足\n");
        bytes_to_write = BUF_LEN - data_size;
    } else {
        bytes_to_write = len;
    }

    // 将数据从用户空间复制到内核空间
    // 同样关键！必须使用 copy_from_user
    if (copy_from_user(device_buffer + data_size, buffer, bytes_to_write) != 0) {
        printk(KERN_ERR "chardev: copy_from_user 失败\n");
        return -EFAULT;
    }

    data_size += bytes_to_write;

    printk(KERN_INFO "chardev: 写入了 %d 字节 (总共 %d 字节)\n", bytes_to_write, data_size);

    return bytes_to_write;
}

/**
 * dev_release - 关闭设备
 *
 * 当用户调用 close() 时触发
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "chardev: 设备被关闭\n");
    return 0;  // 成功
}

/**
 * 模块初始化
 */
static int __init chardev_init(void)
{
    int ret;

    printk(KERN_INFO "=== chardev: 初始化开始 ===\n");

    // 1. 动态申请主设备号
    // 0 表示动态分配，name 用于在 /proc/devices 显示
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ERR "chardev: register_chrdev 失败: %d\n", majorNumber);
        return majorNumber;
    }
    printk(KERN_INFO "chardev: 注册成功，主设备号 = %d\n", majorNumber);

    // 2. 创建设备类（用于自动创建设备节点）
    chardevClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(chardevClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ERR "chardev: class_create 失败\n");
        return PTR_ERR(chardevClass);
    }

    // 3. 创建设备节点（会自动在 /dev/ 创建）
    chardevDevice = device_create(chardevClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(chardevDevice)) {
        class_destroy(chardevClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ERR "chardev: device_create 失败\n");
        return PTR_ERR(chardevDevice);
    }

    printk(KERN_INFO "chardev: 设备节点 /dev/%s 创建成功\n", DEVICE_NAME);

    // 4. 初始化缓冲区
    memset(device_buffer, 0, BUF_LEN);
    data_size = 0;

    printk(KERN_INFO "=== chardev: 初始化完成 ===\n");

    return 0;
}

/**
 * 模块退出
 */
static void __exit chardev_exit(void)
{
    // 清理顺序：先销毁设备，再销毁类，最后注销字符设备
    device_destroy(chardevClass, MKDEV(majorNumber, 0));
    class_unregister(chardevClass);
    class_destroy(chardevClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "=== chardev: 模块已卸载 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("一个简单的字符设备驱动");
MODULE_VERSION("1.0");

module_init(chardev_init);
module_exit(chardev_exit);
