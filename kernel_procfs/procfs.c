/**
 * kernel_procfs - Proc文件系统接口
 *
 * 这个demo演示了如何创建 /proc 文件：
 * - 在 /proc 创建文件
 * - 使用 seq_file 接口实现读取
 * - 实现 write 操作
 * - 展示内核统计数据
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>

#define PROC_NAME "kernel_proc_demo"
#define PROC_DIR "demo_proc"

/**
 * 全局变量 - 统计数据
 */
static struct proc_dir_entry *proc_entry;   // /proc/kernel_proc_demo
static struct proc_dir_entry *proc_dir;     // /proc/demo_proc/

// 示例数据
static unsigned long call_count = 0;        // 被调用次数
static char last_message[256] = {0};        // 最后接收到的消息
static time_t last_update = 0;              // 最后更新时间

/**
 * proc_show - 使用 seq_file 接口显示数据
 *
 * @m: seq_file 结构
 * @v: 迭代器（未使用）
 */
static int proc_show(struct seq_file *m, void *v)
{
    call_count++;

    seq_printf(m, "=== Proc文件系统示例 ===\n\n");
    seq_printf(m, "调用次数: %lu\n", call_count);
    seq_printf(m, "最后消息: %s\n", last_message);
    seq_printf(m, "最后更新: %ld 秒前\n\n", jiffies_to_msecs(jiffies - last_update) / 1000);
    seq_printf(m, "当前Jiffies: %lu\n", jiffies);
    seq_printf(m, "HZ (时钟频率): %d\n", HZ);

    // 显示一些系统信息
    seq_printf(m, "\n=== 系统信息 ===\n");
    seq_printf(m, "当前进程: %s (PID: %d)\n", current->comm, current->pid);
    seq_printf(m, "UID: %d, GID: %d\n", current_uid().val, current_gid().val);

    return 0;
}

/**
 * proc_open - 打开 /proc 文件
 */
static int proc_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc_demo: 打开 /proc/%s\n", PROC_NAME);
    return single_open(file, proc_show, NULL);
}

/**
 * proc_write - 写入 /proc 文件
 *
 * @file: 文件结构
 * @buf: 用户空间缓冲区
 * @count: 写入字节数
 * @ppos: 文件偏移量
 */
static ssize_t proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char *kbuf;
    ssize_t ret;

    printk(KERN_INFO "proc_demo: 写入请求 %zu 字节\n", count);

    // 分配内核缓冲区（+1 用于 null 终止符）
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) {
        printk(KERN_ERR "proc_demo: kmalloc 失败\n");
        return -ENOMEM;
    }

    // 从用户空间复制数据
    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        printk(KERN_ERR "proc_demo: copy_from_user 失败\n");
        return -EFAULT;
    }

    // 添加 null 终止符
    kbuf[count] = '\0';

    // 更新最后消息和时间
    strncpy(last_message, kbuf, sizeof(last_message) - 1);
    last_update = jiffies;

    printk(KERN_INFO "proc_demo: 接收到消息: %s\n", kbuf);

    // 释放缓冲区
    kfree(kbuf);

    ret = count;

    return ret;
}

/**
 * file_operations 结构
 */
static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = proc_open,
    .read = seq_read,
    .write = proc_write,
    .llseek = seq_lseek,
    .release = single_release,
};

/**
 * ==================
 * seq_file 接口演示
 * ==================
 */

/**
 * dir_show - 显示目录中的某个文件
 */
static int dir_show(struct seq_file *m, void *v)
{
    seq_printf(m, "这是 /proc/%s/info 文件\n", PROC_DIR);
    seq_printf(m, "演示在 proc 目录中创建多个文件\n");
    seq_printf(m, "\n数据统计:\n");
    seq_printf(m, "  总调用次数: %lu\n", call_count);
    seq_printf(m, "  消息: %s\n", last_message);
    return 0;
}

static int dir_open(struct inode *inode, struct file *file)
{
    return single_open(file, dir_show, NULL);
}

static const struct file_operations dir_fops = {
    .owner = THIS_MODULE,
    .open = dir_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

/**
 * 模块初始化
 */
static int __init proc_init(void)
{
    printk(KERN_INFO "=== proc_demo: 初始化开始 ===\n");

    // 1. 创建主文件 /proc/kernel_proc_demo
    // 使用 proc_create（旧的 proc_create_data 现已废弃）
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) {
        printk(KERN_ERR "proc_demo: 创建 /proc/%s 失败\n", PROC_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "proc_demo: 创建 /proc/%s\n", PROC_NAME);

    // 2. 创建目录 /proc/demo_proc/
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) {
        proc_remove(proc_entry);
        printk(KERN_ERR "proc_demo: 创建 /proc/%s/ 失败\n", PROC_DIR);
        return -ENOMEM;
    }
    printk(KERN_INFO "proc_demo: 创建 /proc/%s/\n", PROC_DIR);

    // 3. 在目录中创建文件 /proc/demo_proc/info
    if (!proc_create("info", 0444, proc_dir, &dir_fops)) {
        proc_remove(proc_dir);
        proc_remove(proc_entry);
        printk(KERN_ERR "proc_demo: 创建 /proc/%s/info 失败\n", PROC_DIR);
        return -ENOMEM;
    }
    printk(KERN_INFO "proc_demo: 创建 /proc/%s/info\n", PROC_DIR);

    // 4. 初始化统计数据
    last_update = jiffies;
    strncpy(last_message, "等待输入...", sizeof(last_message) - 1);

    printk(KERN_INFO "=== proc_demo: 初始化完成 ===\n");

    return 0;
}

/**
 * 模块退出
 */
static void __exit proc_exit(void)
{
    // 清理顺序：先删除文件，再删除目录
    proc_remove(proc_dir);   // 删除整个目录及其内容
    proc_remove(proc_entry);  // 删除主文件

    printk(KERN_INFO "=== proc_demo: 模块已卸载 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Proc文件系统示例");
MODULE_VERSION("1.0");

module_init(proc_init);
module_exit(proc_exit);
