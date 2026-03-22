# kernel_hello - Hello World 内核模块

这是最简单的内核模块，展示了内核模块的基本结构。

## 学习目标

通过这个demo，你将学到：

- ✅ 内核模块的基本结构（初始化/退出函数）
- ✅ `printk()` 内核日志输出
- ✅ 模块元数据（作者、描述、许可证等）
- ✅ 模块参数 - 运行时传递参数
- ✅ 如何编译、加载、卸载内核模块

## 文件说明

- `hello.c` - 源代码文件
- `Makefile` - 编译脚本
- `README.md` - 本文档

## 代码要点

### 1. 头文件

```c
#include <linux/module.h>  // 所有模块必需
#include <linux/kernel.h>  // printk() 和日志级别
#include <linux/init.h>    // __init 和 __exit 宏
```

### 2. 初始化函数

```c
static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, Kernel!\n");
    return 0;  // 返回0表示成功
}
```

**关键点：**
- `__init` 宏：函数在初始化后可以从内存中释放
- 返回值：0=成功，非0=失败
- `KERN_INFO`：日志级别（DEBUG、INFO、WARNING、ERROR等）

### 3. 退出函数

```c
static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye!\n");
}
```

**关键点：**
- `__exit` 宏：模块内置时可以优化掉
- 没有返回值

### 4. 模块参数

```c
static int count = 3;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "描述信息");
```

**参数类型：** int, bool, charp（字符串指针）
**权限：** 0644 表示所有者可读写，其他只读

### 5. 模块元数据

```c
MODULE_LICENSE("GPL");        // 必需！非GPL会警告
MODULE_AUTHOR("作者");
MODULE_DESCRIPTION("描述");
MODULE_VERSION("1.0");
```

### 6. 注册

```c
module_init(hello_init);
module_exit(hello_exit);
```

## 编译和测试

### 1. 编译

```bash
# 进入目录
cd kernel_hello

# 编译
make

# 查看生成的文件
ls -lh hello.ko
```

**预期输出：**
```
make -C /lib/modules/5.15.0-generic/build M=/path/to/kernel_hello modules
make[1]: Entering directory '/lib/modules/5.15.0-generic/build'
  CC [M]  /path/to/kernel_hello/hello.o
  MODPOST /path/to/kernel_hello/Module.symvers
  CC [M]  /path/to/kernel_hello/hello.mod.o
  LD [M]  /path/to/kernel_hello/hello.ko
make[1]: Leaving directory '/lib/modules/5.15.0-generic/build'
```

### 2. 加载模块

```bash
# 使用默认参数加载
sudo insmod hello.ko

# 查看内核日志
dmesg | tail -10

# 预期输出：
# [12345.678] === Hello模块开始初始化 ===
# [12345.678] 参数 count = 3
# [12345.678] Hello, Linux Kernel! [1/3]
# [12345.678] Hello, Linux Kernel! [2/3]
# [12345.678] Hello, Linux Kernel! [3/3]
# [12345.678] === Hello模块初始化完成 ===
```

### 3. 使用参数加载

```bash
# 先卸载（如果已加载）
sudo rmmod hello

# 用参数加载：打印10次
sudo insmod hello.ko count=10

# 查看日志
dmesg | tail -15
```

### 4. 查看模块信息

```bash
# 查看所有加载的模块
lsmod | grep hello

# 查看模块详细信息
modinfo hello.ko

# 预期输出：
# filename:       /path/to/kernel_hello/hello.ko
# version:        1.0
# description:    一个简单的Hello World内核模块
# author:         Your Name <your.email@example.com>
# license:        GPL
# parm:           count:打印Hello的次数 (默认: 3) (int)
```

### 5. 运行时修改参数

```bash
# 查看当前参数值（模块加载后）
cat /sys/module/hello/parameters/count

# 输出：3
```

注意：模块参数通常只能在加载时设置，运行时修改需要额外代码。

### 6. 卸载模块

```bash
sudo rmmod hello

# 查看日志
dmesg | tail -5

# 预期输出：
# [12345.999] === Hello模块已卸载 ===
# [12345.999] 再见！
```

## 实时查看内核日志

```bash
# 打开一个终端，实时监控
dmesg -w

# 在另一个终端操作模块
sudo insmod hello.ko
sudo rmmod hello
```

## 调试技巧

### 1. 过滤日志

```bash
# 只看本模块的日志
dmesg | grep "Hello"

# 实时过滤
dmesg -w | grep "Hello"
```

### 2. 查看模块依赖

```bash
modprobe --show-depends hello.ko
```

### 3. 符号表查看

```bash
# 查看模块导出的符号
nm hello.ko | grep -i init
```

## 常见问题

### Q: 编译失败，提示找不到内核源码

```bash
# 安装内核头文件
sudo apt-get install linux-headers-$(uname -r)

# 或者手动设置路径
export KDIR=/lib/modules/$(uname -r)/build
make clean && make
```

### Q: insmod时提示"Invalid module format"

通常是内核版本不匹配，确保用当前内核源码编译：
```bash
uname -r  # 查看当前内核版本
modinfo hello.ko | grep vermagic  # 查看模块编译时的内核版本
```

### Q: rmmod时提示"Module is in use"

可能有进程正在使用该模块（这个demo不会有这种情况），用强制卸载：
```bash
sudo rmmod -f hello
```

## 下一步

掌握了基础模块后，继续学习：

- **kernel_char_device** - 字符设备驱动（创建/dev节点）
- **kernel_procfs** - Proc文件系统接口
- **kernel_sysfs** - Sysfs接口

## 扩展练习

尝试修改代码：

1. 添加一个字符串类型的模块参数（`charp`）
2. 实现打印当前时间的功能
3. 添加一个整数数组参数（`int_array`）

---

祝你学习愉快！🚀
