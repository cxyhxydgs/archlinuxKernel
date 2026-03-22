/**
 * kernel_device_tree - 设备树解析示例
 *
 * 这个demo演示了如何解析设备树：
 * - 从设备树读取属性
 * - 解析字符串、整数、布尔值、数组
 * - 遍历设备树节点
 * - 使用 platform device 驱动
 *
 * 注意：这个demo需要在设备树中有对应节点才能完整测试
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/slab.h>

#define DEMO_DT_COMPATIBLE "demo,device-tree"

/**
 * 模块参数 - 用于模拟设备树属性（当没有实际设备树时）
 */
static char *dt_string_param = "default string";
module_param(dt_string_param, charp, 0644);
MODULE_PARM_DESC(dt_string_param, "模拟设备树字符串属性");

static int dt_int_param = 100;
module_param(dt_int_param, int, 0644);
MODULE_PARM_DESC(dt_int_param, "模拟设备树整数属性");

/**
 * ========================
 * 设备树属性解析示例
 * ========================
 */

/**
 * demo_device_tree_probe - platform driver probe函数
 *
 * 当匹配的设备树节点存在时调用
 */
static int demo_device_tree_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const char *string_prop;
    u32 int_prop;
    bool bool_prop;
    u32 array_prop[4];
    int prop_length;
    int ret, i;

    printk(KERN_INFO "device_tree_demo: === 设备树解析开始 ===\n");

    // 检查设备树节点是否存在
    if (!np) {
        printk(KERN_INFO "device_tree_demo: 没有设备树节点，使用模拟参数\n");
        printk(KERN_INFO "device_tree_demo: 字符串: %s\n", dt_string_param);
        printk(KERN_INFO "device_tree_demo: 整数: %d\n", dt_int_param);
        goto success;
    }

    printk(KERN_INFO "device_tree_demo: 设备树节点: %s\n", np->name);
    printk(KERN_INFO "device_tree_demo: 完整路径: %pOF\n", np);

    // 1. 读取字符串属性
    ret = of_property_read_string(np, "demo-string", &string_prop);
    if (ret == 0) {
        printk(KERN_INFO "device_tree_demo: demo-string = %s\n", string_prop);
    } else {
        printk(KERN_INFO "device_tree_demo: demo-string 不存在，使用默认值\n");
        string_prop = dt_string_param;
        printk(KERN_INFO "device_tree_demo: 使用默认值: %s\n", string_prop);
    }

    // 2. 读取整数属性
    ret = of_property_read_u32(np, "demo-int", &int_prop);
    if (ret == 0) {
        printk(KERN_INFO "device_tree_demo: demo-int = %u\n", int_prop);
    } else {
        printk(KERN_INFO "device_tree_demo: demo-int 不存在，使用默认值\n");
        int_prop = dt_int_param;
        printk(KERN_INFO "device_tree_demo: 使用默认值: %u\n", int_prop);
    }

    // 3. 读取布尔属性（of_property_read_bool）
    bool_prop = of_property_read_bool(np, "demo-bool");
    printk(KERN_INFO "device_tree_demo: demo-bool = %s\n", bool_prop ? "true" : "false");

    // 4. 读取整数数组
    prop_length = of_property_count_u32_elems(np, "demo-array");
    if (prop_length > 0) {
        printk(KERN_INFO "device_tree_demo: demo-array 长度 = %d\n", prop_length);

        // 读取整个数组
        ret = of_property_read_u32_array(np, "demo-array", array_prop,
                                          prop_length < 4 ? prop_length : 4);
        if (ret == 0) {
            printk(KERN_INFO "device_tree_demo: 数组值: ");
            for (i = 0; i < (prop_length < 4 ? prop_length : 4); i++) {
                printk("%u ", array_prop[i]);
            }
            printk("\n");
        }
    } else {
        printk(KERN_INFO "device_tree_demo: demo-array 不存在\n");
    }

    // 5. 读取32位整数（更安全的版本）
    {
        u32 u32_value;
        ret = of_property_read_u32(np, "demo-u32", &u32_value);
        if (ret == 0) {
            printk(KERN_INFO "device_tree_demo: demo-u32 = %u\n", u32_value);
        }
    }

    // 6. 读取64位整数
    {
        u64 u64_value;
        ret = of_property_read_u64(np, "demo-u64", &u64_value);
        if (ret == 0) {
            printk(KERN_INFO "device_tree_demo: demo-u64 = %llu\n", u64_value);
        }
    }

    // 7. 遍历子节点
    {
        struct device_node *child;

        printk(KERN_INFO "device_tree_demo: 子节点列表:\n");
        for_each_child_of_node(np, child) {
            printk(KERN_INFO "device_tree_demo:   - %s\n", child->name);
        }
    }

    // 8. 获取phandle
    {
        struct device_node *phandle_node;
        phandle_node = of_parse_phandle(np, "demo-phandle", 0);
        if (phandle_node) {
            printk(KERN_INFO "device_tree_demo: demo-phandle 指向: %pOF\n", phandle_node);
            of_node_put(phandle_node);
        }
    }

    // 9. 检查节点是否兼容
    if (of_device_is_compatible(np, DEMO_DT_COMPATIBLE)) {
        printk(KERN_INFO "device_tree_demo: 节点兼容 '%s'\n", DEMO_DT_COMPATIBLE);
    }

success:
    printk(KERN_INFO "device_tree_demo: === 设备树解析完成 ===\n");

    return 0;
}

/**
 * demo_device_tree_remove - platform driver remove函数
 */
static int demo_device_tree_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "device_tree_demo: 设备已移除\n");
    return 0;
}

/**
 * platform_driver 结构
 */
static struct platform_driver demo_driver = {
    .probe = demo_device_tree_probe,
    .remove = demo_device_tree_remove,
    .driver = {
        .name = "demo-device-tree",
        .of_match_table = NULL,  // 将在下面设置
    },
};

/**
 * 设备树兼容表
 *
 * 这个表定义了驱动匹配哪些设备树节点
 */
static const struct of_device_id demo_dt_ids[] = {
    { .compatible = DEMO_DT_COMPATIBLE },
    { /* sentinel */ }  // 必须以空项结束
};
MODULE_DEVICE_TABLE(of, demo_dt_ids);

// 设置 of_match_table
demo_driver.driver.of_match_table = demo_dt_ids;

/**
 * ========================
 * 模块初始化
 * ========================
 */

/**
 * demo_init - 模块初始化
 */
static int __init demo_init(void)
{
    int ret;

    printk(KERN_INFO "device_tree_demo: === 模块初始化 ===\n");

    // 注册 platform driver
    ret = platform_driver_register(&demo_driver);
    if (ret) {
        printk(KERN_ERR "device_tree_demo: platform_driver_register 失败: %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "device_tree_demo: platform driver 已注册\n");
    printk(KERN_INFO "device_tree_demo: 兼容字符串: %s\n", DEMO_DT_COMPATIBLE);

    // 检查是否存在匹配的设备树节点
    {
        struct device_node *np;
        np = of_find_compatible_node(NULL, NULL, DEMO_DT_COMPATIBLE);
        if (np) {
            printk(KERN_INFO "device_tree_demo: 找到匹配的设备树节点\n");
            of_node_put(np);
        } else {
            printk(KERN_INFO "device_tree_demo: 未找到匹配的设备树节点\n");
            printk(KERN_INFO "device_tree_demo: 将使用模拟参数进行测试\n");
            printk(KERN_INFO "device_tree_demo: 要测试实际设备树，需要在设备树中添加节点\n");
        }
    }

    printk(KERN_INFO "device_tree_demo: === 初始化完成 ===\n");

    return 0;
}

/**
 * demo_exit - 模块退出
 */
static void __exit demo_exit(void)
{
    printk(KERN_INFO "device_tree_demo: === 模块卸载 ===\n");

    // 注销 platform driver
    platform_driver_unregister(&demo_driver);

    printk(KERN_INFO "device_tree_demo: platform driver 已注销\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("设备树解析示例");
MODULE_VERSION("1.0");

module_init(demo_init);
module_exit(demo_exit);
