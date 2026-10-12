# Windows Makefile Tools Example

Open this directory in VS Code. See the [Windows configuration guide](../makefile-tools-configuration-guide/windows-makefile-tools-development-environment-guide.md) for complete instructions.

For the first run:

1. `Makefile: Set the current build configuration` → `gcc-debug`.
2. `Makefile: Set the target to be built by make` → `all`.
3. `Makefile: Clean Configure`: analyze compilation and linking commands and discover programs automatically. After it finishes, run `clangd: Restart language server`.
4. `Makefile: Build the current target`.
5. `Makefile: Set the make launch configuration` → select the discovered `example_app` whose path is under `build/gcc-debug/bin/`.
6. Set a breakpoint on `return result;` in `src/math.cpp`, select `Makefile / GDB`, and press F5.

Merge the shared [keyboard shortcut template](../makefile-tools-configuration-guide/windows-makefile-tools-development-environment-guide.md#88-user-keyboard-shortcut-template) into your user keyboard shortcuts. F4 selects a launch program, F6 analyzes again, F7 builds the current target, and Alt+S restarts clangd. These keys serve similar purposes in CMake projects. Make's Shift+F7 selects and saves the build target; press F7 afterward to build it. The corresponding CMake command selects a target and builds it for that invocation.

You can keep the build target set to `all` and choose the launch program independently. Use F4 to select `demo_calc` from the current configuration, then press F5 to try the second program. F5 / Ctrl+F5 debug / run without debugging using the selected `launch.json` entry; the pre-launch task still builds `all`. For LLDB, select `Makefile / LLDB (CodeLLDB)`. When no debug session is running, Shift+F5 debugs directly through the extension with the default GDB, while Ctrl+Shift+F5 runs through the extension in a terminal. During debugging, these keys retain their Stop / Restart actions. If both tools are enabled, the shared template gives CMake priority; see section 6.3 of the full guide for the conditions.

The template does not require a predefined `binaryPath`. When adding a target, update the Makefile, run Clean Configure and Build, and select the new program. The extension writes launch entries and machine-specific paths back to `settings.json` after selection. When changing environments, analyze and select again. When publishing a reusable template, you can remove automatically generated entries while retaining intentional custom entries. Add explicit launch configurations if program discovery fails or if you need custom working directories or arguments for the extension's Run command.

F5 runs from the project root using `args` from `launch.json`. The extension's own Run / Debug commands use `cwd` and `binaryArgs` from the launch entry; automatically discovered working directories may be in `bin/`.

You can also build and run directly from a PowerShell terminal in this directory:

```powershell
mingw32-make CONFIG=gcc-debug all -j2
./build/gcc-debug/bin/example_app.exe
./build/gcc-debug/bin/demo_calc.exe 10 20
```

A command-line build does not update the compilation database for Makefile Tools. If the Makefile, source list, or compiler options change, run Clean Configure again in the editor.
