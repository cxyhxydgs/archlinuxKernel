# 快速操作指南

## 🚀 3步快速开始

```bash
# 1. 进入目录
cd /home/changxiaoyi/CXY_2026/Kernel_Mode/kernel_user_comm

# 2. 编译
make

# 3. 加载并测试
make test-char
```

---

## 📖 详细操作步骤

### 方式1：字符设备通信（推荐）

```bash
# 编译
make

# 加载模块
sudo insmod user_comm.ko

# 写入数据
echo "Hello" | sudo tee /dev/user_comm > /dev/null

# 读取数据
sudo cat /dev/user_comm

# 查看日志
dmesg | tail -10

# 卸载模块
sudo rmmod user_comm
```

---

### 方式2：Proc文件通信

```bash
# 加载模块
sudo insmod user_comm.ko

# 写入数据
echo "Hello" | sudo tee /proc/user_comm_proc > /dev/null

# 读取数据
sudo cat /proc/user_comm_proc

# 查看日志
dmesg | tail -10

# 卸载模块
sudo rmmod user_comm
```

---

### 方式3：共享内存通信

```bash
# 加载模块
sudo insmod user_comm.ko

# 查看内核日志（应该看到"初始化数据"）
dmesg | tail -10

# 查看Proc文件（显示共享内存内容）
sudo cat /proc/user_comm_proc

# 写入新数据
echo "New Data" | sudo tee /proc/user_comm_proc > /dev/null

# 再次查看
sudo cat /proc/user_comm_proc

# 卸载模块
sudo rmmod user_comm
```

---

## 🎯 一键测试

### 测试所有功能

```bash
make test-all
```

### 测试单个功能

```bash
make test-char      # 字符设备通信
make test-proc      # Proc文件通信
make test-shm       # 共享内存通信
```

---

## 🔍 查看调试信息

```bash
# 查看内核日志
dmesg | tail -50 | grep user_comm

# 实时监控
dmesg -w | grep user_comm

# 查看设备节点
ls -l /dev/user_comm
ls -l /proc/user_comm_proc

# 查看模块信息
modinfo user_comm.ko
```

---

## 📝 Python测试

```bash
# 创建测试脚本
cat > test_comm.py << 'EOF'
#!/usr/bin/env python3
import subprocess
import sys

def run(cmd, desc):
    print(f"\n{desc}")
    print(f"$ {cmd}")
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    print(result.stdout)

while True:
    print("\n1. 字符设备通信")
    print("2. Proc文件通信")
    print("3. 共享内存通信")
    print("0. 退出")
    choice = input("选项: ")
    
    if choice == "1":
        run("echo 'Hello' | sudo tee /dev/user_comm > /dev/null", "写入")
        run("sudo cat /dev/user_comm", "读取")
    elif choice == "2":
        run("echo 'Hello' | sudo tee /proc/user_comm_proc > /dev/null", "写入")
        run("sudo cat /proc/user_comm_proc", "读取")
    elif choice == "3":
        run("dmesg | tail -10 | grep user_comm", "查看日志")
    elif choice == "0":
        break
EOF

chmod +x test_comm.py

# 运行测试
python3 test_comm.py
```

---

## ⚡ 常用命令

```bash
# 编译
make

# 清理
make clean

# 加载
sudo insmod user_comm.ko

# 卸载
sudo rmmod user_comm

# 查看日志
dmesg | grep user_comm

# 查看设备
ls /dev/user_comm
ls /proc/user_comm_proc

# 查看模块
lsmod | grep user_comm
```

---

## 💡 学习要点

### 字符设备
- 使用 `/dev/user_comm`
- `echo` 写入，`cat` 读取
- 适合标准IO操作

### Proc文件
- 使用 `/proc/user_comm_proc`
- 直接文件操作
- 适合调试和配置

### 共享内存
- 使用 `shmget` + `shmat`
- 直接内存访问
- 性能最好

---

**现在开始学习！** 🚀
