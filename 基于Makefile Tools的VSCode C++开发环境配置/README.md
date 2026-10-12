# VS Code C++ Makefile Tools 开发环境

使用 Makefile Tools 管理 GNU Make 工程的构建配置和启动程序，clangd 提供语言服务，GDB 或 CodeLLDB 提供调试支持。配套工程包含两个可执行目标，可练习 GCC / Clang、Debug / Release 的切换和多程序调试。

## 从这里开始

| 开发环境 | 配置指南 | 打开的示例目录 |
| --- | --- | --- |
| Windows 原生 MinGW | [Windows 指南](<VS Code C++ Windows Makefile Tools 开发环境配置指南.md>) | `example-windows/` |
| Linux 本地或 Remote-SSH | [Linux 指南](<VS Code C++ Linux Makefile Tools 开发环境配置指南.md>) | `example-linux/` |

两份指南采用相同的阅读顺序：准备工具与扩展，理解项目文件，完成首次构建和调试，再进入语言服务与日常操作。新建项目可从第 1～4 节开始；接入已有工程可先看第 3.7 节；完整配置集中在指南末尾，Linux 的远程连接步骤单独放在第 8 节。

准备好环境后，用 VS Code 单独打开对应示例目录。首次上手采用 `gcc-debug` 和 GDB，操作顺序如下；各文件为什么这样配置，可结合指南第 3 节阅读。

1. `Makefile: Set the current build configuration`：选择 `gcc-debug`。
2. `Makefile: Set the target to be built by make`：选择 `all`。
3. `Makefile: Clean Configure`：分析整个工程的编译与链接命令，导出编译数据库并识别可执行程序。
4. `clangd: Restart language server`：读取当前配置的数据库。
5. `Makefile: Build the current target`：构建两个程序。
6. `Makefile: Set the make launch configuration`：从识别结果中选择路径属于 `build/gcc-debug/bin/` 的 `example_app`；已设置用户快捷键时可按 F4。
7. 在 `src/math.cpp` 的 `return result;` 处设置断点，选择 `Makefile / GDB` 调试配置，按 F5。

`example_app` 默认计算 `2 + 3 = 5`，`demo_calc` 默认计算 `12 + 30 = 42`。构建目标 `all` 与启动程序独立：保持构建全部目标，需要换程序时用 F4 或命令面板重新选择，再按 F5；使用 LLDB 时选择 `Makefile / LLDB (CodeLLDB)`。

## 配置如何配合

| 文件 | 作用 |
| --- | --- |
| `Makefile` | 定义源码、目标、编译选项及依赖关系，四种配置分别输出到 `build/<配置名>/` |
| `.vscode/settings.json` | 设置 Makefile Tools 和 clangd，程序由 Configure 自动发现 |
| `.vscode/tasks.json` | 为 F5 和 Ctrl+Shift+B 提供构建任务，构建当前配置的 `all` 目标 |
| `.vscode/launch.json` | 读取 Makefile Tools 已选程序路径，从项目根目录启动 GDB 或 CodeLLDB |
| `.vscode/extensions.json` | 推荐所需扩展 |
| `.clang-format` | 统一格式化规则 |
| `keybindings.json` | 用户快捷键模板，按指南合入 VS Code 用户配置后生效 |

Makefile Tools 的 Configure 分析 Make 的编译与链接命令，导出当前配置的 `compile_commands.json` 并发现可执行程序。clangd 从该配置的构建目录读取数据库；切换配置后，重新分析工程、重启 clangd，构建后选择新配置下的程序。示例把编译器解析为完整路径，使构建和语言服务使用同一套工具链。

模板不预填 `makefile.launchConfigurations`，`launch.json` 通过 `${command:makefile.getLaunchTargetPath}` 读取选中的程序。新增目标时更新 Makefile，重新 Clean Configure、Build、选择程序即可，通常无需修改调试模板。选择后插件会向 `settings.json` 写回启动条目及本机具体路径；换环境时重新分析和选择，发布通用模板时可移除这些自动生成条目，保留项目有意维护的自定义条目。自动识别不到程序，或需要自定义插件 Run 的工作目录、参数时，再补充显式启动配置。

F5 先执行 `all` 构建任务，再从项目根目录调试选中的程序，参数使用 `launch.json` 的 `args`。插件自身 Run / Debug 使用启动条目的 `cwd` 和 `binaryArgs`，自动发现的工作目录可能位于 `bin/`。构建失败时先修复错误，不使用“仍然调试”启动旧程序。正常构建根据源码、头文件及 Makefile 的变化增量更新目标。

## 与 CMake Tools 共用的快捷键

配套 [keybindings.json](keybindings.json) 是两套工具共用的用户模板，合入 VS Code 用户快捷键后，打开工程即可按工作区条件使用相应命令。详细命令 ID、生效条件及配置入口见两份指南第 6.3 节。

| 快捷键 | 用途 |
| --- | --- |
| F4 | 选择启动程序，与构建目标独立 |
| F6 | CMake 删除缓存并重新配置；Makefile Tools 清理分析状态并重新分析 |
| F7 | 构建当前目标／范围 |
| Shift+F7 | CMake 选择目标并完成本次构建；Make 保存当前构建目标，随后再按 F7 |
| Shift+F5 / Ctrl+Shift+F5 | 尚未调试时，由插件直接调试／运行启动程序 |
| F5 / Ctrl+F5 | 使用所选 `launch.json` 条目调试／不调试运行 |
| Alt+S | 重启 clangd |
| Ctrl+K，Ctrl+A / Ctrl+K，Ctrl+Z | 打开用户设置／用户快捷键 JSON |

Make 的 F7 使用插件当前构建目标；Ctrl+Shift+B、F5、Ctrl+F5 仍使用示例任务构建 `all`。插件直接调试默认 GDB，使用 F5 选择的 CodeLLDB 条目时继续按 F5。同一工程同时启用两套工具时，共用按键优先 CMake；调试中 Shift+F5、Ctrl+Shift+F5 保留停止／重启用途。

## 目录结构

```text
makefile-tools/
├── README.md
├── VS Code C++ Windows Makefile Tools 开发环境配置指南.md
├── VS Code C++ Linux Makefile Tools 开发环境配置指南.md
├── keybindings.json
├── example-windows/
│   ├── Makefile
│   ├── .vscode/
│   ├── .clang-format
│   ├── .gitignore
│   ├── include/demo/math.hpp
│   └── src/
└── example-linux/           # 与 Windows 示例相同的源码，使用 Linux 工具设置
```

两个示例目录均为独立工程。接入已有 Makefile 工程时，按指南迁移实际的 Make 参数和目标，并保留项目原有的构建规则、依赖和 C++ 标准。
