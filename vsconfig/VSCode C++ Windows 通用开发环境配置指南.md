# VS Code C++ Windows 开发环境配置指南

## 使用说明

### 适用范围与前置条件

本指南适用于 Windows 原生 GNU/MinGW 工具链（MSYS2 MINGW64 或 UCRT64），使用 **CMake Presets** 管理 GCC/Clang 与 Ninja/MinGW Makefiles 的构建组合，由 clangd 提供语言服务，GDB 或 CodeLLDB 提供调试支持。示例工程采用 C++20，并通过两个可执行目标演示程序选择与调试。

| 项目 | 要求或选择 |
| --- | --- |
| 操作系统与权限 | 64 位 Windows 满足 MSYS2 与 VS Code 的系统要求；具备安装软件和修改用户 PATH 的权限 |
| CMake | ≥ 3.21，支持本文采用的 Preset 格式版本 3 和配套生成脚本；`--fresh` 另要求 ≥ 3.24 |
| 编译器 | 选择项目支持的 GCC 或 Clang，支持项目要求的 C++ 标准 |
| 构建工具 | Ninja 预设需要 `ninja`；MinGW Makefiles 预设需要 `mingw32-make` |
| 调试器 | 使用 GDB 时安装 C/C++ 扩展；使用 LLDB 时安装 CodeLLDB，独立的命令行 LLDB 按需安装 |
| clangd 扩展 | ≥ 0.3.0，以支持 `${command:...}` 命令变量；扩展版本与 clangd 程序版本分别管理 |
| 工具链兼容性 | GCC/Clang、调试器和二进制依赖库使用兼容的 MSYS2 环境 |

先按项目要求选定编译器、构建工具和调试器，再准备对应环境。下文的安装命令覆盖两种编译器和构建后端，可按需取用；已有项目的工具版本和依赖安装方式以其构建说明为准。

### 首次配置路径

首次配置可先采用 **GCC + Ninja + Debug + GDB**，使用 `gcc-ninja-debug` 完成一次构建和调试。本文沿着工具安装、项目配置、构建与调试的顺序展开，后续再按需要切换 Clang、Make 或 CodeLLDB。

- **新建示例工程：** 顺序执行第 1～4 节，所需完整文件从[第 8 节](#8-最终配置汇总)复制或按第 3.3.1 节生成。
- **接入已有 CMake 工程：** 先阅读项目自己的构建说明，按第 1～2 节准备环境，再按[第 3.7 节](#37-接入已有-cmake-工程)合入所需配置。

完成首次调试后，日常修改和预设切换可查阅第 6 节。自定义快捷键属于可选设置，使用命令面板也能完成全部操作。

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

- 项目命令默认从项目的 CMake 根目录运行；另有执行位置时，在步骤中标明。
- `bash` 安装命令在所选 MSYS2 终端执行；`powershell` 项目命令在 PowerShell 执行。MinGW Makefiles 的配置、构建使用 Windows 命令环境。
- `<项目根目录>`、`<指南目录>`、`<主机地址>` 等均为占位符，使用前连同尖括号替换为实际值。示例不绑定个人安装位置或主机信息。
- Configure 表示配置，Build 表示构建；Configure/Build Preset 表示配置/构建预设；Launch/Debug Target 表示启动/调试目标。命令与配置项保留 VS Code 原始名称，便于搜索。

## 1. 安装工具链

VS Code 通过扩展调用编译器、构建工具和调试器，因此先准备这些程序，并确认它们能在项目使用的命令环境中运行。

### 1.1 选择工具链环境

从 [MSYS2 官网](https://www.msys2.org/) 获取并安装 MSYS2。根据项目要求选择一种原生 Windows 工具链环境，编译器、调试器和依赖库使用对应环境的软件包：

| 环境 | Windows 工具目录 | 软件包前缀 |
| --- | --- | --- |
| MINGW64 | `<MSYS2安装目录>\mingw64\bin` | `mingw-w64-x86_64-` |
| UCRT64 | `<MSYS2安装目录>\ucrt64\bin` | `mingw-w64-ucrt-x86_64-` |

下文以 MINGW64 为例；选择 UCRT64 时，使用 UCRT64 终端和对应的软件包前缀。编译器、调试器及二进制依赖库应来自同一兼容环境，避免混用两套运行时。MSYS 基础环境中的 `usr\bin\gcc.exe` 不属于这里使用的原生 Windows 工具链。

打开 MSYS2 终端并更新软件包：

```bash
pacman -Syu
```

若更新过程提示关闭终端，应按提示关闭、重新打开后再次执行更新命令，直至更新完成。随后打开所选的 **MSYS2 MINGW64** 或 **MSYS2 UCRT64** 终端，安装工具：

```bash
# MINGW64 使用这一行；UCRT64 改为 mingw-w64-ucrt-x86_64-
PACKAGE_PREFIX=mingw-w64-x86_64-

pacman -S --needed \
  "${PACKAGE_PREFIX}gcc" \
  "${PACKAGE_PREFIX}cmake" \
  "${PACKAGE_PREFIX}ninja" \
  "${PACKAGE_PREFIX}make" \
  "${PACKAGE_PREFIX}gdb" \
  "${PACKAGE_PREFIX}clang"
```

`gcc` 软件包提供 `gcc` 和 `g++`，分别用于 C、C++ 编译；`clang` 软件包提供 `clang`、`clang++` 和 `clang-format`；所选环境的 `make` 软件包提供 `mingw32-make`。第三方开发库应按项目依赖要求另行安装。

### 1.2 配置 Windows PATH

工具安装完成后，还需要让从 Windows 启动的 VS Code 找到它们。在用户环境变量 `Path` 中添加所选工具链的 `bin` 目录，例如 `<MSYS2安装目录>\mingw64\bin`；保存后完全退出并重新打开 VS Code 和终端，使新环境生效。

在新打开的 PowerShell 中检查工具路径和版本：

```powershell
where.exe gcc
where.exe g++
where.exe clang
where.exe clang++
where.exe cmake
where.exe ninja
where.exe mingw32-make
where.exe gdb
where.exe clang-format

gcc --version
g++ --version
clang --version
clang++ --version
cmake --version
ninja --version
mingw32-make --version
gdb --version
clang-format --version
```

确认 `where.exe` 返回的首个程序路径符合所选环境。GCC、Clang、GDB 和依赖库应来自兼容环境，建议 CMake、Ninja、`mingw32-make` 也使用该环境中的版本。若同名工具存在多个候选路径，应检查 PATH 顺序，确保默认使用预期程序。

Clang 应使用所选 MSYS2 环境中的 MinGW 版本。其 `--version` 输出中的目标应属于 Windows GNU/MinGW，例如 `x86_64-w64-windows-gnu`。面向 MSVC 的独立 LLVM 安装和 `clang-cl` 使用不同的运行时与工具链约定，不直接套用本文的预设。

### 1.3 选择构建生成器

生成器决定 CMake 使用哪种构建后端，后面选择 Preset 时也会选定相应的生成器。可按项目约定在以下两种方式中选择。

| CMake 生成器 | 构建工具 | 使用方式 |
| --- | --- | --- |
| `Ninja` | `ninja` | 使用名称中带 `ninja` 的预设 |
| `MinGW Makefiles` | `mingw32-make` | 使用名称中带 `make` 的预设 |

`MinGW Makefiles` 生成面向 Windows 命令环境的 Makefile，应从 PowerShell 或命令提示符执行配置和构建。它与 `MSYS Makefiles` 属于不同生成器；本文不使用 MSYS/Bash 的 Makefile 方案。

两种生成器均支持增量构建并生成 `compile_commands.json`。使用 `cmake --build --preset <预设名称>` 可统一调用相应后端，不必直接管理 `ninja` 或 `mingw32-make` 的参数。不同生成器必须使用独立构建目录。

### 1.4 LLDB 后端与命令行工具（可选）

VS Code 的 LLDB 调试使用 **CodeLLDB** 扩展。该扩展提供调试适配器，并默认使用随扩展安装的平台 LLDB 后端；完成扩展及平台组件安装后，通常无需另外安装系统 LLDB，也无需将 `lldb` 加入 PATH。

需要在终端独立使用 LLDB 时，可另行安装命令行工具：

```bash
# MINGW64 示例；UCRT64 改为 mingw-w64-ucrt-x86_64-
PACKAGE_PREFIX=mingw-w64-x86_64-
pacman -S --needed "${PACKAGE_PREFIX}lldb"
```

安装后可用 `lldb --version` 查看命令行版本。CodeLLDB 的后端由扩展单独管理，可能与终端中的 LLDB 不同；编辑器后端的选择方式在下一节说明。

## 2. 配置 VS Code 扩展

工具链准备好后，将它接入编辑器。从 [VS Code 官网](https://code.visualstudio.com/) 安装 VS Code，在扩展视图中安装 CMake Tools 和 clangd，再根据所用调试器选择 C/C++ 或 CodeLLDB。各扩展承担的功能如下。

| 扩展 | 扩展标识 | 用途 | 安装条件 |
| --- | --- | --- | --- |
| CMake Tools | `ms-vscode.cmake-tools` | Preset、配置、构建与目标选择 | 使用本文的 CMake 集成 |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | C++ 语言服务与格式化 | 使用本文的语言服务 |
| C/C++ | `ms-vscode.cpptools` | `cppdbg` 调试器接口 | 使用 GDB |
| CodeLLDB | `vadimcn.vscode-lldb` | `lldb` 调试器接口与 LLDB 后端 | 使用 LLDB |

也可在 PowerShell 中执行以下安装命令；仅使用一种调试器时，取用对应的调试扩展条目：

```powershell
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.cpptools
code --install-extension vadimcn.vscode-lldb
code --install-extension llvm-vs-code-extensions.vscode-clangd
```

clangd 扩展需要可执行的 `clangd.exe`。可通过扩展提供的安装功能获取该程序，或在 VS Code **用户设置**中通过 `clangd.path` 指定已有程序。与安装位置有关的绝对路径应保存在用户设置中。

选择 CodeLLDB 时，等待扩展的平台组件安装完成。后面的调试配置使用 `"type": "lldb"`，与该扩展对应；使用 `"type": "lldb-dap"` 的其他 LLDB 扩展需要自己的配置格式。

通常保留 CodeLLDB 的默认后端。需要替换后端时，按 [CodeLLDB 使用手册](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md#using-alternate-lldb-backends) 在用户设置中配置 `lldb.library`，填写兼容的 LLDB 动态库路径。Windows 填写 DLL 路径，不填写 `lldb.exe` 路径。

这套配置由 clangd 负责 C++ 语言分析，工作区设置会关闭 C/C++ IntelliSense，同时保留 C/C++ 扩展的 GDB 调试能力。两种调试扩展可以同时安装，使用时选择相应条目即可；其他语言服务器按工作区需要启用，避免重复分析同一文件。

## 3. 项目结构与配置说明

项目配置将前面安装的工具组织起来：CMake 文件定义目标和构建选项，`.vscode` 文件让编辑器读取这些配置并启动调试。本章说明各文件的作用，完整内容集中在[第 8 节](#8-最终配置汇总)，便于创建文件时查阅。

新建示例工程时，创建下面的 `cpp-demo` 目录并用 VS Code 打开它。已有工程保留自己的源码、目标和格式规则，按第 3.7 节接入；下面的两个入口源码和 `CMakeLists.txt` 用于演示配置。

```text
cpp-demo/
├── .vscode/
│   ├── settings.json
│   └── launch.json
├── src/
│   ├── main.cpp
│   └── demo_calc.cpp
├── .clang-format
├── .gitignore
├── CMakeLists.txt
└── CMakePresets.json
```

完成这些文件后，Configure 会在所选预设的构建目录中生成构建规则和 `compile_commands.json`。clangd 通过 CMake Tools 获取该目录，直接读取 CMake 生成的编译命令。

### 3.1 示例源文件

示例用两个独立的 `main()` 入口演示多程序工程，完整源码见[第 8.1 节](#81-示例源文件)。各源文件对应的目标和行为如下。

| 源文件 | 可执行目标 | 示例行为 |
| --- | --- | --- |
| `src/main.cpp` | `example_app` | 输出固定结果，调试时可观察 `answer` |
| `src/demo_calc.cpp` | `demo_calc` | 调用乘法函数，调试时可观察参数和返回值 |

两个程序各自构建和运行，调试时选择与源码对应的启动目标。下面先在 CMake 中建立这种对应关系。

### 3.2 `CMakeLists.txt`

在项目根目录创建 `CMakeLists.txt`，内容见[第 8.2 节](#82-cmakeliststxt)。其中 `LANGUAGES C CXX` 启用 C 和 C++ 编译器，两次 `add_executable()` 将各入口源码分别加入 `example_app` 和 `demo_calc`，并为两个目标设置 C++20、关闭编译器扩展。

每个可执行目标保留自己的 `main()`，避免将多个程序入口加入同一目标。项目新增头文件目录或依赖库时，通过 `target_include_directories()`、`find_package()` 和 `target_link_libraries()` 声明，使构建与语言服务读取同一份目标配置。

### 3.3 `CMakePresets.json`

目标定义完成后，用 Presets 保存编译器、生成器和构建类型，后续配置时直接选择所需组合。新建项目可按第 3.3.1 节生成 `CMakePresets.json`，也可复制[第 8.3 节](#83-cmakepresetsjson)的完整模板；两者结构相同，首次调试选用 `gcc-ninja-debug` 即可。

预设名称采用 `<编译器>-<生成器>-<构建类型>`，例如 `gcc-ninja-debug`、`clang-make-release`；Configure Preset 与对应 Build Preset 使用同名。

`CMAKE_C_COMPILER`、`CMAKE_CXX_COMPILER` 分别指定 C、C++ 编译器，并通过 PATH 查找对应程序。八套预设的关系如下：

| Configure/Build Preset | C/C++ 编译器 | 生成器 | 构建类型 | 构建目录 |
| --- | --- | --- | --- | --- |
| `gcc-ninja-debug` | `gcc` / `g++` | Ninja | Debug | `build/gcc-ninja-debug` |
| `gcc-ninja-release` | `gcc` / `g++` | Ninja | Release | `build/gcc-ninja-release` |
| `gcc-make-debug` | `gcc` / `g++` | MinGW Makefiles | Debug | `build/gcc-make-debug` |
| `gcc-make-release` | `gcc` / `g++` | MinGW Makefiles | Release | `build/gcc-make-release` |
| `clang-ninja-debug` | `clang` / `clang++` | Ninja | Debug | `build/clang-ninja-debug` |
| `clang-ninja-release` | `clang` / `clang++` | Ninja | Release | `build/clang-ninja-release` |
| `clang-make-debug` | `clang` / `clang++` | MinGW Makefiles | Debug | `build/clang-make-debug` |
| `clang-make-release` | `clang` / `clang++` | MinGW Makefiles | Release | `build/clang-make-release` |

Release 预设继承同一编译器、生成器的 Debug 预设，并覆盖构建类型；`${presetName}` 根据所选预设展开，因此构建目录保持独立。与安装位置有关的配置可放入 `CMakeUserPresets.json` 并排除在版本控制之外。切换编译器或生成器时选择对应预设，不复用其他组合的缓存；更换 MINGW64/UCRT64 环境时也应使用新的构建目录。

#### 3.3.1 从配套工具生成预设

生成脚本使用 CMake 自身运行，要求 CMake ≥ 3.21。配套工具统一保存在**指南目录**的 `tools` 子目录中，两个文件保持相邻：

```text
tools/
├── generate-cmake-presets.cmake
└── CMakePresets.template.json
```

在终端执行以下命令，将输出位置指定为项目的 CMake 根目录：

```powershell
cmake -DOUTPUT_DIR="<项目根目录>" -P "<指南目录>/tools/generate-cmake-presets.cmake"
```

运行后，项目根目录会得到包含上述八套预设的 `CMakePresets.json`，并启用编译数据库。脚本按平台填写 Make 生成器：Windows 使用 `MinGW Makefiles`，Linux 使用 `Unix Makefiles`。生成工具继续随指南维护，日常 Configure 和 Build 直接读取项目中的结果。

| 参数或使用方式 | 行为 |
| --- | --- |
| `-DOUTPUT_DIR="<项目根目录>"` | 指定输出目录；相对路径从当前终端目录解析 |
| 省略 `OUTPUT_DIR` | 写入当前终端目录，因此应先切换到项目的 CMake 根目录 |
| `-DPLATFORM=windows` 或 `-DPLATFORM=linux` | 显式选择生成平台；省略时按运行脚本的主机系统选择 |
| `-DOVERWRITE=ON` | 备份已有文件后重新生成；默认保留已有文件并停止 |

所有 `-D` 参数写在 `-P` 前。需要从共享模板重新初始化时执行：

```powershell
cmake -DOUTPUT_DIR="<项目根目录>" -DOVERWRITE=ON -P "<指南目录>/tools/generate-cmake-presets.cmake"
```

重新生成前，旧文件会备份为 `CMakePresets.json.bak-<时间戳>`，随后恢复共享模板的默认内容。已经加入项目选项的预设适合直接编辑；已有工程则优先复用其现有预设，生成脚本用于初始化缺少预设的项目。

Remote-SSH 开发时，将配套工具放在远程用户可访问的位置，从远程终端调用并填写远程路径。生成预设只建立配置，使用其中某套组合前，仍需准备对应的编译器和构建工具。

#### 3.3.2 生成后按需求调整

模板提供常见组合，项目所需的功能开关、依赖位置和构建范围再根据原有构建说明补充。常见调整及其位置如下。

| 需要调整 | 修改位置与要点 |
| --- | --- |
| 只使用部分编译器、生成器或构建类型 | 删除不需要的 Configure Preset 及对应 Build Preset；保留被其他预设继承的预设 |
| 使用指定编译器或工具链 | 调整 `CMAKE_C_COMPILER`、`CMAKE_CXX_COMPILER` 或配置 `toolchainFile`；个人绝对路径放入 `CMakeUserPresets.json` |
| 增加构建类型 | 复制相近预设，设置唯一 `name` 和 `CMAKE_BUILD_TYPE`，并添加引用它的 Build Preset |
| 调整构建目录 | 修改 `binaryDir`；编译器、生成器和构建类型的组合保持独立目录 |
| 增加项目选项或依赖位置 | 在 `cacheVariables` 中加入项目约定的 CMake 变量；目标的源码、宏、头文件目录和链接依赖在 `CMakeLists.txt` 中维护 |
| 限定普通 Build 的目标 | 在相应 Build Preset 的 `targets` 中填写需要的目标；启动目标仍用 `CMake: Set Launch/Debug Target` 选择 |
| 统一后续新项目的初始配置 | 修改 `tools/CMakePresets.template.json`；保留 `@MAKE_GENERATOR@` 以按系统填写 Make 生成器 |

修改后重新选择所需 Configure Preset、执行 Configure、重启 clangd，并选择对应 Build Preset。`CMAKE_EXPORT_COMPILE_COMMANDS` 保持开启，语言服务才能读取生成的编译数据库。C++ 标准按项目目标在 `CMakeLists.txt` 中设置，不由生成脚本决定。

### 3.4 `.vscode/settings.json`

Presets 保存了构建配置，工作区设置再将它连接到 CMake Tools 和 clangd。在 `.vscode/settings.json` 中合入[第 8.4 节](#84-vscodesettingsjson)的内容，保留项目已有设置。下面说明这些配置如何配合。

| 配置项 | 设置或操作 | 用途 |
| --- | --- | --- |
| `cmake.useCMakePresets` | `always` | 以项目 Preset 作为配置与构建入口 |
| CMake 自动配置设置 | 关闭常规打开、编辑时的自动配置 | 按第 4 节主动 Configure，明确配置变化何时生效 |
| `cmake.buildBeforeRun` | `true` | 启动目标前执行构建，保存后的修改参与增量更新 |
| `C_Cpp.intelliSenseEngine` | `disabled` | 使用 clangd 提供语言服务，保留 C/C++ 的 GDB 调试能力 |
| `--compile-commands-dir=${command:cmake.buildDirectory}` | 读取所选 Configure Preset 的构建目录 | 编译数据库直接跟随预设，不复制到项目根目录 |
| `editor.defaultFormatter`、`editor.formatOnSave` | C/C++ 默认格式化器设为 clangd，并启用保存时格式化 | 使用项目 `.clang-format` 统一格式 |

`compile_commands.json` 记录源文件的编译器、标准、宏和头文件路径。成功 Configure 即可生成数据库，语言服务无需等待 Build 完成。

由于构建目录通过命令变量获取，clangd 会在启动时解析它。切换 Configure Preset 后，先执行 `CMake: Configure`，再执行 `clangd: Restart language server`，语言服务便会读取新目录中的数据库。

项目的头文件选项来自编译数据库，工具链的标准库搜索路径则可由 `--query-driver` 补充：它允许 clangd 查询匹配的 GCC/Clang 驱动。Windows 模板列出 MSYS2 MINGW64、UCRT64 的常见路径；使用时保留所选工具链的模式。使用自定义目录或版本化程序名时改为实际编译器路径；多个模式以逗号放在同一个参数中。clangd 程序与项目编译器分别选择。

### 3.5 `.vscode/launch.json`

语言服务接入后，在 `.vscode/launch.json` 中指定调试适配器和程序启动方式。[第 8.5 节](#85-vscodelaunchjson)提供两套可复用的条目，按需要合入已有文件；它们分别对应 GDB 和 CodeLLDB。

| 调试配置 | 调试扩展与类型 | 后端位置 | 暂停与终端配置 |
| --- | --- | --- | --- |
| `CMake / GDB` | C/C++，`cppdbg` | `miDebuggerPath` 通过 VS Code 继承的 PATH 查找 GDB | `stopAtEntry`、`externalConsole` |
| `CMake / LLDB (CodeLLDB)` | CodeLLDB，`lldb` | 默认使用 CodeLLDB 自带 LLDB 后端 | `stopOnEntry`、`terminal` |

GDB 使用 `MIMode`、`miDebuggerPath` 和 `setupCommands`；CodeLLDB 使用自己的配置格式，不添加这些 GDB 字段。LLDB 示例通过 `"terminal": "integrated"` 将程序输入输出连接到 VS Code 集成终端。无需标准输入时，可改为 `"console"` 将输出放入调试控制台。

调试器必须支持目标架构和编译器生成的调试信息。Debug 预设生成调试信息；调试器的选择不要求改变编译器、生成器或构建目录。GCC 和 Clang 均可生成供 GDB、LLDB 使用的调试信息，具体支持程度取决于所用版本及格式。Windows 的 MinGW 目标使用 DWARF 调试信息，与 MSVC/PDB 工具链采用不同约定。

两套配置的 `program` 均使用 `${command:cmake.launchTargetPath}`，读取 CMake Tools 所选启动目标的可执行文件路径。通过 `CMake: Set Launch/Debug Target` 更换目标后，两套配置都可复用，无需改写程序路径。启用 `cmake.buildBeforeRun` 后，CMake Tools 执行运行前构建，成功后启动调试。目标选择与构建范围见第 4.4 节。

运行前构建按需更新产物。Ninja 或 `mingw32-make` 根据 CMake 生成的规则、文件更新时间和依赖关系判断哪些产物需要更新，只重新编译或链接受影响的内容。

| 修改情况 | 构建行为 |
| --- | --- |
| 源文件、依赖文件和构建规则均未变化 | 无需重新编译或链接，完成构建检查后启动调试 |
| 仅切换已有的启动目标 | 检查并更新所选目标及其依赖；已有有效产物可复用，尚未构建的目标需要构建 |
| 修改并保存 `.cpp` 或 `.c` 文件 | 重新编译受影响的源文件，并重新链接对应目标 |
| 修改并保存头文件 | 重新编译依赖该头文件的源文件，并重新链接对应目标 |
| 修改 CMake 配置、宏定义、编译选项、头文件目录或链接依赖 | 先执行 Configure 更新构建规则；构建工具再根据新规则重新编译、链接受影响的目标 |
| 切换编译器、构建类型或生成器 | 选择对应 Configure/Build Preset，在其独立目录中配置和构建；已有有效产物可增量更新，尚未构建的目标需要完整构建 |

重新 Configure 不等于全量编译：构建工具按更新后的规则和依赖关系重建受影响的产物。构建规则通过 CMake 配置维护，不直接修改生成的 `build.ninja` 或 `Makefile`。

调试前应保存修改的文件。构建失败时，应先修复编译或链接错误，再重新启动调试。

两套配置均无需额外的 `preLaunchTask`。如果改为直接指定可执行文件路径，应另外安排调试前构建。

### 3.6 `.clang-format` 与 `.gitignore`

构建与调试配置就绪后，再确定项目的格式规则和忽略项。`.clang-format` 供 clangd 格式化时读取；本示例采用 Google 风格、2 空格缩进和 100 字符行宽，完整内容见第 8.6 节。

团队需要一致的格式化结果时，也要统一工具版本。命令行 `clang-format` 是独立程序，可能与 clangd 内置格式化器的版本不同，即使读取同一份规则也需要核对结果。

`.gitignore` 排除 `build`、`CMakeUserPresets.json`、预设生成备份和工具缓存，完整内容见第 8.7 节。项目预设、格式规则和不含个人安装路径的 `.vscode` 配置可纳入版本控制；构建产物、编译数据库和用户私有配置应排除在版本控制之外。

### 3.7 接入已有 CMake 工程

已有工程沿用自己的目标、依赖和格式约定，再接入前述编辑器配置。可按下面的顺序，逐项核对项目要求与本文方案的对应关系。

1. 阅读项目的 README、构建或贡献指南，确认支持的工具链、CMake 版本、依赖安装方式和可调试目标；这些约定优先于本文的示例选择。
2. 打开项目工作区，保留已有 `CMakeLists.txt`、源码和 `.clang-format`。存在多个 CMake 入口时，按项目约定用 `cmake.sourceDirectory` 指定需要的入口目录。
3. 复用项目提供的 Configure/Build Preset；缺少预设时，以本文模板为起点，填入项目要求的 CMake 选项与依赖。个人编译器、SDK 和依赖路径通过 `CMakeUserPresets.json` 配置。
4. 将第 3.4、3.5 节的语言服务与调试条目合入已有 `.vscode` 文件，保留其他项目设置。检查所用编译器路径与 `--query-driver` 匹配。
5. 按第 4.1 节 Configure、Build，使用 `CMake: Set Launch/Debug Target` 选择项目中的实际可执行目标，再选择调试器并按 F5。

示例中的 `example_app`、`demo_calc` 和 `src` 布局按项目实际目标替换。各程序的参数、工作目录或环境变量不同，按第 4.4 节建立程序专用调试配置。

### 3.8 配置维护与版本打包

配置完成后，需要区分哪些内容由团队共享，哪些随用户或主机变化。将个人路径与生成产物放在各自的位置，成员才能复用同一份项目配置。

| 配置归属 | 内容 | 维护方式 |
| --- | --- | --- |
| 项目共享配置 | `CMakeLists.txt`、项目预设、格式规则、通用 `.vscode` 配置 | 纳入项目版本控制；使用项目选项、相对路径和命令变量 |
| 用户或主机配置 | `CMakeUserPresets.json`、VS Code 用户设置、快捷键、SSH 配置 | 保存在对应用户环境；个人绝对路径放在这一层 |
| 生成内容 | 构建目录、编译数据库、缓存、`CMakePresets.json.bak-*` | 排除出版本控制和源码交付包 |
| 指南配套工具 | 生成脚本与 `CMakePresets.template.json` | 两个文件随指南统一发布；作为新项目初始化工具维护 |

跨 Windows/Linux 的仓库按团队支持的平台维护共享预设。两份示例中的 Make 生成器不同；跨平台项目使用项目维护的平台分组或条件预设，成员不各自重新生成并覆盖同一份共享预设。

项目的构建说明应记录支持的工具链、依赖版本、预设和常用目标。使用 CI 时，按相同的项目预设执行配置与构建，并固定所需工具链和依赖版本。

发布指南、项目源码或运行程序时，交付范围也有所不同，可据此选择需要收集的文件。

| 交付类型 | 包含内容 |
| --- | --- |
| 指南交付包 | Windows/Linux 指南及配套 `tools`，保证说明、脚本和模板版本一致 |
| 项目源码包 | 项目源码、共享 CMake 配置和必要的开发说明；生成工具按项目是否使用它决定 |
| 程序发布包 | 程序、运行时依赖和必要资源；按项目安装或打包规则收集，开发预设与初始化工具无需随运行程序发布 |

源码打包时显式排除用户配置、构建产物和备份文件；`.gitignore` 约束版本控制，不会自动过滤手工复制或压缩的文件。

## 4. 配置、构建与调试

项目文件准备好后，用一次 Configure 和 Build 将配置变成可执行程序，再选择目标开始调试。下面先完成编辑器中的操作，再给出使用同一套预设的命令行方式。

### 4.1 首次配置、构建与调试

在已打开的 CMake 项目工作区中按 `Ctrl+Shift+P`，从命令面板完成以下步骤。示例使用 `gcc-ninja-debug` 和 `example_app`；已有工程选择自己的预设和目标，Remote-SSH 场景则在远程窗口操作。

| 步骤 | 操作 | 作用 |
| --- | --- | --- |
| 1 | `CMake: Select Configure Preset`，选择 `gcc-ninja-debug` 或其他已安装工具链对应的预设 | 确定编译器、生成器、Debug/Release 和构建目录 |
| 2 | `CMake: Configure` | 生成构建规则和 `compile_commands.json`；失败时先处理配置错误 |
| 3 | `clangd: Restart language server` | 使 clangd 重新解析所选构建目录 |
| 4 | `CMake: Select Build Preset`，选择对应的构建预设 | 选择构建范围与参数；本文模板使用同名 Build Preset |
| 5 | `CMake: Build` | 编译并链接目标，生成程序及其依赖 |
| 6 | `CMake: Set Launch/Debug Target`（`cmake.selectLaunchTarget`），选择 `example_app` | 指定要启动的可执行目标 |
| 7 | 在 `src/main.cpp` 的输出行设置断点，选择 `CMake / GDB` 或 `CMake / LLDB (CodeLLDB)`，按 F5 | 运行前增量构建完成后启动调试 |

示例程序在断点处可查看 `answer`，继续执行后输出程序结果。后续调试同一程序直接按 F5；切换可执行程序按第 4.4 节选择启动目标。自定义快捷键见第 6.3 节。

界面使用译名时，在命令面板搜索 `CMake` 和相应关键词。预设不出现或 Configure 失败时按第 7.1 节排查。

### 4.2 使用命令行配置、构建与运行

同一套 Presets 也可以从命令行调用，适合在终端构建或复现问题。在项目的 CMake 根目录使用 PowerShell，以 `gcc-ninja-debug` 为例执行：

```powershell
cmake --list-presets
cmake --list-presets=build
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
& .\build\gcc-ninja-debug\example_app.exe
```

示例输出：

```text
C++ environment is ready. answer=42
```

Clang Ninja Debug 配置、构建与运行命令如下：

```powershell
cmake --preset clang-ninja-debug
cmake --build --preset clang-ninja-debug
& .\build\clang-ninja-debug\example_app.exe
```

Release 构建使用 `gcc-ninja-release` 或 `clang-ninja-release`，将命令中的预设名称和可执行文件目录一并替换。

MinGW Makefiles 构建示例：

```powershell
cmake --preset gcc-make-debug
cmake --build --preset gcc-make-debug --parallel
& .\build\gcc-make-debug\example_app.exe
```

使用 Clang 时选择 `clang-make-debug`；Release 构建使用 `gcc-make-release` 或 `clang-make-release`。`--parallel` 用于启用并行构建，适用于两种后端。

指定单个可执行目标进行构建和运行：

```powershell
cmake --build --preset gcc-ninja-debug --target demo_calc
& .\build\gcc-ninja-debug\demo_calc.exe
```

该命令构建 `demo_calc` 及其依赖，输出为 `[demo_calc] 6 * 7 = 42`。不指定 `--target` 时，本文预设默认构建两个可执行目标。命令行操作不会改变 VS Code 中记忆的启动目标；返回编辑器后，通过 `CMake: Set Launch/Debug Target` 选择要调试的程序。

命令行的 `cmake --preset clang-ninja-debug` 不会改变 CMake Tools 中所选的预设。返回 VS Code 时，应选择对应的 Configure Preset、执行配置并重启 clangd，同时选择对应的 Build Preset。源码断点和变量检查宜使用 Debug 构建。GCC、Clang 以及两种生成器均可复用这两套调试配置，无需分别填写可执行文件路径。

### 4.3 使用与切换调试器

程序采用 Debug 预设构建后，可以在“运行和调试”的下拉框中选择 GDB 或 CodeLLDB。切换调试器时，停止当前会话、选择另一条配置，再按 F5 即可；构建配置未变时，无需重新 Configure 或重启 clangd。

| 操作 | GDB 调试控制台 | CodeLLDB 调试控制台（默认命令模式） |
| --- | --- | --- |
| 查看变量 `answer` | `-exec print answer` | `frame variable answer` |
| 求值表达式 | 使用 Watch，或 `-exec print <表达式>` | 使用 Watch，或 `expression -- <表达式>` |
| 查看调用栈 | 使用“调用堆栈”，或 `-exec backtrace` | 使用“调用堆栈”，或 `thread backtrace` |

两套配置均使用 VS Code 的断点、单步、继续执行、变量和 Watch 视图。调试控制台中的命令格式取决于所用调试适配器；切换调试器时不能直接照搬另一种命令。

### 4.4 多可执行目标的选择与调试

示例中的两个 `add_executable()` 分别定义 `example_app` 和 `demo_calc`。Configure 成功后，CMake Tools 会将它们列为可启动目标；库和普通自定义构建目标不属于可运行程序。工程包含多个程序时，需要同时确认构建配置、启动目标和调试适配器。

| 选择项 | 决定的内容 | 本文示例 |
| --- | --- | --- |
| Configure/Build Preset | 编译器、生成器、构建类型和构建目录 | `gcc-ninja-debug`、`clang-make-debug` 等 |
| Launch/Debug Target | 当前配置中要启动的可执行程序 | `example_app` 或 `demo_calc` |
| “运行和调试”中的配置 | 调试适配器及运行参数 | `CMake / GDB` 或 `CMake / LLDB (CodeLLDB)` |

预设和调试器选定后，用下面的操作选择程序：

1. 按 `Ctrl+Shift+P`，执行 **`CMake: Set Launch/Debug Target`**（`cmake.selectLaunchTarget`），选择 `example_app` 或 `demo_calc`。
2. 打开对应源文件，在实际执行的行设置断点，选择 GDB 或 CodeLLDB 配置后按 F5。
3. 继续调试同一程序时直接按 F5；需要换程序时，停止当前会话，再执行目标选择命令，选择另一目标后按 F5。

CMake Tools 按项目记忆启动目标，重新打开同一项目后通常仍保留有效选择；没有有效选择时需重新确认，只有一个可执行目标时可自动选用。该状态保存在编辑器中，移动项目、更换环境或重置扩展后应重新选择。目标选择命令也可按第 6.3 节绑定自定义快捷键。

切换已有启动目标不会改变编译配置，因此无需 Configure 或重启 clangd。切换 Configure Preset 时，按第 4.1 节更新配置；原目标仍存在时，程序路径会指向新构建目录。新增、删除或重命名目标后，则先保存 CMake 文件、执行 Configure，再选择启动目标。多根工作区还需确认 CMake Tools 当前活动项目。

选择启动程序后，也要留意构建范围。未另选构建目标且 Build Preset 未设置 `targets` 时，普通 Build 默认构建两个程序；本文预设模式下，F5 前构建所选启动目标及其依赖。如果构建范围设为 `all` 或包含其他目标，这些目标也会参与构建，其错误可能阻止调试。

构建范围通过 CMake Tools 的构建目标或 Build Preset 的 `targets` 调整，选择启动目标不会自动修改普通 Build 的范围。`cmake.buildBeforeRun` 会确保启动目标参与运行前构建；从终端指定单个目标的方式见第 4.2 节。

两个示例程序共用空参数和项目根目录，因此可以复用同一条调试配置。真实程序需要不同参数、工作目录或环境变量时，建立专用条目，并明确启动目标和运行前构建。需要同时调试多个程序时，使用独立配置及 `compounds`；上述目标选择流程一次启动一个程序。

## 5. 语言服务与格式化

构建与调试读取 CMake 的目标配置，代码编辑也通过编译数据库使用同一份信息。Configure 成功并重启 clangd 后，即可按当前预设进行补全、跳转、诊断和格式化。

### 5.1 代码编辑与格式化

clangd 根据当前编译选项分析源码，并结合项目索引提供导航。常用编辑操作及其配置来源如下。

| 操作 | 使用方式 | 配置来源 |
| --- | --- | --- |
| 补全与类型信息 | 输入成员、命名空间或 `std::`，使用补全列表与悬停信息 | 编译数据库中的标准、宏和头文件路径 |
| 跳转与引用 | 使用编辑器的转到定义、转到声明和查找引用命令 | clangd 对项目源码及依赖的索引 |
| 查看诊断 | 查看编辑器标记和“问题”面板 | 当前编译数据库与源码 |
| 格式化当前文件 | 执行 `Format Document`，或使用 `Shift+Alt+F` | 项目 `.clang-format` 与 clangd 格式化器 |
| 保存时格式化 | 保存文件 | 第 3.4 节的 `editor.formatOnSave` 设置 |

Remote-SSH 使用客户端的编辑器快捷键。团队需要一致的格式化结果时，统一 `.clang-format` 和格式化工具版本；clangd 内置格式化器与命令行 `clang-format` 的版本可能不同。

### 5.2 切换预设与查看语言服务日志

当编译器、宏或头文件路径随预设变化时，语言服务也需要更新。切换 Configure Preset 后执行 Configure，再运行 `clangd: Restart language server`。在“输出”面板选择 **clangd**，即可查看实际服务器版本、启动参数和数据库目录；其中 `--compile-commands-dir` 应指向当前构建目录。

标准库或项目头文件无法解析时，按第 7.3 节检查数据库、编译器路径和 `--query-driver`。终端中的 `clangd --version` 反映终端 PATH 的程序；编辑器实际使用的程序以 clangd 扩展日志为准。

## 6. 日常操作与快捷键

首次配置完成后，后续操作取决于修改影响的是源码、构建规则还是运行目标。先按修改类型选择需要更新的步骤，再用快捷键完成常用操作。

### 6.1 修改后的操作顺序

| 修改内容 | 推荐操作 |
| --- | --- |
| 没有修改，继续调试同一目标 | 直接按 F5；复用已选目标，构建检查通常无需重新编译 |
| 切换已有可执行目标 | 停止调试 → `CMake: Set Launch/Debug Target` → F5；无需 Configure 或重启 clangd |
| `.cpp`、`.c` 或已有头文件 | 保存 → Build 或 F5，增量编译 |
| `CMakeLists.txt`、被包含的 `.cmake`、Preset、依赖或编译选项 | 保存 → Configure → Build/F5 |
| 新增 `.cpp` 或 `.c` 文件 | 加入 CMake 目标 → Configure → Build |
| 新增、删除或重命名可执行目标 | 更新 `add_executable()` → Configure → `CMake: Set Launch/Debug Target` → F5 |
| 切换 Debug/Release | 选择 Configure Preset → Configure → 重启 clangd → 选择对应 Build Preset → Build |
| 切换 GCC/Clang | 选择对应 Configure Preset → Configure → 重启 clangd → 选择同名 Build Preset → Build |
| 切换 Ninja/MinGW Makefiles | 选择对应生成器预设 → Configure → 重启 clangd → 选择同名 Build Preset → Build |
| 切换 GDB/LLDB | 停止调试 → 选择另一套调试配置 → F5；预设不变时无需重新配置 |
| 更换 MINGW64/UCRT64 环境或工具链安装位置 | 使用新的构建目录 → Configure → 重启 clangd → Build |

CMake Tools 和 CMake 生成的构建规则在发现配置变化时，仍可能触发必要的重新配置。工作区的自动配置设置用于控制常规编辑行为，日常操作应遵循上表所列流程。

### 6.2 默认快捷键

常用的构建和调试命令已有默认绑定，可在 CMake Tools 识别项目后使用。

| 快捷键 | 操作 | 生效条件 |
| --- | --- | --- |
| F5 | 启动当前调试配置，复用已选启动目标 | 第 3.5 节的调试配置已设置 |
| F7 | `CMake: Build` | CMake Tools 已启用并识别项目 |
| Shift+F7 | `CMake: Build with Target` | CMake Tools 已启用并识别项目 |

F5 的运行前构建由 `cmake.buildBeforeRun` 和 `cmake.launchTargetPath` 配合完成。快捷键被覆盖时，在 VS Code“键盘快捷方式”中检查。

### 6.3 自定义快捷键

日常开发时，可为目标切换、缓存重配置和语言服务重启增加快捷键，同时保留用户设置与快捷键文件的直接入口。下面的五项绑定与最终配置汇总一致。

| 自定义按键 | 执行命令 | 命令 ID |
| --- | --- | --- |
| F6 | `CMake: Delete Cache and Reconfigure` | `cmake.cleanConfigure` |
| Alt+S | `clangd: Restart language server` | `clangd.restart` |
| Ctrl+K，Ctrl+A | `Preferences: Open User Settings (JSON)` | `workbench.action.openSettingsJson` |
| Ctrl+K，Ctrl+Z | `Preferences: Open Keyboard Shortcuts (JSON)` | `workbench.action.openGlobalKeybindingsFile` |
| F4 | `CMake: Set Launch/Debug Target` | `cmake.selectLaunchTarget` |

1. 执行 `Preferences: Open Keyboard Shortcuts (JSON)`，打开用户快捷键文件。
2. 将[第 8.8 节](#88-自定义快捷键)的五项绑定合入现有数组，保留其他条目；已有相同按键时核对并调整原绑定。
3. 在“键盘快捷方式”中检查冲突。F4 的 `when: "cmake:enableFullFeatureSet"` 将该绑定限定在 CMake Tools 完整功能启用时生效，其余四项保持示例中的设置。

`ctrl+k ctrl+a` 和 `ctrl+k ctrl+z` 是分两次按下的组合键：先按 Ctrl+K，再按 Ctrl+A 或 Ctrl+Z。首次配置时从命令面板打开快捷键 JSON，保存后即可用 Ctrl+K，Ctrl+Z 再次打开该文件。

F4 对应 `CMake: Set Launch/Debug Target`：选择一次，后续 F5 复用已选目标，需要换程序时再按 F4。Alt+S 重启当前工作区的 clangd；切换 Configure Preset 后先 Configure，再用 Alt+S 使语言服务读取新的编译数据库。

F6 执行 `CMake: Delete Cache and Reconfigure`，清除 CMake 配置缓存后按当前预设重新生成构建规则。普通配置仍可从命令面板执行 `CMake: Configure`；日常源码修改保存后使用 Build 或 F5，按第 3.5 节进行增量构建。

快捷键保存在用户配置中，项目无需创建 `.vscode/keybindings.json`。两个设置入口分别打开用户设置与用户快捷键文件；项目的共享配置仍按第 3 节维护。

## 7. 常见问题

排查时先定位失败的阶段：Configure 查看 CMake 输出，Build 查看编译和链接错误，启动或调试查看调试控制台，语言服务查看 clangd 日志。第一条错误通常更接近原因，处理后再重试对应操作，可减少后续错误的干扰。

### 7.1 工具链、预设与构建

| 现象 | 检查与处理 |
| --- | --- |
| 找不到工具，或用到了另一套工具链 | `where.exe` 的第一条路径；PATH 修改后完全重启 VS Code |
| Preset 不出现 | 打开的是项目根目录；JSON 合法；CMake ≥ 3.21；扩展已启用 |
| Configure 失败 | 输出中的第一条错误；所选 C/C++ 编译器与 Ninja/mingw32-make 可用；依赖与工具链兼容；缓存是否来自另一编译器或生成器 |
| Build 失败 | 第一条编译或链接错误；源文件是否加入目标；头文件目录和链接库是否声明 |
| Clang 检测到 MSVC 目标或无法链接 MinGW 库 | 检查 `where.exe clang`、`where.exe clang++`；使用所选 MSYS2 环境中的 GNU/MinGW 版本，并在独立目录重新配置 |
| MinGW Makefiles 无法找到构建工具 | 检查 `where.exe mingw32-make`；安装所选 MSYS2 环境的 make 软件包，并重新打开终端 |
| 提示生成器与已有缓存不一致 | 不在同一构建目录交替使用 Ninja 和 MinGW Makefiles；改用对应预设的独立目录 |

### 7.2 目标选择、运行与调试

| 现象 | 检查与处理 |
| --- | --- |
| 启动目标列表没有新增程序 | 已用 `add_executable()` 定义目标；保存后 Configure 成功；当前活动项目和预设正确；再执行 `CMake: Set Launch/Debug Target` |
| F5 启动了另一个程序 | 用 `CMake: Set Launch/Debug Target` 确认目标；`program` 使用 `${command:cmake.launchTargetPath}`；选择本文的调试配置 |
| 调试一个程序时，被另一个目标的构建错误阻止 | 当前构建范围是否包含该目标；按第 4.4 节检查构建目标或 Build Preset 的 `targets` |
| 调试运行旧结果 | 文件已保存；Build 成功；Configure/Build Preset 和启动目标一致；没有使用另一目录的 exe |
| 断点未验证或无法命中 | 使用 Debug；目标包含该源码和调试信息；断点位于实际执行的行；调试器支持目标架构与调试信息格式 |
| VS Code 不识别调试类型 `lldb` | 安装并启用 CodeLLDB（`vadimcn.vscode-lldb`）；远程开发时安装到远程侧；确认未使用 `lldb-dap` 的配置格式 |
| CodeLLDB 后端启动失败 | 检查“输出”面板中的 LLDB 日志和扩展平台组件；默认使用自带后端；自定义 `lldb.library` 时检查兼容性及配套服务器 |
| LLDB 能运行但源码断点或变量异常 | 使用 Debug；检查目标与源码对应关系、DWARF 调试信息及工具版本；外部构建路径需要正确的 `sourceMap` |
| 程序提示缺少 DLL | 所选环境的 `bin` 在启动程序的 PATH 中；发布到其他机器时另行处理运行时依赖 |
| GDB 提示 pretty-printer 自动加载被拒绝 | 按 GDB 提示检查脚本及其允许路径；需要 STL 容器可视化时，为可信脚本设置精确的允许路径 |

### 7.3 语言服务与格式化

| 现象 | 检查与处理 |
| --- | --- |
| clangd 无法解析标准库 | 所选构建目录中的数据库已生成；编译器路径匹配 `--query-driver`；检查 clangd 输出日志 |
| 切换预设后 clangd 仍使用原配置 | 选择对应 Configure Preset 并配置，重启 clangd，检查日志中的数据库目录 |
| clangd 构建目录为空或命令变量未展开 | CMake Tools 已启用并识别项目；先选 Configure Preset 并 Configure；clangd 扩展支持命令变量；再重启 clangd |
| 格式化与命令行结果不同 | clangd 内置格式化器与 PATH 中 `clang-format` 的版本、读取的规则是否一致 |

### 7.4 缓存与重新配置

如果错误指向旧的编译器路径、生成器或缓存记录，可重新初始化所选预设的配置。确认使用的是目标构建目录后，在项目根目录执行：

```powershell
# --fresh 要求 CMake 3.24 或更新版本
cmake --fresh --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

更换工具链时应使用新的构建目录，避免复用旧产物。CMake 3.21～3.23 可使用 CMake Tools 的 `CMake: Delete Cache and Reconfigure` 处理配置缓存。按第 6.3 节设置绑定后，也可用 F6 执行该命令。

## 8. 最终配置汇总

前文说明了配置的作用和使用顺序，本节将所需文件集中列出，便于创建项目时逐项查阅。新建示例工程可使用下面的完整内容；已有工程按第 3.7 节合入所需部分。文件的维护归属和打包方式见第 3.8 节。

| 配置位置 | 需要完成的配置 | 用途 |
| --- | --- | --- |
| 系统环境 | 安装所选 MSYS2 环境的 GCC、Clang、CMake、Ninja、make、GDB；将该环境的 `bin` 加入用户 PATH | 提供编译、构建和调试工具 |
| VS Code 扩展 | 安装 CMake Tools、clangd 及调试器对应的 C/C++、CodeLLDB 扩展 | 提供项目集成、调试接口和语言服务 |
| CodeLLDB 平台组件 | 使用扩展自带 LLDB 后端；独立命令行 LLDB 按需安装；外部后端按兼容性要求配置 | 提供 LLDB 调试能力 |
| VS Code 用户设置 | 确保 clangd 扩展能找到 `clangd.exe`；需要时设置 `clangd.path` | 配置语言服务器程序位置 |
| 项目根目录 | `CMakeLists.txt`、`CMakePresets.json`、`.clang-format`、`.gitignore` | 定义目标、编译器预设、格式规则和忽略项 |
| 指南配套工具（可选） | 指南目录中的 `tools/generate-cmake-presets.cmake`、`tools/CMakePresets.template.json` | 按第 3.3.1 节向指定项目生成预设，后续维护项目配置 |
| 项目 `.vscode` 目录 | `settings.json`、`launch.json` | 连接 CMake Tools、clangd 和 GDB/LLDB |
| 项目 `src` 目录 | `main.cpp`、`demo_calc.cpp` | 为两个可执行目标提供独立入口 |
| VS Code 用户快捷键（可选） | 按第 8.8 节配置五项用户绑定 | 清除配置缓存、重启语言服务、打开用户配置和选择启动目标 |

用户设置中的绝对路径按实际安装位置填写；项目配置通过 PATH、预设和命令变量获取工具及目标路径。`CMakeUserPresets.json` 仅在需要个人配置时创建。

### 8.1 示例源文件

`src/main.cpp`：

```cpp
#include <iostream>

int main() {
  const int answer = 42;
  std::cout << "C++ environment is ready. answer=" << answer << '\n';
  return 0;
}
```

`src/demo_calc.cpp`：

```cpp
#include <iostream>

int multiply(int left, int right) {
  return left * right;
}

int main() {
  const int left = 6;
  const int right = 7;
  const int result = multiply(left, right);
  std::cout << "[demo_calc] " << left << " * " << right << " = " << result << '\n';
  return 0;
}
```

### 8.2 `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.21)
project(cpp_demo LANGUAGES C CXX)

add_executable(example_app src/main.cpp)
target_compile_features(example_app PRIVATE cxx_std_20)
set_target_properties(example_app PROPERTIES CXX_EXTENSIONS OFF)

add_executable(demo_calc src/demo_calc.cpp)
target_compile_features(demo_calc PRIVATE cxx_std_20)
set_target_properties(demo_calc PROPERTIES CXX_EXTENSIONS OFF)
```

### 8.3 `CMakePresets.json`

以下为生成脚本的默认输出，可直接复制使用。配套的 `tools/CMakePresets.template.json` 使用相同结构，其中 `@MAKE_GENERATOR@` 由脚本按系统替换；运行方式及生成后修改说明见第 3.3.1、3.3.2 节。

```json
{
  "version": 3,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 21,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "gcc-ninja-debug",
      "displayName": "GCC Ninja Debug",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_C_COMPILER": "gcc",
        "CMAKE_CXX_COMPILER": "g++",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "gcc-ninja-release",
      "displayName": "GCC Ninja Release",
      "inherits": "gcc-ninja-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "clang-ninja-debug",
      "displayName": "Clang Ninja Debug",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_C_COMPILER": "clang",
        "CMAKE_CXX_COMPILER": "clang++",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "clang-ninja-release",
      "displayName": "Clang Ninja Release",
      "inherits": "clang-ninja-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "gcc-make-debug",
      "displayName": "GCC Make Debug",
      "inherits": "gcc-ninja-debug",
      "generator": "MinGW Makefiles"
    },
    {
      "name": "gcc-make-release",
      "displayName": "GCC Make Release",
      "inherits": "gcc-make-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "clang-make-debug",
      "displayName": "Clang Make Debug",
      "inherits": "clang-ninja-debug",
      "generator": "MinGW Makefiles"
    },
    {
      "name": "clang-make-release",
      "displayName": "Clang Make Release",
      "inherits": "clang-make-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "gcc-ninja-debug",
      "configurePreset": "gcc-ninja-debug"
    },
    {
      "name": "gcc-ninja-release",
      "configurePreset": "gcc-ninja-release"
    },
    {
      "name": "clang-ninja-debug",
      "configurePreset": "clang-ninja-debug"
    },
    {
      "name": "clang-ninja-release",
      "configurePreset": "clang-ninja-release"
    },
    {
      "name": "gcc-make-debug",
      "configurePreset": "gcc-make-debug"
    },
    {
      "name": "gcc-make-release",
      "configurePreset": "gcc-make-release"
    },
    {
      "name": "clang-make-debug",
      "configurePreset": "clang-make-debug"
    },
    {
      "name": "clang-make-release",
      "configurePreset": "clang-make-release"
    }
  ]
}
```

### 8.4 `.vscode/settings.json`

```json
{
  "cmake.useCMakePresets": "always",
  "cmake.configureOnOpen": false,
  "cmake.configureOnEdit": false,
  "cmake.automaticReconfigure": false,
  "cmake.buildBeforeRun": true,
  "C_Cpp.intelliSenseEngine": "disabled",
  "clangd.arguments": [
    "--background-index",
    "--compile-commands-dir=${command:cmake.buildDirectory}",
    "--query-driver=**/mingw64/bin/gcc.exe,**/mingw64/bin/g++.exe,**/mingw64/bin/clang.exe,**/mingw64/bin/clang++.exe,**/ucrt64/bin/gcc.exe,**/ucrt64/bin/g++.exe,**/ucrt64/bin/clang.exe,**/ucrt64/bin/clang++.exe"
  ],
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
    "editor.formatOnSave": true
  },
  "[c]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
    "editor.formatOnSave": true
  }
}
```

### 8.5 `.vscode/launch.json`

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "CMake / GDB",
      "type": "cppdbg",
      "request": "launch",
      "program": "${command:cmake.launchTargetPath}",
      "cwd": "${workspaceFolder}",
      "args": [],
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
      "name": "CMake / LLDB (CodeLLDB)",
      "type": "lldb",
      "request": "launch",
      "program": "${command:cmake.launchTargetPath}",
      "cwd": "${workspaceFolder}",
      "args": [],
      "stopOnEntry": false,
      "terminal": "integrated"
    }
  ]
}
```

### 8.6 `.clang-format`

```yaml
BasedOnStyle: Google
IndentWidth: 2
ColumnLimit: 100
```

### 8.7 `.gitignore`

```gitignore
/build/
/CMakeUserPresets.json
/CMakePresets.json.bak-*
/.cache/
```

### 8.8 自定义快捷键

以下绑定将清除配置缓存、重启语言服务、打开用户配置和选择启动目标放到常用按键上。执行 `Preferences: Open Keyboard Shortcuts (JSON)`，把条目合入用户 `keybindings.json` 的现有数组，保留其他绑定。

```json
[
  {
    "key": "f6",
    "command": "cmake.cleanConfigure"
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
  },
  {
    "key": "f4",
    "command": "cmake.selectLaunchTarget",
    "when": "cmake:enableFullFeatureSet"
  }
]
```

| 快捷键 | 使用方式 |
| --- | --- |
| F6 | 删除当前构建目录中的 CMake 配置缓存并重新 Configure；完成后按需 Build 或 F5 |
| Alt+S | 重启当前工作区的 clangd；切换 Configure Preset 后，先 Configure，再重启语言服务 |
| Ctrl+K，Ctrl+A | 打开用户 `settings.json`，用于 clangd 程序路径等个人设置；项目设置仍在 `.vscode/settings.json` |
| Ctrl+K，Ctrl+Z | 打开用户 `keybindings.json`，用于修改或追加快捷键 |
| F4 | 选择启动目标，后续 F5 复用当前有效选择；需要换程序时再按 F4 |

`ctrl+k ctrl+a` 和 `ctrl+k ctrl+z` 是分两次按下的组合键：先按 Ctrl+K，再按 Ctrl+A 或 Ctrl+Z。首次配置时从命令面板打开快捷键 JSON，保存后即可用 Ctrl+K，Ctrl+Z 再次打开该文件。

F6 使用 `cmake.cleanConfigure`，执行缓存清理与重新配置。需要常规 Configure 时，仍使用 `CMake: Configure`；编译和链接通过 Build 或 F5 执行。F4 中的 `when` 条件只作用于该条绑定。

这些快捷键写入用户配置，项目无需新增 `.vscode/keybindings.json`。F5 保持 VS Code 的启动调试绑定；设置入口打开用户配置，工作区文件仍按项目约定维护。

## 参考资料

- [CMake Presets 参考手册](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)：预设格式、继承、平台条件与用户预设。
- [VS Code CMake 入门](https://code.visualstudio.com/docs/cpp/cmake-quickstart)：项目配置、构建与调试操作。
- [clangd 编译命令说明](https://clangd.llvm.org/design/compile-commands)：编译数据库与语言服务使用的编译选项。
- [CodeLLDB 使用手册](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md)：调试配置、后端及平台相关设置。
- [MSYS2 环境说明](https://www.msys2.org/docs/environments/)：工具链环境、运行时与软件包约定。
