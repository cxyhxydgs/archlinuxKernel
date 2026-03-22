# kernel_interrupt - 中断处理示例

这个demo展示了Linux中断处理的基本概念和API。

## 学习目标

通过这个demo，你将学到：

- ✅ 中断的基本概念
- ✅ 中断顶半部和底半部
- ✅ Tasklet 的使用
- ✅ Workqueue 的使用
- ✅ 中断处理函数的编写
- ✅ 中断统计和调试

## 什么是中断？

中断是硬件或软件信号，通知CPU需要立即处理某些事情：

- **硬件中断**：外部设备产生（键盘、网络、定时器）
- **软件中断**：软件触发（系统调用、异常）
- **顶半部**：快速响应，不能睡眠
- **底半部**：延迟处理，可以睡眠

## 中断处理流程

```
硬件中断发生
    ↓
顶半部 (ISR - 快速，不睡眠)
    ↓
调度底半部
    ↓
底半部 (Tasklet/Workqueue - 可以睡眠)
    ↓
完成处理
```

## 核心概念

### 1. 中断顶半部

```c
irqreturn_t handler(int irq, void *dev_id)
{
    // 快速处理，不能睡眠
    // 返回 IRQ_HANDLED 或 IRQ_NONE
    return IRQ_HANDLED;
}
```

### 2. Tasklet（软中断）

```c
// 声明tasklet
DECLARE_TASKLET(name, func, data);

// 处理函数
void tasklet_func(unsigned long data)
{
    // 在软中断上下文，不能睡眠
}

// 调度
tasklet_schedule(&tasklet);
```

### 3. Workqueue

```c
// 声明work
static DECLARE_WORK(work, func);

// 处理函数
void work_func(struct work_struct *work)
{
    // 在进程上下文，可以睡眠
}

// 调度
schedule_work(&work);
```

## 编译和测试

### 1. 编译

```bash
cd kernel_interrupt
make

# 预期输出：
# CC [M]  /path/to/interrupt.o
# MODPOST /path/to/Module.symvers
# CC [M]  /path/to/interrupt.mod.o
# LD [M]  /path/to/interrupt.ko
```

### 2. 加载模块

```bash
sudo insmod interrupt.ko

# 查看内核日志
dmesg | tail -20

# 预期输出：
# [12345.100] irq_demo: === 模块初始化 ===
# [12345.101] irq_demo: 创建 /proc/irq_demo
# [12345.102] irq_demo: 演示中断API
# [12345.103] irq_demo: 查找可用的IRQ...
# [12345.104] irq_demo: 系统中断总数: 256
# [12345.105] irq_demo: CPU数量: 8
# [12345.106] irq_demo: 模拟中断处理流程...
# [12345.107] irq_demo: 中断触发! [1], 间隔: 0 ms
# [12345.108] irq_demo: Tasklet执行 [1], 数据: 0
# [12345.109] irq_demo: Workqueue执行 [1]
# [12345.110] irq_demo: === 初始化完成 ===
```

### 3. 查看统计信息

```bash
# 通过proc接口查看
cat /proc/irq_demo

# 预期输出：
# === 中断统计 ===
#
# 中断总次数:       1
# Tasklet次数:      1
# Workqueue次数:    1
#
# 当前Jiffies:      4294967295
# HZ (时钟频率):    250
#
# === 中断说明 ===
# 这是一个演示模块，展示中断处理API的使用
# 实际使用需要硬件产生中断
```

### 4. 一键测试

```bash
make test

# 预期输出：
# === 测试中断模块 ===
# 1. 加载模块
#
# 2. 查看统计
# === 中断统计 ===
# ...
```

### 5. 查看系统中断

```bash
make sys-irq

# 预期输出：
# === 系统中断信息 ===
# 1. 中断总数: 45
# 2. IRQ数量: 256
#
# 3. 前10个中断:
#            CPU0       CPU1       CPU2       CPU3
#   0:        100         50         80         60   IO-APIC-edge      timer
#   1:          0          0          0          0   IO-APIC-edge      i8042
# ...
#
# 4. CPU0中断:
# 0 100
# 1 0
# ...
```

### 6. 查看特定中断

```bash
# 查看中断1（键盘）
make show-irq IRQ=1

# 或手动
cat /proc/irq/1/* 2>/dev/null

# 预期输出（如果存在）：
# i8042
# 0
# 0
```

### 7. 查看所有可用中断

```bash
ls /proc/irq | grep -E '^[0-9]+' | head -20

# 预期输出：
# 1
# 2
# 3
# ...
```

### 8. 卸载模块

```bash
sudo rmmod interrupt

# 查看最终统计
dmesg | tail -10

# 预期输出：
# [12345.200] irq_demo: === 模块卸载 ===
# [12345.201] irq_demo: 中断总次数: 1
# [12345.201] irq_demo: Tasklet次数: 1
# [12345.201] irq_demo: Workqueue次数: 1
# [12345.202] irq_demo: === 卸载完成 ===
```

## 中断调试技巧

### 1. 查看实时中断

```bash
# 监控中断变化
watch -n 1 'cat /proc/interrupts | head -20'

# 或使用实时监控
cat /proc/interrupts | grep "timer"
sleep 1
cat /proc/interrupts | grep "timer"
```

### 2. 查看中断分布

```bash
# 查看每个CPU的中断
cat /proc/interrupts

# 统计总中断
cat /proc/interrupts | awk '{sum+=$2} END {print "总中断:", sum}'
```

### 3. 追踪中断延迟

```bash
# 使用ftrace（高级）
echo 1 > /proc/sys/kernel/ftrace_enabled
echo function > /sys/kernel/debug/tracing/current_tracer
cat /sys/kernel/debug/tracing/trace
```

### 4. 检查中断平衡

```bash
# 查看IRQ平衡
cat /proc/irq/*/smp_affinity_list 2>/dev/null | head -10
```

## Python监控脚本

```python
#!/usr/bin/env python3
"""中断监控脚本"""

import subprocess
import time
import re

def get_irq_count(irq_num):
    """获取特定中断的次数"""
    result = subprocess.run(
        f"cat /proc/interrupts | awk '/^{irq_num}:/ {{print $$2}}'",
        shell=True, capture_output=True, text=True
    )
    return int(result.stdout.strip()) if result.stdout.strip() else 0

def monitor_irq(irq_num, interval=2):
    """监控中断"""
    print(f"监控中断 {irq_num}，每 {interval} 秒更新")
    print("按 Ctrl+C 退出\n")

    prev_count = get_irq_count(irq_num)

    try:
        while True:
            time.sleep(interval)
            curr_count = get_irq_count(irq_num)
            delta = curr_count - prev_count
            prev_count = curr_count

            print(f"中断 {irq_num}: 总计={curr_count}, 增量={delta}")

    except KeyboardInterrupt:
        print("\n停止监控")

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        print("用法: python3 monitor_irq.py <中断号>")
        sys.exit(1)

    irq_num = int(sys.argv[1])
    monitor_irq(irq_num)
```

使用：
```bash
python3 monitor_irq.py 1  # 监控中断1
```

## 核心API详解

### 注册中断

```c
int request_irq(
    unsigned int irq,              // 中断号
    irq_handler_t handler,         // 处理函数
    unsigned long flags,           // 标志位
    const char *name,              // 名称
    void *dev_id                   // 设备ID
);

// 示例
ret = request_irq(irq_num, handler,
                  IRQF_SHARED | IRQF_TRIGGER_RISING,
                  "my_irq", dev);
```

### 标志位

- `IRQF_SHARED` - 中断可以共享
- `IRQF_TRIGGER_RISING` - 上升沿触发
- `IRQF_TRIGGER_FALLING` - 下降沿触发
- `IRQF_TRIGGER_HIGH` - 高电平触发
- `IRQF_TRIGGER_LOW` - 低电平触发

### 释放中断

```c
void free_irq(unsigned int irq, void *dev_id);
```

### Tasklet操作

```c
// 初始化
void tasklet_init(struct tasklet_struct *t,
                  void (*func)(unsigned long), unsigned long data);

// 调度
void tasklet_schedule(struct tasklet_struct *t);

// 杀死（取消）
void tasklet_kill(struct tasklet_struct *t);
```

### Workqueue操作

```c
// 创建工作队列
struct workqueue_struct *create_workqueue(const char *name);

// 提交工作
int queue_work(struct workqueue_struct *wq, struct work_struct *work);

// 等待工作完成
void flush_workqueue(struct workqueue_struct *wq);

// 销毁工作队列
void destroy_workqueue(struct workqueue_struct *wq);
```

## 常见问题

### Q: 如何获取正确的中断号？

从设备树或硬件文档获取：
```bash
# 从设备树
grep "interrupts" /sys/firmware/devicetree/base/*
```

### Q: 中断处理函数能睡眠吗？

不能！顶半部处理函数不能睡眠。
- 可以使用：Tasklet（软中断，不能睡眠）
- 可以使用：Workqueue（进程上下文，可以睡眠）

### Q: 如何调试中断延迟？

使用ftrace和perf工具：
```bash
# 启用中断延迟追踪
echo 1 > /proc/sys/kernel/irq_debug
```

### Q: Tasklet vs Workqueue 如何选择？

| 特性 | Tasklet | Workqueue |
|------|---------|-----------|
| 上下文 | 软中断 | 进程 |
| 能否睡眠 | 否 | 是 |
| 优先级 | 高 | 普通 |
| 适用场景 | 快速处理 | 耗时操作 |

## 扩展练习

1. **实际硬件中断**：与GPIO或定时器配合
2. **共享中断**：多个设备共享同一中断
3. **线程化中断**：使用 threaded_irq
4. **中断亲和性**：设置中断到特定CPU
5. **延迟测量**：测量中断响应时间

## 下一步

继续学习：

- **kernel_gpio** - GPIO控制（可产生中断）
- **kernel_spi** - SPI设备驱动

## 实际应用示例

在真实驱动中，中断处理通常这样组织：

```c
// 顶半部：快速响应，只设置标志
static irqreturn_t my_isr(int irq, void *dev)
{
    struct my_device *dev = dev;

    // 读取硬件状态，清空中断
    u32 status = readl(dev->base + STATUS_REG);

    // 调度底半部处理
    tasklet_schedule(&dev->tasklet);

    return IRQ_HANDLED;
}

// 底半部：执行耗时操作
static void my_tasklet(unsigned long data)
{
    struct my_device *dev = (struct my_device *)data;

    // 处理数据
    process_data(dev);
}
```

---

祝你学习愉快！🚀
