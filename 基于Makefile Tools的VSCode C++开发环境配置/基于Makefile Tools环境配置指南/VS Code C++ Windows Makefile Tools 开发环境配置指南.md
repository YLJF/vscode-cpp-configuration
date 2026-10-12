# VS Code C++ Windows Makefile Tools 开发环境配置指南

## 使用说明

### 适用范围与前置条件

本指南适用于 Windows 原生 GNU/MinGW 工具链（MSYS2 MINGW64 或 UCRT64），使用 **Makefile Tools** 管理 GNU Make 工程，由 clangd 提供语言服务，GDB 或 CodeLLDB 提供调试支持。配套工程采用 C++20，并通过两个可执行目标演示构建配置切换与多程序调试。

| 项目 | 要求或选择 |
| --- | --- |
| 系统与权限 | 满足 VS Code 和 MSYS2 系统要求的 64 位 Windows；具备安装软件和修改用户 PATH 的权限 |
| 构建工具 | `mingw32-make`，配套 Makefile 使用 Windows 命令环境 |
| 编译器 | 选择项目支持的 GCC 或 Clang；配套示例要求支持 C++20 |
| 语言服务 | 安装 clangd 扩展和 clangd 程序；扩展须支持参数中的 `${command:...}` 命令变量 |
| 调试器 | GDB 由 C/C++ 扩展接入；LLDB 由 CodeLLDB 接入 |
| 已有工程 | 先按项目构建说明准备依赖、环境和 Makefile，再接入编辑器 |

安装命令覆盖示例提供的两种编译器和调试方式，可按需取用。已有工程的 C++ 标准、工具版本、依赖和构建规则以项目要求为准。

### 首次配置路径

第一次配置，建议先用 **GCC + Debug + GDB** 跑通 `gcc-debug`：准备工具和扩展，理解项目文件，再完成一次构建与断点调试。确认这条流程可用后，就可以按项目需要切换 Clang、Release 或 CodeLLDB。

- **使用配套示例：** 用 VS Code 单独打开 `example-windows/`，顺序执行第 1～4 节；全部文件集中在[第 8 节](#8-最终配置汇总)。
- **新建 Makefile 工程：** 参考第 3 节理解文件关系，再从最终配置汇总创建项目文件，并按实际目标和依赖修改 Makefile。
- **接入已有工程：** 先阅读项目自己的构建说明，按第 1～2 节准备环境，再按[第 3.7 节](#37-接入已有-makefile-工程)合入所需配置。

完成首次调试后，日常修改和配置切换可查阅第 6 节。自定义快捷键属于可选设置，命令面板也能完成相同操作。

### 阅读导航

- [1. 安装工具链](#1-安装工具链)
- [2. 配置 VS Code 扩展](#2-配置-vs-code-扩展)
- [3. 项目结构与配置说明](#3-项目结构与配置说明)
- [4. 配置、构建与调试](#4-配置构建与调试)
- [5. 语言服务与格式化](#5-语言服务与格式化)
- [6. 日常操作与快捷键](#6-日常操作与快捷键)
- [7. 常见问题](#7-常见问题)
- [8. 最终配置汇总](#8-最终配置汇总)

### 命令与术语约定

- 项目命令默认在包含 `Makefile` 的项目根目录执行；已有工程使用其原有工作目录。
- `bash` 安装命令在所选 MSYS2 终端执行；`powershell` 项目命令在 PowerShell 执行。
- `<项目根目录>`、`<主机地址>` 等均为占位符，使用前连同尖括号替换为实际值。
- Configure 表示 Makefile Tools 分析 Make 命令、提取编译信息和目标；Build 表示 Make 执行编译与链接。Configure 不生成项目 Makefile，已有 Autotools 等工程仍先完成自己的准备步骤。
- 构建配置决定 Make 参数，构建目标决定更新哪些产物，启动配置决定运行哪个程序。命令与配置项保留 VS Code 原始名称，便于在命令面板和设置中搜索。

## 1. 安装工具链

VS Code 通过扩展调用 Make、编译器和调试器。先准备项目需要的工具，并让从 Windows 启动的编辑器能够找到它们。

### 1.1 选择并安装 MSYS2 工具

从 [MSYS2 官网](https://www.msys2.org/) 安装 MSYS2，根据项目要求选择 MINGW64 或 UCRT64 环境。编译器、调试器和二进制依赖库使用同一兼容环境的软件包。

| 环境 | Windows 工具目录 | 软件包前缀 |
| --- | --- | --- |
| MINGW64 | `<MSYS2安装目录>/mingw64/bin` | `mingw-w64-x86_64-` |
| UCRT64 | `<MSYS2安装目录>/ucrt64/bin` | `mingw-w64-ucrt-x86_64-` |

在 MSYS2 终端更新系统；如提示关闭终端，按提示重新打开后继续更新：

```bash
pacman -Syu
```

随后在所选环境的终端安装工具。以下以 MINGW64 为例，选择 UCRT64 时修改包前缀：

```bash
PACKAGE_PREFIX=mingw-w64-x86_64-
pacman -S --needed \
  "${PACKAGE_PREFIX}gcc" \
  "${PACKAGE_PREFIX}make" \
  "${PACKAGE_PREFIX}gdb" \
  "${PACKAGE_PREFIX}clang" \
  "${PACKAGE_PREFIX}clang-tools-extra"
```

`gcc` 提供 GCC/G++，`clang` 提供 Clang/Clang++，所选环境的 `make` 提供 `mingw32-make`。clangd 程序也可通过扩展提供的安装功能准备；仅使用一种编译器或调试器时，安装所需组合即可。

### 1.2 配置 Windows PATH

将所选环境的 `bin` 目录加入 Windows 用户 PATH，例如 `<MSYS2安装目录>/mingw64/bin`。保存后完全退出并重新打开 VS Code 和终端，使新环境生效。

在新打开的 PowerShell 中检查工具位置：

```powershell
Get-Command mingw32-make, g++, clang++, clangd, gdb
```

检查当前方案实际使用的工具即可。若存在多套同名程序，确认优先找到的是预期工具链；工具未加入 PATH 时，在用户设置或项目所用环境中指定实际位置。Clang 使用面向 Windows GNU/MinGW 的版本，工具和依赖库保持兼容。

### 1.3 确定项目命令环境

配套 Makefile 的 Windows 分支使用 `cmd.exe` 执行目录命令，并用系统自带的 Windows PowerShell 解析编译器完整路径。因此，MSYS2 终端用于安装软件包，项目命令在 PowerShell 中执行。

已有 Makefile 若依赖 `sh`、`rm`、`sed` 等 Unix 命令，按项目要求准备兼容环境。需要在 Linux 上执行构建时，可使用 WSL 或 Remote-SSH，并参考 Linux 指南。

使用 CodeLLDB 时，通常由扩展提供配套 LLDB 后端；需要在终端单独使用 LLDB 时，再安装所选环境的命令行工具。

## 2. 配置 VS Code 扩展

工具链准备好后，将它接入编辑器。在扩展视图中安装 Makefile Tools 和 clangd，再根据所用调试器选择 C/C++ 或 CodeLLDB。

| 扩展 | 标识符 | 用途 | 安装条件 |
| --- | --- | --- | --- |
| Makefile Tools | `ms-vscode.makefile-tools` | 分析 Makefile、选择构建配置和目标、导出编译数据库 | 使用本文的 Make 集成 |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | 补全、诊断、跳转和格式化 | 使用本文的语言服务 |
| C/C++ | `ms-vscode.cpptools` | 提供 `cppdbg` 调试适配器 | 使用 GDB |
| CodeLLDB | `vadimcn.vscode-lldb` | 提供 `lldb` 调试适配器和配套 LLDB 后端 | 使用 LLDB |

clangd 扩展需要可执行的 clangd 程序，且须支持参数中的 `${command:...}` 命令变量。可使用扩展提供的安装功能，或在 VS Code 用户设置中通过 `clangd.path` 指定已有程序；个人安装路径保存在用户设置中。

选择 CodeLLDB 时，等待扩展的平台组件安装完成，通常保留默认后端。需要替换后端时，按 [CodeLLDB 手册](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md#using-alternate-lldb-backends) 配置兼容的 `lldb.library` 动态库路径。

这套配置由 clangd 负责 C++ 语言分析，工作区设置会关闭 C/C++ IntelliSense，同时保留 C/C++ 扩展的 GDB 调试能力。两种调试扩展可以同时安装，使用时选择相应调试条目。

## 3. 项目结构与配置说明

环境和扩展就绪后，还需要告诉它们怎样构建、分析和调试这个项目。Makefile 定义源码、目标和构建规则；`.vscode` 文件保存插件设置、构建任务和调试方式，让编辑器调用前面准备好的工具。本章按这些文件的关系逐步展开，完整内容集中在[第 8 节](#8-最终配置汇总)。

使用配套示例时，用 VS Code 单独打开 `example-windows/`；新建工程可按下面的结构创建文件。已有工程保留自己的目录和构建规则，按第 3.7 节接入。

```text
example-windows/
├── .vscode/
│   ├── settings.json
│   ├── tasks.json
│   ├── launch.json
│   └── extensions.json
├── include/
│   └── demo/math.hpp
├── src/
│   ├── main.cpp
│   ├── calc.cpp
│   └── math.cpp
├── .clang-format
├── .gitignore
└── Makefile
```

构建产物、编译数据库和插件日志随后生成在 `build/<配置名>/`。该目录由工具创建，各主机分别生成；用户快捷键保存在 VS Code 用户配置中。

### 3.1 示例源文件

示例用两个独立的 `main()` 入口演示多程序工程，并共用一个加法函数。完整源码见[第 8.1 节](#81-示例源文件)。

| 文件 | 对应目标或用途 | 示例行为 |
| --- | --- | --- |
| `src/main.cpp` | `example_app` 的入口 | 计算 `2 + 3`，输出结果和构建类型宏 |
| `src/calc.cpp` | `demo_calc` 的入口 | 默认计算 `12 + 30`，也可接受两个整数参数 |
| `src/math.cpp` | 两个目标共用的实现 | 在 `demo::add()` 中计算并返回结果 |
| `include/demo/math.hpp` | 公共头文件 | 声明 `demo::add()` |

每个程序保留自己的入口，共享实现分别链接到两个目标。调试时可在 `src/math.cpp` 的 `return result;` 行观察局部变量 `result`；下面先在 Makefile 中建立这些源码与目标的关系。

### 3.2 `Makefile`

在项目根目录创建 `Makefile`，完整内容见[第 8.2 节](#82-makefile)。它定义 `example_app`、`demo_calc` 和汇总目标 `all`，并通过 `CONFIG` 选择编译器与构建类型。

| 配置名 | 编译器 | 构建类型 | 输出目录 |
| --- | --- | --- | --- |
| `gcc-debug` | G++ | Debug | `build/gcc-debug/` |
| `gcc-release` | G++ | Release | `build/gcc-release/` |
| `clang-debug` | Clang++ | Debug | `build/clang-debug/` |
| `clang-release` | Clang++ | Release | `build/clang-release/` |

这四个名称是示例 Makefile 约定的参数值。`all` 构建两个程序，单独指定 `example_app` 或 `demo_calc` 可只更新对应目标；可执行文件位于当前配置的 `bin/` 目录，名称分别为 `example_app.exe` 和 `demo_calc.exe`。

Makefile 从 PATH 解析所选编译器的完整路径，让 Makefile Tools 导出的编译数据库记录实际使用的驱动。示例采用 C++20 和常用警告选项，Debug 使用 `-O0 -g`，Release 使用 `-O2`；已有工程沿用自己的标准、优化选项和依赖。

`-MMD -MP` 生成头文件依赖，Make 在后续构建中读取 `.d` 文件。对象文件依赖源码和 Makefile，程序还依赖对象文件与 Makefile，因此源码、头文件和构建规则变化都能触发相应更新。不同配置使用独立目录，日常增量构建行为见第 6.2 节。

复制 Makefile 时保留命令行开头的 **Tab** 缩进；这些行是 Make 的 recipe，不能直接改为普通空格。

### 3.3 `.vscode/settings.json`

Makefile 已经定义了如何构建两个程序，插件还需要知道从哪个目录、用哪些参数调用它。这些信息保存在工作区的 `settings.json` 中，同时供语言服务和程序选择使用。完整文件见[第 8.3 节](#83-vscodesettingsjson)，其中的主要配置关系如下。

| 设置 | 配置目的 |
| --- | --- |
| `makefile.makePath` | 使用 `mingw32-make` 调用 GNU Make |
| `makefile.makeDirectory` / `makefile.makefilePath` | 确定工作目录和入口 Makefile |
| `makefile.configurations` | 将构建配置名映射为 Make 参数，例如 `CONFIG=gcc-debug` |
| `makefile.extensionOutputFolder` | 将插件输出放在当前配置的构建目录，并创建数据库所需的父目录 |
| `makefile.compileCommandsPath` | 将当前配置的 `compile_commands.json` 导出到对应目录 |
| `clangd.arguments` | 读取当前配置的数据库，并限定可查询的编译器驱动 |
| `makefile.defaultLaunchConfiguration` | 明确设置插件自身调试命令使用 GDB |

Makefile Tools 设置中的 `${configuration}` 表示当前构建配置名；clangd 参数使用 `${command:makefile.getConfiguration}` 取得同一名称。它们分别由对应扩展展开，使数据库的输出目录和读取目录保持一致，语言服务的具体设置见第 5 节。

示例关闭自动 Configure，按第 4 节明确执行分析和构建。程序名和输出位置由 Makefile 定义，模板不预填 `makefile.launchConfigurations`。Configure 分析编译与链接命令，识别可执行产物；构建完成后，从识别结果中选择当前配置下要启动的程序，调试配置再读取它的路径。这样新增程序时，通常只需修改 Makefile 并重新分析，无需在编辑器配置里重复维护程序名。

选择程序后，插件会将相应启动条目写入工作区 `settings.json`，其中可能包含本机的具体路径。这是保存启动选择的正常行为；换目录、换主机或切换构建配置后，重新分析并选择当前环境的程序即可。复杂工程若未能自动识别某个产物，或需要自定义插件自身 Run / Debug 的工作目录和参数，再按需补充 `makefile.launchConfigurations`，其中 `binaryPath` 填写真实可执行文件路径。

### 3.4 `.vscode/tasks.json`

工作区设置让 Makefile Tools 能够调用 Make。接着，在 `tasks.json` 中定义构建任务，供 VS Code 的 Ctrl+Shift+B 和调试前构建使用，让两种操作采用同一条命令。完整文件见[第 8.4 节](#84-vscodetasksjson)。

任务使用 `type: process` 直接调用 `${config:makefile.makePath}`，通过 `${command:makefile.getConfiguration}` 读取当前配置，并将 `CONFIG=<配置名>` 传给 Make。任务始终构建 `all`，保证示例中选中的任一程序都得到更新；插件当前选中的构建目标不会改变这份任务的 `all` 参数。

`options.cwd` 指定工程根目录，`$gcc` 问题匹配器将编译错误关联到源码。示例使用不带数字的 `-j`，不限制并发任务数量；较大工程应按 CPU 和内存情况改为 `-j<N>`，例如 `-j4`。

接入已有工程时，任务中的目录、入口文件、变量和目标都要与项目真实命令一致。`makefile.configurations` 中的 `makeArgs` 不会自动传入这份自定义任务。

### 3.5 `.vscode/launch.json`

构建任务负责更新程序，`launch.json` 再指定用哪个调试器、怎样启动所选程序。通过 `preLaunchTask` 把两者连接起来，F5 就能先构建，再进入调试。完整文件见[第 8.5 节](#85-vscodelaunchjson)，提供 GDB 与 CodeLLDB 两个条目。

| 配置项 | 作用 |
| --- | --- |
| `program` | `${command:makefile.getLaunchTargetPath}` 读取 Makefile Tools 已选程序的路径 |
| `cwd` | `${workspaceFolder}` 让 F5 始终从项目根目录运行程序 |
| `preLaunchTask` | 执行 `Makefile: build active configuration`，构建完成后再启动调试 |
| `args` | 保存 F5 启动程序时的参数 |
| `type: cppdbg` / `MIMode: gdb` | 通过 C/C++ 扩展调用 GDB |
| `type: lldb` | 通过 CodeLLDB 调用 LLDB |

按 F5 时先执行构建任务。Make 根据依赖和时间戳进行增量更新：没有变化就复用产物，有变化才重新编译、链接受影响目标。完整行为见第 6.2 节。构建失败时先修复任务报告的错误，再重新调试，避免使用“仍然调试”运行旧产物。

`getLaunchTargetPath` 读取已经选好的启动程序，构建由 `preLaunchTask` 完成。构建目标 `all` 与启动程序分别选择，因此任务可以构建全部程序，F5 仍然调试选中的那一个。程序路径无需通过 `${buildTarget}` 拼接；`all` 是汇总目标，没有对应的可执行文件。

本指南的 F5 采用 `launch.json` 中的项目根目录和 `args`。插件自身的 Debug / Run 则使用启动条目的 `cwd`、`binaryArgs`，并由 `makefile.buildBeforeLaunch` 控制运行前构建；自动发现的条目工作目录可能位于 `bin/`，需要读取项目相对资源时可按需调整。Makefile Tools 的 `MIMode: lldb` 属于 C/C++ 的 MI 调试方式，CodeLLDB 使用独立的 `type: lldb` 条目。

### 3.6 扩展推荐、格式化与忽略文件

项目文件就绪后，再补充团队共享的编辑约定。`.vscode/extensions.json` 推荐所需扩展，`.clang-format` 保存格式规则，`.gitignore` 排除构建产物、数据库和插件日志。完整内容见第 8.6～8.7 节。

工作区设置将 clangd 设为 C++ 默认格式化器，并启用保存时格式化。已有工程采用自己的格式规则；合入配置时保留原有扩展推荐与忽略条目。

### 3.7 接入已有 Makefile 工程

已有工程先按项目文档在终端完成一次构建，确认工作目录、Make 程序、入口文件、变量和目标。随后将这条真实命令接入编辑器，保留原有源码、标准、依赖和构建规则。

1. **确定 Make 入口。** 在 `makefile.makePath` 中指定实际程序，用 `makefile.makeDirectory` 表示原命令的工作目录，用 `makefile.makefilePath` 表示入口文件。
2. **保存构建组合。** 在 `makefile.configurations` 的 `makeArgs` 中保存原命令使用的参数，例如 `MODE=debug` 或 `CXX=clang++`。配置名是插件中的选择名称，是否传给 Make、如何映射，由项目参数决定。
3. **分析所需目标。** 选择能覆盖相关源码和程序的构建目标，例如项目的 `all`，执行 `Makefile: Clean Configure`。检查数据库中的编译器、宏、标准和包含目录，同时确认插件能从链接命令识别可执行产物。
4. **确认启动程序。** 构建后执行 `Makefile: Set the make launch configuration`，从识别结果中选择当前配置的程序。先使用自动发现；确实无法识别，或需要自定义插件运行目录与参数时，再补充 `makefile.launchConfigurations`。
5. **接入 F5 构建。** 合入 `tasks.json` 和 `launch.json`，将任务中的 Make 程序、目录、入口文件、变量和目标改为与原命令一致。示例的 `CONFIG=${command:makefile.getConfiguration}` 只适用于其命名约定；已有工程采用自己的映射。
6. **接入语言服务与调试。** 合入 clangd 参数，重新 Configure 并重启 clangd，构建后选择启动程序和调试器，再在源码中设置断点、按 F5。

如果项目需要先运行 `configure`、初始化 SDK 或生成 Makefile，先完成原有准备流程。可从已初始化环境的终端启动 VS Code，或按需使用 `makefile.preConfigureScript`；脚本类型与执行位置须符合项目平台。

Configure 需要看到完整的编译和链接命令，才能同时生成语言服务数据库并发现启动程序。Makefile 隐藏命令时，使用项目支持的详细输出参数；也可通过 `makefile.buildLog` 读取构建日志。日志应与当前参数一致，并覆盖所需源码与可执行产物的链接过程，普通增量构建的日志可能不完整。分析异常的处理见第 7 节。

### 3.8 配置维护与版本打包

接入完成后，将项目共用配置与个人环境配置分别维护，便于其他开发者复用，并保持发布包内容明确。

| 内容 | 维护与打包方式 |
| --- | --- |
| 源码、头文件、Makefile、依赖说明 | 纳入项目版本管理；发布时按项目交付要求保留 |
| 通用 `.vscode` 文件、`.clang-format`、`.gitignore` | 纳入版本管理；共享路径使用工作区变量，保留项目约定 |
| 插件自动写回的启动条目 | 发布通用模板时移除含本机具体路径的条目，由使用者重新分析和选择；有意维护的自定义条目按项目约定保留 |
| `build/`、`.cache/`、编译数据库、插件日志 | 按忽略规则排除，由使用者在当前环境生成 |
| 个人工具路径、SSH 连接和私钥 | 保存在用户环境或用户配置中 |
| 用户快捷键模板 | 随指南提供，由使用者合入用户 `keybindings.json` |

发布指南与示例时，保留两份指南、配套源码、Makefile 和通用配置，排除本机构建产物，并检查 `settings.json` 是否带入插件自动写回的本机启动路径。通用模板可移除这些自动生成条目；为项目有意设置的运行目录、参数等自定义条目仍按项目约定保留。`.gitignore` 约束版本控制，不会自动过滤手工复制或压缩的文件，也不会阻止插件改写已共享的设置文件，因此打包时仍需检查实际内容。已有项目的正式发布范围按项目自身规则确定。

## 4. 配置、构建与调试

现在可以用这些配置完成一次实际构建与调试：先选定构建参数，让插件分析编译命令，再生成程序并选择要调试的目标。以下命令从命令面板（Ctrl+Shift+P）执行；界面译文可能变化，可用英文名称搜索。

### 4.1 首次配置、构建与调试

Makefile Tools 维护三项独立选择，操作前先确认它们分别控制什么。

| 选择 | 命令 | 决定的内容 |
| --- | --- | --- |
| 构建配置 | `Makefile: Set the current build configuration` | 传给 Make 的参数，例如 `CONFIG=gcc-debug` |
| 构建目标 | `Makefile: Set the target to be built by make` | 插件 Configure / Build 使用的目标，例如 `all` |
| 启动配置 | `Makefile: Set the make launch configuration` | 选中的可执行程序；其工作目录和参数供插件自身 Run / Debug 使用 |

首次操作采用 `gcc-debug`，构建目标保持为 `all`，使 Configure 能收集两个程序的编译与链接信息。随后单独选择启动程序：`all` 负责构建全部目标，`example_app` 或 `demo_calc` 决定实际调试哪个程序，两项选择互不替代。

1. 执行 **`Makefile: Set the current build configuration`**，选择 `gcc-debug`。
2. 执行 **`Makefile: Set the target to be built by make`**，选择 `all`。
3. 执行 **`Makefile: Clean Configure`**，等待插件分析编译与链接命令、导出数据库并识别可执行程序。
4. 执行 **`clangd: Restart language server`**，让语言服务读取当前数据库。
5. 执行 **`Makefile: Build the current target`**，构建两个程序。
6. 执行 **`Makefile: Set the make launch configuration`**，从自动识别的列表中选择路径属于当前 `build/gcc-debug/bin/` 的 `example_app`；已设置第 6.3 节用户快捷键时可按 F4。
7. 在 `src/math.cpp` 的 `return result;` 行设置断点，在“运行和调试”视图中选择 **`Makefile / GDB`**，按 F5。

断点停下后，可观察参数 `a`、`b` 与局部变量 `result`；继续运行，`example_app` 输出 `2 + 3 = 5`。后续 F5 会复用当前有效的启动选择，并按第 3.5 节执行运行前增量构建。选择后 `settings.json` 中出现插件保存的程序路径属于正常现象，项目模板本身无需提前列出这些路径。

### 4.2 在命令行构建与运行

同一 Makefile 也可以直接从项目根目录调用。下面构建 `gcc-debug` 下的两个程序，再运行它们：

```powershell
mingw32-make CONFIG=gcc-debug all -j2
./build/gcc-debug/bin/example_app.exe
./build/gcc-debug/bin/demo_calc.exe 10 20
```

第二个命令运行默认示例，第三个命令让 `demo_calc` 计算 `10 + 20 = 30`。其他组合替换 `CONFIG` 值即可；构建单个程序时，将 `all` 改为对应目标。

命令行 Build 不会替 Makefile Tools 更新编译数据库。源码实现未改变编译参数时可直接构建；Makefile、源文件列表或编译选项变化后，在编辑器中重新 Configure 并重启 clangd。

### 4.3 选择 GDB 或 CodeLLDB

程序选择与调试器选择分别进行。保持 Makefile Tools 已选程序，在“运行和调试”视图选择所需条目，再按 F5。

| 调试条目 | 扩展与后端 | 使用要点 |
| --- | --- | --- |
| `Makefile / GDB` | C/C++ 扩展与 GDB | `miDebuggerPath` 指向兼容的 GDB |
| `Makefile / LLDB (CodeLLDB)` | CodeLLDB 与其 LLDB 后端 | 等待平台组件安装完成，使用 `type: lldb` |

GCC 和 Clang 均可配合合适的 GDB 或 LLDB 使用；编译器与调试器的兼容性取决于平台、产物和调试信息。需要稳定的断点和变量观察时选择 Debug 配置。

### 4.4 调试多个可执行文件

需要更换程序时，结束当前调试会话，执行 **`Makefile: Set the make launch configuration`**（命令 ID：**`makefile.setLaunchConfiguration`**），从分析结果中选择当前构建配置下的 `demo_calc`，再按 F5。它默认输出 `12 + 30 = 42`；构建目标仍可保持 `all`。选定后，后续调试复用该启动选择，无需每次重新选择。

两个示例程序都从项目根目录运行，默认不带参数，因此复用一套 GDB / CodeLLDB 条目，程序路径随启动选择变化。需要给 `demo_calc` 传参时，在对应调试条目的 `args` 中填写，例如 `["10", "20"]`；改选其他程序后，这组参数仍会生效，按实际需要调整。不同程序需要不同参数、环境变量或工作目录时，可为它们保留独立调试条目。

插件自身的 Debug / Run 命令从 `makefile.launchConfigurations` 的 `binaryArgs` 获取参数，F5 从 `launch.json` 的 `args` 获取参数。命令变量返回的字符串不能自动展开为多个数组参数，应在相应数组中逐项填写。

新增可执行程序时，在 Makefile 中声明相应源码与链接规则，并将目标加入 `all`。重新 Clean Configure、Build 后，用 **F4**（需按第 6.3 节设置）或命令面板选择新程序，再按 F5；无需为每个新程序在模板里手写 `binaryPath`。

### 4.5 切换构建配置

换用 Clang 或另一构建类型时，先结束调试，再让构建、语言服务和启动程序对应到同一配置：

1. `Makefile: Set the current build configuration`：例如选择 `clang-debug`。
2. `Makefile: Set the target to be built by make`：保持或选择 `all`。
3. `Makefile: Clean Configure`：重新分析当前配置，导出数据库并识别该配置的可执行产物。
4. `clangd: Restart language server`：切换语言服务使用的数据库。
5. `Makefile: Build the current target`：生成新配置下的程序。
6. `Makefile: Set the make launch configuration`：选择路径属于新配置构建目录的程序，再按 F5。

插件可能保留之前选择过的启动条目，切换后要核对程序路径并重新选择，确保启动的是新配置的产物。Release 主要用于优化构建；调试时使用对应的 Debug 产物。

## 5. 语言服务与格式化

构建和调试已经使用当前配置，代码编辑也需要同一套编译信息。Configure 导出的数据库让 clangd 按实际标准、宏和头文件路径分析源码；项目的格式规则则统一代码布局。下面把这两部分接好，之后修改代码时就能同时获得补全、诊断和跳转支持。

### 5.1 让 clangd 读取当前配置

`compile_commands.json` 记录编译器、标准、宏和头文件路径。Makefile Tools 将它导出到 `build/<配置名>/`，clangd 通过下面的参数读取当前配置对应的目录：

```text
--compile-commands-dir=${workspaceFolder}/build/${command:makefile.getConfiguration}
```

数据库保留在构建目录即可。切换配置后，重新 Configure 并重启 clangd，使启动参数中的命令变量重新求值。新增源码、改变宏或包含目录时也重新分析；只修改现有函数实现，通常无需更新数据库。

Configure 使用的目标应覆盖需要分析的源码。示例保持为 `all`；已有大型工程按所需范围选择目标，并确认导出的数据库包含对应编译命令。

### 5.2 限定 `--query-driver`

`--query-driver` 允许 clangd 查询匹配的编译器，补充目标平台与标准库搜索路径。示例的驱动匹配范围为 `**/mingw64/bin/gcc.exe,**/mingw64/bin/g++.exe,**/mingw64/bin/clang.exe,**/mingw64/bin/clang++.exe`。

多个模式用逗号放在**同一个参数**中，只匹配自己使用的工具链。使用 UCRT64 时将目录模式改为 `ucrt64/bin`；自定义安装目录或其他布局按实际驱动路径修改。同时检查数据库记录的编译器与实际构建一致；路径或版本变化后重新 Configure 并重启 clangd。

### 5.3 使用项目格式规则

工作区设置将 clangd 设为 C++ 默认格式化器，并启用保存时格式化。项目根目录的 `.clang-format` 定义缩进与行宽，示例采用 LLVM 风格、4 空格缩进和 100 列行宽。

打开 `src/main.cpp`，可以用补全查看接口，用 F12 跳到声明或定义，并在“问题”面板查看诊断。需要手动格式化时执行 `Format Document`，确认当前文件使用 clangd 格式化器；已有工程沿用自身格式规则。

### 5.4 查看语言服务日志

如果程序能构建，编辑器却提示找不到头文件，先打开“输出”面板并选择 **clangd**。检查日志中的服务器版本、启动参数和编译数据库目录，再核对当前源码对应的编译命令是否包含正确的编译器、标准、宏和头文件路径。

切换构建配置后，日志中的 `--compile-commands-dir` 应指向新配置目录。仍指向旧目录时，确认新配置已完成 Configure，再执行 `clangd: Restart language server`。修正工具路径或 `--query-driver` 后，也按这个顺序重新检查。

## 6. 日常操作与快捷键

语言服务与构建配置对应后，日常开发通常只需保存代码并构建。修改影响编译参数、构建规则或运行目标时，再补上重新分析或选择程序的步骤；下面按常见修改说明该执行哪些操作。

### 6.1 保存、构建与调试

保存修改后，执行 `Makefile: Build the current target`；按第 6.3 节合入共同快捷键模板后，可直接按 F7。插件 Build 使用当前选中的构建目标；要改变这个范围，用 Shift+F7 选择并保存目标，再按 F7。Ctrl+Shift+B、F5 和 Ctrl+F5 则使用本指南的 `tasks.json` 任务，始终构建当前配置的 `all`，不会随着插件构建目标的选择改变。

只更换启动程序时，执行 `Makefile: Set the make launch configuration`（设置用户快捷键后为 F4）即可。随后 F5 使用所选 `launch.json` 条目调试，Ctrl+F5 使用该条目运行而不进入调试；两者都从项目根目录启动，参数读取 `args`。构建配置和启动选择保持有效时，无需每次重新选择程序。

希望直接使用插件操作时，在尚未进入调试的状态下，Shift+F5 对应 `Makefile: Debug the selected binary target`，Ctrl+Shift+F5 对应 `Makefile: Run the selected binary target in the terminal`。这两条命令采用插件启动条目的 `cwd` 和 `binaryArgs`；本示例的插件调试默认使用 GDB，F5 在“运行和调试”中选中的 CodeLLDB 条目不会改变它。使用 CodeLLDB 或统一项目根目录和参数时，继续采用 F5／Ctrl+F5 的 `launch.json` 流程。

### 6.2 增量构建与重新分析

Make 根据依赖关系和文件时间戳决定更新范围。示例已将 Makefile 纳入对象文件和程序的依赖，因此构建规则变化也会触发重建。

| 修改内容 | 示例中的构建行为 | 是否重新 Configure |
| --- | --- | --- |
| 某个 `.cpp` 的实现 | 重新编译该文件，重新链接依赖它的程序 | 编译选项未变时通常不需要 |
| 共享头文件 | 根据 `.d` 文件重新编译引用它的源码，再链接受影响目标 | 包含目录、宏等未变时通常不需要 |
| Makefile 的规则或编译选项 | Makefile 依赖使对象文件重新编译，再重新链接 | 需要，更新编译数据库并重启 clangd |
| 增加／删除源文件或可执行目标 | 按更新后的 Makefile 构建目标 | 需要；重新识别程序，Build 后重新选择，按需移除已失效的启动条目 |
| 切换 GCC / Clang 或 Debug / Release | 使用该配置的独立输出目录 | 需要；重启 clangd 并重新选择启动配置 |
| 没有变化 | 检查依赖后复用已有产物 | 不需要 |

这些行为取决于 Makefile 实际声明的依赖，已有工程以自身规则为准。通过命令行临时覆盖编译选项时，Make 一般不会把参数变化当作文件依赖。为常用组合建立独立配置，或清理受影响配置后重建，并让插件使用相同参数重新 Configure。

**`Makefile: Clean Configure` 清理插件分析状态并重新分析工程；工程的 `clean` 目标清理构建产物。** 配套 Makefile 的 `clean` 只删除当前 `CONFIG` 目录。需要清理时，在项目根目录执行：

```powershell
mingw32-make CONFIG=gcc-debug clean
```

该目录也包含编译数据库和插件输出。清理后重新 Configure、重启 clangd，再 Build，以恢复当前配置的开发环境。

### 6.3 自定义快捷键

在 CMake 与 Make 工程之间切换时，相近操作可以沿用同一组按键。执行 `Preferences: Open Keyboard Shortcuts (JSON)`，将[第 8.8 节](#88-用户快捷键模板)的完整模板合入用户数组；两套工具的命令由工作区条件决定，无需换工程时手动更换绑定。

| 快捷键 | CMake Tools | Makefile Tools |
| --- | --- | --- |
| F4 | 选择启动目标（`cmake.selectLaunchTarget`） | 选择启动程序（`makefile.setLaunchConfiguration`） |
| F6 | 删除 CMake 配置缓存并重新配置（`cmake.cleanConfigure`） | 清理插件分析状态并重新分析（`makefile.cleanConfigure`） |
| F7 | 构建当前范围（`cmake.build`） | 构建当前目标（`makefile.buildTarget`） |
| Shift+F7 | 选择目标并完成本次构建（`cmake.buildWithTarget`） | 切换当前构建目标（`makefile.setBuildTarget`），再按 F7 构建 |
| Shift+F5 | 插件直接调试启动目标（`cmake.debugTarget`） | 插件直接调试启动程序（`makefile.launchDebug`） |
| Ctrl+Shift+F5 | 插件直接运行启动目标（`cmake.launchTarget`） | 插件在终端运行启动程序（`makefile.launchRun`） |
| F5 / Ctrl+F5 | VS Code 启动所选 `launch.json` 配置，分别调试／不调试 | VS Code 启动所选 `launch.json` 配置，分别调试／不调试 |
| Alt+S | 重启 clangd（`clangd.restart`），两套工程共用 | 同左 |
| Ctrl+K，Ctrl+A | 打开用户设置 JSON（`workbench.action.openSettingsJson`） | 同左 |
| Ctrl+K，Ctrl+Z | 打开用户快捷键 JSON（`workbench.action.openGlobalKeybindingsFile`） | 同左 |

Shift+F7 的操作节奏有一处区别：CMake 选完目标立即完成本次构建，不改变长期默认的构建范围；Makefile Tools 只切换并保存当前构建目标，需要随后按 F7。例如在 Make 工程中选 `demo_calc` 后，后续 F7 继续构建该目标，需要构建全部程序时再选回 `all`。启动程序仍由 F4 单独选择。

F6 都用于重新配置，但处理的内容不同。CMake 删除配置缓存后按当前预设重新生成构建规则；Makefile Tools 清理插件的分析状态，再读取编译与链接命令，不删除对象文件或可执行程序。Make 工程要清理产物时，仍执行工程自己的 `clean` 目标。普通源码修改保存后按 F7 或 F5 增量构建即可。

Shift+F5 和 Ctrl+Shift+F5 采用插件自身的调试／运行配置；Make 示例的直接调试默认是 GDB。F5 与 Ctrl+F5 则使用“运行和调试”中选中的 `launch.json` 条目，GDB／CodeLLDB、项目根目录和 `args` 均按该条目执行。第 6.1 节说明了两条流程的使用方式。

模板让 Makefile Tools 绑定在 `makefile:fullFeatureSet && !cmake:enableFullFeatureSet` 时生效，CMake 绑定使用对应命令的原生条件。同一工作区同时启用两套功能时，这组按键优先执行 CMake 操作；需要操作 Makefile Tools 时从命令面板明确选择。CMake 的 F7 / Shift+F7 保留 `!cmake:hideBuildCommand`，直接调试／运行保留插件原有的项目和命令可见性条件。

两套插件的直接调试／运行绑定都限定为 `!inDebugMode`。调试已经开始时，Shift+F5 继续用于停止，Ctrl+Shift+F5 继续用于重启。F5、Ctrl+F5 沿用 VS Code 默认绑定，模板不会另行覆盖它们。

Alt+S 重启当前工作区的 clangd；切换构建配置或 Configure Preset 后，先完成分析／配置，再重启语言服务。两组 Ctrl+K 快捷键分两次按下：先按 Ctrl+K，再按 Ctrl+A 或 Ctrl+Z。快捷键保存在用户配置中，项目无需创建 `.vscode/keybindings.json`；合入模板时保留已有绑定，并在“键盘快捷方式”中检查相同按键和 `when` 条件的冲突。

## 7. 常见问题

出现问题时，先确认终端中的原始 Make 命令是否可用，再检查插件分析、数据库读取和程序选择。构建错误查看任务终端，分析问题查看“输出”面板中的 Makefile Tools 或 clangd 日志。

| 现象 | 优先检查与操作 |
| --- | --- |
| 插件未识别 Makefile | 核对打开的目录、`makefile.makeDirectory` 和 `makefile.makefilePath`；生成式工程先完成原有准备流程 |
| Make 或编译器找不到 | 核对实际路径和 VS Code 继承的 PATH；修改环境后完全重启 VS Code |
| Configure 成功但数据库缺失或不完整 | 选择覆盖所需源码的目标，确认完整编译命令可见，再 Clean Configure；核对导出路径及父目录 |
| 标准库头文件报错 | 核对数据库中的编译器路径、目标平台与 `--query-driver`；确认 clangd 读取当前配置目录 |
| 切换配置后语言服务仍使用旧参数 | 当前配置重新 Configure，再重启 clangd，使命令变量重新求值 |
| F5 构建与插件 Build 使用的参数不同 | 检查 `tasks.json` 的目录、入口、变量和目标；自定义任务不会自动继承 `makefile.configurations` 的全部参数 |
| 启动列表没有所需程序 | 选择覆盖该程序的目标（示例为 `all`），确认完整链接命令可见，再 Clean Configure 和 Build；仍无法识别时补充显式启动条目 |
| F5 启动了其他配置或其他程序 | 重新执行 `Makefile: Set the make launch configuration`，核对程序路径属于当前配置；移除换环境后已失效的自动生成条目 |
| 构建目标选 `all` 后尝试运行 `bin/all` | 检查是否误用 `${buildTarget}` 拼接程序路径；恢复 `${command:makefile.getLaunchTargetPath}` 并单独选择真实程序 |
| 终端 Run 与 F5 查找相对资源的位置不同 | F5 固定从项目根目录运行；插件 Run 使用启动条目的 `cwd`，自动发现时可能是 `bin/`，按项目需要调整 |
| 构建成功但断点不生效 | 使用 Debug；核对所选程序、源码位置、调试信息和调试器兼容性 |
| CodeLLDB 无法启动 | 确认平台组件完成安装且符合主机架构；检查所选条目为 `type: lldb`，并查看调试输出 |
| 快捷键没有响应或执行了另一工具的命令 | 检查用户绑定、命令 ID、`when` 条件及冲突；同开两套工具时模板优先 CMake，可从命令面板执行 Make 命令 |
| Shift+F7 选完目标却没有构建 | Makefile Tools 的该绑定只保存构建目标，随后按 F7；CMake 的对应命令才会立即完成本次构建 |
| Shift+F5 使用了 GDB，而 F5 使用 CodeLLDB | 两者使用不同入口：插件直接调试默认 GDB，F5 使用选中的 `launch.json` 条目；使用 CodeLLDB 时按 F5 |

## 8. 最终配置汇总

前文说明了文件的作用和操作顺序，本节集中列出需要完成的配置及完整文件，便于创建项目时逐项查阅。已有工程按第 3.7 节合入需要的部分；文件维护与打包方式见第 3.8 节。

| 配置位置 | 需要完成的配置 |
| --- | --- |
| 系统环境 | 安装 `mingw32-make`、所用编译器、clangd 和调试器，并配置项目使用的 PATH 与依赖 |
| VS Code 扩展 | 安装 Makefile Tools、clangd，以及调试器对应的 C/C++ 或 CodeLLDB |
| VS Code 用户设置 | 确保扩展能找到 clangd；需要时配置 `clangd.path`，个人绝对路径保存在用户侧 |
| 项目源码 | 为两个程序保留独立入口，共享头文件和实现按 Makefile 声明参与构建 |
| 项目根目录 | 创建或合入 `Makefile`、`.clang-format` 与 `.gitignore` |
| 项目 `.vscode/` | 创建或合入 `settings.json`、`tasks.json`、`launch.json` 与 `extensions.json` |
| 用户快捷键（可选） | 按第 8.8 节合入 CMake／Make 共用模板，目标选择、配置、构建与直接调试／运行沿用同一组按键；用途见第 6.3 节 |

下列文件与配套 `example-windows/` 工程对应。工具名、路径、目标和编译参数按项目需要调整；两种调试条目可按使用情况保留。

| 文件 | 保存位置 | 配套文件 |
| --- | --- | --- |
| `main.cpp`、`calc.cpp`、`math.cpp` | `src/` | [main.cpp](example-windows/src/main.cpp)、[calc.cpp](example-windows/src/calc.cpp)、[math.cpp](example-windows/src/math.cpp) |
| `math.hpp` | `include/demo/` | [math.hpp](example-windows/include/demo/math.hpp) |
| `Makefile` | 项目根目录 | [Makefile](example-windows/Makefile) |
| `settings.json` | `.vscode/` | [settings.json](example-windows/.vscode/settings.json) |
| `tasks.json` | `.vscode/` | [tasks.json](example-windows/.vscode/tasks.json) |
| `launch.json` | `.vscode/` | [launch.json](example-windows/.vscode/launch.json) |
| `extensions.json` | `.vscode/` | [extensions.json](example-windows/.vscode/extensions.json) |
| `.clang-format`、`.gitignore` | 项目根目录 | [.clang-format](example-windows/.clang-format)、[.gitignore](example-windows/.gitignore) |
| 用户快捷键 | VS Code 用户 `keybindings.json` | [合入模板](keybindings.json) |

### 8.1 示例源文件

`src/main.cpp`：

```cpp
#include "demo/math.hpp"
#include <iostream>

int main() {
    const int a = 2;
    const int b = 3;
    std::cout << "example_app: " << a << " + " << b << " = "
              << demo::add(a, b) << '\n';
    std::cout << "Debug build: " << DEMO_BUILD_DEBUG << '\n';
    return 0;
}
```

`src/calc.cpp`：

```cpp
#include "demo/math.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const int a = argc > 1 ? std::stoi(argv[1]) : 12;
    const int b = argc > 2 ? std::stoi(argv[2]) : 30;
    std::cout << "demo_calc: " << a << " + " << b << " = "
              << demo::add(a, b) << '\n';
    return 0;
}
```

`src/math.cpp`：

```cpp
#include "demo/math.hpp"

int demo::add(int a, int b) {
    const int result = a + b;
    return result;
}
```

`include/demo/math.hpp`：

```cpp
#pragma once

namespace demo {
int add(int a, int b);
}
```

### 8.2 `Makefile`

```makefile
# Each configuration has its own outputs and dependency files.
CONFIG ?= gcc-debug
VALID_CONFIGS := gcc-debug gcc-release clang-debug clang-release
ifneq ($(words $(CONFIG)),1)
$(error CONFIG must be a single configuration name)
endif
ifeq ($(filter $(CONFIG),$(VALID_CONFIGS)),)
$(error Unsupported CONFIG "$(CONFIG)". Choose one of: $(VALID_CONFIGS))
endif

TOOLCHAIN := $(word 1,$(subst -, ,$(CONFIG)))
MODE := $(word 2,$(subst -, ,$(CONFIG)))
ifeq ($(TOOLCHAIN),clang)
CXX_NAME := clang++
else
CXX_NAME := g++
endif

override BUILD_DIR := build/$(CONFIG)
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
.SHELLFLAGS := /D /C
EXE_EXT := .exe
CXX := $(subst \,/,$(shell powershell.exe -NoProfile -Command "(Get-Command $(CXX_NAME) -CommandType Application | Select-Object -First 1).Source"))
else
SHELL := /bin/sh
EXE_EXT :=
CXX := $(shell command -v $(CXX_NAME))
endif

ifeq ($(strip $(CXX)),)
$(error Compiler $(CXX_NAME) was not found on PATH)
endif

CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++20 -Wall -Wextra -Wpedantic
ifeq ($(MODE),debug)
CPPFLAGS += -DDEMO_BUILD_DEBUG=1
CXXFLAGS += -O0 -g
else
CPPFLAGS += -DNDEBUG -DDEMO_BUILD_DEBUG=0
CXXFLAGS += -O2
endif

APP := $(BIN_DIR)/example_app$(EXE_EXT)
CALC := $(BIN_DIR)/demo_calc$(EXE_EXT)
OBJECTS := $(OBJ_DIR)/main.o $(OBJ_DIR)/calc.o $(OBJ_DIR)/math.o
DEPS := $(OBJECTS:.o=.d)

.DEFAULT_GOAL := all
.PHONY: all example_app demo_calc clean
all: example_app demo_calc
example_app: $(APP)
demo_calc: $(CALC)

$(APP): $(OBJ_DIR)/main.o $(OBJ_DIR)/math.o Makefile | $(BIN_DIR)
	"$(CXX)" $(LDFLAGS) $(filter %.o,$^) $(LDLIBS) -o "$@"

$(CALC): $(OBJ_DIR)/calc.o $(OBJ_DIR)/math.o Makefile | $(BIN_DIR)
	"$(CXX)" $(LDFLAGS) $(filter %.o,$^) $(LDLIBS) -o "$@"

$(OBJ_DIR)/%.o: src/%.cpp Makefile | $(OBJ_DIR)
	"$(CXX)" $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

$(OBJ_DIR) $(BIN_DIR):
ifeq ($(OS),Windows_NT)
	if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
else
	mkdir -p "$@"
endif

# Clean only the active, validated configuration.
clean:
ifeq ($(OS),Windows_NT)
	if exist "$(subst /,\,$(BUILD_DIR))" rmdir /S /Q "$(subst /,\,$(BUILD_DIR))"
else
	rm -rf "$(BUILD_DIR)"
endif

-include $(DEPS)
```

### 8.3 `.vscode/settings.json`

```json
{
  "makefile.makePath": "mingw32-make",
  "makefile.makeDirectory": "${workspaceFolder}",
  "makefile.makefilePath": "${workspaceFolder}/Makefile",
  "makefile.configurations": [
    {
      "name": "gcc-debug",
      "makeArgs": [
        "CONFIG=gcc-debug"
      ]
    },
    {
      "name": "gcc-release",
      "makeArgs": [
        "CONFIG=gcc-release"
      ]
    },
    {
      "name": "clang-debug",
      "makeArgs": [
        "CONFIG=clang-debug"
      ]
    },
    {
      "name": "clang-release",
      "makeArgs": [
        "CONFIG=clang-release"
      ]
    }
  ],
  "makefile.configureOnOpen": false,
  "makefile.configureOnEdit": false,
  "makefile.configureAfterCommand": false,
  "makefile.buildBeforeLaunch": true,
  "makefile.extensionOutputFolder": "${workspaceFolder}/build/${configuration}/.makefile-tools",
  "makefile.compileCommandsPath": "${workspaceFolder}/build/${configuration}/compile_commands.json",
  "C_Cpp.intelliSenseEngine": "disabled",
  "clangd.arguments": [
    "--background-index",
    "--compile-commands-dir=${workspaceFolder}/build/${command:makefile.getConfiguration}",
    "--query-driver=**/mingw64/bin/gcc.exe,**/mingw64/bin/g++.exe,**/mingw64/bin/clang.exe,**/mingw64/bin/clang++.exe"
  ],
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
    "editor.formatOnSave": true
  },
  "makefile.defaultLaunchConfiguration": {
    "MIMode": "gdb",
    "miDebuggerPath": "gdb",
    "stopAtEntry": false
  }
}
```

### 8.4 `.vscode/tasks.json`

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Makefile: build active configuration",
      "type": "process",
      "command": "${config:makefile.makePath}",
      "args": [
        "-f",
        "Makefile",
        "CONFIG=${command:makefile.getConfiguration}",
        "all",
        "-j"
      ],
      "options": {
        "cwd": "${workspaceFolder}"
      },
      "problemMatcher": [
        "$gcc"
      ],
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
```

### 8.5 `.vscode/launch.json`

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Makefile / GDB",
      "type": "cppdbg",
      "request": "launch",
      "program": "${command:makefile.getLaunchTargetPath}",
      "cwd": "${workspaceFolder}",
      "args": [],
      "preLaunchTask": "Makefile: build active configuration",
      "stopAtEntry": false,
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "gdb",
      "setupCommands": [
        {
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    },
    {
      "name": "Makefile / LLDB (CodeLLDB)",
      "type": "lldb",
      "request": "launch",
      "program": "${command:makefile.getLaunchTargetPath}",
      "cwd": "${workspaceFolder}",
      "args": [],
      "preLaunchTask": "Makefile: build active configuration",
      "stopOnEntry": false,
      "terminal": "integrated"
    }
  ]
}
```

### 8.6 `.vscode/extensions.json`

```json
{
  "recommendations": [
    "ms-vscode.makefile-tools",
    "llvm-vs-code-extensions.vscode-clangd",
    "ms-vscode.cpptools",
    "vadimcn.vscode-lldb"
  ]
}
```

### 8.7 格式化与忽略文件

`.clang-format`：

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
```

`.gitignore`：

```gitignore
/build/
/.cache/
*.local.json
```

### 8.8 用户快捷键模板

以下完整模板同时适用于 CMake Tools 和 Makefile Tools，与 CMake 指南使用相同内容。将条目合入用户 `keybindings.json` 的数组，保留已有绑定；操作对应关系、两种 Shift+F7 行为及生效条件见第 6.3 节。

```json
[
  {
    "key": "f4",
    "command": "cmake.selectLaunchTarget",
    "when": "cmake:enableFullFeatureSet"
  },
  {
    "key": "f4",
    "command": "makefile.setLaunchConfiguration",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet"
  },
  {
    "key": "f6",
    "command": "cmake.cleanConfigure",
    "when": "cmake:enableFullFeatureSet"
  },
  {
    "key": "f6",
    "command": "makefile.cleanConfigure",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet"
  },
  {
    "key": "f7",
    "when": "!cmake:hideBuildCommand && cmake:enableFullFeatureSet",
    "command": "cmake.build"
  },
  {
    "key": "f7",
    "command": "makefile.buildTarget",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet"
  },
  {
    "key": "shift+f7",
    "when": "!cmake:hideBuildCommand && cmake:enableFullFeatureSet",
    "command": "cmake.buildWithTarget"
  },
  {
    "key": "shift+f7",
    "command": "makefile.setBuildTarget",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet"
  },
  {
    "key": "shift+f5",
    "command": "cmake.debugTarget",
    "when": "!inDebugMode && inCMakeProject && !cmake:hideDebugCommand && cmake:enableFullFeatureSet"
  },
  {
    "key": "shift+f5",
    "command": "makefile.launchDebug",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet && !inDebugMode"
  },
  {
    "key": "shift+ctrl+f5",
    "command": "cmake.launchTarget",
    "when": "!inDebugMode && !cmake:hideDebugCommand && cmake:enableFullFeatureSet"
  },
  {
    "key": "ctrl+shift+f5",
    "command": "makefile.launchRun",
    "when": "makefile:fullFeatureSet && !cmake:enableFullFeatureSet && !inDebugMode"
  },
  {
    "key": "alt+s",
    "command": "clangd.restart"
  },
  {
    "key": "ctrl+k ctrl+a",
    "command": "workbench.action.openSettingsJson"
  },
  {
    "key": "ctrl+k ctrl+z",
    "command": "workbench.action.openGlobalKeybindingsFile"
  }
]
```

F4 选择启动程序，F6 重新配置／分析，F7 构建；Shift+F7 在 Make 工程中只选择构建目标，随后再按 F7。Shift+F5 和 Ctrl+Shift+F5 在尚未调试时调用插件直接调试／运行；F5、Ctrl+F5 保留 `launch.json` 流程。两套工具同时启用时，模板优先 CMake。

## 参考资料

- [Makefile Tools](https://github.com/microsoft/vscode-makefile-tools)：扩展说明、配置和故障排查。
- [GNU Make 手册](https://www.gnu.org/software/make/manual/make.html)：目标、变量、依赖与增量构建。
- [clangd 编译命令](https://clangd.llvm.org/design/compile-commands)：编译数据库与编译器查询。
- [VS Code 调试配置](https://code.visualstudio.com/docs/debugtest/debugging-configuration)：启动配置与构建任务。
- [CodeLLDB 手册](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md)：LLDB 调试配置和后端。
