/**
 * kernel_gpio - GPIO控制示例
 *
 * 这个demo演示了Linux GPIO子系统的使用：
 * - GPIO请求和释放
 * - GPIO方向设置（输入/输出）
 * - GPIO读写操作
 * - GPIO中断配置
 * - GPIO sysfs接口
 *
 * 注意：这个demo演示GPIO API的使用
 * 实际使用需要硬件GPIO引脚
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/init.h>

/**
 * 模块参数 - 用于指定GPIO引脚
 *
 * 使用说明：
 * - 如果有实际硬件，使用真实的GPIO号
 * - 如果没有硬件，使用虚拟GPIO进行演示
 */
static int gpio_out_num = 18;   // 输出GPIO
static int gpio_in_num = 4;     // 输入GPIO

module_param(gpio_out_num, int, 0644);
MODULE_PARM_DESC(gpio_out_num, "输出GPIO引脚号");

module_param(gpio_in_num, int, 0644);
MODULE_PARM_DESC(gpio_in_num, "输入GPIO引脚号");

/**
 * 全局变量
 */
static int irq_num = -1;                 // GPIO中断号
static struct work_struct work;         // 工作队列
static unsigned int event_count = 0;    // 事件计数
static bool use_virtual_gpio = false;   // 是否使用虚拟GPIO

/**
 * ========================
 * 工作队列处理
 * ========================
 */

/**
 * gpio_work_func - GPIO事件工作队列处理
 */
static void gpio_work_func(struct work_struct *work)
{
    event_count++;

    printk(KERN_INFO "gpio_demo: GPIO事件 [%u]\n", event_count);
}

// 初始化工作
static DECLARE_WORK(gpio_work, gpio_work_func);

/**
 * ========================
 * GPIO中断处理
 * ========================
 */

/**
 * gpio_irq_handler - GPIO中断处理函数
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    // 调度工作队列（底半部）
    schedule_work(&gpio_work);

    return IRQ_HANDLED;
}

/**
 * ========================
 * GPIO操作示例
 * ========================
 */

/**
 * demo_gpio_output - 演示GPIO输出操作
 */
static void demo_gpio_output(void)
{
    int i;

    printk(KERN_INFO "gpio_demo: === 演示GPIO输出 ===\n");

    // 设置GPIO方向：输出
    if (gpio_is_valid(gpio_out_num)) {
        if (gpio_direction_output(gpio_out_num, 0) < 0) {
            printk(KERN_ERR "gpio_demo: 设置GPIO %d 方向失败\n", gpio_out_num);
            return;
        }
        printk(KERN_INFO "gpio_demo: GPIO %d 设置为输出\n", gpio_out_num);
    } else {
        printk(KERN_INFO "gpio_demo: GPIO %d 无效，使用模拟\n", gpio_out_num);
        use_virtual_gpio = true;
    }

    // 闪烁GPIO（模拟LED）
    for (i = 0; i < 5; i++) {
        if (!use_virtual_gpio) {
            gpio_set_value(gpio_out_num, 1);  // 高电平
            printk(KERN_INFO "gpio_demo: GPIO %d = HIGH\n", gpio_out_num);
            msleep(200);

            gpio_set_value(gpio_out_num, 0);  // 低电平
            printk(KERN_INFO "gpio_demo: GPIO %d = LOW\n", gpio_out_num);
            msleep(200);
        } else {
            printk(KERN_INFO "gpio_demo: 模拟 GPIO %d = HIGH\n", gpio_out_num);
            msleep(200);
            printk(KERN_INFO "gpio_demo: 模拟 GPIO %d = LOW\n", gpio_out_num);
            msleep(200);
        }
    }
}

/**
 * demo_gpio_input - 演示GPIO输入操作
 */
static void demo_gpio_input(void)
{
    int value;

    printk(KERN_INFO "gpio_demo: === 演示GPIO输入 ===\n");

    if (use_virtual_gpio) {
        printk(KERN_INFO "gpio_demo: 模拟读取 GPIO %d\n", gpio_in_num);
        value = (jiffies % 2);
        printk(KERN_INFO "gpio_demo: GPIO %d = %d (模拟)\n", gpio_in_num, value);
        return;
    }

    if (!gpio_is_valid(gpio_in_num)) {
        printk(KERN_INFO "gpio_demo: GPIO %d 无效\n", gpio_in_num);
        return;
    }

    // 设置GPIO方向：输入
    if (gpio_direction_input(gpio_in_num) < 0) {
        printk(KERN_ERR "gpio_demo: 设置GPIO %d 方向失败\n", gpio_in_num);
        return;
    }

    printk(KERN_INFO "gpio_demo: GPIO %d 设置为输入\n", gpio_in_num);

    // 读取GPIO值
    value = gpio_get_value(gpio_in_num);
    printk(KERN_INFO "gpio_demo: GPIO %d = %d\n", gpio_in_num, value);

    // 多次读取
    printk(KERN_INFO "gpio_demo: 连续读取...\n");
    for (i = 0; i < 5; i++) {
        value = gpio_get_value(gpio_in_num);
        printk(KERN_INFO "gpio_demo: 读取 %d: %d\n", i, value);
        msleep(100);
    }
}

/**
 * demo_gpio_interrupt - 演示GPIO中断
 */
static int demo_gpio_interrupt(void)
{
    int ret;
    int flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;

    printk(KERN_INFO "gpio_demo: === 演示GPIO中断 ===\n");

    if (use_virtual_gpio) {
        printk(KERN_INFO "gpio_demo: 模拟GPIO中断\n");
        printk(KERN_INFO "gpio_demo: 触发模拟中断...\n");
        gpio_irq_handler(0, NULL);
        return 0;
    }

    if (!gpio_is_valid(gpio_in_num)) {
        printk(KERN_INFO "gpio_demo: GPIO %d 无效，跳过中断\n", gpio_in_num);
        return -EINVAL;
    }

    // 设置为输入
    if (gpio_direction_input(gpio_in_num) < 0) {
        printk(KERN_ERR "gpio_demo: 设置GPIO %d 方向失败\n", gpio_in_num);
        return -EINVAL;
    }

    // 获取GPIO的中断号
    irq_num = gpio_to_irq(gpio_in_num);
    if (irq_num < 0) {
        printk(KERN_ERR "gpio_demo: 获取GPIO %d 中断号失败: %d\n",
               gpio_in_num, irq_num);
        return irq_num;
    }
    printk(KERN_INFO "gpio_demo: GPIO %d 中断号: %d\n", gpio_in_num, irq_num);

    // 注册中断处理函数
    ret = request_irq(irq_num, gpio_irq_handler, flags, "demo_gpio", NULL);
    if (ret) {
        printk(KERN_ERR "gpio_demo: 注册中断失败: %d\n", ret);
        return ret;
    }
    printk(KERN_INFO "gpio_demo: 中断已注册（上升沿+下降沿）\n");

    return 0;
}

/**
 * ========================
 * 模块初始化
 * ========================
 */

/**
 * gpio_init - 模块初始化
 */
static int __init gpio_init(void)
{
    int ret;

    printk(KERN_INFO "gpio_demo: === GPIO驱动初始化 ===\n");

    // 显示GPIO子系统信息
    printk(KERN_INFO "gpio_demo: 输出GPIO: %d\n", gpio_out_num);
    printk(KERN_INFO "gpio_demo: 输入GPIO: %d\n", gpio_in_num);

    // 检查GPIO有效性
    if (!gpio_is_valid(gpio_out_num)) {
        printk(KERN_WARNING "gpio_demo: GPIO %d 无效，使用模拟模式\n", gpio_out_num);
        use_virtual_gpio = true;
    }

    // 演示GPIO输出
    demo_gpio_output();

    // 演示GPIO输入
    demo_gpio_input();

    // 演示GPIO中断（可选）
    if (!use_virtual_gpio) {
        ret = demo_gpio_interrupt();
        if (ret == 0) {
            printk(KERN_INFO "gpio_demo: 中断配置成功\n");
        } else {
            printk(KERN_INFO "gpio_demo: 中断配置失败，跳过\n");
            irq_num = -1;
        }
    }

    printk(KERN_INFO "gpio_demo: === 初始化完成 ===\n");

    return 0;
}

/**
 * gpio_exit - 模块退出
 */
static void __exit gpio_exit(void)
{
    printk(KERN_INFO "gpio_demo: === GPIO驱动卸载 ===\n");

    // 释放工作队列
    cancel_work_sync(&gpio_work);

    // 释放中断
    if (irq_num >= 0) {
        free_irq(irq_num, NULL);
        printk(KERN_INFO "gpio_demo: 中断已释放\n");
    }

    // 释放GPIO
    if (!use_virtual_gpio) {
        if (gpio_is_valid(gpio_out_num)) {
            gpio_free(gpio_out_num);
            printk(KERN_INFO "gpio_demo: GPIO %d 已释放\n", gpio_out_num);
        }
        if (gpio_is_valid(gpio_in_num)) {
            gpio_free(gpio_in_num);
            printk(KERN_INFO "gpio_demo: GPIO %d 已释放\n", gpio_in_num);
        }
    }

    printk(KERN_INFO "gpio_demo: GPIO事件总数: %u\n", event_count);
    printk(KERN_INFO "gpio_demo: === 卸载完成 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GPIO控制示例");
MODULE_VERSION("1.0");

module_init(gpio_init);
module_exit(gpio_exit);
