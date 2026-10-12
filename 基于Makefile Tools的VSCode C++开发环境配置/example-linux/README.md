# Linux Makefile Tools 示例

用 VS Code 打开当前目录，完整说明见 [Linux 配置指南](<../VS Code C++ Linux Makefile Tools 开发环境配置指南.md>)。Remote-SSH 场景在远程主机准备工具和扩展，并打开远程工程目录。

首次操作：

1. `Makefile: Set the current build configuration` → `gcc-debug`。
2. `Makefile: Set the target to be built by make` → `all`。
3. `Makefile: Clean Configure`：分析编译与链接命令，自动发现程序；完成后执行 `clangd: Restart language server`。
4. `Makefile: Build the current target`。
5. `Makefile: Set the make launch configuration` → 选择自动识别列表中路径属于 `build/gcc-debug/bin/` 的 `example_app`。
6. 在 `src/math.cpp` 的 `return result;` 行设置断点，选择 `Makefile / GDB`，按 F5。

将共用 [keybindings.json](../keybindings.json) 合入用户快捷键后，F4 选择启动程序、F6 重新分析、F7 构建当前目标，Alt+S 重启 clangd；这些按键在 CMake 工程中承担相近用途。Make 的 Shift+F7 只选择并保存构建目标，随后再按 F7；CMake 的对应操作则选择目标并完成本次构建。

构建目标可一直保持 `all`，启动程序独立选择；用 F4 改选当前配置的 `demo_calc`，再按 F5，即可体验第二个程序。F5／Ctrl+F5 使用所选 `launch.json` 条目调试／不调试运行，运行前任务仍构建 `all`；使用 LLDB 时选择 `Makefile / LLDB (CodeLLDB)`。尚未调试时，Shift+F5 由插件直接使用默认 GDB 调试，Ctrl+Shift+F5 由插件在终端运行；调试中仍保留停止／重启用途。同开两套工具时，共用模板优先 CMake，具体条件见完整指南第 6.3 节。

模板无需预填程序的 `binaryPath`；新增目标时更新 Makefile，重新 Clean Configure、Build，再选择新程序。选择后插件会向 `settings.json` 写回启动条目及本机具体路径，换环境时重新分析和选择。发布通用模板时可移除自动生成的条目，保留有意维护的自定义条目；自动识别不到程序或需要自定义插件运行目录、参数时，再补充显式启动配置。

F5 从项目根目录运行，参数来自 `launch.json` 的 `args`；插件自身 Run / Debug 则使用启动条目的 `cwd` 和 `binaryArgs`，自动发现的工作目录可能位于 `bin/`。

在当前目录的 Linux 终端中也可以直接构建和运行：

```bash
make CONFIG=gcc-debug all -j2
./build/gcc-debug/bin/example_app
./build/gcc-debug/bin/demo_calc 10 20
```

命令行 Build 不会替 Makefile Tools 更新编译数据库。Makefile、源文件列表或编译选项变化后，在编辑器中重新 Clean Configure。
