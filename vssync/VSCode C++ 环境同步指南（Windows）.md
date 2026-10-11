# VS Code C/C++ 环境同步指南（Windows）

Settings Sync 同步编辑器设置和扩展，Git 同步源码与工程配置。开发工具需要在每台电脑上单独准备。

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

```powershell
Set-Location "<工程目录>"

git init
git add .
git diff --cached --stat
git commit -m "Save project configuration"
git remote add origin "<仓库地址>"
git push -u origin HEAD
```

**已有远程仓库：**

```powershell
Set-Location "<工程目录>"

git add .
git diff --cached --stat
git commit -m "Update project configuration"
git push
```

## 二、新电脑：恢复同步

前提：VS Code、Git、CMake，以及工程使用的编译器、调试器和 clangd 已准备好。

### 1. 恢复编辑器配置

在 VS Code 命令面板中执行：

```text
Settings Sync: Turn On
```

使用与原电脑相同的账号和开发 Profile。

### 2. 获取工程

**首次获取：**

```powershell
git clone "<仓库地址>" "<本地工程目录>"
Set-Location "<本地工程目录>"
code .
```

**更新已有工程：**

```powershell
Set-Location "<本地工程目录>"
git pull
code .
```

### 3. 重新配置与构建

在打开工程的 VS Code 命令面板中依次执行：

```text
CMake: Select Configure Preset
CMake: Configure
CMake: Build
```

选择工程已有的 Windows 配置。构建目录和编译数据库按工程已有方案在新电脑上重新生成。

需要刷新 clangd 时执行：

```text
clangd: Restart language server
```

## 三、日常同步

**当前电脑：提交并推送修改。**

```powershell
Set-Location "<工程目录>"

git add .
git diff --cached --stat
git commit -m "Update project"
git push
```

**其他电脑：获取最新提交。**

```powershell
Set-Location "<工程目录>"
git pull
```

VS Code 用户设置由 Settings Sync 自动同步；工程文件通过上述 Git 命令同步。

## 四、新项目：复用模板

前提：已有模板仓库，并已创建一个空的新项目远程仓库。

```powershell
git clone "<模板仓库地址>" "<新工程目录>"
Set-Location "<新工程目录>"

git remote set-url origin "<新项目仓库地址>"
git push -u origin HEAD

code .
```

这种方式保留模板仓库的文件和提交历史。新工程打开后，按第二部分的命令重新配置与构建。
