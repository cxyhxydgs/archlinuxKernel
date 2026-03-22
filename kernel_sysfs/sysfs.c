/**
 * kernel_sysfs - Sysfs接口示例
 *
 * 这个demo演示了如何使用 sysfs 创建内核接口：
 * - 创建 kobject
 * - 在 /sys/ 创建属性文件
 * - 实现 show 和 store 函数
 * - 属性组和批量属性
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>

#define SYSFS_DIR "demo_sysfs"

/**
 * 全局变量
 */
static struct kobject *demo_kobj;     // kobject 结构
static int int_value = 42;            // 整数属性
static char string_value[32] = "default";  // 字符串属性
static bool bool_value = true;        // 布尔属性

/**
 * ========================
 * 单个属性示例
 * ========================
 */

/**
 * int_show - 读取整数属性
 */
static ssize_t int_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", int_value);
}

/**
 * int_store - 写入整数属性
 */
static ssize_t int_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    int ret;

    ret = kstrtoint(buf, 10, &int_value);
    if (ret)
        return ret;

    printk(KERN_INFO "sysfs_demo: int_value 更新为 %d\n", int_value);

    return count;
}

/**
 * str_show - 读取字符串属性
 */
static ssize_t str_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", string_value);
}

/**
 * str_store - 写入字符串属性
 */
static ssize_t str_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    // 限制字符串长度
    if (count >= sizeof(string_value))
        return -EINVAL;

    strncpy(string_value, buf, count);
    // 移除换行符
    if (string_value[count-1] == '\n')
        string_value[count-1] = '\0';

    printk(KERN_INFO "sysfs_demo: string_value 更新为 %s\n", string_value);

    return count;
}

/**
 * bool_show - 读取布尔属性
 */
static ssize_t bool_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", bool_value ? 1 : 0);
}

/**
 * bool_store - 写入布尔属性
 */
static ssize_t bool_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    int ret;

    ret = kstrtobool(buf, &bool_value);
    if (ret)
        return ret;

    printk(KERN_INFO "sysfs_demo: bool_value 更新为 %s\n", bool_value ? "true" : "false");

    return count;
}

/**
 * info_show - 只读信息属性
 */
static ssize_t info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Sysfs Demo Module\n"
                      "Built-in: %s\n"
                      "Kernel: %d.%d.%d\n",
                      THIS_MODULE->name,
                      LINUX_VERSION_CODE >> 16,
                      (LINUX_VERSION_CODE >> 8) & 0xff,
                      LINUX_VERSION_CODE & 0xff);
}

/**
 * 使用 __ATTR 宏定义属性
 * __ATTR(name, mode, show, store)
 *
 * mode: 权限
 *   0444 - 只读
 *   0644 - 读写
 *   0200 - 只写
 */
static struct kobj_attribute int_attr =
    __ATTR(int_value, 0644, int_show, int_store);

static struct kobj_attribute str_attr =
    __ATTR(str_value, 0644, str_show, str_store);

static struct kobj_attribute bool_attr =
    __ATTR(bool_value, 0644, bool_show, bool_store);

static struct kobj_attribute info_attr =
    __ATTR(info, 0444, info_show, NULL);

/**
 * ========================
 * 属性组示例
 * ========================
 */

/**
 * 创建属性组 - 批量创建多个属性
 */
static struct attribute *demo_attrs[] = {
    &int_attr.attr,
    &str_attr.attr,
    &bool_attr.attr,
    &info_attr.attr,
    NULL,  // 必须以 NULL 结尾
};

/**
 * 属性组
 */
static struct attribute_group demo_attr_group = {
    .attrs = demo_attrs,
};

/**
 * ========================
 * 动态属性示例（运行时创建）
 * ========================
 */

static struct kobj_attribute dynamic_attr;

/**
 * dynamic_show
 */
static ssize_t dynamic_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "这是一个动态创建的属性\n");
}

/**
 * 模块初始化
 */
static int __init sysfs_init(void)
{
    int ret;

    printk(KERN_INFO "=== sysfs_demo: 初始化开始 ===\n");

    // 1. 创建 kobject
    // 在 /sys/kernel/ 下创建 demo_sysfs 目录
    demo_kobj = kobject_create_and_add(SYSFS_DIR, kernel_kobj);
    if (!demo_kobj) {
        printk(KERN_ERR "sysfs_demo: kobject_create_and_add 失败\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "sysfs_demo: 创建 /sys/kernel/%s\n", SYSFS_DIR);

    // 2. 创建属性组（批量创建属性）
    ret = sysfs_create_group(demo_kobj, &demo_attr_group);
    if (ret) {
        printk(KERN_ERR "sysfs_demo: sysfs_create_group 失败: %d\n", ret);
        kobject_put(demo_kobj);
        return ret;
    }
    printk(KERN_INFO "sysfs_demo: 创建属性组成功\n");

    // 3. 动态创建单个属性（演示）
    __init_kobj_attr(dynamic, 0444, dynamic_show, NULL);
    ret = sysfs_create_file(demo_kobj, &dynamic_attr.attr);
    if (ret) {
        printk(KERN_WARNING "sysfs_demo: 创建动态属性失败: %d\n", ret);
    } else {
        printk(KERN_INFO "sysfs_demo: 创建动态属性成功\n");
    }

    printk(KERN_INFO "=== sysfs_demo: 初始化完成 ===\n");

    return 0;
}

/**
 * 模块退出
 */
static void __exit sysfs_exit(void)
{
    printk(KERN_INFO "=== sysfs_demo: 清理中 ===\n");

    // 清理顺序与创建相反
    sysfs_remove_group(demo_kobj, &demo_attr_group);
    sysfs_remove_file(demo_kobj, &dynamic_attr.attr);
    kobject_put(demo_kobj);

    printk(KERN_INFO "=== sysfs_demo: 模块已卸载 ===\n");
}

/**
 * 模块元数据
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Sysfs接口示例");
MODULE_VERSION("1.0");

module_init(sysfs_init);
module_exit(sysfs_exit);
