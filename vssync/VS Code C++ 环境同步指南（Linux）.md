# VS Code C/C++ 环境同步指南（Linux）

Settings Sync 同步编辑器设置和扩展，Git 同步源码与工程配置。开发工具需要在实际编译和调试的 Linux 主机上准备。

以下命令中的 `<...>` 为占位内容，使用时替换为实际值。

## 一、原电脑：准备同步

### 1. 开启 VS Code 同步

在 VS Code 命令面板中执行：

```text
Settings Sync: Turn On
```

同步账号：Microsoft 或 GitHub。

同步内容：设置、快捷键、扩展、用户代码片段、配置文件（Profiles）。

### 2. 保存工程配置

工程的 `.gitignore` 应已排除构建目录、生成的 `compile_commands.json`、`CMakeUserPresets.json` 等本机文件。

源码、CMake 配置和 `.vscode/` 中可共享的配置随 Git 保存。

**尚未建立 Git 仓库：**

```bash
cd "<工程目录>"

git init
git add .
git diff --cached --stat
git commit -m "Save project configuration"
git remote add origin "<仓库地址>"
git push -u origin HEAD
```

**已有远程仓库：**

```bash
cd "<工程目录>"

git add .
git diff --cached --stat
git commit -m "Update project configuration"
git push
```

## 二、目标环境：恢复同步

根据开发方式，选择“本地 Linux”或“远程 Ubuntu”。

### 方式 A：本地 Linux

前提：Linux 电脑上的 VS Code、Git、CMake、编译器、调试器和 clangd 已准备好。

**恢复编辑器配置——VS Code 命令面板：**

```text
Settings Sync: Turn On
```

使用与原电脑相同的账号和开发 Profile。

**首次获取工程——Linux 终端：**

```bash
git clone "<仓库地址>" "<本地工程目录>"
cd "<本地工程目录>"
code .
```

**更新已有工程——Linux 终端：**

```bash
cd "<本地工程目录>"
git pull
code .
```

### 方式 B：远程 Ubuntu

前提：Ubuntu 主机上的 Git、CMake、编译器、调试器和 clangd 已准备好；本地 VS Code 已安装 Remote-SSH。

**恢复本地编辑器配置——本地 VS Code 命令面板：**

```text
Settings Sync: Turn On
```

**连接 Ubuntu——本地 VS Code 命令面板：**

```text
Remote-SSH: Connect to Host...
```

**首次获取工程——连接后的远程终端：**

```bash
git clone "<仓库地址>" "<远程工程目录>"
```

**更新已有工程——连接后的远程终端：**

```bash
cd "<远程工程目录>"
git pull
```

**打开远程工程——远程窗口的命令面板：**

```text
File: Open Folder...
```

打开位置为上面的 `<远程工程目录>`。

**安装远程开发扩展——远程窗口的命令面板：**

```text
Extensions: Install Extensions
```

扩展安装目标为当前 SSH 主机。Settings Sync 不会自动恢复远程扩展，Ubuntu 主机也无需单独登录同步账号。

### 两种方式共同的最后一步：重新配置与构建

在工程所在的 VS Code 窗口中依次执行：

```text
CMake: Select Configure Preset
CMake: Configure
CMake: Build
```

选择工程已有的 Linux 配置。构建目录和编译数据库按工程已有方案在当前 Linux 主机上重新生成。

需要刷新 clangd 时执行：

```text
clangd: Restart language server
```

## 三、日常同步

**当前环境：提交并推送修改。**

```bash
cd "<工程目录>"

git add .
git diff --cached --stat
git commit -m "Update project"
git push
```

**其他电脑或远程主机：获取最新提交。**

```bash
cd "<工程目录>"
git pull
```

Remote-SSH 模式下，Git 命令在远程终端中执行。

## 四、新项目：复用模板

前提：已有模板仓库，并已创建一个空的新项目远程仓库。

```bash
git clone "<模板仓库地址>" "<新工程目录>"
cd "<新工程目录>"

git remote set-url origin "<新项目仓库地址>"
git push -u origin HEAD
```

本地 Linux 中打开工程：

```bash
code .
```

Remote-SSH 中打开工程，使用远程窗口的命令面板：

```text
File: Open Folder...
```

这种方式保留模板仓库的文件和提交历史。新工程打开后，按第二部分的命令重新配置与构建。