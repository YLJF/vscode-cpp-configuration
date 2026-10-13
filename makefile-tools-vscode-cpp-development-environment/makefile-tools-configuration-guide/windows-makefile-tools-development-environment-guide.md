# VS Code C++ Development Environment Guide for Windows with Makefile Tools

## How to use this guide

### Scope and prerequisites

This guide covers native Windows GNU/MinGW toolchains (MSYS2 MINGW64 or UCRT64). It uses **Makefile Tools** to manage GNU Make projects, clangd for language services, and GDB or CodeLLDB for debugging. The accompanying C++20 project has two executable targets to demonstrate switching build configurations and debugging multiple programs.

| Item | Requirement or choice |
| --- | --- |
| Operating system and permissions | 64-bit Windows meeting the system requirements of VS Code and MSYS2; permission to install software and modify the user PATH |
| Build tool | `mingw32-make`; the accompanying Makefile uses the Windows command environment |
| Compiler | GCC or Clang supported by your project; the accompanying example requires C++20 support |
| Language services | Install both the clangd extension and the clangd executable; the extension must support `${command:...}` command variables in its arguments |
| Debugger | GDB through the C/C++ extension, or LLDB through CodeLLDB |
| Existing projects | Follow the project build instructions to prepare dependencies, the environment, and the Makefile before connecting the editor |

The installation commands cover both compilers and debugging options offered by the example; install the combinations you need. For existing projects, follow their requirements for the C++ standard, tool versions, dependencies, and build rules.

### First-time setup

For your first setup, start with **GCC + Debug + GDB** and the `gcc-debug` configuration: prepare the tools and extensions, understand the project files, then complete a build and a debugging session with a breakpoint. Once that workflow works, switch to Clang, Release, or CodeLLDB as your project requires.

- **Use the accompanying example:** Open `../example-windows/` as its own VS Code workspace and follow Sections 1-4 in order. All complete files are collected in [Section 8](#8-complete-configuration-reference).
- **Create a new Makefile project:** Read Section 3 to understand how the files fit together, then create the project files from the complete configuration reference and adapt the Makefile to your targets and dependencies.
- **Connect an existing project:** Read its own build instructions first, prepare the environment using Sections 1-2, then merge the required settings as described in [Section 3.7](#37-connect-an-existing-makefile-project).

After your first debugging session, use Section 6 for everyday changes and configuration switching. Custom keyboard shortcuts are optional; the Command Palette provides the same operations.

### Contents

- [1. Install the toolchain](#1-install-the-toolchain)
- [2. Configure VS Code extensions](#2-configure-vs-code-extensions)
- [3. Project structure and configuration](#3-project-structure-and-configuration)
- [4. Configure, build, and debug](#4-configure-build-and-debug)
- [5. Language services and formatting](#5-language-services-and-formatting)
- [6. Everyday operations and keyboard shortcuts](#6-everyday-operations-and-keyboard-shortcuts)
- [7. Troubleshooting](#7-troubleshooting)
- [8. Complete configuration reference](#8-complete-configuration-reference)

### Command and terminology conventions

- Unless stated otherwise, run project commands from the project root containing `Makefile`. Existing projects use their original working directory.
- Run `bash` installation commands in the selected MSYS2 terminal and `powershell` project commands in PowerShell.
- Values such as `<project-root>` and `<host-address>` are placeholders. Replace the entire placeholder, including its angle brackets, with the actual value.
- Configure means that Makefile Tools analyzes Make commands and extracts compilation information and targets. Build means that Make compiles and links the project. Configure does not generate the project Makefile; projects using Autotools or similar systems must still perform their own preparation steps first.
- The build configuration determines Make arguments, the build target determines which outputs are updated, and the launch configuration determines which program runs. Commands and settings retain their original VS Code names so that you can search for them in the Command Palette and Settings.

## 1. Install the toolchain

VS Code invokes Make, the compiler, and the debugger through extensions. First install the tools your project needs and make them discoverable to the editor when it starts from Windows.

### 1.1 Select and install MSYS2 tools

Install MSYS2 from the [official website](https://www.msys2.org/) and select MINGW64 or UCRT64 according to your project requirements. Use compiler, debugger, and binary dependency packages from the same compatible environment.

| Environment | Windows tool directory | Package prefix |
| --- | --- | --- |
| MINGW64 | `<MSYS2-install-directory>/mingw64/bin` | `mingw-w64-x86_64-` |
| UCRT64 | `<MSYS2-install-directory>/ucrt64/bin` | `mingw-w64-ucrt-x86_64-` |

Update the system in an MSYS2 terminal. If prompted to close the terminal, reopen it as instructed and continue the update:

```bash
pacman -Syu
```

Then install the tools in the terminal for your selected environment. The following example uses MINGW64; change the package prefix for UCRT64:

```bash
PACKAGE_PREFIX=mingw-w64-x86_64-
pacman -S --needed \
  "${PACKAGE_PREFIX}gcc" \
  "${PACKAGE_PREFIX}make" \
  "${PACKAGE_PREFIX}gdb" \
  "${PACKAGE_PREFIX}clang" \
  "${PACKAGE_PREFIX}clang-tools-extra"
```

`gcc` provides GCC/G++, `clang` provides Clang/Clang++, and the selected environment's `make` package provides `mingw32-make`. You can also obtain the clangd executable through the extension's installation feature. If you use only one compiler or debugger, install just the required combination.

### 1.2 Configure the Windows PATH

Add the selected environment's `bin` directory to your Windows user PATH, for example `<MSYS2-install-directory>/mingw64/bin`. After saving, fully exit and reopen VS Code and your terminals so that they inherit the new environment.

Check the tool locations in a newly opened PowerShell terminal:

```powershell
Get-Command mingw32-make, g++, clang++, clangd, gdb
```

Check the tools your chosen setup actually uses. If multiple programs share a name, confirm that the expected toolchain is found first. When a tool is not on PATH, specify its actual location in user settings or in the environment used by the project. Use a Clang build targeting Windows GNU/MinGW, and keep tools and dependency libraries compatible.

### 1.3 Establish the project command environment

The Windows branch of the accompanying Makefile uses `cmd.exe` for directory commands and the Windows PowerShell supplied with the operating system to resolve the compiler's full path. Use MSYS2 terminals to install packages and PowerShell to run project commands.

If an existing Makefile relies on Unix commands such as `sh`, `rm`, or `sed`, prepare the compatible environment required by that project. For builds that run on Linux, use WSL or Remote-SSH and consult the Linux guide.

CodeLLDB normally supplies its own compatible LLDB backend. Install command-line LLDB tools for your selected environment only if you also need to run LLDB directly in a terminal.

## 2. Configure VS Code extensions

Once the toolchain is ready, connect it to the editor. Install Makefile Tools and clangd from the Extensions view, then choose C/C++ or CodeLLDB according to your debugger.

| Extension | Identifier | Purpose | When to install |
| --- | --- | --- | --- |
| Makefile Tools | `ms-vscode.makefile-tools` | Analyze Makefiles, select build configurations and targets, and export the compilation database | For the Make integration used in this guide |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | Completion, diagnostics, navigation, and formatting | For the language services used in this guide |
| C/C++ | `ms-vscode.cpptools` | Provide the `cppdbg` debug adapter | When using GDB |
| CodeLLDB | `vadimcn.vscode-lldb` | Provide the `lldb` debug adapter and a compatible LLDB backend | When using LLDB |

The clangd extension needs an executable clangd program and must support `${command:...}` command variables in its arguments. Use the extension's installation feature or set `clangd.path` in VS Code user settings to point to an existing executable. Keep personal installation paths in user settings.

For CodeLLDB, wait for its platform components to finish installing and normally retain the default backend. If you need another backend, configure a compatible `lldb.library` dynamic library path as described in the [CodeLLDB manual](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md#using-alternate-lldb-backends).

In this setup, clangd performs C++ language analysis. The workspace settings disable C/C++ IntelliSense while retaining the C/C++ extension's GDB debugging support. You can install both debugger extensions and select the appropriate launch entry when needed.

## 3. Project structure and configuration

With the environment and extensions ready, you still need to tell them how to build, analyze, and debug this project. The Makefile defines sources, targets, and build rules; the `.vscode` files hold extension settings, build tasks, and debugging configurations so that the editor can invoke the tools installed earlier. This section follows the relationships between those files. Complete contents are collected in [Section 8](#8-complete-configuration-reference).

For the accompanying example, open `example-windows/` as its own VS Code workspace. For a new project, create the files in the layout below. Existing projects retain their own directories and build rules and follow Section 3.7.

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

Build outputs, the compilation database, and extension logs are generated later under `build/<configuration-name>/`. The tools create this directory separately on each host. User keyboard shortcuts live in the VS Code user configuration.

### 3.1 Example source files

The example uses two independent `main()` entry points and a shared addition function to demonstrate a project with multiple programs. See [Section 8.1](#81-example-source-files) for the complete source code.

| File | Target or purpose | Example behavior |
| --- | --- | --- |
| `src/main.cpp` | Entry point for `example_app` | Calculate `2 + 3` and print the result and the build-type macro |
| `src/calc.cpp` | Entry point for `demo_calc` | Calculate `12 + 30` by default, or accept two integer arguments |
| `src/math.cpp` | Implementation shared by both targets | Calculate and return the result in `demo::add()` |
| `include/demo/math.hpp` | Shared header | Declare `demo::add()` |

Each program has its own entry point, and the shared implementation is linked into both targets. During debugging, inspect the local variable `result` at the `return result;` line in `src/math.cpp`. First, establish the relationship between these sources and targets in the Makefile.

### 3.2 `Makefile`

Create `Makefile` in the project root; its complete contents appear in [Section 8.2](#82-makefile). It defines `example_app`, `demo_calc`, and the aggregate target `all`, and uses `CONFIG` to select the compiler and build type.

| Configuration name | Compiler | Build type | Output directory |
| --- | --- | --- | --- |
| `gcc-debug` | G++ | Debug | `build/gcc-debug/` |
| `gcc-release` | G++ | Release | `build/gcc-release/` |
| `clang-debug` | Clang++ | Debug | `build/clang-debug/` |
| `clang-release` | Clang++ | Release | `build/clang-release/` |

These four names are parameter values defined by the example Makefile. `all` builds both programs; specifying `example_app` or `demo_calc` updates only that target. Executables are placed in the current configuration's `bin/` directory as `example_app.exe` and `demo_calc.exe`.

The Makefile resolves the selected compiler's full path from PATH so that the compilation database exported by Makefile Tools records the actual driver used. The example uses C++20 and common warning flags, with `-O0 -g` for Debug and `-O2` for Release. Existing projects retain their own standard, optimization flags, and dependencies.

`-MMD -MP` generates header dependencies, and Make reads the resulting `.d` files on subsequent builds. Object files depend on their sources and the Makefile; programs also depend on their object files and the Makefile. Changes to sources, headers, or build rules therefore trigger the appropriate updates. Each configuration has its own directory; Section 6.2 explains everyday incremental builds.

When copying the Makefile, preserve the leading **Tab** on recipe command lines. These are Make recipes, and ordinary spaces cannot simply replace that indentation.

### 3.3 `.vscode/settings.json`

The Makefile now defines how to build both programs, but the extension also needs to know which directory and arguments to use. Workspace `settings.json` stores that information and also supports language services and program selection. See [Section 8.3](#83-vscodesettingsjson) for the complete file. Its main relationships are listed below.

| Setting | Purpose |
| --- | --- |
| `makefile.makePath` | Invoke GNU Make using `mingw32-make` |
| `makefile.makeDirectory` / `makefile.makefilePath` | Set the working directory and entry Makefile |
| `makefile.configurations` | Map build configuration names to Make arguments, such as `CONFIG=gcc-debug` |
| `makefile.extensionOutputFolder` | Store extension output under the current configuration's build directory and create the parent directory needed for the database |
| `makefile.compileCommandsPath` | Export the current configuration's `compile_commands.json` to its corresponding directory |
| `clangd.arguments` | Read the current configuration's database and restrict the compiler drivers that clangd may query |
| `makefile.defaultLaunchConfiguration` | Explicitly use GDB for the extension's own debug command |

In Makefile Tools settings, `${configuration}` is the current build configuration name; clangd arguments use `${command:makefile.getConfiguration}` to obtain the same name. The respective extensions expand these values, keeping the database export and read directories aligned. Section 5 covers the language-service settings in detail.

The example disables automatic Configure and follows the explicit analysis and build steps in Section 4. The Makefile defines program names and output locations, and the template does not prepopulate `makefile.launchConfigurations`. Configure analyzes compilation and linking commands to discover executable outputs. After building, select the program to launch for the current configuration from those results; the debugging configuration then reads its path. Adding a program usually requires only a Makefile change and a new analysis, without duplicating program names in editor settings.

After you select a program, the extension writes its launch entry back to workspace `settings.json`; that entry may contain paths specific to your machine. This is the normal way it remembers the launch selection. After moving the project, changing hosts, or switching build configurations, analyze the project again and select the program for the current environment. In a complex project where an output is not discovered, or when you need custom working directories or arguments for the extension's own Run / Debug commands, add `makefile.launchConfigurations` as needed and set `binaryPath` to the actual executable path.

### 3.4 `.vscode/tasks.json`

The workspace settings let Makefile Tools invoke Make. Next, define a build task in `tasks.json` for both VS Code's Ctrl+Shift+B and the pre-debug build, so that the two operations use the same command. The complete file appears in [Section 8.4](#84-vscodetasksjson).

The task uses `type: process` to invoke `${config:makefile.makePath}` directly, reads the current configuration through `${command:makefile.getConfiguration}`, and passes `CONFIG=<configuration-name>` to Make. It always builds `all`, ensuring that either selected example program is updated. The build target selected in the extension does not change this task's `all` argument.

`options.cwd` sets the project root, and the `$gcc` problem matcher links compilation errors to source code. The example uses `-j` without a number, which does not limit the number of parallel jobs. For larger projects, choose `-j<N>` according to CPU and memory capacity, for example `-j4`.

When connecting an existing project, match the task's directory, entry file, variables, and target to the actual project command. The `makeArgs` in `makefile.configurations` are not automatically passed to this custom task.

### 3.5 `.vscode/launch.json`

The build task updates the program, and `launch.json` then specifies the debugger and how to launch the selected program. `preLaunchTask` connects the two so that F5 builds first and then starts debugging. The complete file in [Section 8.5](#85-vscodelaunchjson) provides GDB and CodeLLDB entries.

| Setting | Role |
| --- | --- |
| `program` | `${command:makefile.getLaunchTargetPath}` reads the path of the program selected in Makefile Tools |
| `cwd` | `${workspaceFolder}` makes F5 run the program from the project root |
| `preLaunchTask` | Run `Makefile: build active configuration` before starting the debugger |
| `args` | Store the arguments passed to the program launched by F5 |
| `type: cppdbg` / `MIMode: gdb` | Invoke GDB through the C/C++ extension |
| `type: lldb` | Invoke LLDB through CodeLLDB |

F5 runs the build task first. Make uses dependencies and timestamps to update incrementally: unchanged outputs are reused, while affected targets are recompiled and relinked when necessary. Section 6.2 describes the full behavior. If the build fails, fix the errors reported by the task before debugging again; avoid using "Debug Anyway" to run an old executable.

`getLaunchTargetPath` reads the program already selected for launch; `preLaunchTask` performs the build. The `all` build target and the launch program are independent selections, so the task can build every program while F5 debugs the selected one. Do not assemble the executable path from `${buildTarget}`: `all` is an aggregate target with no executable of its own.

In this guide, F5 uses the project-root `cwd` and `args` in `launch.json`. The extension's own Debug / Run commands instead use the launch entry's `cwd` and `binaryArgs`, with `makefile.buildBeforeLaunch` controlling the pre-launch build. An automatically discovered entry may use `bin/` as its working directory; adjust it if the program reads resources relative to the project. Makefile Tools' `MIMode: lldb` uses the C/C++ MI debugging interface, whereas CodeLLDB uses a separate `type: lldb` entry.

### 3.6 Extension recommendations, formatting, and ignore rules

Once the project files are in place, add the team's shared editing conventions. `.vscode/extensions.json` recommends the required extensions, `.clang-format` stores formatting rules, and `.gitignore` excludes build outputs, the database, and extension logs. Complete files appear in Sections 8.6-8.7.

The workspace settings make clangd the default C++ formatter and enable format on save. Existing projects use their own formatting rules; preserve existing extension recommendations and ignore entries when merging settings.

### 3.7 Connect an existing Makefile project

First build the existing project once in a terminal according to its documentation, confirming the working directory, Make executable, entry file, variables, and target. Then connect that actual command to the editor while retaining the project's sources, standard, dependencies, and build rules.

1. **Identify the Make entry point.** Set the actual executable in `makefile.makePath`, the original command's working directory in `makefile.makeDirectory`, and the entry file in `makefile.makefilePath`.
2. **Save build combinations.** Store the original command's arguments in `makefile.configurations` under `makeArgs`, for example `MODE=debug` or `CXX=clang++`. The configuration name is a selection label in the extension; the project's parameters determine whether and how that name is passed to Make.
3. **Analyze the required targets.** Select a build target covering the relevant sources and programs, such as the project's `all`, and run `Makefile: Clean Configure`. Check the compiler, macros, standard, and include directories in the database, and confirm that the extension discovers executables from the linking commands.
4. **Confirm the launch program.** After building, run `Makefile: Set the make launch configuration` and select the current configuration's program from the discovered results. Start with automatic discovery. Add `makefile.launchConfigurations` only if discovery cannot identify the program or you need custom working directories and arguments for extension launches.
5. **Connect the F5 build.** Merge `tasks.json` and `launch.json`, adapting the task's Make executable, directory, entry file, variables, and target to the original command. The example's `CONFIG=${command:makefile.getConfiguration}` is specific to its naming convention; existing projects use their own mapping.
6. **Connect language services and debugging.** Merge the clangd arguments, run Configure again, and restart clangd. After building, select the launch program and debugger, set a breakpoint in the source code, and press F5.

If the project first needs `configure`, SDK initialization, or Makefile generation, perform its original preparation steps. You can launch VS Code from a terminal with the environment already initialized or use `makefile.preConfigureScript` as needed. The script type and execution location must match the project platform.

Configure needs complete compilation and linking commands to generate the language-service database and discover launch programs. If the Makefile hides commands, use the project's verbose-output options. You can also read a build log through `makefile.buildLog`. The log must match the current arguments and include the required sources and executable linking steps; an ordinary incremental build log may be incomplete. Section 7 covers analysis problems.

### 3.8 Maintain configurations and package releases

After integration, maintain shared project settings separately from personal environment settings so that other developers can reuse the configuration and release packages have a clear scope.

| Content | Maintenance and packaging |
| --- | --- |
| Sources, headers, Makefile, and dependency instructions | Keep under project version control and include according to the project's delivery requirements |
| Portable `.vscode` files, `.clang-format`, and `.gitignore` | Keep under version control; use workspace variables for shared paths and preserve project conventions |
| Launch entries written back automatically by the extension | Remove entries containing machine-specific paths from portable templates and let users analyze and select again; retain deliberately maintained custom entries according to project conventions |
| `build/`, `.cache/`, compilation databases, and extension logs | Exclude through ignore rules and generate in the user's current environment |
| Personal tool paths, SSH connections, and private keys | Keep in the user environment or user configuration |
| User keyboard-shortcut template | Provide with the guides for users to merge into their user `keybindings.json` |

When publishing the guides and examples, include both guides, the accompanying sources, Makefile, and portable settings; exclude local build outputs and check whether `settings.json` contains machine-specific launch paths written back by the extension. Remove those generated entries from portable templates, while retaining deliberately configured working directories, arguments, and other custom entries according to project conventions. `.gitignore` affects version control; it does not filter manual copies or archives or prevent the extension from rewriting shared settings, so inspect the actual package contents. Existing projects define their own formal release scope.

## 4. Configure, build, and debug

You can now use these settings for a real build and debugging session: select build arguments, let the extension analyze compilation commands, then build the programs and select the one to debug. Run the following commands from the Command Palette (Ctrl+Shift+P). Interface translations may vary; search using the English names.

### 4.1 First configuration, build, and debugging session

Makefile Tools maintains three independent selections. Before proceeding, confirm what each one controls.

| Selection | Command | What it controls |
| --- | --- | --- |
| Build configuration | `Makefile: Set the current build configuration` | Arguments passed to Make, such as `CONFIG=gcc-debug` |
| Build target | `Makefile: Set the target to be built by make` | The target used by the extension's Configure / Build operations, such as `all` |
| Launch configuration | `Makefile: Set the make launch configuration` | The selected executable; its working directory and arguments are used by the extension's own Run / Debug commands |

For the first run, use `gcc-debug` and keep the build target at `all` so that Configure collects compilation and linking information for both programs. Then select the launch program separately: `all` builds every target, while `example_app` or `demo_calc` determines which program is debugged. Neither selection replaces the other.

1. Run **`Makefile: Set the current build configuration`** and select `gcc-debug`.
2. Run **`Makefile: Set the target to be built by make`** and select `all`.
3. Run **`Makefile: Clean Configure`** and wait for the extension to analyze compilation and linking commands, export the database, and discover executables.
4. Run **`clangd: Restart language server`** so that language services read the current database.
5. Run **`Makefile: Build the current target`** to build both programs.
6. Run **`Makefile: Set the make launch configuration`** and select `example_app` from the discovered list, checking that its path belongs to the current `build/gcc-debug/bin/`. If you installed the user shortcuts in Section 6.3, press F4 instead.
7. Set a breakpoint at `return result;` in `src/math.cpp`, select **`Makefile / GDB`** in the Run and Debug view, and press F5.

When execution stops at the breakpoint, inspect parameters `a` and `b` and the local variable `result`. Continue execution and `example_app` prints `2 + 3 = 5`. Subsequent F5 launches reuse the current valid launch selection and perform the incremental pre-launch build described in Section 3.5. It is normal for `settings.json` to contain the program path saved by the extension after selection; the project template does not need to list these paths in advance.

### 4.2 Build and run from the command line

You can invoke the same Makefile directly from the project root. The commands below build both `gcc-debug` programs and then run them:

```powershell
mingw32-make CONFIG=gcc-debug all -j2
./build/gcc-debug/bin/example_app.exe
./build/gcc-debug/bin/demo_calc.exe 10 20
```

The second command runs the default example, and the third makes `demo_calc` calculate `10 + 20 = 30`. Replace the `CONFIG` value for other combinations. To build a single program, replace `all` with its target.

A command-line build does not update the Makefile Tools compilation database. Build directly when source implementation changes leave compilation arguments unchanged. After changes to the Makefile, source list, or compiler flags, run Configure again in the editor and restart clangd.

### 4.3 Select GDB or CodeLLDB

Select the program and the debugger separately. Keep the program selected in Makefile Tools, choose the required entry in the Run and Debug view, then press F5.

| Debug entry | Extension and backend | Key points |
| --- | --- | --- |
| `Makefile / GDB` | C/C++ extension and GDB | Set `miDebuggerPath` to a compatible GDB executable |
| `Makefile / LLDB (CodeLLDB)` | CodeLLDB and its LLDB backend | Wait for the platform components to install and use `type: lldb` |

Both GCC and Clang can work with a suitable GDB or LLDB. Compatibility depends on the platform, binary output, and debug information. Choose a Debug configuration when you need reliable breakpoints and variable inspection.

### 4.4 Debug multiple executables

To switch programs, end the current debugging session and run **`Makefile: Set the make launch configuration`** (command ID: **`makefile.setLaunchConfiguration`**). Select `demo_calc` for the current build configuration from the analysis results, then press F5. It prints `12 + 30 = 42` by default; the build target can remain `all`. Subsequent debugging sessions reuse this launch selection without requiring you to select it again every time.

Both example programs run from the project root without arguments by default, so they share the same GDB / CodeLLDB entries and the program path follows the launch selection. To pass arguments to `demo_calc`, enter them in the debug entry's `args`, for example `["10", "20"]`. These arguments remain active if you select another program, so adjust them as needed. Keep separate debug entries when programs need different arguments, environment variables, or working directories.

The extension's own Debug / Run commands read arguments from `binaryArgs` in `makefile.launchConfigurations`, while F5 reads `args` from `launch.json`. A string returned by a command variable does not automatically expand into multiple array arguments; enter each argument as a separate element in the appropriate array.

To add an executable, declare its sources and linking rules in the Makefile and add its target to `all`. Run Clean Configure and Build again, select the new program with **F4** (after the setup in Section 6.3) or the Command Palette, then press F5. You do not need to write a `binaryPath` into the template for every new program.

### 4.5 Switch build configurations

To use Clang or another build type, end debugging first, then align the build, language services, and launch program with the same configuration:

1. `Makefile: Set the current build configuration`: select, for example, `clang-debug`.
2. `Makefile: Set the target to be built by make`: keep or select `all`.
3. `Makefile: Clean Configure`: analyze the current configuration again, export its database, and discover its executable outputs.
4. `clangd: Restart language server`: switch language services to the appropriate database.
5. `Makefile: Build the current target`: create the programs for the new configuration.
6. `Makefile: Set the make launch configuration`: select a program whose path belongs to the new configuration's build directory, then press F5.

The extension may retain launch entries selected earlier. After switching, check the program path and select again to ensure that you launch the new configuration's output. Release is primarily for optimized builds; use the corresponding Debug output for debugging.

## 5. Language services and formatting

The build and debugger now use the current configuration; code editing needs the same compilation information. The database exported by Configure lets clangd analyze sources with the actual standard, macros, and include paths, while project formatting rules keep code layout consistent. Connect both here to get completion, diagnostics, and navigation as you edit.

### 5.1 Make clangd read the current configuration

`compile_commands.json` records the compiler, standard, macros, and include paths. Makefile Tools exports it under `build/<configuration-name>/`, and clangd reads the directory for the current configuration using this argument:

```text
--compile-commands-dir=${workspaceFolder}/build/${command:makefile.getConfiguration}
```

Keep the database in the build directory. After switching configurations, run Configure again and restart clangd so that command variables in the startup arguments are reevaluated. Analyze again when adding sources or changing macros or include directories. Changes limited to existing function implementations usually do not require a database update.

The target used for Configure must cover the sources you need to analyze. Keep the example at `all`. In a larger existing project, select the required scope and verify that the exported database contains the corresponding compilation commands.

### 5.2 Restrict `--query-driver`

`--query-driver` allows clangd to query matching compilers for target-platform and standard-library search paths. The example permits `**/mingw64/bin/gcc.exe,**/mingw64/bin/g++.exe,**/mingw64/bin/clang.exe,**/mingw64/bin/clang++.exe`.

Separate multiple patterns with commas inside **one argument**, matching only the toolchains you use. For UCRT64, change the directory patterns to `ucrt64/bin`; adapt custom installations or other layouts to their actual driver paths. Also check that the database compiler matches the build. After path or version changes, run Configure again and restart clangd.

### 5.3 Use project formatting rules

The workspace settings make clangd the default C++ formatter and enable format on save. `.clang-format` in the project root defines indentation and line width. The example uses LLVM style, four-space indentation, and a 100-column limit.

Open `src/main.cpp`, use completion to explore interfaces, press F12 to jump to a declaration or definition, and check diagnostics in the Problems panel. For manual formatting, run `Format Document` and confirm that the current file uses the clangd formatter. Existing projects retain their own formatting rules.

### 5.4 Inspect language-service logs

If the program builds but the editor reports missing headers, open the Output panel and select **clangd**. Check the server version, startup arguments, and compilation database directory in the logs, then verify that the current source's compilation command contains the correct compiler, standard, macros, and include paths.

After switching build configurations, `--compile-commands-dir` in the log should point to the new configuration directory. If it still points to the old one, confirm that Configure has completed for the new configuration, then run `clangd: Restart language server`. Follow the same sequence after fixing tool paths or `--query-driver`.

## 6. Everyday operations and keyboard shortcuts

Once language services match the build configuration, everyday development usually involves saving code and building. When changes affect compiler arguments, build rules, or the program to run, add analysis or program-selection steps as needed. The following common changes explain which operations to perform.

### 6.1 Save, build, and debug

After saving changes, run `Makefile: Build the current target`, or press F7 after merging the shared shortcut template in Section 6.3. The extension's Build uses the currently selected build target. To change that scope, use Shift+F7 to select and save a target, then press F7. Ctrl+Shift+B, F5, and Ctrl+F5 use this guide's `tasks.json` task, which always builds `all` for the current configuration regardless of the extension's build-target selection.

To change only the launch program, run `Makefile: Set the make launch configuration` (F4 after configuring user shortcuts). F5 then debugs with the selected `launch.json` entry, while Ctrl+F5 runs that entry without debugging. Both launch from the project root and read arguments from `args`. You do not need to select the program again while the build configuration and launch selection remain valid.

To use the extension directly before debugging has started, Shift+F5 runs `Makefile: Debug the selected binary target`, and Ctrl+Shift+F5 runs `Makefile: Run the selected binary target in the terminal`. Both use the extension launch entry's `cwd` and `binaryArgs`. The example's extension debugger defaults to GDB; choosing a CodeLLDB entry in Run and Debug for F5 does not change that. For CodeLLDB or consistent project-root working directories and arguments, continue using the F5 / Ctrl+F5 `launch.json` workflow.

### 6.2 Incremental builds and reanalysis

Make determines what to update from dependencies and file timestamps. In the example, object files and programs depend on the Makefile, so build-rule changes also trigger rebuilding.

| Change | Build behavior in the example | Run Configure again? |
| --- | --- | --- |
| Implementation of a `.cpp` file | Recompile that file and relink programs that depend on it | Usually unnecessary if compiler flags are unchanged |
| Shared header | Use `.d` files to recompile sources that include it, then relink affected targets | Usually unnecessary if include directories, macros, and similar settings are unchanged |
| Makefile rules or compiler flags | Makefile dependencies trigger object recompilation and relinking | Yes; update the compilation database and restart clangd |
| Add or remove source files or executable targets | Build targets according to the updated Makefile | Yes; rediscover programs, select again after Build, and remove obsolete launch entries as needed |
| Switch GCC / Clang or Debug / Release | Use that configuration's separate output directory | Yes; restart clangd and select the launch configuration again |
| No changes | Check dependencies and reuse existing outputs | No |

This behavior depends on the dependencies actually declared in the Makefile; existing projects follow their own rules. When compiler flags are overridden temporarily on the command line, Make generally does not treat argument changes as file dependencies. Create separate configurations for common combinations, or clean and rebuild the affected configuration, then run Configure with the same arguments in the extension.

**`Makefile: Clean Configure` clears the extension's analysis state and analyzes the project again; the project's `clean` target removes build outputs.** The accompanying Makefile's `clean` removes only the current `CONFIG` directory. When cleanup is needed, run this from the project root:

```powershell
mingw32-make CONFIG=gcc-debug clean
```

That directory also contains the compilation database and extension output. After cleaning, run Configure again, restart clangd, and then Build to restore the development environment for the current configuration.

### 6.3 Custom keyboard shortcuts

When switching between CMake and Make projects, similar operations can use the same keys. Run `Preferences: Open Keyboard Shortcuts (JSON)` and merge the complete template from [Section 8.8](#88-user-keyboard-shortcut-template) into your user array. Workspace conditions select the appropriate commands, so you do not need to replace bindings manually when changing projects.

| Shortcut | CMake Tools | Makefile Tools |
| --- | --- | --- |
| F4 | Select the launch target (`cmake.selectLaunchTarget`) | Select the launch program (`makefile.setLaunchConfiguration`) |
| F6 | Delete the CMake configuration cache and configure again (`cmake.cleanConfigure`) | Clear extension analysis state and analyze again (`makefile.cleanConfigure`) |
| F7 | Build the current scope (`cmake.build`) | Build the current target (`makefile.buildTarget`) |
| Shift+F7 | Select a target and perform this build (`cmake.buildWithTarget`) | Change the current build target (`makefile.setBuildTarget`), then press F7 to build |
| Shift+F5 | Debug the launch target directly through the extension (`cmake.debugTarget`) | Debug the launch program directly through the extension (`makefile.launchDebug`) |
| Ctrl+Shift+F5 | Run the launch target directly through the extension (`cmake.launchTarget`) | Run the launch program in a terminal through the extension (`makefile.launchRun`) |
| F5 / Ctrl+F5 | Start the selected `launch.json` configuration with / without debugging in VS Code | Start the selected `launch.json` configuration with / without debugging in VS Code |
| Alt+S | Restart clangd (`clangd.restart`); shared by both project types | Same |
| Ctrl+K, Ctrl+A | Open user settings JSON (`workbench.action.openSettingsJson`) | Same |
| Ctrl+K, Ctrl+Z | Open user keyboard-shortcut JSON (`workbench.action.openGlobalKeybindingsFile`) | Same |

Shift+F7 differs in its sequence: CMake builds immediately after target selection without changing the long-term default build scope. Makefile Tools only changes and saves the current build target, so press F7 afterward. For example, after selecting `demo_calc` in a Make project, subsequent F7 builds continue to use that target; select `all` again to build every program. F4 still selects the launch program independently.

F6 reconfigures both project types, but it processes different data. CMake deletes its configuration cache and regenerates build rules from the current preset. Makefile Tools clears the extension's analysis state and rereads compilation and linking commands without deleting object files or executables. To remove Make build outputs, run the project's own `clean` target. For ordinary source changes, save and press F7 or F5 for an incremental build.

Shift+F5 and Ctrl+Shift+F5 use the extension's own debugging / launch settings; direct debugging in the Make example defaults to GDB. F5 and Ctrl+F5 use the `launch.json` entry selected in Run and Debug, including its GDB / CodeLLDB choice, project-root working directory, and `args`. Section 6.1 explains both workflows.

Makefile Tools bindings in the template apply when `makefile:fullFeatureSet && !cmake:enableFullFeatureSet`; CMake bindings retain each command's native conditions. If both feature sets are enabled in one workspace, these keys favor CMake. Select Makefile Tools commands explicitly from the Command Palette when needed. CMake F7 / Shift+F7 retain `!cmake:hideBuildCommand`, and direct debug / run retain the extension's original project and command-visibility conditions.

Direct debug / run bindings for both extensions include `!inDebugMode`. Once debugging starts, Shift+F5 continues to stop debugging and Ctrl+Shift+F5 continues to restart it. F5 and Ctrl+F5 keep the VS Code default bindings; the template does not override them.

Alt+S restarts clangd for the current workspace. After switching a build configuration or Configure Preset, complete analysis / configuration before restarting language services. Enter the Ctrl+K shortcuts as two successive key combinations: Ctrl+K first, then Ctrl+A or Ctrl+Z. Shortcuts belong in the user configuration; the project does not need `.vscode/keybindings.json`. Preserve existing bindings when merging the template, and check matching keys and `when` conditions for conflicts in Keyboard Shortcuts.

## 7. Troubleshooting

When something goes wrong, first check whether the original Make command works in a terminal, then check extension analysis, database reading, and program selection. Build errors appear in the task terminal; analysis issues appear in the Makefile Tools or clangd logs in the Output panel.

| Symptom | First checks and actions |
| --- | --- |
| The extension does not recognize the Makefile | Check the opened directory, `makefile.makeDirectory`, and `makefile.makefilePath`; generated projects must complete their original preparation steps first |
| Make or the compiler cannot be found | Check actual paths and the PATH inherited by VS Code; fully restart VS Code after environment changes |
| Configure succeeds but the database is missing or incomplete | Select a target covering the required sources, confirm that complete compilation commands are visible, then run Clean Configure; check the export path and parent directory |
| Standard-library header errors | Check the database's compiler path, target platform, and `--query-driver`; confirm that clangd reads the current configuration directory |
| Language services use old arguments after switching configurations | Run Configure for the current configuration, then restart clangd to reevaluate command variables |
| F5 and the extension's Build use different arguments | Check the directory, entry file, variables, and target in `tasks.json`; custom tasks do not automatically inherit all `makefile.configurations` arguments |
| The required program is missing from the launch list | Select a target covering that program (`all` in the example), confirm complete linking commands are visible, then run Clean Configure and Build; add an explicit launch entry if it still is not discovered |
| F5 launches another configuration or program | Run `Makefile: Set the make launch configuration` again and verify that the program path belongs to the current configuration; remove generated entries invalidated by environment changes |
| Selecting build target `all` leads to an attempt to run `bin/all` | Check whether `${buildTarget}` was incorrectly used to construct the program path; restore `${command:makefile.getLaunchTargetPath}` and select a real program separately |
| Terminal Run and F5 look for relative resources in different directories | F5 runs from the project root; extension Run uses the launch entry's `cwd`, which automatic discovery may set to `bin/`; adjust it for the project |
| The build succeeds but breakpoints do not work | Use Debug and check the selected program, source location, debug information, and debugger compatibility |
| CodeLLDB cannot start | Confirm that platform components have finished installing and match the host architecture; check that the selected entry uses `type: lldb` and inspect debug output |
| A shortcut does nothing or invokes the other tool | Check user bindings, command IDs, `when` conditions, and conflicts; the template favors CMake when both tools are enabled, so run Make commands through the Command Palette |
| Shift+F7 selects a target without building | The Makefile Tools binding only saves the build target; press F7 afterward. The corresponding CMake command performs the build immediately |
| Shift+F5 uses GDB while F5 uses CodeLLDB | They use different entry points: direct extension debugging defaults to GDB, while F5 uses the selected `launch.json` entry; press F5 for CodeLLDB |

## 8. Complete configuration reference

The preceding sections explain each file and the operation sequence. This section collects the required settings and complete files for reference when creating a project. Existing projects merge the relevant parts using Section 3.7; Section 3.8 covers maintenance and packaging.

| Configuration location | Required setup |
| --- | --- |
| System environment | Install `mingw32-make`, the chosen compiler, clangd, and debugger; configure the project's PATH and dependencies |
| VS Code extensions | Install Makefile Tools, clangd, and C/C++ or CodeLLDB according to the debugger |
| VS Code user settings | Ensure that the extension can find clangd; set `clangd.path` if needed and keep personal absolute paths on the user side |
| Project sources | Keep a separate entry point for each program and build the shared headers and implementation as declared in the Makefile |
| Project root | Create or merge `Makefile`, `.clang-format`, and `.gitignore` |
| Project `.vscode/` | Create or merge `settings.json`, `tasks.json`, `launch.json`, and `extensions.json` |
| User shortcuts (optional) | Merge the shared CMake / Make template in Section 8.8 to use the same keys for target selection, configuration, builds, and direct debug / run; Section 6.3 explains their use |

The following files correspond to the accompanying `example-windows/` project. Adapt tool names, paths, targets, and compiler flags to your project, and retain either or both debugger entries as needed.

| File | Save location | Accompanying file |
| --- | --- | --- |
| `main.cpp`, `calc.cpp`, `math.cpp` | `src/` | [main.cpp](../example-windows/src/main.cpp), [calc.cpp](../example-windows/src/calc.cpp), [math.cpp](../example-windows/src/math.cpp) |
| `math.hpp` | `include/demo/` | [math.hpp](../example-windows/include/demo/math.hpp) |
| `Makefile` | Project root | [Makefile](../example-windows/Makefile) |
| `settings.json` | `.vscode/` | [settings.json](../example-windows/.vscode/settings.json) |
| `tasks.json` | `.vscode/` | [tasks.json](../example-windows/.vscode/tasks.json) |
| `launch.json` | `.vscode/` | [launch.json](../example-windows/.vscode/launch.json) |
| `extensions.json` | `.vscode/` | [extensions.json](../example-windows/.vscode/extensions.json) |
| `.clang-format`, `.gitignore` | Project root | [.clang-format](../example-windows/.clang-format), [.gitignore](../example-windows/.gitignore) |
| User shortcuts | VS Code user `keybindings.json` | [Merge the template](#88-user-keyboard-shortcut-template) |

### 8.1 Example source files

`src/main.cpp`:

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

`src/calc.cpp`:

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

`src/math.cpp`:

```cpp
#include "demo/math.hpp"

int demo::add(int a, int b) {
    const int result = a + b;
    return result;
}
```

`include/demo/math.hpp`:

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
  "editor.inlayHints.enabled": "off",
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

### 8.7 Formatting and ignore files

`.clang-format`:

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
```

`.gitignore`:

```gitignore
/build/
/.cache/
*.local.json
```

### 8.8 User keyboard-shortcut template

This complete template works with both CMake Tools and Makefile Tools and matches the CMake guides. Merge its entries into the user `keybindings.json` array while preserving existing bindings. Section 6.3 explains the operation mapping, the two Shift+F7 behaviors, and activation conditions.

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

F4 selects the launch program, F6 reconfigures / analyzes, and F7 builds. In Make projects, Shift+F7 only selects the build target; press F7 afterward. Before debugging begins, Shift+F5 and Ctrl+Shift+F5 invoke direct extension debug / run. F5 and Ctrl+F5 retain the `launch.json` workflow. When both tools are enabled, the template favors CMake.

## References

- [Makefile Tools](https://github.com/microsoft/vscode-makefile-tools): extension documentation, configuration, and troubleshooting.
- [GNU Make manual](https://www.gnu.org/software/make/manual/make.html): targets, variables, dependencies, and incremental builds.
- [clangd compile commands](https://clangd.llvm.org/design/compile-commands): compilation databases and compiler queries.
- [VS Code debugging configuration](https://code.visualstudio.com/docs/debugtest/debugging-configuration): launch configurations and build tasks.
- [CodeLLDB manual](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md): LLDB debugging configuration and backends.
