/**
 * kernel_user_comm - 内核态和用户态通信演示
 *
 * 这个模块演示了三种内核态和用户态通信方式：
 * 1. 字符设备通信 - 通过 /dev 设备文件
 * 2. Proc文件通信 - 通过 /proc 文件
 * 3. 共享内存通信 - 通过共享内存区域
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/kfifo.h>
#include <linux/sched.h>

// ========================
// 方案1: 字符设备通信
// ========================
#define CHAR_DEV_NAME "user_comm"
#define CHAR_DEV_CLASS "user_comm_class"
#define CHAR_DEV_BUF_SIZE 256

static int char_major;
static struct class *char_class;
static struct device *char_device;
static char char_buf[CHAR_DEV_BUF_SIZE];
static int char_buf_len = 0;

// 字符设备文件操作
static int char_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "字符设备: 打开\n");
    return 0;
}

static int char_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "字符设备: 关闭\n");
    return 0;
}

static ssize_t char_read(struct file *file, char __user *buffer, size_t len, loff_t *offp) {
    int bytes_read;

    if (char_buf_len == 0) {
        return 0;  // 没有数据
    }

    bytes_read = len < char_buf_len ? len : char_buf_len;
    copy_to_user(buffer, char_buf, bytes_read);
    char_buf_len = 0;

    printk(KERN_INFO "字符设备: 读取 %d 字节\n", bytes_read);
    return bytes_read;
}

static ssize_t char_write(struct file *file, const char __user *buffer, size_t len, loff_t *offp) {
    if (len >= CHAR_DEV_BUF_SIZE) {
        len = CHAR_DEV_BUF_SIZE - 1;
    }

    copy_from_user(char_buf, buffer, len);
    char_buf[len] = '\0';
    char_buf_len = len;

    printk(KERN_INFO "字符设备: 写入 '%s'\n", char_buf);
    return len;
}

static struct file_operations char_fops = {
    .owner = THIS_MODULE,
    .open = char_open,
    .read = char_read,
    .write = char_write,
    .release = char_release,
};

// ========================
// 方案2: Proc文件通信
// ========================
#define PROC_NAME "user_comm_proc"

static char proc_msg[256];
static struct proc_dir_entry *proc_entry;

static ssize_t proc_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offp) {
    if (len >= sizeof(proc_msg)) {
        len = sizeof(proc_msg) - 1;
    }

    copy_from_user(proc_msg, buffer, len);
    proc_msg[len] = '\0';

    printk(KERN_INFO "Proc文件: 写入 '%s'\n", proc_msg);
    return len;
}

static ssize_t proc_read(struct file *filep, char __user *buffer, size_t len, loff_t *offp) {
    return sprintf(buffer, "%s\n", proc_msg);
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .read = proc_read,
    .write = proc_write,
};

// ========================
// 方案3: 共享内存通信
// ========================
#define SHM_SIZE 4096

static int shm_id;
static void *shm_addr;

// ========================
// 模块初始化
// ========================
static int __init user_comm_init(void) {
    int ret;

    printk(KERN_INFO "=== 内核态和用户态通信模块加载 ===\n");

    // ========================
    // 1. 字符设备初始化
    // ========================
    char_major = register_chrdev(0, CHAR_DEV_NAME, &char_fops);
    if (char_major < 0) {
        printk(KERN_ERR "字符设备注册失败\n");
        return char_major;
    }

    char_class = class_create(THIS_MODULE, CHAR_DEV_CLASS);
    if (IS_ERR(char_class)) {
        unregister_chrdev(char_major, CHAR_DEV_NAME);
        printk(KERN_ERR "字符设备类创建失败\n");
        return PTR_ERR(char_class);
    }

    char_device = device_create(char_class, NULL, MKDEV(char_major, 0), NULL, CHAR_DEV_NAME);
    if (IS_ERR(char_device)) {
        class_destroy(char_class);
        unregister_chrdev(char_major, CHAR_DEV_NAME);
        printk(KERN_ERR "字符设备创建失败\n");
        return PTR_ERR(char_device);
    }

    printk(KERN_INFO "字符设备创建成功: /dev/%s (主设备号: %d)\n", CHAR_DEV_NAME, char_major);

    // ========================
    // 2. Proc文件初始化
    // ========================
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) {
        printk(KERN_ERR "Proc文件创建失败\n");
        device_destroy(char_class, MKDEV(char_major, 0));
        class_destroy(char_class);
        unregister_chrdev(char_major, CHAR_DEV_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "Proc文件创建成功: /proc/%s\n", PROC_NAME);

    // ========================
    // 3. 共享内存初始化
    // ========================
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE, 0666);
    if (shm_id < 0) {
        printk(KERN_ERR "共享内存创建失败\n");
        proc_remove(proc_entry);
        device_destroy(char_class, MKDEV(char_major, 0));
        class_destroy(char_class);
        unregister_chrdev(char_major, CHAR_DEV_NAME);
        return shm_id;
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        printk(KERN_ERR "共享内存映射失败\n");
        shmctl(shm_id, IPC_RMID, NULL);
        proc_remove(proc_entry);
        device_destroy(char_class, MKDEV(char_major, 0));
        class_destroy(char_class);
        unregister_chrdev(char_major, CHAR_DEV_NAME);
        return -1;
    }

    // 初始化共享内存
    memset(shm_addr, 0, SHM_SIZE);

    printk(KERN_INFO "共享内存创建成功 (ID: %d)\n", shm_id);

    // ========================
    // 4. 显示信息
    // ========================
    printk(KERN_INFO "\n");
    printk(KERN_INFO "=== 通信方式列表 ===\n");
    printk(KERN_INFO "1. 字符设备: /dev/%s\n", CHAR_DEV_NAME);
    printk(KERN_INFO "2. Proc文件: /proc/%s\n", PROC_NAME);
    printk(KERN_INFO "3. 共享内存: ID=%d\n", shm_id);
    printk(KERN_INFO "==================\n");
    printk(KERN_INFO "\n");

    printk(KERN_INFO "=== 模块初始化完成 ===\n");

    return 0;
}

// ========================
// 模块退出
// ========================
static void __exit user_comm_exit(void) {
    printk(KERN_INFO "=== 模块卸载 ===\n");

    // 清理共享内存
    shmdt(shm_addr);
    shmctl(shm_id, IPC_RMID, NULL);

    // 清理Proc文件
    proc_remove(proc_entry);

    // 清理字符设备
    device_destroy(char_class, MKDEV(char_major, 0));
    class_destroy(char_class);
    unregister_chrdev(char_major, CHAR_DEV_NAME);

    printk(KERN_INFO "=== 模块卸载完成 ===\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("内核态和用户态通信演示");
MODULE_VERSION("1.0");

module_init(user_comm_init);
module_exit(user_comm_exit);
