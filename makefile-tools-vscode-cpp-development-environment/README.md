# VS Code C++ Development Environment with Makefile Tools

Makefile Tools manages build configurations and launch programs for GNU Make projects, clangd provides language services, and GDB or CodeLLDB provides debugging. The example projects contain two executable targets so you can practice switching between GCC / Clang and Debug / Release, and debugging multiple programs.

## Start Here

| Development environment | Guide | Example directory to open |
| --- | --- | --- |
| Windows native MinGW | [Windows guide](makefile-tools-configuration-guide/windows-makefile-tools-development-environment-guide.md) | `example-windows/` |
| Linux local development or Remote-SSH | [Linux guide](makefile-tools-configuration-guide/linux-makefile-tools-development-environment-guide.md) | `example-linux/` |

Both guides follow the same sequence: prepare the tools and extensions, understand the project files, complete the first build and debug session, then configure language services and learn everyday operations. For a new project, start with sections 1–4. For an existing project, start with section 3.7. Complete configurations are collected at the end of each guide; the Linux guide covers remote connections separately in section 8.

Once the environment is ready, open the appropriate example directory as its own project in VS Code. Use `gcc-debug` and GDB for the first run, following these steps. Section 3 of the guide explains how the files work together.

1. `Makefile: Set the current build configuration`: select `gcc-debug`.
2. `Makefile: Set the target to be built by make`: select `all`.
3. `Makefile: Clean Configure`: analyze the project's compilation and linking commands, export the compilation database, and discover executables.
4. `clangd: Restart language server`: read the database for the current configuration.
5. `Makefile: Build the current target`: build both programs.
6. `Makefile: Set the make launch configuration`: select the discovered `example_app` whose path is under `build/gcc-debug/bin/`. If you have installed the user shortcuts, press F4.
7. Set a breakpoint at `return result;` in `src/math.cpp`, select the `Makefile / GDB` debug configuration, and press F5.

By default, `example_app` calculates `2 + 3 = 5`, while `demo_calc` calculates `12 + 30 = 42`. The build target `all` and the launch program are independent. Keep building all targets, and use F4 or the Command Palette to select a different program before pressing F5. For LLDB, select `Makefile / LLDB (CodeLLDB)`.

## How the Configuration Fits Together

| File | Purpose |
| --- | --- |
| `Makefile` | Defines source files, targets, compiler options, and dependencies; the four configurations write to separate `build/<configuration>/` directories |
| `.vscode/settings.json` | Configures Makefile Tools and clangd; Configure discovers programs automatically |
| `.vscode/tasks.json` | Provides build tasks for F5 and Ctrl+Shift+B, building `all` for the current configuration |
| `.vscode/launch.json` | Reads the program selected in Makefile Tools and starts GDB or CodeLLDB from the project root |
| `.vscode/extensions.json` | Recommends the required extensions |
| `.clang-format` | Defines consistent formatting rules |
| Keyboard shortcut template in the guide | Provides user shortcuts to merge into VS Code's user configuration |

Makefile Tools Configure analyzes Make's compilation and linking commands, exports `compile_commands.json` for the current configuration, and discovers executables. clangd reads the database from that configuration's build directory. After switching configurations, analyze the project again, restart clangd, build, and select a program from the new configuration. The examples resolve compilers to full paths so the build and language services use the same toolchain.

The template does not prepopulate `makefile.launchConfigurations`. Instead, `launch.json` reads the selected program through `${command:makefile.getLaunchTargetPath}`. After adding a target, update the Makefile, run Clean Configure and Build, and select the program; normally, no changes to the debug template are needed. The extension writes launch entries and machine-specific paths back to `settings.json` after selection. When changing environments, analyze and select again. When publishing a reusable template, you can remove these automatically generated entries while retaining any custom entries maintained by the project. Add explicit launch configurations if automatic discovery fails or if the extension's Run command requires a custom working directory or arguments.

F5 first runs the `all` build task, then debugs the selected program from the project root using `args` from `launch.json`. The extension's own Run / Debug commands use `cwd` and `binaryArgs` from the launch entry; an automatically discovered working directory may be in `bin/`. Fix build failures before launching, rather than selecting Debug Anyway to run an old executable. Normal builds incrementally update targets when source files, headers, or the Makefile change.

## Keyboard Shortcuts Shared with CMake Tools

The guides include a shared user shortcut template: see [Windows section 8.8](makefile-tools-configuration-guide/windows-makefile-tools-development-environment-guide.md#88-user-keyboard-shortcut-template) or [Linux section 9.8](makefile-tools-configuration-guide/linux-makefile-tools-development-environment-guide.md#98-user-keyboard-shortcut-template). Merge it into VS Code's user keyboard shortcuts to select the appropriate commands by workspace conditions. Section 6.3 of each guide explains the command IDs, conditions, and configuration entry point.

| Shortcut | Purpose |
| --- | --- |
| F4 | Select a launch program independently of the build target |
| F6 | CMake deletes its cache and configures again; Makefile Tools clears its analysis state and analyzes again |
| F7 | Build the current target or scope |
| Shift+F7 | CMake selects a target and builds it for this invocation; Make saves the current build target, then F7 builds it |
| Shift+F5 / Ctrl+Shift+F5 | When no debug session is running, debug / run the launch program directly through the extension |
| F5 / Ctrl+F5 | Debug / run without debugging using the selected `launch.json` entry |
| Alt+S | Restart clangd |
| Ctrl+K, Ctrl+A / Ctrl+K, Ctrl+Z | Open user settings / keyboard shortcuts as JSON |

For Make, F7 uses the extension's current build target, while Ctrl+Shift+B, F5, and Ctrl+F5 still use the example task that builds `all`. Direct debugging through the extension uses GDB by default; to use the CodeLLDB entry selected for F5, continue using F5. If both tools are enabled for the same project, shared shortcuts give CMake priority. During a debug session, Shift+F5 and Ctrl+Shift+F5 retain their Stop / Restart actions.

## Directory Structure

```text
makefile-tools-vscode-cpp-development-environment/
├── README.md
├── makefile-tools-configuration-guide/
│   ├── windows-makefile-tools-development-environment-guide.md
│   └── linux-makefile-tools-development-environment-guide.md
├── example-windows/
│   ├── Makefile
│   ├── .vscode/
│   ├── .clang-format
│   ├── .gitignore
│   ├── include/demo/math.hpp
│   └── src/
└── example-linux/           # Same source as the Windows example, with Linux tool settings
```

Each example directory is an independent project. For an existing Makefile project, follow the guide to adapt its actual Make arguments and targets while retaining its original build rules, dependencies, and C++ standard.
