/**
 * kernel_spi - SPI设备驱动示例
 *
 * 这个demo演示了Linux SPI子系统的使用：
 * - SPI设备注册和识别
 * - SPI消息和传输
 * - SPI同步和异步操作
 * - SPI设备树配置
 *
 * 注意：这个demo演示SPI API的使用
 * 实际使用需要SPI硬件和设备
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>

#define SPI_DRIVER_NAME "demo_spi"

/**
 * 模块参数
 */
static int bus_num = 0;
module_param(bus_num, int, 0644);
MODULE_PARM_DESC(bus_num, "SPI总线号");

static int chip_select = 0;
module_param(chip_select, int, 0644);
MODULE_PARM_DESC(chip_select, "SPI片选号");

static int max_speed = 1000000;  // 1 MHz
module_param(max_speed, int, 0644);
MODULE_PARM_DESC(max_speed, "SPI最大速度(Hz)");

/**
 * ========================
 * SPI传输示例
 * ========================
 */

/**
 * spi_transfer_example - 演示基本SPI传输
 *
 * @spi: SPI设备结构
 */
static void spi_transfer_example(struct spi_device *spi)
{
    u8 tx_buf[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    u8 rx_buf[4] = {0};
    int ret;

    printk(KERN_INFO "spi_demo: === 演示SPI传输 ===\n");
    printk(KERN_INFO "spi_demo: 总线: %d, 片选: %d, 速度: %d Hz\n",
           spi->master->bus_num, spi->chip_select, spi->max_speed_hz);

    // 方法1: spi_write_then_read (简单的写后读)
    ret = spi_write_then_read(spi, tx_buf, 2, rx_buf, 2);
    if (ret == 0) {
        printk(KERN_INFO "spi_demo: spi_write_then_read 成功\n");
        printk(KERN_INFO "spi_demo: 发送: %02X %02X\n", tx_buf[0], tx_buf[1]);
        printk(KERN_INFO "spi_demo: 接收: %02X %02X\n", rx_buf[0], rx_buf[1]);
    } else {
        printk(KERN_ERR "spi_demo: spi_write_then_read 失败: %d\n", ret);
    }

    // 方法2: spi_sync (完整控制)
    {
        struct spi_transfer xfer;
        struct spi_message msg;

        // 清空接收缓冲区
        memset(rx_buf, 0, sizeof(rx_buf));

        // 初始化SPI传输
        memset(&xfer, 0, sizeof(xfer));
        xfer.tx_buf = tx_buf;
        xfer.rx_buf = rx_buf;
        xfer.len = 4;
        xfer.speed_hz = spi->max_speed_hz;
        xfer.bits_per_word = 8;
        xfer.cs_change = 0;  // 保持片选
        xfer.delay_usecs = 0;  // 传输间延迟

        // 初始化SPI消息
        spi_message_init(&msg);
        spi_message_add_tail(&xfer, &msg);

        // 执行SPI传输
        ret = spi_sync(spi, &msg);
        if (ret == 0) {
            printk(KERN_INFO "spi_demo: spi_sync 成功\n");
            printk(KERN_INFO "spi_demo: 接收: %02X %02X %02X %02X\n",
                   rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3]);
        } else {
            printk(KERN_ERR "spi_demo: spi_sync 失败: %d\n", ret);
        }
    }

    // 方法3: 多段传输（CS保持）
    {
        struct spi_transfer xfers[2];
        struct spi_message msg;
        u8 cmd_buf[2] = {0x01, 0x02};  // 命令
        u8 data_buf[4] = {0x00, 0x00, 0x00, 0x00};  // 数据

        printk(KERN_INFO "spi_demo: === 多段传输演示 ===\n");

        // 第一段：发送命令
        memset(&xfers[0], 0, sizeof(xfers[0]));
        xfers[0].tx_buf = cmd_buf;
        xfers[0].len = 2;
        xfers[0].cs_change = 1;  // 保持片选

        // 第二段：读/写数据
        memset(&xfers[1], 0, sizeof(xfers[1]));
        xfers[1].tx_buf = data_buf;
        xfers[1].rx_buf = data_buf;
        xfers[1].len = 4;
        xfers[1].cs_change = 0;  // 释放片选

        // 初始化消息并添加传输
        spi_message_init(&msg);
        spi_message_add_tail(&xfers[0], &msg);
        spi_message_add_tail(&xfers[1], &msg);

        // 执行传输
        ret = spi_sync(spi, &msg);
        if (ret == 0) {
            printk(KERN_INFO "spi_demo: 多段传输成功\n");
            printk(KERN_INFO "spi_demo: 命令: %02X %02X\n", cmd_buf[0], cmd_buf[1]);
            printk(KERN_INFO "spi_demo: 数据: %02X %02X %02X %02X\n",
                   data_buf[0], data_buf[1], data_buf[2], data_buf[3]);
        } else {
            printk(KERN_ERR "spi_demo: 多段传输失败: %d\n", ret);
        }
    }

    printk(KERN_INFO "spi_demo: === SPI传输演示完成 ===\n");
}

/**
 * spi_device_info - 显示SPI设备信息
 */
static void spi_device_info(struct spi_device *spi)
{
    printk(KERN_INFO "spi_demo: === SPI设备信息 ===\n");
    printk(KERN_INFO "spi_demo: 设备名称: %s\n", spi->modalias);
    printk(KERN_INFO "spi_demo: 总线号: %d\n", spi->master->bus_num);
    printk(KERN_INFO "spi_demo: 片选号: %d\n", spi->chip_select);
    printk(KERN_INFO "spi_demo: 最大速度: %d Hz\n", spi->max_speed_hz);
    printk(KERN_INFO "spi_demo: 位宽: %d 位\n", spi->bits_per_word);
    printk(KERN_INFO "spi_demo: 模式: %d\n", spi->mode);
    printk(KERN_INFO "spi_demo:  ");
    if (spi->mode & SPI_CPOL) printk("CPOL ");
    if (spi->mode & SPI_CPHA) printk("CPHA ");
    if (spi->mode & SPI_CS_HIGH) printk("CS_HIGH ");
    if (spi->mode & SPI_LSB_FIRST) printk("LSB_FIRST ");
    if (spi->mode & SPI_3WIRE) printk("3WIRE ");
    if (spi->mode & SPI_LOOP) printk("LOOP ");
    printk("\n");
    printk(KERN_INFO "spi_demo: ===================\n");
}

/**
 * ========================
 * SPI驱动函数
 * ========================
 */

/**
 * demo_spi_probe - SPI设备探测函数
 *
 * @spi: SPI设备
 */
static int demo_spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "spi_demo: === SPI设备探测 ===\n");

    // 显示设备信息
    spi_device_info(spi);

    // 演示SPI传输
    spi_transfer_example(spi);

    printk(KERN_INFO "spi_demo: SPI设备初始化完成\n");

    return 0;
}

/**
 * demo_spi_remove - SPI设备移除函数
 *
 * @spi: SPI设备
 */
static int demo_spi_remove(struct spi_device *spi)
{
    printk(KERN_INFO "spi_demo: SPI设备已移除\n");
    return 0;
}

/**
 * ========================
 * SPI驱动结构
 * ========================
 */

static struct spi_driver demo_spi_driver = {
    .driver = {
        .name = SPI_DRIVER_NAME,
        .owner = THIS_MODULE,
    },
    .probe = demo_spi_probe,
    .remove = demo_spi_remove,
};

/**
 * ========================
 * 设备树匹配表
 * ========================
 */

static const struct of_device_id demo_spi_of_match[] = {
    { .compatible = "demo,spi-device", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, demo_spi_of_match);

// 设置of_match_table
demo_spi_driver.driver.of_match_table = demo_spi_of_match;

/**
 * ========================
 * 模块初始化
 * ========================
 */

/**
 * spi_init - 模块初始化
 */
static int __init spi_init(void)
{
    int ret;

    printk(KERN_INFO "spi_demo: === SPI驱动初始化 ===\n");
    printk(KERN_INFO "spi_demo: 驱动名称: %s\n", SPI_DRIVER_NAME);

    // 检查SPI子系统
    printk(KERN_INFO "spi_demo: 检查SPI子系统...\n");
    printk(KERN_INFO "spi_demo: SPI控制器数量（模拟）\n");

    // 注册SPI驱动
    ret = spi_register_driver(&demo_spi_driver);
    if (ret) {
        printk(KERN_ERR "spi_demo: spi_register_driver 失败: %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "spi_demo: SPI驱动已注册\n");
    printk(KERN_INFO "spi_demo: 兼容字符串: demo,spi-device\n");
    printk(KERN_INFO "spi_demo: 等待SPI设备匹配...\n");

    printk(KERN_INFO "spi_demo: === 初始化完成 ===\n");

    return 0;
}

/**
 * spi_exit - 模块退出
 */
static void __exit spi_exit(void)
{
    printk(KERN_INFO "spi_demo: === SPI驱动卸载 ===\n");

    // 注销SPI驱动
    spi_unregister_driver(&demo_spi_driver);

    printk(KERN_INFO "spi_demo: SPI驱动已注销\n");
    printk(KERN_INFO "spi_demo: === 卸载完成 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI设备驱动示例");
MODULE_VERSION("1.0");

module_init(spi_init);
module_exit(spi_exit);
