/**
 * kernel_interrupt - 中断处理示例
 *
 * 这个demo演示了Linux中断处理的基本概念：
 * - 注册中断处理函数
 * - 顶半部和底半部（tasklet/workqueue）
 * - 中断共享
 * - 中断统计
 *
 * 注意：这个demo演示中断API的使用，不会实际操作硬件
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/init.h>

#define IRQ_NAME "demo_interrupt"
#define PROC_NAME "irq_demo"

/**
 * 统计数据
 */
static unsigned long irq_count = 0;            // 中断总次数
static unsigned long tasklet_count = 0;       // tasklet执行次数
static unsigned long workqueue_count = 0;      // workqueue执行次数
static unsigned long last_jiffies = 0;         // 上次中断时间
static struct proc_dir_entry *proc_entry;      // /proc/irq_demo

/**
 * ========================
 * Tasklet - 底半部之一
 * ========================
 */

/**
 * demo_tasklet_func - tasklet处理函数
 *
 * @data: 传递给tasklet的数据
 */
static void demo_tasklet_func(unsigned long data)
{
    tasklet_count++;

    printk(KERN_INFO "irq_demo: Tasklet执行 [%lu], 数据: %lu\n",
           tasklet_count, data);
}

// 声明tasklet
// DECLARE_TASKLET(name, func, data)
DECLARE_TASKLET(demo_tasklet, demo_tasklet_func, 0);

/**
 * ========================
 * Workqueue - 底半部之二
 * ========================
 */

/**
 * demo_work_func - workqueue处理函数
 *
 * @work: work_struct 结构
 */
static void demo_work_func(struct work_struct *work)
{
    workqueue_count++;

    printk(KERN_INFO "irq_demo: Workqueue执行 [%lu]\n", workqueue_count);
}

// 声明work
static DECLARE_WORK(demo_work, demo_work_func);

/**
 * ========================
 * 中断处理函数
 * ========================
 */

/**
 * demo_irq_handler - 中断顶半部处理函数
 *
 * @irq: 中断号
 * @dev_id: 设备ID（在注册时传递）
 */
static irqreturn_t demo_irq_handler(int irq, void *dev_id)
{
    unsigned long now = jiffies;
    unsigned long interval = jiffies_to_msecs(now - last_jiffies);

    irq_count++;
    last_jiffies = now;

    printk(KERN_INFO "irq_demo: 中断触发! [%lu], 间隔: %lu ms\n",
           irq_count, interval);

    // 调度tasklet（底半部）
    // tasklet在软中断上下文执行，不能睡眠
    tasklet_schedule(&demo_tasklet);

    // 调度workqueue（底半部）
    // workqueue在进程上下文执行，可以睡眠
    schedule_work(&demo_work);

    return IRQ_HANDLED;  // 表示中断已被处理
}

/**
 * ========================
 * Proc文件接口
 * ======================== */

/**
 * proc_show - 显示中断统计
 */
static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "=== 中断统计 ===\n\n");
    seq_printf(m, "中断总次数:       %lu\n", irq_count);
    seq_printf(m, "Tasklet次数:      %lu\n", tasklet_count);
    seq_printf(m, "Workqueue次数:    %lu\n", workqueue_count);
    seq_printf(m, "\n");
    seq_printf(m, "当前Jiffies:      %lu\n", jiffies);
    seq_printf(m, "HZ (时钟频率):    %d\n", HZ);
    seq_printf(m, "\n");
    seq_printf(m, "=== 中断说明 ===\n");
    seq_printf(m, "这是一个演示模块，展示中断处理API的使用\n");
    seq_printf(m, "实际使用需要硬件产生中断\n");

    return 0;
}

/**
 * proc_open
 */
static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

/**
 * ========================
 * 模块初始化
 * ========================
 */

/**
 * irq_init - 模块初始化
 */
static int __init irq_init(void)
{
    int ret;
    int test_irq;

    printk(KERN_INFO "irq_demo: === 模块初始化 ===\n");

    // 1. 创建proc接口
    proc_entry = proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    if (!proc_entry) {
        printk(KERN_ERR "irq_demo: 创建 /proc/%s 失败\n", PROC_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "irq_demo: 创建 /proc/%s\n", PROC_NAME);

    // 2. 演示：尝试注册一个虚拟IRQ
    // 注意：这通常会失败，因为IRQ不存在
    // 实际使用时，应该从设备树或硬件获取正确的IRQ号

    printk(KERN_INFO "irq_demo: 演示中断API\n");
    printk(KERN_INFO "irq_demo: 查找可用的IRQ...\n");

    // 查找第一个可用的虚拟IRQ
    test_irq = irq_alloc_desc(-1);
    if (test_irq < 0) {
        printk(KERN_INFO "irq_demo: 无法分配虚拟IRQ，继续演示\n");
    } else {
        printk(KERN_INFO "irq_demo: 分配虚拟IRQ: %d\n", test_irq);
        // 释放分配的IRQ
        irq_free_desc(test_irq);
    }

    // 3. 显示系统中断信息
    printk(KERN_INFO "irq_demo: 系统中断总数: %d\n", nr_irqs);
    printk(KERN_INFO "irq_demo: CPU数量: %d\n", nr_cpu_ids);

    // 4. 模拟中断（用于演示）
    printk(KERN_INFO "irq_demo: 模拟中断处理流程...\n");
    demo_irq_handler(0, NULL);

    last_jiffies = jiffies;

    printk(KERN_INFO "irq_demo: === 初始化完成 ===\n");

    return 0;
}

/**
 * irq_exit - 模块退出
 */
static void __exit irq_exit(void)
{
    printk(KERN_INFO "irq_demo: === 模块卸载 ===\n");

    // 清理workqueue
    cancel_work_sync(&demo_work);

    // 清理tasklet
    tasklet_kill(&demo_tasklet);

    // 删除proc文件
    proc_remove(proc_entry);

    printk(KERN_INFO "irq_demo: 中断总次数: %lu\n", irq_count);
    printk(KERN_INFO "irq_demo: Tasklet次数: %lu\n", tasklet_count);
    printk(KERN_INFO "irq_demo: Workqueue次数: %lu\n", workqueue_count);

    printk(KERN_INFO "irq_demo: === 卸载完成 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("中断处理示例");
MODULE_VERSION("1.0");

module_init(irq_init);
module_exit(irq_exit);
