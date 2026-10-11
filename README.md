# VS Code C++ 开发环境配置

面向 Windows 和 Linux 的 VS Code C++ 开发环境指南，包含工具链安装、CMake 项目配置、语言服务、调试、环境同步，以及接入已有 CMake 工程的完整示例。

使用 CMake Tools 和 CMake Presets 管理配置与构建，clangd 提供代码补全、诊断和格式化，GDB 或 CodeLLDB 提供调试支持。Windows 指南采用 MSYS2 原生 MinGW 工具链，Linux 指南同时覆盖本地开发与 Remote-SSH。

## 文档导航

按当前任务选择入口，具体配置文件和操作说明见对应文档。

| 任务 | 文档 |
| --- | --- |
| 在 Windows 上配置开发环境 | [Windows 开发环境配置指南](<vsconfig/VSCode C++ Windows 通用开发环境配置指南.md>) |
| 在 Linux 上配置开发环境，或通过 SSH 远程开发 | [Linux 开发环境配置指南](<vsconfig/VSCode C++ Linux 通用开发环境配置指南.md>) |
| 将配置接入已有 CMake 工程 | [已有工程接入教程与完整示例](example/README.md) |
| 在另一台 Windows 电脑恢复环境 | [Windows 环境同步指南](<vssync/VSCode C++ 环境同步指南（Windows）.md>) |
| 在本地或远程 Linux 环境恢复配置 | [Linux 环境同步指南](<vssync/VS Code C++ 环境同步指南（Linux）.md>) |

首次搭建环境时，先阅读对应系统的配置指南；工具准备好后，可运行下面的完整示例。接入自己的工程时，按接入教程保留原有目标、依赖、C++ 标准和构建选项，再合入所需配置。更换电脑或开发主机时，参考环境同步指南。

## 配置方案

| 功能 | 配置方式 |
| --- | --- |
| 编译器 | GCC / G++ 或 Clang / Clang++ |
| 构建生成器 | Ninja；Windows 的 MinGW Makefiles；Linux 的 Unix Makefiles |
| 构建组合 | 用 Presets 区分编译器、生成器以及 Debug / Release，各组合使用独立构建目录 |
| 语言服务 | clangd 从当前 Configure Preset 的构建目录读取 `compile_commands.json` |
| 调试 | GDB 配合 C/C++ 扩展，或 LLDB 配合 CodeLLDB 扩展 |
| 多可执行目标 | 通过 CMake Tools 选择启动目标，F5 启动所选程序 |
| 远程开发 | Remote-SSH 连接 Linux 主机，在远程侧配置、构建和调试 |
| 环境同步 | Settings Sync 同步编辑器配置，Git 同步源码与共享工程配置；工具链在目标主机安装 |

主指南中的示例采用 C++20，`example/` 的已有工程示例采用 C++17，用于演示保留工程原有标准的接入过程。

## 运行完整示例

### 1. 准备工具

按对应系统的配置指南安装 VS Code、Git、CMake、clangd，以及所选的编译器、构建工具和调试器。完整接入示例要求 CMake 3.21 或更新版本。

首次运行建议选择 **GCC + Ninja + Debug + GDB**。VS Code 中安装 CMake Tools、clangd 和 C/C++ 扩展；选择 LLDB 调试时安装 CodeLLDB。Windows 工具链应使用兼容的 MSYS2 环境，工具程序应能在 VS Code 使用的终端环境中找到。

### 2. 获取仓库并打开工程

在 PowerShell 或 Linux 终端执行：

```sh
git clone https://github.com/YLJF/vscode-configuration.git
cd vscode-configuration
```

然后在 VS Code 中通过 **文件 → 打开文件夹**，按系统选择以下目录：

| 系统 | 打开的工程目录 |
| --- | --- |
| Windows | `example/02-vscode-windows` |
| Linux | `example/03-vscode-linux` |

这些目录各自包含独立的 `CMakeLists.txt`、`CMakePresets.json` 和 `.vscode/` 配置。首次运行前，按实际工具链位置检查 clangd 的 `--query-driver` 匹配路径，以及所用调试器的路径设置。

### 3. 配置并构建

在命令面板（Ctrl+Shift+P）中依次执行：

1. `CMake: Select Configure Preset`：选择 `gcc-ninja-debug`。
2. `CMake: Configure`：生成构建规则和编译数据库。
3. `clangd: Restart language server`：让语言服务读取刚生成的数据库。
4. `CMake: Select Build Preset`：选择同名的 `gcc-ninja-debug`。
5. `CMake: Build`：构建 `acme_calc` 和 `acme_info`。

生成的文件位于工程内的 `build/gcc-ninja-debug/`。需要 Clang 或 Make 时，选择对应预设，并准备相应工具；切换 Configure Preset 后重新 Configure、重启 clangd，再选择匹配的 Build Preset。

### 4. 选择程序并调试

在 `app/calc.cpp` 中设置断点，执行 **`CMake: Set Launch/Debug Target`**，选择 `acme_calc`。在“运行和调试”视图中选择 `CMake / GDB`，按 F5 启动调试。使用 CodeLLDB 时，选择 `CMake / LLDB (CodeLLDB)`。

要调试另一个程序，结束当前会话后再次执行目标选择命令，选择 `acme_info`，然后按 F5。后续 F5 会复用当前有效的目标选择。

示例启用了 `cmake.buildBeforeRun`，F5 启动前会触发构建。Ninja 或 Make 根据依赖关系执行增量构建：通常只重新编译受影响的文件并链接需要更新的目标；没有变化时无需重新编译。

若希望从原有工程开始亲手接入，按照 [接入教程](example/README.md) 复制 `example/01-existing-project`，再逐步添加配置。

## 生成 CMake Presets 模板

在仓库根目录执行以下命令，将模板生成到指定项目：

```sh
cmake -DOUTPUT_DIR="<项目根目录>" -P ./vsconfig/tools/generate-cmake-presets.cmake
```

将 `<项目根目录>` 连同尖括号替换为实际路径。脚本按运行主机选择 Windows 或 Linux 的 Make 生成器，生成 GCC / Clang、Ninja / Make、Debug / Release 共八组预设；名称包含生成器，例如 `gcc-ninja-debug`、`gcc-make-debug`。

脚本生成 `CMakePresets.json`，项目目标、依赖和编辑器配置仍按指南设置。生成后可根据项目要求调整编译器、构建选项和预设组合。已有 `CMakePresets.json` 时脚本默认拒绝覆盖；已有工程应优先扩展原有预设。参数和文件维护方式见 [Windows 指南](<vsconfig/VSCode C++ Windows 通用开发环境配置指南.md>) 或 [Linux 指南](<vsconfig/VSCode C++ Linux 通用开发环境配置指南.md>)。

## 自定义快捷键

以下快捷键需按指南合入 VS Code 的**用户 `keybindings.json`** 后生效。命令面板同样可以完成这些操作。

| 快捷键 | 操作 |
| --- | --- |
| F4 | 选择启动／调试目标：`CMake: Set Launch/Debug Target`（`cmake.selectLaunchTarget`） |
| F6 | 删除 CMake 配置缓存并重新 Configure，完成后按需构建 |
| Alt+S | 重启 clangd；切换预设时先 Configure，再重启 |
| Ctrl+K，Ctrl+A | 打开用户设置 JSON |
| Ctrl+K，Ctrl+Z | 打开用户快捷键 JSON |

两组 Ctrl+K 快捷键分两次按下：先按 Ctrl+K，再按 Ctrl+A 或 Ctrl+Z。F4 的绑定条件和完整 JSON 见两份配置指南的“最终配置汇总”。用户快捷键由 VS Code 管理，项目配置放在 `.vscode/` 中。

## 仓库结构

```text
vscode-configuration/
├── README.md
├── vsconfig/                 # Windows / Linux 开发环境配置指南
│   └── tools/                # Presets 生成脚本和模板
├── vssync/                   # Windows / Linux 环境同步指南
├── example/
│   ├── README.md             # 已有 CMake 工程接入教程
│   ├── 01-existing-project/  # 接入前的原有工程
│   ├── 02-vscode-windows/    # Windows 完整接入结果
│   ├── 03-vscode-linux/      # Linux 完整接入结果
│   └── tools/               # 随接入示例提供的生成工具
├── .vscode/                  # 根目录示例的 VS Code 配置
├── CMakeLists.txt            # 根目录单目标示例，目标名为 test
├── CMakePresets.json          # 根目录示例的预设
└── src/                      # 根目录示例源码
```

根目录还保留一个单目标 `test` 示例。体验完整接入流程和多可执行目标调试时，按上面的步骤单独打开 `example/` 下对应系统的工程目录。

共享的源码、CMake 配置和编辑器项目配置纳入 Git；构建目录、编译数据库、本机 `CMakeUserPresets.json`、验证记录和备份由 `.gitignore` 排除。个人工具路径和 SSH 连接信息按实际环境配置。
