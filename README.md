# Linux 内核模块使用说明

## 文件说明
- `hello_module.c` - Hello World 内核模块源代码
- `Makefile` - 编译内核模块的配置文件

## 编译步骤

1. 确保已安装内核开发包：
   ```bash
   sudo pacman -S linux-headers  # 对于 Arch Linux
   # 或者对于其他发行版：
   # sudo apt-get install linux-headers-generic  # Debian/Ubuntu
   ```

2. 编译内核模块：
   ```bash
   make
   ```

3. 编译成功后，会生成以下文件：
   - `hello_module.ko` - 内核模块对象文件
   - `hello_module.mod.c` - 模块定义文件
   - `hello_module.mod.o` - 模块定义对象文件
   - 其他辅助文件

## 加载和测试

### 加载模块：
```bash
sudo insmod hello_module.ko
```

### 查看内核日志：
```bash
dmesg | tail
# 或者
sudo dmesg | grep "Hello World"
```

### 卸载模块：
```bash
sudo rmmod hello_module
```

### 再次查看内核日志：
```bash
dmesg | tail
```

## 注意事项

1. 需要root权限来加载和卸载内核模块
2. 确保内核版本与安装的内核头文件版本匹配
3. 内核模块出错时，可能需要重启系统
4. 生产环境中应谨慎使用自定义内核模块

## 故障排除

如果编译失败，可能是因为：
- 缺少内核开发头文件
- 内核版本不匹配
- Makefile路径问题

可以使用以下命令检查内核版本：
```bash
uname -r
```