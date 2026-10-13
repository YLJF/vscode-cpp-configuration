# VS Code C++ Development Environment Guide for Linux

## How to Use This Guide

### Scope and Prerequisites

This guide covers local Linux development and development on Linux hosts through Remote-SSH. It uses **CMake Presets** to manage GCC/Clang builds with Ninja or Unix Makefiles, clangd for language services, and GDB or CodeLLDB for debugging. The sample project uses C++20 and two executable targets to demonstrate program selection and debugging.

| Component | Requirement or Choice |
| --- | --- |
| Operating system and permissions | A distribution, architecture, and system libraries meeting VS Code and toolchain requirements; permission to install packages and read/write the project directory |
| CMake | ≥ 3.21 for preset format version 3 and the accompanying generator script; `--fresh` additionally requires ≥ 3.24 |
| Compiler | A project-supported version of GCC or Clang that supports the required C++ standard |
| Build tool | `ninja` for Ninja presets; GNU Make (`make`) for Unix Makefiles presets |
| Debugger | Install the C/C++ extension for GDB or CodeLLDB for LLDB; install standalone command-line LLDB only if needed |
| clangd extension | ≥ 0.3.0 for `${command:...}` command variables; the extension and the clangd executable have separate versions |
| Toolchain compatibility | Compiler, standard library, debugger, and binary-dependency versions must be compatible with the target architecture |

Choose the compiler, build tool, and debugger required by your project before preparing the environment. The installation commands below cover both compilers and build backends; use the components you need. For an existing project, follow its build instructions for tool versions and dependency installation.

### First-Time Setup

Start with **GCC + Ninja + Debug + GDB** and use `gcc-ninja-debug` to complete a build and a debugging session. The guide proceeds from tool installation to project configuration, building, and debugging. You can switch to Clang, Make, or CodeLLDB afterward as needed.

- **Create the sample project:** Follow Sections 1–4 in order. Copy the complete files from [Section 9](#9-complete-configuration-reference), or generate the presets as described in Section 3.3.1.
- **Integrate an existing CMake project:** Read the project's build instructions first, prepare the environment using Sections 1–2, and merge the required configuration using [Section 3.7](#37-integrating-an-existing-cmake-project).
- **Remote-SSH:** Establish a remote workspace using [Section 8](#8-remote-ssh-development), then complete Sections 1–4 on the remote host.

After the first debugging session, use Section 6 for everyday changes and preset switching. Custom shortcuts are optional; every operation is also available from the Command Palette.

### Contents

- [1. Install the Toolchain](#1-install-the-toolchain)
- [2. Configure VS Code Extensions](#2-configure-vs-code-extensions)
- [3. Project Structure and Configuration](#3-project-structure-and-configuration)
- [4. Configure, Build, and Debug](#4-configure-build-and-debug)
- [5. Language Services and Formatting](#5-language-services-and-formatting)
- [6. Everyday Workflow and Shortcuts](#6-everyday-workflow-and-shortcuts)
- [7. Troubleshooting](#7-troubleshooting)
- [8. Remote-SSH Development](#8-remote-ssh-development)
- [9. Complete Configuration Reference](#9-complete-configuration-reference)

### Command and Terminology Conventions

- Run project commands from the project's CMake root directory unless a step specifies another location.
- Run `bash` commands in Bash or a compatible Linux terminal. For Remote-SSH, run toolchain, configuration, build, and debug commands on the remote host.
- Values such as `<project-root>`, `<guide-directory>`, and `<host-address>` are placeholders. Replace them, including the angle brackets, with actual values. The examples do not assume personal installation paths or host details.
- Configure generates the build configuration; Build compiles and links targets. A Configure/Build Preset stores configuration/build settings, and a Launch/Debug Target identifies the program to start. Commands and setting names retain their original VS Code names so you can search for them.

## 1. Install the Toolchain

VS Code extensions invoke the compiler, build tool, and debugger. Install these programs first and verify that they run in the command environment used by the project.

### 1.1 Install Compiler, Build, and Debug Tools

Use the distribution's package manager to install GCC/G++, Clang/Clang++, CMake, Ninja, GNU Make, GDB, clangd, and clang-format. Follow the distribution's documentation for package names and installation methods.

Debian/Ubuntu installation example:

```bash
sudo apt update
sudo apt install build-essential clang cmake ninja-build gdb clangd clang-format
```

`build-essential` includes foundational tools such as GCC/G++ and GNU Make. After installation, check that tool versions meet the project's requirements. If the default distribution repositories do not provide the required versions, use distribution-supported tool repositories or the project's specified toolchain. Install third-party development libraries separately according to the project's dependencies.

### 1.2 Check Tool Paths and Versions

After installation, check program paths and versions in the terminal used for the project. If several installations exist, the presets select the first matching program on PATH. Checking paths identifies which installation is actually used.

```bash
command -v gcc g++ clang clang++ cmake ninja make gdb clangd clang-format

gcc --version
g++ --version
clang --version
clang++ --version
cmake --version
ninja --version
make --version
gdb --version
clangd --version
clang-format --version
```

When multiple compilers are installed, choose the project's required executable through PATH, a toolchain file, or `CMakeUserPresets.json`. Use a new build directory after changing toolchains to avoid reusing the old cache. Keep personal installation paths out of shared project configuration.

### 1.3 Choose a Build Generator

The generator determines which build backend CMake uses. Choosing a preset later also selects its generator. Choose one of the following according to the project's conventions.

| CMake Generator | Build Tool | Presets to Use |
| --- | --- | --- |
| `Ninja` | `ninja` | Presets with `ninja` in the name |
| `Unix Makefiles` | GNU Make (`make`) | Presets with `make` in the name |

Both generators support incremental builds and generate `compile_commands.json`. `cmake --build --preset <preset-name>` invokes the selected backend; use a separate build directory for each generator. With `Unix Makefiles`, CMake looks for GNU Make and may use its `gmake` alias on some systems.

### 1.4 LLDB Backend and Command-Line Tools (Optional)

Use the **CodeLLDB** extension for LLDB debugging in VS Code. It provides the debug adapter and, by default, the platform LLDB backend installed with the extension. Once the extension and its platform components are installed, you usually do not need to install system LLDB or add `lldb` to PATH.

If you also need LLDB as a standalone terminal tool, install the command-line package separately:

```bash
# Debian/Ubuntu example; use the appropriate package manager on other distributions
sudo apt install lldb
```

Run `lldb --version` to check the command-line version after installation. CodeLLDB manages its backend separately, so it may differ from LLDB in the terminal. The next section explains how to choose the editor's backend.

## 2. Configure VS Code Extensions

With the toolchain ready, connect it to the editor. Install VS Code from the [VS Code website](https://code.visualstudio.com/), then install CMake Tools and clangd in the Extensions view. Choose C/C++ or CodeLLDB according to the debugger you use. Their responsibilities are listed below.

| Extension | Extension ID | Purpose | When to Install |
| --- | --- | --- | --- |
| CMake Tools | `ms-vscode.cmake-tools` | Presets, configuration, building, and target selection | For the CMake integration in this guide |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | C++ language services and formatting | For the language services in this guide |
| C/C++ | `ms-vscode.cpptools` | `cppdbg` debug adapter | When using GDB |
| CodeLLDB | `vadimcn.vscode-lldb` | `lldb` debug adapter and LLDB backend | When using LLDB |

You can also install the extensions from a Linux terminal with the commands below. If you use only one debugger, install its corresponding debug extension:

```bash
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.cpptools
code --install-extension vadimcn.vscode-lldb
code --install-extension llvm-vs-code-extensions.vscode-clangd
```

The clangd extension needs a `clangd` executable. Obtain it using the extension's installation feature, or point `clangd.path` in VS Code **User Settings** to an existing executable. Keep absolute installation paths in User Settings.

If you choose CodeLLDB, wait for its platform components to finish installing. The debug configurations below use `"type": "lldb"` for this extension. Other LLDB extensions that use `"type": "lldb-dap"` require their own configuration format.

Normally, keep CodeLLDB's default backend. To replace it, follow the [CodeLLDB manual](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md#using-alternate-lldb-backends) and set `lldb.library` in User Settings to a compatible LLDB dynamic library. On Linux, supply the shared-library path, not the `lldb` command path. An external backend may also need a matching `lldb.server`.

In this setup, clangd analyzes C++ code. Workspace settings disable C/C++ IntelliSense while preserving the C/C++ extension's GDB debugging support. Both debug extensions can be installed together; select the relevant configuration when debugging. Enable other language servers only as needed by the workspace to avoid duplicate analysis of the same files.

## 3. Project Structure and Configuration

Project configuration brings the installed tools together: CMake files define targets and build options, while `.vscode` files let the editor read those settings and start debugging. This section explains each file's role. [Section 9](#9-complete-configuration-reference) collects their full contents for reference when creating them.

For a new sample project, create the following `cpp-demo` directory and open it in VS Code. Keep an existing project's sources, targets, and formatting rules, and integrate it as described in Section 3.7. The two entry-point source files and `CMakeLists.txt` below demonstrate the setup.

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

Once these files are in place, Configure generates build rules and `compile_commands.json` in the selected preset's build directory. clangd obtains that directory through CMake Tools and reads the compilation commands generated by CMake directly.

### 3.1 Sample Source Files

The sample uses two separate `main()` functions to demonstrate a project with multiple programs. The complete sources are in [Section 9.1](#91-sample-source-files). Each source file corresponds to the following target and behavior.

| Source File | Executable Target | Sample Behavior |
| --- | --- | --- |
| `src/main.cpp` | `example_app` | Prints a fixed result; inspect `answer` while debugging |
| `src/demo_calc.cpp` | `demo_calc` | Calls a multiplication function; inspect its arguments and return value |

The two programs build and run independently. Choose the launch target corresponding to the source file you want to debug. First, establish that relationship in CMake.

### 3.2 `CMakeLists.txt`

Create `CMakeLists.txt` in the project root using the contents in [Section 9.2](#92-cmakeliststxt). `LANGUAGES C CXX` enables the C and C++ compilers. The two `add_executable()` calls assign each entry-point source to `example_app` or `demo_calc`, set C++20 for both targets, and disable compiler extensions.

Give each executable target its own `main()`; do not add multiple program entry points to one target. Declare additional header directories and dependencies through `target_include_directories()`, `find_package()`, and `target_link_libraries()` so builds and language services read the same target configuration.

### 3.3 `CMakePresets.json`

With the targets defined, use presets to store the compiler, generator, and build type so later configuration only requires choosing a combination. For a new project, generate `CMakePresets.json` using Section 3.3.1 or copy the complete template in [Section 9.3](#93-cmakepresetsjson). Both have the same structure; select `gcc-ninja-debug` for the first debugging session.

Preset names follow `<compiler>-<generator>-<build-type>`, for example `gcc-ninja-debug` or `clang-make-release`. Each Configure Preset and its corresponding Build Preset use the same name.

`CMAKE_C_COMPILER` and `CMAKE_CXX_COMPILER` specify the C and C++ compilers, respectively, and find them through PATH. The eight preset combinations are listed below:

| Configure/Build Preset | C/C++ Compilers | Generator | Build Type | Build Directory |
| --- | --- | --- | --- | --- |
| `gcc-ninja-debug` | `gcc` / `g++` | Ninja | Debug | `build/gcc-ninja-debug` |
| `gcc-ninja-release` | `gcc` / `g++` | Ninja | Release | `build/gcc-ninja-release` |
| `gcc-make-debug` | `gcc` / `g++` | Unix Makefiles | Debug | `build/gcc-make-debug` |
| `gcc-make-release` | `gcc` / `g++` | Unix Makefiles | Release | `build/gcc-make-release` |
| `clang-ninja-debug` | `clang` / `clang++` | Ninja | Debug | `build/clang-ninja-debug` |
| `clang-ninja-release` | `clang` / `clang++` | Ninja | Release | `build/clang-ninja-release` |
| `clang-make-debug` | `clang` / `clang++` | Unix Makefiles | Debug | `build/clang-make-debug` |
| `clang-make-release` | `clang` / `clang++` | Unix Makefiles | Release | `build/clang-make-release` |

Each Release preset inherits the Debug preset for the same compiler and generator, then overrides the build type. `${presetName}` expands to the selected preset's name, keeping the build directories separate. Put installation-specific settings in `CMakeUserPresets.json` and exclude it from version control. When changing compilers or generators, select the matching preset rather than reusing another combination's cache. Use a new build directory when changing the standard library or toolchain installation location as well.

#### 3.3.1 Generate Presets with the Accompanying Tools

The generator script runs through CMake itself and requires CMake ≥ 3.21. Keep its two files together in the `tools` subdirectory of the **guide directory**:

```text
tools/
├── generate-cmake-presets.cmake
└── CMakePresets.template.json
```

Run the following command in a terminal, setting the output directory to the project's CMake root:

```bash
cmake -DOUTPUT_DIR="<project-root>" -P "<guide-directory>/tools/generate-cmake-presets.cmake"
```

The script creates `CMakePresets.json` in the project root with all eight combinations listed above and enables the compilation database. It selects the Make generator by platform: `MinGW Makefiles` on Windows and `Unix Makefiles` on Linux. The generator remains part of the guide's tooling; everyday Configure and Build operations read the generated file in the project.

| Argument or Usage | Behavior |
| --- | --- |
| `-DOUTPUT_DIR="<project-root>"` | Sets the output directory; relative paths are resolved from the current terminal directory |
| Omit `OUTPUT_DIR` | Writes to the current terminal directory, so first change to the project's CMake root |
| `-DPLATFORM=windows` or `-DPLATFORM=linux` | Selects the target platform explicitly; if omitted, uses the host system running the script |
| `-DOVERWRITE=ON` | Backs up an existing file and regenerates it; by default, preserves an existing file and stops |

Place every `-D` argument before `-P`. To reinitialize the presets from the shared template, run:

```bash
cmake -DOUTPUT_DIR="<project-root>" -DOVERWRITE=ON -P "<guide-directory>/tools/generate-cmake-presets.cmake"
```

Before regenerating, the script backs up the old file as `CMakePresets.json.bak-<timestamp>`, then restores the shared template's defaults. Edit presets directly once they contain project-specific options. Prefer an existing project's own presets; use the generator to initialize projects that lack them.

For Remote-SSH development, place the accompanying tools somewhere the remote user can access, invoke them from a remote terminal, and supply remote paths. Generating presets only creates configuration; install the relevant compiler and build tool before using a combination.

#### 3.3.2 Adapt the Generated Presets

The template supplies common combinations. Add project-specific feature switches, dependency locations, and build scope according to the project's build instructions. Common changes and their locations are listed below.

| Required Change | Where and How to Change It |
| --- | --- |
| Use only some compilers, generators, or build types | Remove unused Configure Presets and matching Build Presets; retain presets inherited by others |
| Use a specific compiler or toolchain | Adjust `CMAKE_C_COMPILER` and `CMAKE_CXX_COMPILER`, or set `toolchainFile`; keep personal absolute paths in `CMakeUserPresets.json` |
| Add a build type | Copy a similar preset, give it a unique `name` and `CMAKE_BUILD_TYPE`, and add a Build Preset referencing it |
| Change build directories | Edit `binaryDir`; keep separate directories for each compiler, generator, and build-type combination |
| Add project options or dependency locations | Add the project's CMake variables to `cacheVariables`; maintain target sources, macros, header directories, and link dependencies in `CMakeLists.txt` |
| Restrict the targets built by a normal Build | Set `targets` in the corresponding Build Preset; continue selecting the launch target with `CMake: Set Launch/Debug Target` |
| Standardize initial settings for future projects | Edit `tools/CMakePresets.template.json`; preserve `@MAKE_GENERATOR@` so the script selects the platform's Make generator |

After editing, select the required Configure Preset again, run Configure, restart clangd, and select the corresponding Build Preset. Keep `CMAKE_EXPORT_COMPILE_COMMANDS` enabled so language services can read the generated compilation database. Set the C++ standard per target in `CMakeLists.txt`; the generator script does not choose it.

### 3.4 `.vscode/settings.json`

Presets store the build configuration; workspace settings connect it to CMake Tools and clangd. Merge [Section 9.4](#94-vscodesettingsjson) into `.vscode/settings.json` while preserving existing project settings. The following table explains how the settings work together.

| Setting | Value or Action | Purpose |
| --- | --- | --- |
| `cmake.useCMakePresets` | `always` | Uses project presets as the entry point for configuration and builds |
| CMake automatic configuration settings | Disable routine automatic configuration on opening or editing | Run Configure explicitly as described in Section 4 to control when configuration changes take effect |
| `cmake.buildBeforeRun` | `true` | Builds before launching a target so saved changes are updated incrementally |
| `C_Cpp.intelliSenseEngine` | `disabled` | Uses clangd for language services while retaining C/C++ GDB debugging support |
| `--compile-commands-dir=${command:cmake.buildDirectory}` | Reads the selected Configure Preset's build directory | Follows the preset's compilation database directly without copying it to the project root |
| `editor.defaultFormatter`, `editor.formatOnSave` | Set clangd as the C/C++ formatter and enable format on save | Applies the project's `.clang-format` consistently |

`compile_commands.json` records the compiler, language standard, macros, and header paths for each source file. A successful Configure generates the database; language services do not need to wait for Build to finish.

Because a command variable supplies the build directory, clangd resolves it on startup. After changing Configure Presets, run `CMake: Configure`, then `clangd: Restart language server` so language services read the database from the new directory.

Project header options come from the compilation database. `--query-driver` can supply the toolchain's standard-library search paths by allowing clangd to query matching GCC/Clang drivers. The Linux template lists common system compiler paths. For custom directories or versioned executable names, use the actual compiler paths. Put multiple patterns in one comma-separated argument. Choose the clangd executable independently of the project's compiler.

### 3.5 `.vscode/launch.json`

With language services connected, specify the debug adapter and launch behavior in `.vscode/launch.json`. [Section 9.5](#95-vscodelaunchjson) provides reusable GDB and CodeLLDB entries; merge them into an existing file as needed.

| Debug Configuration | Extension and Type | Backend Location | Entry and Terminal Settings |
| --- | --- | --- | --- |
| `CMake / GDB` | C/C++, `cppdbg` | `miDebuggerPath` finds GDB through the PATH inherited by VS Code | `stopAtEntry`, `externalConsole` |
| `CMake / LLDB (CodeLLDB)` | CodeLLDB, `lldb` | Uses CodeLLDB's bundled LLDB backend by default | `stopOnEntry`, `terminal` |

GDB uses `MIMode`, `miDebuggerPath`, and `setupCommands`. CodeLLDB has its own configuration format; do not add those GDB fields. The LLDB example uses `"terminal": "integrated"` to connect program input and output to VS Code's integrated terminal. If standard input is unnecessary, use `"console"` to send output to the Debug Console instead.

The debugger must support the target architecture and the debug information produced by the compiler. Debug presets generate debug information. Choosing a debugger does not require changing the compiler, generator, or build directory. GCC and Clang can both produce information usable by GDB and LLDB, but support depends on the versions and formats involved. The Linux sample uses DWARF information; with a custom toolchain, verify that the debugger supports its format.

Both configurations use `${command:cmake.launchTargetPath}` for `program`, reading the executable path of the launch target selected in CMake Tools. After changing targets with `CMake: Set Launch/Debug Target`, you can reuse either configuration without editing the program path. With `cmake.buildBeforeRun` enabled, CMake Tools builds before launch and starts debugging after a successful build. Section 4.4 covers target selection and build scope.

The build before launch updates artifacts as needed. Ninja or `make` uses CMake-generated rules, file timestamps, and dependencies to determine what needs updating, then recompiles or relinks only the affected parts.

| Change | Build Behavior |
| --- | --- |
| Sources, dependencies, and build rules are unchanged | No recompilation or relinking is needed; debugging starts after the build check |
| Switch only to another existing launch target | Checks and updates that target and its dependencies; valid artifacts are reused, and targets not yet built are built |
| Edit and save a `.cpp` or `.c` file | Recompiles the affected source and relinks its target |
| Edit and save a header | Recompiles sources depending on the header and relinks their targets |
| Change CMake configuration, macros, compiler options, header directories, or link dependencies | Run Configure first to update build rules; the build tool then recompiles and relinks affected targets using those rules |
| Change the compiler, build type, or generator | Select the corresponding Configure/Build Presets and configure and build in their separate directory; valid artifacts can be updated incrementally, while unbuilt targets require a full build |

Running Configure again does not necessarily trigger a full compilation. The build tool rebuilds affected artifacts according to the updated rules and dependencies. Maintain build rules through CMake configuration rather than editing generated `build.ninja` or `Makefile` files directly.

Save modified files before debugging. If the build fails, fix the compilation or linking errors before starting another debugging session.

Neither configuration requires an additional `preLaunchTask`. If you replace the command variable with a direct executable path, arrange a separate build before debugging.

### 3.6 `.clang-format` and `.gitignore`

Once building and debugging are configured, set the project's formatting rules and ignored files. clangd reads `.clang-format` when formatting. This sample uses Google style, 2-space indentation, and a 100-character line limit; Section 9.6 contains the complete file.

For consistent formatting across a team, standardize tool versions as well. Command-line `clang-format` is a separate executable and may have a different version from clangd's built-in formatter. Check their results even when they read the same rules.

`.gitignore` excludes `build`, `CMakeUserPresets.json`, preset-generator backups, and tool caches; Section 9.7 contains the complete file. Version project presets, formatting rules, and `.vscode` configuration without personal installation paths. Exclude build artifacts, compilation databases, and private user configuration from version control.

### 3.7 Integrating an Existing CMake Project

Keep an existing project's targets, dependencies, and formatting conventions while integrating the editor settings above. Follow these steps to check how the project's requirements correspond to this setup.

1. Read the project's README, build instructions, or contribution guide. Confirm its supported toolchains, CMake version, dependency installation process, and debuggable targets. Those conventions take precedence over the sample choices in this guide.
2. Open the project workspace and keep its existing `CMakeLists.txt`, sources, and `.clang-format`. If it has multiple CMake entry points, use `cmake.sourceDirectory` to select the required source directory according to the project's conventions.
3. Reuse the project's Configure/Build Presets. If none exist, start with this template and add the CMake options and dependencies required by the project. Configure personal compiler, SDK, and dependency paths in `CMakeUserPresets.json`.
4. Merge the language-service and debugging entries from Sections 3.4 and 3.5 into the existing `.vscode` files, preserving other project settings. Verify that the compiler path matches `--query-driver`.
5. Configure and build as described in Section 4.1. Use `CMake: Set Launch/Debug Target` to select an actual executable target in the project, then choose the debugger and press F5.

Replace the sample's `example_app`, `demo_calc`, and `src` layout with the project's actual targets and structure. If programs need different arguments, working directories, or environment variables, create dedicated debug configurations as described in Section 4.4.

### 3.8 Maintaining Configuration and Packaging Releases

After configuration, distinguish team-shared files from settings that vary by user or host. Keeping personal paths and generated artifacts in their appropriate locations lets everyone reuse the same project configuration.

| Configuration Ownership | Contents | Maintenance |
| --- | --- | --- |
| Shared project configuration | `CMakeLists.txt`, project presets, formatting rules, and general `.vscode` configuration | Version with the project; use project options, relative paths, and command variables |
| User or host configuration | `CMakeUserPresets.json`, VS Code User Settings, shortcuts, and SSH configuration | Store in the corresponding user environment; keep personal absolute paths at this level |
| Generated content | Build directories, compilation databases, caches, and `CMakePresets.json.bak-*` | Exclude from version control and source distributions |
| Accompanying guide tools | Generator script and `CMakePresets.template.json` | Publish both files together with the guide; maintain them as initialization tools for new projects |

For repositories supporting Windows and Linux, maintain shared presets for the platforms supported by the team. The Make generators differ between these two examples. Cross-platform projects should use project-maintained platform groups or conditional presets; individual members should not regenerate and overwrite the same shared preset file.

The project's build documentation should record supported toolchains, dependency versions, presets, and common targets. In CI, configure and build with the same project presets and pin the required toolchain and dependency versions.

Guides, project sources, and runnable programs have different delivery requirements. Use the following table to decide which files to collect.

| Distribution Type | Contents |
| --- | --- |
| Guide package | Windows/Linux guides and accompanying `tools`, keeping the documentation, script, and template versions aligned |
| Project source package | Sources, shared CMake configuration, and necessary development instructions; include the generator tools if the project uses them |
| Program release package | The program, runtime dependencies, and required resources, collected according to the project's installation or packaging rules; development presets and initialization tools need not accompany the executable |

When packaging sources, explicitly exclude user configuration, build artifacts, and backups. `.gitignore` controls version tracking; it does not automatically filter files copied or archived manually.

## 4. Configure, Build, and Debug

With the project files ready, run Configure and Build to turn the configuration into executable programs, then choose a target to debug. Start with the editor workflow below, followed by command-line operations using the same presets.

### 4.1 First Configuration, Build, and Debugging Session

In the open CMake project workspace, press `Ctrl+Shift+P` and complete the following steps from the Command Palette. The sample uses `gcc-ninja-debug` and `example_app`. For existing projects, choose their presets and targets; for Remote-SSH, work in the remote window.

| Step | Action | Purpose |
| --- | --- | --- |
| 1 | Run `CMake: Select Configure Preset` and select `gcc-ninja-debug` or a preset for another installed toolchain | Chooses the compiler, generator, Debug/Release mode, and build directory |
| 2 | Run `CMake: Configure` | Generates build rules and `compile_commands.json`; resolve configuration errors before proceeding |
| 3 | Run `clangd: Restart language server` | Makes clangd resolve the selected build directory again |
| 4 | Run `CMake: Select Build Preset` and select the matching preset | Chooses the build scope and arguments; this template uses the same name as its Configure Preset |
| 5 | Run `CMake: Build` | Compiles and links the targets, producing the programs and their dependencies |
| 6 | Run `CMake: Set Launch/Debug Target` (`cmake.selectLaunchTarget`) and select `example_app` | Chooses the executable target to launch |
| 7 | Set a breakpoint on the output line in `src/main.cpp`, select `CMake / GDB` or `CMake / LLDB (CodeLLDB)`, and press F5 | Starts debugging after the incremental build before launch completes |

At the breakpoint, inspect `answer`; continue execution to see the program's output. Press F5 directly for subsequent sessions with the same program. To change executables, select a launch target as described in Section 4.4. Custom shortcuts are covered in Section 6.3.

If the interface uses translated command names, search for `CMake` and the relevant keyword in the Command Palette. If presets are missing or Configure fails, use Section 7.1 to troubleshoot.

### 4.2 Configure, Build, and Run from the Command Line

The same presets work from the command line, which is useful for terminal builds or reproducing problems. In a Linux terminal at the project's CMake root, run the following example for `gcc-ninja-debug`:

```bash
cmake --list-presets
cmake --list-presets=build
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
./build/gcc-ninja-debug/example_app
```

Example output:

```text
C++ environment is ready. answer=42
```

To configure, build, and run Clang Ninja Debug:

```bash
cmake --preset clang-ninja-debug
cmake --build --preset clang-ninja-debug
./build/clang-ninja-debug/example_app
```

For Release builds, use `gcc-ninja-release` or `clang-ninja-release`, replacing both the preset name and executable directory in the commands.

Unix Makefiles build example:

```bash
cmake --preset gcc-make-debug
cmake --build --preset gcc-make-debug --parallel
./build/gcc-make-debug/example_app
```

For Clang, select `clang-make-debug`. For Release, use `gcc-make-release` or `clang-make-release`. `--parallel` enables parallel builds with either backend.

To build and run a single executable target:

```bash
cmake --build --preset gcc-ninja-debug --target demo_calc
./build/gcc-ninja-debug/demo_calc
```

This command builds `demo_calc` and its dependencies; the program prints `[demo_calc] 6 * 7 = 42`. Without `--target`, these presets build both executables by default. Command-line operations do not change the launch target remembered by VS Code. When returning to the editor, use `CMake: Set Launch/Debug Target` to choose the program to debug.

Running `cmake --preset clang-ninja-debug` in the terminal does not change the preset selected in CMake Tools. Back in VS Code, select the corresponding Configure Preset, run Configure, restart clangd, and select the matching Build Preset. Use Debug builds for source breakpoints and variable inspection. Both debug configurations work with GCC, Clang, and either generator without separate executable paths.

### 4.3 Use and Switch Debuggers

After building with a Debug preset, select GDB or CodeLLDB from the Run and Debug dropdown. To switch debuggers, stop the session, select the other configuration, and press F5. If the build configuration is unchanged, no new Configure or clangd restart is needed.

| Operation | GDB Debug Console | CodeLLDB Debug Console (Default Command Mode) |
| --- | --- | --- |
| Inspect `answer` | `-exec print answer` | `frame variable answer` |
| Evaluate an expression | Use Watch or `-exec print <expression>` | Use Watch or `expression -- <expression>` |
| View the call stack | Use Call Stack or `-exec backtrace` | Use Call Stack or `thread backtrace` |

Both configurations support VS Code breakpoints, stepping, continuing, variables, and Watch views. Debug Console command syntax depends on the adapter; do not reuse one debugger's commands unchanged in the other.

### 4.4 Select and Debug Multiple Executable Targets

The sample's two `add_executable()` calls define `example_app` and `demo_calc`. After a successful Configure, CMake Tools lists them as launchable targets. Libraries and ordinary custom build targets are not runnable programs. In a project with multiple programs, check the build configuration, launch target, and debug adapter together.

| Selection | What It Determines | Example in This Guide |
| --- | --- | --- |
| Configure/Build Preset | Compiler, generator, build type, and build directory | `gcc-ninja-debug`, `clang-make-debug`, and others |
| Launch/Debug Target | Executable to start in the current configuration | `example_app` or `demo_calc` |
| Run and Debug configuration | Debug adapter and runtime arguments | `CMake / GDB` or `CMake / LLDB (CodeLLDB)` |

After choosing the presets and debugger, select a program as follows:

1. Press `Ctrl+Shift+P`, run **`CMake: Set Launch/Debug Target`** (`cmake.selectLaunchTarget`), and select `example_app` or `demo_calc`.
2. Open the corresponding source file, place a breakpoint on a line that executes, select the GDB or CodeLLDB configuration, and press F5.
3. Press F5 directly to debug the same program again. To change programs, stop the current session, run the target-selection command, select the other target, and press F5.

CMake Tools remembers the launch target per project and usually retains a valid selection when reopening it. Confirm the target again if there is no valid selection; a project with one executable can select it automatically. This state is stored in the editor. Reselect after moving the project, changing environments, or resetting the extension. You can also bind target selection to a shortcut as described in Section 6.3.

Changing to another existing launch target does not change compilation settings, so Configure and a clangd restart are unnecessary. When changing Configure Presets, update the configuration using Section 4.1. If the original target still exists, its program path points to the new build directory. After adding, deleting, or renaming targets, save the CMake files, run Configure, and select the launch target. In a multi-root workspace, also check the active CMake Tools project.

After selecting the program, check the build scope as well. With no separately selected build target and no `targets` in the Build Preset, a normal Build builds both programs by default. In this preset workflow, the build before F5 builds the selected launch target and its dependencies. If the build scope is `all` or includes other targets, those targets also participate, and their errors may prevent debugging.

Adjust the build scope through CMake Tools' build target or the Build Preset's `targets`. Selecting a launch target does not automatically change the scope of a normal Build. `cmake.buildBeforeRun` ensures the launch target is included in the build before launch. Section 4.2 shows how to build a single target from the terminal.

The two sample programs use no arguments and share the project root as their working directory, so one debug configuration serves both. For real programs with different arguments, working directories, or environment variables, create dedicated entries and specify the launch target and build before launch clearly. To debug multiple programs simultaneously, use separate configurations and `compounds`; the target-selection workflow above starts one program at a time.

## 5. Language Services and Formatting

Building and debugging read CMake's target configuration, and code editing uses the same information through the compilation database. Once Configure succeeds and clangd restarts, completion, navigation, diagnostics, and formatting follow the current preset.

### 5.1 Code Editing and Formatting

clangd analyzes source code using the current compiler options and provides navigation through its project index. Common editing operations and their configuration sources are listed below.

| Operation | How to Use It | Configuration Source |
| --- | --- | --- |
| Completion and type information | Type a member, namespace, or `std::`; use completion suggestions and hover information | Standard, macros, and header paths in the compilation database |
| Navigation and references | Use Go to Definition, Go to Declaration, and Find All References | clangd's index of project sources and dependencies |
| Diagnostics | Inspect editor markers and the Problems panel | Current compilation database and source code |
| Format the current file | Run `Format Document` or press `Ctrl+Shift+I` | Project `.clang-format` and the clangd formatter |
| Format on save | Save the file | `editor.formatOnSave` from Section 3.4 |

Remote-SSH uses the client's editor shortcuts. For consistent team formatting, standardize `.clang-format` and the formatting tool version; clangd's built-in formatter and command-line `clang-format` may use different versions.

### 5.2 Switch Presets and Inspect Language-Service Logs

When a preset changes the compiler, macros, or header paths, language services need updating too. After changing Configure Presets, run Configure and then `clangd: Restart language server`. Select **clangd** in the Output panel to inspect the actual server version, startup arguments, and database directory. `--compile-commands-dir` should point to the current build directory.

If standard-library or project headers cannot be resolved, check the database, compiler paths, and `--query-driver` using Section 7.3. `clangd --version` in a terminal identifies the executable on that terminal's PATH; the clangd extension logs identify the executable actually used by the editor.

## 6. Everyday Workflow and Shortcuts

After the initial setup, the next steps depend on whether a change affects sources, build rules, or the program being launched. Use the type of change to determine what must be updated, then use shortcuts for common operations.

### 6.1 Workflow After Changes

| Change | Recommended Workflow |
| --- | --- |
| No changes; debug the same target again | Press F5 directly; the selected target is reused, and the build check usually requires no recompilation |
| Switch to another existing executable | Stop debugging → `CMake: Set Launch/Debug Target` → F5; no Configure or clangd restart needed |
| `.cpp`, `.c`, or an existing header | Save → Build or F5 for incremental compilation |
| `CMakeLists.txt`, included `.cmake` files, presets, dependencies, or compiler options | Save → Configure → Build/F5 |
| Add a `.cpp` or `.c` file | Add it to a CMake target → Configure → Build |
| Add, delete, or rename an executable target | Update `add_executable()` → Configure → `CMake: Set Launch/Debug Target` → F5 |
| Switch Debug/Release | Select Configure Preset → Configure → Restart clangd → Select matching Build Preset → Build |
| Switch GCC/Clang | Select matching Configure Preset → Configure → Restart clangd → Select same-name Build Preset → Build |
| Switch Ninja/Unix Makefiles | Select the generator's preset → Configure → Restart clangd → Select same-name Build Preset → Build |
| Switch GDB/LLDB | Stop debugging → Select the other debug configuration → F5; no reconfiguration needed if the preset is unchanged |
| Change the toolchain installation location or standard library | Use a new build directory → Configure → Restart clangd → Build |

CMake Tools and CMake-generated build rules may still trigger necessary reconfiguration when they detect changes. Workspace automatic-configuration settings control routine editor behavior; follow the workflows above for everyday operations.

### 6.2 Default Shortcuts

Common build and debug commands already have default bindings and are available once CMake Tools recognizes the project.

| Shortcut | Action | Availability |
| --- | --- | --- |
| F5 | Starts the current debug configuration using the selected launch target | The debug configuration in Section 3.5 is set up |
| F7 | `CMake: Build` | CMake Tools is enabled and recognizes the project |
| Shift+F7 | `CMake: Build with Target` | CMake Tools is enabled and recognizes the project |

The build before F5 is handled by `cmake.buildBeforeRun` together with `cmake.launchTargetPath`. If a shortcut is overridden, inspect it in VS Code's Keyboard Shortcuts editor.

### 6.3 Custom Shortcuts

For everyday development, add shortcuts for target selection, cache reconfiguration, and restarting language services, while keeping direct access to User Settings and the keyboard-shortcut file. The five bindings below match the complete configuration reference.

| Custom Shortcut | Command | Command ID |
| --- | --- | --- |
| F6 | `CMake: Delete Cache and Reconfigure` | `cmake.cleanConfigure` |
| Alt+S | `clangd: Restart language server` | `clangd.restart` |
| Ctrl+K, Ctrl+A | `Preferences: Open User Settings (JSON)` | `workbench.action.openSettingsJson` |
| Ctrl+K, Ctrl+Z | `Preferences: Open Keyboard Shortcuts (JSON)` | `workbench.action.openGlobalKeybindingsFile` |
| F4 | `CMake: Set Launch/Debug Target` | `cmake.selectLaunchTarget` |

1. Run `Preferences: Open Keyboard Shortcuts (JSON)` to open the user shortcut file.
2. Merge the five bindings from [Section 9.8](#98-custom-shortcuts) into the existing array, preserving other entries. If a key already has a binding, check and adjust it.
3. Check for conflicts in Keyboard Shortcuts. F4's `when: "cmake:enableFullFeatureSet"` limits that binding to when CMake Tools' full feature set is enabled. Keep the other four bindings as shown in the example.

`ctrl+k ctrl+a` and `ctrl+k ctrl+z` are two-part key chords: press Ctrl+K, then Ctrl+A or Ctrl+Z. During initial setup, open the shortcut JSON from the Command Palette. After saving, use Ctrl+K, Ctrl+Z to open it again.

F4 runs `CMake: Set Launch/Debug Target`. Select once and subsequent F5 sessions reuse the target; press F4 again to change programs. Alt+S restarts clangd in the current workspace. After changing Configure Presets, run Configure first, then Alt+S so language services read the new compilation database.

F6 runs `CMake: Delete Cache and Reconfigure`, clearing the CMake configuration cache and regenerating build rules with the current preset. For a normal configuration, run `CMake: Configure` from the Command Palette. Save everyday source changes and use Build or F5 for the incremental build described in Section 3.5.

Shortcuts belong in user configuration; the project does not need `.vscode/keybindings.json`. The two settings shortcuts open User Settings and the user keyboard-shortcut file. Maintain shared project settings as described in Section 3.

In a Remote-SSH window, F6, Alt+S, and F4 operate on the current remote workspace; the client still manages the shortcuts. Put personal remote-tool paths in Remote User Settings or remote User Presets as described in Section 8.6.

## 7. Troubleshooting

First identify the failing stage: inspect CMake output for Configure, compiler and linker errors for Build, the Debug Console for launching or debugging, and clangd logs for language services. The first error usually points most directly to the cause. Resolve it before retrying that operation to avoid noise from subsequent errors.

### 7.1 Toolchain, Presets, and Builds

| Symptom | Checks and Remedies |
| --- | --- |
| A tool is missing or a different toolchain is used | Check `command -v` output; reopen the terminal or remote window after changing PATH |
| Presets are missing | Open the project root; validate the JSON; use CMake ≥ 3.21; enable the extension |
| Configure fails | Inspect the first error; verify the selected C/C++ compilers and Ninja/make are available, dependencies match the toolchain, and the cache is not from another compiler or generator |
| Build fails | Inspect the first compilation or linking error; confirm sources belong to the target and header directories and linked libraries are declared |
| Unix Makefiles cannot find the build tool | Check `command -v make`; install the distribution's GNU Make package |
| The generator does not match the existing cache | Do not alternate Ninja and Unix Makefiles in one build directory; use the matching preset's separate directory |

### 7.2 Target Selection, Running, and Debugging

| Symptom | Checks and Remedies |
| --- | --- |
| A new program is absent from the launch-target list | Define it with `add_executable()`; save and Configure successfully; check the active project and preset, then run `CMake: Set Launch/Debug Target` again |
| F5 launches a different program | Confirm the target with `CMake: Set Launch/Debug Target`; use `${command:cmake.launchTargetPath}` for `program` and select this guide's debug configuration |
| Another target's build error prevents debugging one program | Check whether the current build scope includes that target; review the build target or Build Preset's `targets` using Section 4.4 |
| Debugging shows old results | Save the files; complete Build successfully; match Configure/Build Presets and launch target; ensure the executable is not from another directory |
| Breakpoints are unverified or never hit | Use Debug; ensure the target includes the source and debug information, the breakpoint is on an executed line, and the debugger supports the architecture and debug-information format |
| VS Code does not recognize debug type `lldb` | Install and enable CodeLLDB (`vadimcn.vscode-lldb`), on the remote side for remote development; ensure the configuration is not for `lldb-dap` |
| The CodeLLDB backend cannot start | Inspect LLDB logs in Output and the extension's platform components; use the bundled backend by default; check compatibility and the matching server when customizing `lldb.library` |
| LLDB runs, but source breakpoints or variables behave incorrectly | Use Debug; check source/target correspondence, DWARF information, and tool versions; use the correct `sourceMap` for external build paths |
| The program cannot load a shared library | Check that dependencies are installed and the dynamic linker can find the libraries; handle runtime dependencies when distributing to other machines |
| GDB rejects pretty-printer auto-loading | Check the script and permitted paths reported by GDB; for STL container visualization, allow the precise path of a trusted script |

### 7.3 Language Services and Formatting

| Symptom | Checks and Remedies |
| --- | --- |
| clangd cannot resolve the standard library | Generate the database in the selected build directory; match compiler paths to `--query-driver`; inspect clangd Output logs |
| clangd still uses the old configuration after switching presets | Select and configure the matching Configure Preset, restart clangd, and check the database directory in its logs |
| clangd's build directory is empty or a command variable is unexpanded | Enable CMake Tools and ensure it recognizes the project; select a Configure Preset and Configure first; use a clangd extension supporting command variables, then restart clangd |
| Formatting differs from the command-line result | Compare the versions of clangd's built-in formatter and `clang-format` on PATH, and check that both read the same rules |

### 7.4 Caches and Reconfiguration

If an error refers to an old compiler path, generator, or cache entry, reinitialize the selected preset's configuration. Confirm the intended build directory, then run from the project root:

```bash
# --fresh requires CMake 3.24 or newer
cmake --fresh --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

Use a new build directory when changing toolchains to avoid reusing old artifacts. With CMake 3.21–3.23, use CMake Tools' `CMake: Delete Cache and Reconfigure` to clear the configuration cache. After applying Section 6.3's binding, F6 runs the same command.

## 8. Remote-SSH Development

When a project must build on a Linux server or virtual machine, open a remote workspace through Remote-SSH. The client displays the editor, while the toolchain, language services, and debugger run on the remote host. The project settings and workflow above still apply; establish SSH access first to connect the remote environment.

### 8.1 Configuration Locations and Setup Order

Remote development involves a client and a remote host. First identify what each side needs. Virtual machines and physical hosts follow the same sequence: prepare the remote SSH service, establish the client connection and optional key authentication, open the remote project, install development extensions, then configure, build, and debug.

| Configuration Location | Required Installation or Configuration | Purpose |
| --- | --- | --- |
| Client | VS Code, OpenSSH client, Remote-SSH extension, SSH configuration, and an optional private key | Displays the editor, establishes the connection, and authenticates |
| Remote Linux host | OpenSSH service, project sources, the toolchain from Section 1, and project dependencies | Stores sources and performs configuration, building, running, and debugging |
| VS Code remote window | CMake Tools, clangd, and the chosen debugger's extensions installed on the remote side | Provides project integration, language services, and debugging in the remote environment |

`<host-address>`, `<ssh-port>`, `<remote-username>`, and `<connection-alias>` in the examples below are placeholders. Replace them, including the angle brackets, before running commands or editing configuration. The host address can be an IP address or hostname. Store personal connection details in the client's user configuration.

### 8.2 Prepare the Remote Linux Host

Install and start OpenSSH on the remote Linux host. On Debian/Ubuntu with systemd, run the following in the remote host's terminal:

```bash
sudo apt update
sudo apt install openssh-server
sudo systemctl enable --now ssh
systemctl status ssh
```

`enable --now` starts the service and enables it at boot. Other distributions use their own package manager, and the service may be named `sshd`. On systems without systemd, start SSH using the distribution's service-management method.

After starting SSH, find the host's network address for the client connection:

```bash
ip -brief address
```

Choose a network-interface address reachable by the client. For virtual machines, verify reachability according to bridged, host-only, or NAT network settings. If NAT port forwarding is required, connect the client to the forwarding entry's address and port.

If UFW is enabled on the host, allow the actual SSH listening port:

```bash
sudo ufw allow <ssh-port>/tcp
sudo ufw status
```

SSH listens on port `22` by default. If you use another listening port, match the firewall rule to it. For other firewalls, cloud security groups, or gateway rules, allow the required port in the corresponding configuration. A NAT forwarding entry port may differ from the host's listening port.

The remote user must be able to log in and read/write the project directory. Install the tools from Section 1 and the project's dependencies on the host. VS Code Server, clangd, and debug-extension platform components must also meet their system and architecture requirements; see the [Linux remote-development requirements](https://code.visualstudio.com/docs/remote/linux).

### 8.3 Configure the Client SSH Connection

With the remote service and network ready, return to the client and test the connection. Both Windows and Linux clients need OpenSSH. Check the program in the client terminal, then connect to the remote account:

```text
ssh -V
ssh -p <ssh-port> <remote-username>@<host-address>
```

On the first connection, compare the host-key fingerprint shown by SSH with the fingerprint provided by the host administrator or a trusted management channel before accepting it. Authenticate using the account password or key method allowed by the host. After logging in successfully, run `exit` to return to the client.

Once command-line login works, save those connection details as an alias so the terminal and VS Code can reuse the same SSH configuration.

1. Install **Remote - SSH** (`ms-vscode-remote.remote-ssh`) in VS Code on the client.
2. Run `Remote-SSH: Open SSH Configuration File...` and select the client's user SSH configuration file.
3. Merge the template from [Section 9.9](#99-remote-ssh-client-configuration-optional) into the file, replace the placeholders, and save. Alternatively, use `Remote-SSH: Add New SSH Host...` with the SSH command above, then edit the generated entry.
4. Run `ssh <connection-alias>` in the client terminal to verify the alias works.

| Client | Default User SSH Configuration File |
| --- | --- |
| Windows | `%USERPROFILE%\.ssh\config`, or `$env:USERPROFILE\.ssh\config` in PowerShell |
| Linux | `~/.ssh/config` |

`~` denotes the corresponding user's home directory. The `config`, private key, and host-key records described here are on the client; the remote account's `~/.ssh/authorized_keys` is on the remote host. For a custom SSH configuration file, set `remote.SSH.configFile` in the client's VS Code User Settings to its path.

### 8.4 Configure SSH Key Authentication (Optional)

Once the connection works, you can set up key authentication to avoid repeatedly entering the remote account's password. Reuse a suitable existing key or create a new one in the client terminal:

```text
ssh-keygen -t ed25519
```

Follow the prompts to choose a client-side location and a private-key passphrase. The default files are `~/.ssh/id_ed25519` for the private key and `~/.ssh/id_ed25519.pub` for the public key; on Windows they are usually under `%USERPROFILE%\.ssh`. If keys with those names already exist, choose a different name and use its path in SSH configuration.

Add the generated public key to the remote account. Choose one of the following methods depending on whether the client provides `ssh-copy-id`.

| Client Capability | Action |
| --- | --- |
| Linux or a compatible terminal with `ssh-copy-id` | Run `ssh-copy-id -i ~/.ssh/id_ed25519.pub <connection-alias>`; replace the path after `-i` if using a custom public-key location |
| A client without `ssh-copy-id`, including typical Windows OpenSSH installations | Open the client's `.pub` file and copy its entire line; log in with `ssh <connection-alias>` and add it using the steps below |

To add the key manually, run these commands in the **remote Linux host's terminal**:

```bash
mkdir -p ~/.ssh
chmod 700 ~/.ssh
touch ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys
cat >> ~/.ssh/authorized_keys
```

Paste the entire public-key line, press Enter for a newline, then press `Ctrl+D` to finish input. `>>` appends the key while preserving existing entries. Exit the remote terminal afterward. The directory and files must belong to the current remote user.

Enable `IdentityFile` from Section 9.9 in the client's SSH configuration and enter the corresponding **client private-key** path. The file added to the remote account is the `.pub` public key. Run `ssh <connection-alias>` again to verify key authentication; VS Code uses the same configuration.

If the private key has a passphrase, SSH may still request it. You can manage keys with the client's `ssh-agent`: start the agent using the client operating system's method, then run `ssh-add <local-private-key-path>`. The remote service must allow public-key authentication. For managed hosts, use the authentication method provided by the administrator.

### 8.5 Open the Remote Project and Install Extensions

After verifying the client SSH configuration, open a remote window in VS Code and install the development extensions on the remote side where the project tools run.

1. Run `Remote-SSH: Connect to Host...` in the client VS Code and select the configured connection alias.
2. Authenticate and wait for VS Code Server to install or start. The status bar should show `SSH: <connection-alias>`; the remote host does not need the VS Code desktop application.
3. In the remote window, run `File: Open Folder...` and open the remote project root containing `CMakeLists.txt` and `CMakePresets.json`. Check out sources on the remote host or deploy them using the project's chosen method.
4. Open Extensions and install CMake Tools, clangd, and C/C++ or CodeLLDB for the chosen debugger on the **SSH remote host**. Confirm they are enabled in the remote workspace. CodeLLDB's platform components must match the remote system and architecture.
5. Open the integrated terminal in the remote window, check tool paths and versions using Section 1, and complete the project configuration in Section 3.

Remote-SSH is installed on the client. CMake Tools, clangd, and debug extensions must run on the remote side; installing them only on the client does not replace remote installation. The first connection needs access to VS Code Server and required extension components. For restricted networks, configure downloads, proxies, or network access using the [Remote-SSH documentation](https://code.visualstudio.com/docs/remote/ssh).

### 8.6 Build and Debug Remotely

Once the remote project and extensions are ready, building and debugging follow the local workflow. Complete the following in the remote VS Code window, using presets matching the remote toolchain.

1. `CMake: Select Configure Preset`: Select the preset for the remote toolchain.
2. `CMake: Configure`: Generate the remote build directory and `compile_commands.json`.
3. `clangd: Restart language server`: Make remote clangd read the selected preset's compilation database.
4. `CMake: Select Build Preset`: Select the corresponding build preset, then run `CMake: Build`.
5. `CMake: Set Launch/Debug Target` (command ID: `cmake.selectLaunchTarget`): Select the remote executable target to run.
6. Set source breakpoints, choose `CMake / GDB` or `CMake / LLDB (CodeLLDB)` in Run and Debug, and press F5.

After the first session, F5 reuses the chosen target and performs the incremental build before launch described in Section 3.5. Run target selection again to change programs, and confirm the target when first opening the project in a new remote environment. The following table shows how the same settings resolve in a remote window.

| Setting or Operation | Meaning in the Remote Window |
| --- | --- |
| `${workspaceFolder}` | Remote project root |
| `${command:cmake.buildDirectory}` | Build directory of the remotely selected Configure Preset |
| `${command:cmake.launchTargetPath}` | Program path of the remotely selected executable target |
| Integrated terminal, Configure, Build, language services, and debugging | Run on the remote Linux host using its PATH, tools, and dependencies |
| Editor shortcuts | Use the client VS Code bindings; customization is covered in Section 6.3 |

Put personal remote-tool paths in VS Code **Remote User Settings** or the remote `CMakeUserPresets.json`. Store SSH connection configuration and shortcuts in the client's user configuration. Shared project files retain general tool names, presets, and command variables.

Generate separate build directories and compilation databases locally and remotely. Run Configure and Build again after changing environments. CMake caches, compilation databases, and artifacts contain environment-specific paths; do not reuse another environment's generated files. After switching presets, follow the same process to reconfigure and restart remote clangd.

### 8.7 Troubleshoot Remote Development

Follow the connection sequence to locate remote problems: first verify SSH in a terminal, then inspect VS Code Server and remote extensions, and finally review project configuration, build, and debug logs.

| Symptom | Priority Checks and Actions |
| --- | --- |
| SSH times out or the connection is refused | Reproduce with `ssh <connection-alias>` on the client; check address, port, host SSH service, virtual-machine network, forwarding rules, firewall, and security groups |
| Password or key authentication fails | Inspect authentication with `ssh -v <connection-alias>`; check username, client private-key path, remote public-key entry, ownership, permissions, and server-allowed authentication methods |
| SSH reports a changed host key | Verify the reason, such as host reinstallation, address reuse, or key renewal, and the new fingerprint; after confirmation, remove only that host's old record and reconnect |
| Terminal SSH works but VS Code cannot connect | Read Remote - SSH logs in Output; check the SSH executable, configuration file, proxy, and authentication interaction used by VS Code |
| VS Code Server cannot install or start | Check remote system requirements, home and temporary-directory permissions, free space, and successful component download and extraction |
| The remote project cannot be opened or saved | Verify the remote path and the current remote user's read/write permission for the project directory |
| CMake, clangd, or the debugger is unavailable | Check remote-side extension installation and enablement, tool paths, dependencies, architecture, and platform components |
| Building works but breakpoints or language services fail | Use Section 7 to check remote presets, compilation database, Debug artifacts, target selection, and tool versions; paths must refer to the remote project |

After verifying the host key, remove only the corresponding old record in the **client terminal**:

```text
# Default port
ssh-keygen -R <host-address>
# Non-default port: use the host and port identifier reported by SSH
ssh-keygen -R "[<host-address>]:<ssh-port>"
```

Choose the command for the default or non-default port according to the connection. Verify the fingerprint again when reconnecting. With settings such as `HostKeyAlias`, use the actual record identifier reported by SSH.

## 9. Complete Configuration Reference

The earlier sections explain the settings and workflow. This section collects the required files in one place for reference when creating a project. Use the complete contents below for a new sample project, or merge the necessary parts into an existing project using Section 3.7. Section 3.8 covers ownership and packaging.

| Configuration Location | Required Setup | Purpose |
| --- | --- | --- |
| System environment | Install GCC, Clang, CMake, Ninja, make, GDB, clangd, and clang-format on Linux and check PATH | Provides compilation, build, and debug tools |
| VS Code extensions | Install CMake Tools, clangd, and the C/C++ or CodeLLDB extension for the chosen debugger, on the remote side for remote development | Provides project integration, debug adapters, and language services |
| CodeLLDB platform components | Use the bundled LLDB backend; install command-line LLDB if needed; configure external backends according to compatibility requirements | Provides LLDB debugging |
| VS Code User Settings | Ensure the clangd extension can find `clangd`; set `clangd.path` if needed | Sets the language-server executable location |
| Project root | `CMakeLists.txt`, `CMakePresets.json`, `.clang-format`, and `.gitignore` | Defines targets, compiler presets, formatting rules, and ignored files |
| Accompanying guide tools (optional) | `tools/generate-cmake-presets.cmake` and `tools/CMakePresets.template.json` in the guide directory | Generates presets into a selected project using Section 3.3.1; maintain project settings afterward |
| Project `.vscode` directory | `settings.json` and `launch.json` | Connects CMake Tools, clangd, and GDB/LLDB |
| Project `src` directory | `main.cpp` and `demo_calc.cpp` | Provides separate entry points for both executable targets |
| VS Code user shortcuts (optional) | Configure the five user bindings from Section 9.8 | Clears the configuration cache, restarts language services, opens user configuration, and selects launch targets |
| Remote development (optional) | Prepare the SSH service, client Remote-SSH, and remote toolchain and extensions using Section 8; the client SSH template is in Section 9.9 | Establishes a remote workspace for building and debugging on Linux |

Use actual installation paths for absolute paths in User Settings. Project configuration obtains tool and target paths through PATH, presets, and command variables. Create `CMakeUserPresets.json` only when personal configuration is needed.

### 9.1 Sample Source Files

`src/main.cpp`:

```cpp
#include <iostream>

int main() {
  const int answer = 42;
  std::cout << "C++ environment is ready. answer=" << answer << '\n';
  return 0;
}
```

`src/demo_calc.cpp`:

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

### 9.2 `CMakeLists.txt`

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

### 9.3 `CMakePresets.json`

The following is the generator script's default output and can be copied directly. The accompanying `tools/CMakePresets.template.json` has the same structure; the script replaces `@MAKE_GENERATOR@` according to the operating system. Sections 3.3.1 and 3.3.2 explain generation and subsequent customization.

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
      "generator": "Unix Makefiles"
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
      "generator": "Unix Makefiles"
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

### 9.4 `.vscode/settings.json`

```json
{
  "editor.inlayHints.enabled": "off",
  "cmake.useCMakePresets": "always",
  "cmake.configureOnOpen": false,
  "cmake.configureOnEdit": false,
  "cmake.automaticReconfigure": false,
  "cmake.buildBeforeRun": true,
  "C_Cpp.intelliSenseEngine": "disabled",
  "clangd.arguments": [
    "--background-index",
    "--compile-commands-dir=${command:cmake.buildDirectory}",
    "--query-driver=/usr/bin/gcc,/usr/bin/g++,/usr/bin/clang,/usr/bin/clang++"
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

### 9.5 `.vscode/launch.json`

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

### 9.6 `.clang-format`

```yaml
BasedOnStyle: Google
IndentWidth: 2
ColumnLimit: 100
```

### 9.7 `.gitignore`

```gitignore
/build/
/CMakeUserPresets.json
/CMakePresets.json.bak-*
/.cache/
```

### 9.8 Custom Shortcuts

These bindings assign common keys to clearing the configuration cache, restarting language services, opening user configuration, and selecting launch targets. Run `Preferences: Open Keyboard Shortcuts (JSON)` and merge the entries into the existing user `keybindings.json` array, preserving other bindings.

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

| Shortcut | Usage |
| --- | --- |
| F6 | Deletes the CMake configuration cache in the current build directory and runs Configure again; afterward, use Build or F5 as needed |
| Alt+S | Restarts clangd in the current workspace; after switching Configure Presets, Configure first, then restart language services |
| Ctrl+K, Ctrl+A | Opens user `settings.json` for personal settings such as the clangd executable path; project settings remain in `.vscode/settings.json` |
| Ctrl+K, Ctrl+Z | Opens user `keybindings.json` to change or add shortcuts |
| F4 | Selects the launch target; later F5 sessions reuse the current valid selection; press F4 again to change programs |

`ctrl+k ctrl+a` and `ctrl+k ctrl+z` are two-part key chords: press Ctrl+K, then Ctrl+A or Ctrl+Z. During initial setup, open the shortcut JSON from the Command Palette. After saving, use Ctrl+K, Ctrl+Z to open it again.

F6 uses `cmake.cleanConfigure` to clear the cache and reconfigure. Continue using `CMake: Configure` for a normal configuration and Build or F5 for compilation and linking. F4's `when` condition applies only to that binding.

Store these shortcuts in user configuration; no `.vscode/keybindings.json` is needed in the project. F5 retains VS Code's Start Debugging binding. The settings shortcuts open user configuration, while workspace files follow the project's conventions.

### 9.9 Remote-SSH Client Configuration (Optional)

Merge the following entry into the **client user's** SSH `config` file and replace every angle-bracket placeholder. Section 8.3 covers the file location and connection process. Use a separate `Host` entry for each host.

```sshconfig
Host <connection-alias>
    HostName <host-address>
    User <remote-username>
    Port <ssh-port>
    # For key authentication, uncomment the next line and set the client private-key path
    # IdentityFile ~/.ssh/id_ed25519
```

| Field | Value to Enter |
| --- | --- |
| `Host` | Client-side connection alias used by `ssh <connection-alias>` and Remote-SSH host selection |
| `HostName` | IP address or hostname reachable by the client; for port forwarding, use the forwarding entry's address |
| `User` | Remote Linux account name |
| `Port` | Client connection port; defaults to `22`; for forwarding, use the entry port |
| `IdentityFile` | Uncomment for key authentication and enter the client private-key path; enclose paths containing spaces in double quotes |

SSH configuration, private keys, and shortcuts belong to the client's user configuration; remote `authorized_keys` belongs to the remote account. Set up project files and development extensions in the remote environment as described in Section 8.

## References

- [CMake Presets manual](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html): Preset format, inheritance, platform conditions, and user presets.
- [VS Code CMake quick start](https://code.visualstudio.com/docs/cpp/cmake-quickstart): Project configuration, building, and debugging.
- [clangd compilation commands](https://clangd.llvm.org/design/compile-commands): Compilation databases and the compiler options used by language services.
- [CodeLLDB manual](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md): Debug configurations, backends, and platform-specific settings.
- [VS Code Remote-SSH](https://code.visualstudio.com/docs/remote/ssh): Connections, remote extensions, and server requirements.
