# 将当前 VS Code 配置接入已有 CMake 工程：完整示例

本文从一个已经能通过命令行构建的 C++17 工程开始，逐步加入 CMake Presets、clangd 和 GDB／CodeLLDB 调试配置。每一步都沿用工程原有的构建逻辑，并说明新增配置如何与源码、目标和项目选项配合。

## 1. 选择练习方式

配套示例同时保留接入前工程和 Windows、Linux 两套接入结果，既可以从头练习，也可以对照完成后的文件检查配置。

```text
cmake-existing-project-example/
├── README.md                  # 本接入说明
├── 01-existing-project/       # 原有工程：源码、CMakeLists.txt、构建说明
├── 02-vscode-windows/         # 完整接入结果：Windows / MinGW
├── 03-vscode-linux/           # 完整接入结果：Linux
└── tools/                     # 指南配套工具，位于工程目录之外
    ├── generate-cmake-presets.cmake
    └── CMakePresets.template.json
```

如果希望亲手完成接入，先在示例包目录中复制原有工程，后面的修改都在这份练习副本中进行。

Windows PowerShell：

```powershell
Copy-Item -LiteralPath "./01-existing-project" -Destination "./my-existing-project" -Recurse
Set-Location "./my-existing-project"
```

Linux：

```bash
cp -a ./01-existing-project ./my-existing-project
cd ./my-existing-project
```

之后除特别说明外，所有命令均在 `my-existing-project` 根目录执行。

希望先体验完成后的效果时，用 VS Code 打开 `02-vscode-windows` 或 `03-vscode-linux`，检查工具路径后从[第 7 节](#7-在-vs-code-中完成首次配置与构建)开始。每个目录都是独立的 CMake 工程，应打开所选工程的目录。

接入自己的工程时，使用同样的操作顺序，并将示例选项和目标名称对应到项目实际定义。如果项目已经提供 Presets，可先看[第 11 节](#11-工程已经有-presets-时如何接入)，再复用后面的编辑器接入步骤。

## 2. 确认原有工程的构建入口

编辑配置前，先查看工程如何组织源码和构建入口。示例包含一个内部库、一个主程序和一个可选工具，目录如下。

```text
my-existing-project/
├── CMakeLists.txt
├── README.md
├── .clang-format
├── .gitignore
├── app/
│   ├── calc.cpp
│   └── info.cpp
└── lib/
    ├── CMakeLists.txt
    ├── include/acme/math.hpp
    └── src/math.cpp
```

根目录的 `CMakeLists.txt` 定义 C++ 标准、项目选项和两个程序目标，并通过 `add_subdirectory(lib)` 引入内部库：

```cmake
cmake_minimum_required(VERSION 3.16)
project(AcmeCalc VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ACME_BUILD_TOOLS "Build the information tool" OFF)
set(ACME_APP_LABEL "AcmeCalc" CACHE STRING "Application label")

add_subdirectory(lib)

add_executable(acme_calc app/calc.cpp)
target_link_libraries(acme_calc PRIVATE acme_math)

if(ACME_BUILD_TOOLS)
    add_executable(acme_info app/info.cpp)
    target_link_libraries(acme_info PRIVATE acme_math)
endif()
```

库目录继续维护自己的源码、公共头文件路径和编译宏。程序链接该库后，所需头文件目录会通过目标依赖传递：

```cmake
add_library(acme_math STATIC src/math.cpp)
target_include_directories(acme_math PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(acme_math PRIVATE ACME_APP_LABEL="${ACME_APP_LABEL}")
```

从这两份 CMake 文件和原有构建说明中，可以得到接入时需要保留的信息。真实工程也应先核对这些内容，再选择工具链和 Preset。

| 确认项 | 本示例 | 接入真实工程时的依据 |
| --- | --- | --- |
| CMake 入口 | 根目录 `CMakeLists.txt` | 项目构建说明中的 `cmake -S` 目录 |
| 编译器与生成器 | GCC + Ninja | 项目支持的编译器、工具链及生成器 |
| C++ 标准 | C++17 | 原有 `CMakeLists.txt` 中的要求 |
| 项目选项 | `ACME_BUILD_TOOLS=ON`、`ACME_APP_LABEL=team-demo` | 原构建命令中的项目 `-D` 参数 |
| 可执行目标 | `acme_calc`、`acme_info` | `add_executable()` 定义以及相关开关 |
| 依赖 | 工程内部的 `acme_math` 库 | 项目依赖安装步骤、包管理器或工具链文件 |

其中 `ACME_BUILD_TOOLS` 默认关闭，原构建命令通过 `-DACME_BUILD_TOOLS=ON` 启用 `acme_info`。这类项目选项需要一并迁移，否则接入后可能看不到原先能够构建的目标。

原有 `CMakeLists.txt` 继续作为目标、标准、宏和头文件路径的配置来源。编辑器读取 CMake 的结果即可，无需另行维护同样的编译信息。

## 3. 先运行原有构建命令

确认工程入口后，先按原有方式构建和运行，之后就可以比较新增配置是否保留了相同的行为。本示例使用 GCC + Ninja，需要以下工具；其他 Preset 在准备对应工具后再使用。

运行本文 Presets 版本 3 的 CMake 需要至少为 3.21，原工程在 `CMakeLists.txt` 中声明的最低版本仍可保持 3.16。

```sh
cmake --version
ninja --version
gcc --version
g++ --version
```

工具版本满足要求后，在工程根目录执行原有构建命令，保留其中的两个项目选项：

```sh
cmake -S . -B build/cli-debug -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DACME_BUILD_TOOLS=ON -DACME_APP_LABEL=team-demo
cmake --build build/cli-debug
```

Windows PowerShell：

```powershell
./build/cli-debug/acme_calc.exe 12 30
./build/cli-debug/acme_info.exe
```

Linux：

```bash
./build/cli-debug/acme_calc 12 30
./build/cli-debug/acme_info
```

预期输出分别为：

```text
team-demo: 12 + 30 = 42
```

```text
Application: team-demo
Library check: 6 + 7 = 13
```

两个程序运行成功后，原有构建流程就有了可对照的结果。如果此时遇到依赖或编译错误，先按项目说明处理；后面的 Presets 和 VS Code 配置会调用这套流程，仍然需要相同的依赖与工具链。

## 4. 为没有 Presets 的工程生成模板

### 4.1 生成文件

原命令已经明确了编译器、生成器和项目选项，可以将这些内容保存为 Presets，方便后续从编辑器或终端调用。练习工程尚未提供 `CMakePresets.json`，因此先使用工程外 `../tools` 目录中的生成工具建立模板。

Windows：

```powershell
cmake -DPLATFORM=windows -P "../tools/generate-cmake-presets.cmake"
```

Linux：

```bash
cmake -DPLATFORM=linux -P "../tools/generate-cmake-presets.cmake"
```

输出文件为当前工程根目录的 `CMakePresets.json`。在其他目录调用指南原有工具时，也可以显式指定输出位置：

```sh
cmake -DOUTPUT_DIR="<CMake入口目录>" -P "<指南目录>/tools/generate-cmake-presets.cmake"
```

所有 `-D` 参数写在 `-P` 前。脚本和 `CMakePresets.template.json` 保持相邻。后续编辑的是工程中的生成结果，工具继续放在指南配套目录中。

已有 `CMakePresets.json` 的工程按第 11 节处理；生成器默认会保留文件并停止。

### 4.2 迁移项目原有选项

生成的模板包含常用工具组合，还需要补入原工程自己的选项。打开 `CMakePresets.json`，找到隐藏预设 `base`，将刚才命令中的 `ACME_BUILD_TOOLS` 和 `ACME_APP_LABEL` 加入 `cacheVariables`。

修改后的 **`base` 对象**如下；保留文件中其余 Configure Preset 和 Build Preset：

```json
{
  "name": "base",
  "hidden": true,
  "generator": "Ninja",
  "binaryDir": "${sourceDir}/build/${presetName}",
  "cacheVariables": {
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
    "ACME_BUILD_TOOLS": "ON",
    "ACME_APP_LABEL": "team-demo"
  }
}
```

这里的两个项目选项适用于所有组合，因此放在 `base`。只适用于某套组合的选项放在对应的 Configure Preset 中。

`CMAKE_EXPORT_COMPILE_COMMANDS=ON` 用于生成 clangd 所需的编译数据库；头文件目录和宏仍来自原有 CMake 目标定义。

完整生成结果可对照 [Windows CMakePresets.json](02-vscode-windows/CMakePresets.json) 或 [Linux CMakePresets.json](03-vscode-linux/CMakePresets.json)。

### 4.3 确认预设与目录

补齐项目选项后，就可以选择构建组合。模板中的 Configure Preset 和 Build Preset 使用同名，便于选定配置后找到对应的构建入口。

| 名称 | 编译器 | 生成器 | 构建类型 |
| --- | --- | --- | --- |
| `gcc-ninja-debug` | `gcc` / `g++` | Ninja | Debug |
| `gcc-ninja-release` | `gcc` / `g++` | Ninja | Release |
| `clang-ninja-debug` | `clang` / `clang++` | Ninja | Debug |
| `clang-ninja-release` | `clang` / `clang++` | Ninja | Release |
| `gcc-make-debug` | `gcc` / `g++` | Windows：MinGW Makefiles；Linux：Unix Makefiles | Debug |
| `gcc-make-release` | `gcc` / `g++` | 同上 | Release |
| `clang-make-debug` | `clang` / `clang++` | 同上 | Debug |
| `clang-make-release` | `clang` / `clang++` | 同上 | Release |

每套预设使用独立目录，例如：

```text
build/
├── gcc-ninja-debug/
├── clang-ninja-debug/
└── gcc-make-debug/
```

原来的 `build/cli-debug` 可以继续用于原命令，不与新预设共用缓存。更换编译器或生成器时使用对应的新目录。

Windows 示例使用 MSYS2 MINGW64 的 GCC/Clang；使用 UCRT64 时统一使用该环境下的工具。不要让 `clang++` 意外指向默认生成 MSVC 目标的另一套 Clang。Linux 的 Make 预设需要 `make`，Windows 的 MinGW Makefiles 预设需要 `mingw32-make`。

## 5. 合入 VS Code 工作区设置

### 5.1 打开工程并安装扩展

Presets 将原有构建命令保存下来后，再把它接入 VS Code。打开 **`my-existing-project` 根目录**，安装或启用以下扩展，让编辑器能够调用 CMake、启动语言服务和调试器。

| 扩展 | ID | 用途 |
| --- | --- | --- |
| CMake Tools | `ms-vscode.cmake-tools` | Configure、Build、目标选择 |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | C/C++ 语言服务和格式化 |
| C/C++ | `ms-vscode.cpptools` | GDB 调试 |
| CodeLLDB | `vadimcn.vscode-lldb` | LLDB 调试 |

按所用调试器安装 C/C++ 或 CodeLLDB；同时保留两套调试配置时安装两者。clangd 程序也需可供扩展启动。

Remote-SSH 场景在远程窗口打开远程工程，将 CMake Tools、clangd 和所用调试扩展安装到远程端；编译器、构建工具、clangd 及 GDB 在远程端准备。以下项目配置使用远程文件路径。

### 5.2 `.vscode/settings.json`

扩展启用后，在 `.vscode/settings.json` 中指定它们如何配合。下面给出 Windows 的完整设置：

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
    "--query-driver=**/mingw64/bin/gcc.exe,**/mingw64/bin/g++.exe,**/mingw64/bin/clang.exe,**/mingw64/bin/clang++.exe"
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

Linux 使用相同设置，将 `clangd.arguments` 中的 `--query-driver` 参数改为：

```text
--query-driver=/usr/bin/gcc,/usr/bin/g++,/usr/bin/clang,/usr/bin/clang++
```

完整 Linux 文件见 [Linux settings.json](03-vscode-linux/.vscode/settings.json)。

`--query-driver` 只保留自己使用的工具链路径。Windows 使用 UCRT64 时将模式中的 `mingw64` 改为 `ucrt64`；自定义安装位置或版本化编译器名称按实际路径调整。多个模式以逗号写在同一个参数中。

| 设置 | 配置原因 |
| --- | --- |
| `cmake.useCMakePresets` | Configure 和 Build 使用已迁移项目选项的 Presets |
| 三个自动配置开关为 `false` | 主动执行 Configure，明确构建规则何时更新 |
| `cmake.buildBeforeRun` | 动态解析启动目标时，先构建再调试 |
| `C_Cpp.intelliSenseEngine=disabled` | 由 clangd 提供语言服务，C/C++ 扩展继续提供 GDB 调试 |
| `--compile-commands-dir` | 从当前 Configure Preset 的构建目录读取数据库 |
| `--query-driver` | 允许 clangd 查询匹配的驱动，获取工具链的标准库搜索路径 |
| C/C++ 格式化设置 | 使用 clangd，并沿用工程已有的 `.clang-format` |

工程已有 `settings.json` 时，将对应键合入现有内容。`[cpp]`、`[c]` 保留其他子键，`clangd.arguments` 保留仍需要的参数，并确保数据库目录参数只有当前方案。原有 `cmake.configureArgs`、`cmake.configureSettings` 等如果与 Preset 重复或冲突，先迁移其中必要的构建选项，再清理旧值。

采用这套语言服务配置后，C++17、宏和库头文件路径都由 CMake 的编译命令提供，因此无需再增加 `c_cpp_properties.json` 来重复填写。

## 6. 合入 GDB / CodeLLDB 调试配置

工作区设置确定了配置与构建方式，`launch.json` 再决定如何启动调试。创建 `.vscode/launch.json` 并写入以下内容；已有文件时，将所需条目加入原有 `configurations` 数组，保留其他配置和 `compounds`。

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

`program` 由 CMake Tools 返回所选可执行目标的实际路径，因此无需猜测可执行文件放在哪个子目录，也无需手动添加 Windows 的 `.exe` 后缀。

GDB 从 VS Code 继承的 PATH 查找；需要指定个人路径时使用自己的调试条目。CodeLLDB 默认使用扩展自带的 LLDB 后端。调试器需支持目标架构及编译器生成的调试信息。

当前的动态 `program` 与 `cmake.buildBeforeRun=true` 已安排调试前构建，不再为这些条目重复添加 `preLaunchTask`。工程已有其他调试条目的构建任务仍按其用途保留。

示例中的两个程序都能使用空参数和项目根目录。真实工程的运行参数、工作目录及环境变量按[第 10 节](#10-程序需要参数资源目录或环境变量时)调整。

## 7. 在 VS Code 中完成首次配置与构建

至此，项目的构建选项、语言服务和调试入口已经关联起来。按 `Ctrl+Shift+P` 打开命令面板，完成第一次 Configure 和 Build，让这些配置生成实际的构建规则与程序。

1. 执行 **`CMake: Select Configure Preset`**，选择 `gcc-ninja-debug`。
2. 执行 **`CMake: Configure`**，等待成功。
3. 执行 **`clangd: Restart language server`**，让 clangd 使用这次选择的构建目录。
4. 执行 **`CMake: Select Build Preset`**，选择 `gcc-ninja-debug`。
5. 执行 **`CMake: Build`**，等待两个程序构建完成。

Configure 成功后，目录中应包含：

```text
build/gcc-ninja-debug/
├── CMakeCache.txt
├── compile_commands.json
├── build.ninja
└── ...
```

Build 成功后，本示例增加 `acme_calc.exe`、`acme_info.exe`，Linux 下对应为 `acme_calc`、`acme_info`。其他工程可能通过输出目录设置将程序放在 `bin` 或更深的子目录，动态 `program` 会读取 CMake 提供的路径。

`compile_commands.json` 在成功 Configure 后生成，clangd 无需等待 Build。打开 `app/calc.cpp`，`acme/math.hpp` 的头文件路径由 `acme_math` 的 CMake 目标依赖传递；打开库源码时，`ACME_APP_LABEL` 宏来自原有 `target_compile_definitions()`。无需在编辑器里再维护这些内容。

数据库留在构建目录中，不复制到项目根目录。命令变量在 clangd 启动时解析，因此首次配置及切换 Configure Preset 后重启语言服务器。

这一流程仍然可以从终端调用同一套 Presets。需要复现配置或构建问题时，在工程根目录使用以下等价命令：

```sh
cmake --list-presets
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

终端命令不会替你更新 CMake Tools 的预设选择。返回 VS Code 后仍选择相同的 Configure/Build Preset，并在需要更新语言服务时重启 clangd。

## 8. 逐个调试两个可执行程序

两个程序构建完成后，用同一份调试配置分别启动它们。调试配置选择 GDB 或 CodeLLDB，目标选择命令决定本次运行哪个程序。

### 8.1 先调试 `acme_calc`

1. 执行 **`CMake: Set Launch/Debug Target`**，选择 **`acme_calc`**。
2. 打开 `app/calc.cpp`，在输出结果的 `std::cout` 行设置断点。
3. 在“运行和调试”面板的下拉列表中选择 **`CMake / GDB`** 或 **`CMake / LLDB (CodeLLDB)`**。
4. 保存修改，按 **F5**。
5. 程序停在断点处时查看 `left`、`right`、`result`，默认值分别为 `2`、`3`、`5`。
6. 继续运行，应输出 `team-demo: 2 + 3 = 5`。

完成这次调试后，CMake Tools 会保留当前有效的 `acme_calc` 选择，再次按 F5 即可继续调试同一程序。

### 8.2 再调试 `acme_info`

切换到工具程序时，保留当前预设和调试器，只更换启动目标。

1. 停止当前调试。
2. 执行 **`CMake: Set Launch/Debug Target`**，选择 **`acme_info`**。
3. 打开 `app/info.cpp`，在第一处 `std::cout` 行设置断点。
4. 保持同一套 GDB 或 CodeLLDB 调试配置，按 **F5**。
5. 在断点处查看 `check`，值应为 `13`；继续运行会输出应用标签和库检查结果。

切换已有启动目标无需重新 Configure，也无需重启 clangd。CMake Tools 记忆当前有效选择；目标不存在、编辑器状态重置或更换环境后重新确认。

### 8.3 分清三类选择

刚才的操作涉及三类选择，它们分别控制构建环境、运行程序和调试方式。分清这些作用后，后续切换就能只调整需要变化的部分。

| 选择 | 本示例的值 | 决定什么 |
| --- | --- | --- |
| Configure/Build Preset | `gcc-ninja-debug` 等 | 用哪套工具链、构建类型和构建目录 |
| Launch/Debug Target | `acme_calc` 或 `acme_info` | 启动哪个程序 |
| “运行和调试”的配置 | GDB 或 CodeLLDB | 用哪个调试适配器、参数和工作目录 |

`acme_math` 是库，不能作为程序启动。`acme_info` 不出现时，先检查当前 Configure Preset 的 `ACME_BUILD_TOOLS` 是否为 `ON`，然后重新 Configure。

本文 Build Preset 未设置 `targets`，也未另选其他构建范围：普通 Build 构建默认目标，包括两个程序；当前预设模式下，F5 前构建所选启动目标及其依赖。真实工程若另设构建目标或 Build Preset 的 `targets`，需同时检查该范围；选择启动目标并不等于修改普通 Build 的范围。

### 8.4 可选：配置自定义快捷键

经常切换程序、重启语言服务或调整个人设置时，可增加以下五项用户绑定。执行 `Preferences: Open Keyboard Shortcuts (JSON)`，将条目合入用户 `keybindings.json` 的现有数组。

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

| 自定义按键 | 执行命令 | 命令 ID |
| --- | --- | --- |
| F6 | `CMake: Delete Cache and Reconfigure` | `cmake.cleanConfigure` |
| Alt+S | `clangd: Restart language server` | `clangd.restart` |
| Ctrl+K，Ctrl+A | `Preferences: Open User Settings (JSON)` | `workbench.action.openSettingsJson` |
| Ctrl+K，Ctrl+Z | `Preferences: Open Keyboard Shortcuts (JSON)` | `workbench.action.openGlobalKeybindingsFile` |
| F4 | `CMake: Set Launch/Debug Target` | `cmake.selectLaunchTarget` |

`ctrl+k ctrl+a` 和 `ctrl+k ctrl+z` 是分两次按下的组合键：先按 Ctrl+K，再按 Ctrl+A 或 Ctrl+Z。首次配置时从命令面板打开快捷键 JSON，保存后即可用 Ctrl+K，Ctrl+Z 再次打开该文件。

F4 对应 **`CMake: Set Launch/Debug Target`**。换程序时按 F4，之后用 F5 调试；继续调试同一程序时直接按 F5。Alt+S 重启 clangd，切换 Configure Preset 后先 Configure，再按 Alt+S，让语言服务读取新目录中的数据库。

F6 清除当前构建目录中的 CMake 配置缓存并重新 Configure。普通配置仍使用命令面板中的 `CMake: Configure`；保存源码后用 Build 或 F5 更新程序。F6 本身执行配置，后续按需要构建目标。

Ctrl+K，Ctrl+A 打开用户 `settings.json`，项目设置仍在 `.vscode/settings.json`；Ctrl+K，Ctrl+Z 打开用户 `keybindings.json`。快捷键属于用户配置，项目无需增加 `.vscode/keybindings.json`。首次设置 F4 时核对其 `when` 条件和其他绑定是否冲突，继续使用命令面板也能完成相同操作。

## 9. 体验增量构建与切换 Preset

首次调试完成后，再做一次源码修改和预设切换，观察配置如何参与日常开发。源码变化主要触发增量构建，工具链或构建规则变化则需要更新 CMake 配置。

### 9.1 修改源码

将 `app/calc.cpp` 的默认 `right` 从 `3` 改为 `4`，保存后按 F5。构建工具重新编译受影响的源文件并链接 `acme_calc`；本次输出变为 `team-demo: 2 + 4 = 6`。

练习后可将默认值恢复为 `3` 并保存。其他修改也会按其影响范围更新，常见情况如下。

| 修改情况 | 操作 | 构建行为 |
| --- | --- | --- |
| 源码、依赖和构建规则都未变化 | F5 | 检查构建状态，无需重新编译或链接 |
| 保存了 `.cpp` 修改 | F5 | 重新编译受影响源码并链接相关目标 |
| 保存了头文件修改 | F5 | 更新依赖该头文件的编译单元及相关目标 |
| 修改 `CMakeLists.txt`、项目开关、宏、编译/链接选项 | Configure → 必要时重启 clangd → F5 | 先更新构建规则，再按依赖和新规则更新产物 |
| 切换编译器、生成器或构建类型 | 选择对应 Configure/Build Preset并配置 | 使用该组合的独立目录，已有有效产物可复用 |

重新 Configure 不等于全量编译。日常修改按影响范围增量更新；项目构建规则通过 CMake 文件和 Presets 调整。

### 9.2 切换到 Clang、Make 或 Release

除了修改源码，还可以复用现有配置切换工具组合。以 `clang-ninja-debug` 为例，选择新预设后更新构建规则和语言服务，再继续调试。

1. 准备对应平台的 `clang`、`clang++` 和 Ninja，并确认 `--query-driver` 匹配实际驱动。
2. 执行 `CMake: Select Configure Preset`，选择 `clang-ninja-debug`。
3. 执行 `CMake: Configure`。
4. 执行 `clangd: Restart language server`。
5. 执行 `CMake: Select Build Preset`，选择 `clang-ninja-debug`。
6. 确认 Launch/Debug Target 仍是要调试的程序，按 F5。

切换到 `gcc-make-debug` 或 `clang-make-debug` 时采用同一顺序，使用该平台的 Make 工具。Release 预设主要用于发布构建；练习断点与变量查看时使用 Debug。

## 10. 程序需要参数、资源目录或环境变量时

前面的两个程序都能使用默认参数和工程根目录运行。真实工程可能需要命令行参数、资源文件或运行时环境，这些要求在调试条目中配置。

### 10.1 传入参数

先用计算程序练习参数传递。停止调试，将当前使用的 `launch.json` 条目中的 `args` 改为：

```json
"args": ["12", "30"]
```

用 `CMake: Set Launch/Debug Target` 选择 `acme_calc`，按 F5，应输出 `team-demo: 12 + 30 = 42`。

`args` 是字符串数组。调试配置内的设置会随该条目使用，切换启动目标不会自动换成另一个程序的专用参数。

### 10.2 调整工作目录和环境变量

如果程序通过相对路径读取资源，按项目运行说明设置 `cwd`。例如资源路径相对于工程根目录时使用 `${workspaceFolder}`；相对于工程中的 `runtime` 目录时使用 `${workspaceFolder}/runtime`。本示例没有外部运行资源，保持项目根目录即可。

两种适配器的环境变量语法不同。以下都是合入所选调试条目的字段片段，示例路径需对应实际存在的资源目录。

GDB：

```json
"cwd": "${workspaceFolder}",
"args": ["12", "30"],
"environment": [
  { "name": "ACME_DATA_DIR", "value": "${workspaceFolder}/resources" }
]
```

CodeLLDB：

```json
"cwd": "${workspaceFolder}",
"args": ["12", "30"],
"env": {
  "ACME_DATA_DIR": "${workspaceFolder}/resources"
}
```

构建期依赖位置放在 CMake 配置中；程序运行期参数和环境放在调试配置中。

不同程序的运行要求较多时，复制所需调试条目并用名称标明用途，例如 `Calculator / GDB`、`Information / GDB`。保留动态 `program` 时，每次仍要确认启动目标；条目名称不会把目标固定为某个程序。需要同时调试多个程序时，应另设各自独立的程序路径和调试前构建，再组织 `compounds`。

## 11. 工程已经有 Presets 时如何接入

前面的练习从缺少 Presets 的工程开始；实际项目如果已经提供预设，接入时直接沿用它们。这些预设往往还配置了第三方依赖、工具链或交叉编译环境。先选择项目提供的 Debug 预设，再按第 5～8 节连接 VS Code，可保留项目已有的构建约定。

### 11.1 共享修改：补充编译数据库开关

需要统一给团队使用时，在合适的现有 Configure Preset 或公共父预设中加入：

```json
"CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
```

保留原有 `cacheVariables` 的其他键、`toolchainFile`、`environment` 和继承关系。使用 Ninja 或 Make 生成器时可按本文方案生成数据库；已有预设使用 Visual Studio 等生成器时，先按项目支持情况增加兼容预设或采用项目的语言服务方案。

### 11.2 个人修改：继承现有预设

假设项目已有 Configure Preset 和 Build Preset 都叫 `gcc-ninja-debug`。在 **同一 CMake 入口目录**新建 `CMakeUserPresets.json`：

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "local-gcc-ninja-debug",
      "inherits": "gcc-ninja-debug",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "local-gcc-ninja-debug",
      "inherits": "gcc-ninja-debug",
      "configurePreset": "local-gcc-ninja-debug"
    }
  ]
}
```

保存后，CMake Tools 会列出 `local-gcc-ninja-debug`。选择它并按第 7 节完成 Configure、重启 clangd 和 Build Preset 选择，就能使用继承得到的项目配置。

使用时作如下调整：

- `inherits` 改为工程实际的预设名称。Configure/Build 两类分别继承各自已有的预设；如果工程没有相应 Build Preset，删除上述 Build Preset 的 `inherits`，保留 `configurePreset`。
- 将文件 `version` 与项目使用的 Presets 版本及所需 CMake 版本保持兼容。项目使用较新格式时采用其既有格式。
- `binaryDir` 示例用于分离个人缓存。工程对构建目录有特定约定时按其约定设置。
- 个人依赖位置等差异可以加入本地预设的 `cacheVariables`；不要清除继承得到的项目必要选项。
- 继承的 Build Preset 会保留其 `targets` 等设置。确认该范围满足项目构建要求，并用启动目标命令选择程序。

`CMakeUserPresets.json` 会隐式包含同目录的 `CMakePresets.json`，因此可以继承公开预设，不需要额外添加 `include`。个人文件加入 `.gitignore`。

## 12. 将示例步骤映射到真实工程

无论使用新生成的模板还是项目已有的预设，接入真实工程时都要保留原有构建要求。下面将命令参数和目录布局逐项对应到配置位置。

### 12.1 迁移原构建命令

以原构建命令为依据，将生成器、构建目录、项目开关和依赖位置一并迁移。常见参数与 Presets、调试配置的对应关系如下。

| 原有命令或要求 | Presets 中的位置 |
| --- | --- |
| `-G Ninja` / Make 生成器 | Configure Preset 的 `generator` |
| `-B <构建目录>` | `binaryDir` |
| `-DCMAKE_BUILD_TYPE=Debug` | `cacheVariables.CMAKE_BUILD_TYPE`，用于本文单配置生成器 |
| `-DCMAKE_C_COMPILER`、`-DCMAKE_CXX_COMPILER` | 对应 `cacheVariables`；项目使用工具链文件时按其约定 |
| 项目 `-D` 开关、宏配置变量 | `cacheVariables` 中同名的项目变量 |
| `-DCMAKE_PREFIX_PATH=...` 或依赖包目录 | 同名 `cacheVariables`；个人路径放在 User Presets |
| `-DCMAKE_TOOLCHAIN_FILE=...` | 原有工具链配置，或等价的 Configure Preset `toolchainFile` |
| `cmake --build ... --target <目标>` | Build Preset 的 `targets` 或显式 Build 命令 |
| 程序参数、运行时工作目录、环境变量 | `launch.json` 对应适配器字段 |

工程中的 `find_package()`、`target_link_libraries()`、`target_include_directories()` 和目标 C++ 标准继续由 CMake 维护。只有原工程没有正确声明依赖时，才按项目实际需求补充构建逻辑。

真实工程只生成库时，需要按项目选项启用测试、示例或工具程序，或者使用依赖该库的可执行工程，才能选择本机启动调试目标。

### 12.2 CMake 入口位于工作区子目录

有些仓库同时存放多种组件，VS Code 打开的目录与 CMake 入口并不相同。假设工作区为 `repository`，所需工程的顶层 CMake 入口在 `components/acme`：

```text
repository/
├── .vscode/
│   ├── settings.json
│   └── launch.json
└── components/acme/
    ├── CMakeLists.txt
    ├── CMakePresets.json
    ├── app/
    └── lib/
```

1. VS Code 打开 `repository`。
2. 将 Presets 放在 `components/acme`，即所用顶层 CMake 入口目录。
3. 在工作区 `.vscode/settings.json` 中加入：

```json
"cmake.sourceDirectory": "${workspaceFolder}/components/acme"
```

4. 继续按第 7～8 节操作。`--compile-commands-dir=${command:cmake.buildDirectory}` 和动态 `program` 保持不变。
5. 若程序要求以入口目录为工作目录，将调试条目的 `cwd` 改为 `${workspaceFolder}/components/acme`；否则按程序自己的运行约定设置。

此时 `${sourceDir}` 指向 `components/acme`，示例的数据库路径为 `components/acme/build/gcc-ninja-debug/compile_commands.json`；`${workspaceFolder}` 仍指向 `repository`。

终端使用 Presets 时先切换到入口目录：

```sh
cd components/acme
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

这里只接入一个已有顶层 CMake 工程。工程里已有 `add_subdirectory()` 管理的库目录继续作为其子项目，不逐个拆成新的 VS Code 工程。多根工作区使用目标命令前先确认 CMake Tools 的活动项目。

## 13. 整理项目配置

接入完成后，整理本次新增的文件。本示例的源码和 CMake 目标定义保持原样，新增内容集中在 Presets 与 `.vscode` 目录中。

```text
my-existing-project/
├── CMakePresets.json
└── .vscode/
    ├── settings.json
    ├── launch.json
    └── extensions.json        # 可选：扩展推荐
```

如果希望其他成员打开工程时收到扩展推荐，还可以添加 `.vscode/extensions.json`：

```json
{
  "recommendations": [
    "ms-vscode.cmake-tools",
    "llvm-vs-code-extensions.vscode-clangd",
    "ms-vscode.cpptools",
    "vadimcn.vscode-lldb"
  ]
}
```

扩展推荐不会自动安装。已有文件时将所需 ID 合入现有 `recommendations` 数组。

在工程已有 `.gitignore` 中追加所需规则：

```gitignore
/build/
/CMakeUserPresets.json
/CMakePresets.json.bak-*
/.cache/
```

本示例原来已有 `/build/`，只追加其余三项。构建目录位于工作区子目录时，在相应工程目录维护规则，或在仓库根 `.gitignore` 中填写实际相对路径。

按项目约定提交共享 Presets 和必要的 `.vscode` 配置，保留原有源码、CMake 文件和 `.clang-format`。个人 Presets、构建产物与用户快捷键不进入共享工程配置；生成工具随指南维护，工程只保留生成后按项目需求调整的配置。

## 14. 按现象定位接入问题

出现问题时，先比较原构建命令与当前 Preset，再确认 VS Code 的工作区、工具路径和目标选择。下表按接入过程中常见的现象列出检查位置。

| 现象 | 检查与处理 |
| --- | --- |
| 找不到 Preset | 确认 VS Code 打开的目录、`cmake.sourceDirectory`、Presets 所在位置与格式版本 |
| 命令行能构建，VS Code 找不到工具 | 检查 VS Code 继承的 PATH、远程运行位置以及项目是否需要工具链环境；更新环境后重新启动 VS Code |
| 配置时报第三方依赖缺失 | 对照原构建命令，检查依赖安装步骤、项目选项、包目录和工具链文件是否已迁移 |
| 找不到 `compile_commands.json` | 确认当前 Preset 的数据库开关和生成器，先完成 Configure，再检查实际构建目录 |
| 标准库头文件报错 | 检查 `--query-driver` 是否匹配数据库中的编译器及其工具链 |
| 切换 Preset 后 clangd 仍使用旧配置 | Configure 成功后执行 `clangd: Restart language server` |
| 目标列表中没有某个程序 | 检查控制该目标的项目开关；示例需要 `ACME_BUILD_TOOLS=ON`，修改后重新 Configure |
| F5 启动了另一个程序 | 执行 `CMake: Set Launch/Debug Target`，确认目标，再确认所选调试条目的参数 |
| 已有调试条目没有触发构建 | 检查其 `program` 是否采用当前动态方案，或者它原有的调试前构建任务是否正确 |
| 断点未命中或变量不可用 | 使用匹配的 Debug 产物和调试器，在实际执行的源码行设断点，检查源码与二进制是否对应 |
| 程序找不到相对路径下的资源 | 按项目运行要求设置 `cwd` 和运行期环境变量 |
