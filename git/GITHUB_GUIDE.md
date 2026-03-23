# Git 与 GitHub 使用指南

## 一、当前配置状态

**已完成：**
- Git 用户名：`cxyhxydgs`
- Git 邮箱：`cxyhxydgs@users.noreply.github.com`
- 默认分支：`main`
- SSH 密钥：已生成

**你的 SSH 公钥（需要添加到 GitHub）：**
```
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIE9sk2Ncbx+UILBfizniUMlhn9SLETlozW+EDKOjabl9 cxyhxydgs@users.noreply.github.com
```

---

## 二、添加 SSH 公钥到 GitHub（必须步骤）

1. 打开 https://github.com/settings/keys
2. 点击 **New SSH key**
3. 填写：
   - Title: `Arch Linux` （或任意名称）
   - Key: 粘贴上面的公钥
4. 点击 **Add SSH key**

**获取公钥命令：**
```bash
cat ~/.ssh/id_ed25519.pub
```

**测试连接：**
```bash
ssh -T git@github.com
```
成功会显示：`Hi cxyhxydgs! You've successfully authenticated...`

---

## 三、上传代码到 GitHub

### 方法一：新建仓库并上传

```bash
# 1. 在 GitHub 网页上创建新仓库（不要勾选 README）

# 2. 本地初始化
cd /home/changxiaoyi/CXY_2026/Kernel_Mode
git init

# 3. 添加所有文件
git add .

# 4. 提交
git commit -m "首次提交"

# 5. 关联远程仓库（替换 REPO_NAME 为你的仓库名）
git remote add origin git@github.com:cxyhxydgs/REPO_NAME.git

# 6. 推送
git push -u origin main
```

### 方法二：已有仓库，推送更新

```bash
# 添加修改的文件
git add .

# 提交
git commit -m "更新说明"

# 推送
git push
```

---

## 四、从 GitHub 下载代码

### 克隆仓库

```bash
# SSH 方式（推荐）
git clone git@github.com:cxyhxydgs/REPO_NAME.git

# HTTPS 方式
git clone https://github.com/cxyhxydgs/REPO_NAME.git
```

### 拉取更新

```bash
cd REPO_NAME
git pull
```

---

## 五、常用命令速查

| 操作 | 命令 |
|------|------|
| 初始化仓库 | `git init` |
| 查看状态 | `git status` |
| 添加文件 | `git add .` 或 `git add 文件名` |
| 提交 | `git commit -m "说明"` |
| 推送 | `git push` |
| 拉取 | `git pull` |
| 克隆 | `git clone URL` |
| 查看日志 | `git log --oneline` |
| 查看远程仓库 | `git remote -v` |
| 创建分支 | `git branch 分支名` |
| 切换分支 | `git checkout 分支名` |
| 合并分支 | `git merge 分支名` |

---

## 六、配置 Git 别名（可选，简化命令）

```bash
git config --global alias.st status
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.lg "log --oneline --graph --all"
```

之后可用：
- `git st` 代替 `git status`
- `git co` 代替 `git checkout`
- `git lg` 查看图形化日志

---

## 七、保存凭据（可选）

```bash
# 缓存密码 1 小时
git config --global credential.helper 'cache --timeout=3600'

# 永久保存（使用 SSH 则不需要）
git config --global credential.helper store
```

---

## 八、常见问题

### 推送被拒绝

```bash
# 先拉取再推送
git pull --rebase origin main
git push origin main
```

### 撤销未提交的修改

```bash
# 撤销单个文件
git checkout -- 文件名

# 撤销所有
git checkout -- .
```

### 撤销最后一次提交

```bash
# 保留修改
git reset --soft HEAD~1

# 丢弃修改
git reset --hard HEAD~1
```

### 查看 SSH 连接状态

```bash
ssh -vT git@github.com
```

---

## 九、你的信息

- GitHub 用户名：`cxyhxydgs`
- GitHub 主页：https://github.com/cxyhxydgs
- 你的仓库列表：https://github.com/cxyhxydgs?tab=repositories

---

## 十、快速推送当前项目

```bash
cd /home/changxiaoyi/CXY_2026/Kernel_Mode

# 初始化
git init

# 创建 .gitignore
cat > .gitignore << 'EOF'
*.o
*.ko
*.mod.c
*.mod.o
*.order
*.symvers
.*.cmd
.tmp_versions/
modules.order
Module.symvers
*.log
*.pid
__pycache__/
*.pyc
.DS_Store
EOF

# 添加文件
git add .

# 提交
git commit -m "初始提交：内核学习项目和监控服务"

# 在 GitHub 创建仓库后执行（替换 Kernel_Mode 为你的仓库名）
git remote add origin git@github.com:cxyhxydgs/Kernel_Mode.git
git push -u origin main
```


   git add .
   git commit -m "更新说明"
   git push

