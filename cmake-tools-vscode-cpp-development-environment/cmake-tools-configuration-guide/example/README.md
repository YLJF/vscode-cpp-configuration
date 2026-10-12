# Add the Current VS Code Configuration to an Existing CMake Project: Complete Example

This guide starts with a C++17 project that already builds from the command line, then adds CMake Presets, clangd, and GDB/CodeLLDB debugging step by step. Each step retains the project's existing build logic and explains how the new configuration works with its source code, targets, and project options.

## 1. Choose how to practice

The example package includes the original project and the completed Windows and Linux configurations. You can practice from scratch or compare your configuration with the completed files.

```text
cmake-existing-project-example/
├── README.md                  # This integration guide
├── 01-existing-project/       # Original project: source, CMakeLists.txt, and build instructions
├── 02-vscode-windows/         # Completed configuration: Windows / MinGW
├── 03-vscode-linux/           # Completed configuration: Linux
└── tools/                     # Companion tools, outside the project directories
    ├── generate-cmake-presets.cmake
    └── CMakePresets.template.json
```

To complete the integration yourself, copy the original project from the example package directory. Make all subsequent changes in this practice copy.

Windows PowerShell:

```powershell
Copy-Item -LiteralPath "./01-existing-project" -Destination "./my-existing-project" -Recurse
Set-Location "./my-existing-project"
```

Linux:

```bash
cp -a ./01-existing-project ./my-existing-project
cd ./my-existing-project
```

Unless stated otherwise, run all subsequent commands from the `my-existing-project` root directory.

To try the completed configuration first, open `02-vscode-windows` or `03-vscode-linux` in VS Code, check the tool paths, and start with [section 7](#7-complete-the-first-configure-and-build-in-vs-code). Each directory is an independent CMake project; open the directory of your selected project.

When integrating your own project, follow the same sequence and match the example options and target names to your project's actual definitions. If the project already provides Presets, read [section 11](#11-integrate-a-project-that-already-has-presets) first, then reuse the editor integration steps that follow.

## 2. Identify the existing project's build entry point

Before editing configuration, inspect how the project organizes its source code and build entry point. The example contains an internal library, a main application, and an optional tool, with the following layout.

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

The root `CMakeLists.txt` defines the C++ standard, project options, and two executable targets, and includes the internal library through `add_subdirectory(lib)`:

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

The library directory continues to maintain its own source files, public include paths, and compile definitions. Linking the library to an application propagates the required include directory through the target dependency:

```cmake
add_library(acme_math STATIC src/math.cpp)
target_include_directories(acme_math PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(acme_math PRIVATE ACME_APP_LABEL="${ACME_APP_LABEL}")
```

These two CMake files and the original build instructions identify the information that the integration must preserve. For a real project, check the same items before choosing a toolchain and Preset.

| Item to verify | This example | Reference when integrating a real project |
| --- | --- | --- |
| CMake entry point | Root `CMakeLists.txt` | The `cmake -S` directory in the project's build instructions |
| Compiler and generator | GCC + Ninja | Compilers, toolchains, and generators supported by the project |
| C++ standard | C++17 | Requirements in the existing `CMakeLists.txt` |
| Project options | `ACME_BUILD_TOOLS=ON`, `ACME_APP_LABEL=team-demo` | Project `-D` arguments in the original build command |
| Executable targets | `acme_calc`, `acme_info` | `add_executable()` definitions and their controlling options |
| Dependencies | Internal `acme_math` library | Dependency installation steps, package manager, or toolchain file |

`ACME_BUILD_TOOLS` is disabled by default. The original build command enables `acme_info` with `-DACME_BUILD_TOOLS=ON`. Migrate project options like this together with the other settings; otherwise, targets that previously built may be missing after integration.

The existing `CMakeLists.txt` remains the source of configuration for targets, standards, definitions, and include paths. The editor reads CMake's results, so there is no need to maintain the same compilation information separately.

## 3. Run the original build commands first

After identifying the entry point, build and run the project in its original way. You can then compare whether the new configuration preserves the same behavior. This example uses GCC + Ninja and requires the following tools; prepare the corresponding tools before using other Presets.

CMake must be at least version 3.21 to use the version 3 Presets in this guide. The minimum version declared by the original project in `CMakeLists.txt` can remain 3.16.

```sh
cmake --version
ninja --version
gcc --version
g++ --version
```

Once the tool versions meet the requirements, run the original build commands from the project root, retaining both project options:

```sh
cmake -S . -B build/cli-debug -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DACME_BUILD_TOOLS=ON -DACME_APP_LABEL=team-demo
cmake --build build/cli-debug
```

Windows PowerShell:

```powershell
./build/cli-debug/acme_calc.exe 12 30
./build/cli-debug/acme_info.exe
```

Linux:

```bash
./build/cli-debug/acme_calc 12 30
./build/cli-debug/acme_info
```

The expected outputs are, respectively:

```text
team-demo: 12 + 30 = 42
```

```text
Application: team-demo
Library check: 6 + 7 = 13
```

Once both programs run successfully, the original build workflow provides a result for comparison. If dependency or compilation errors occur at this stage, resolve them using the project's instructions first. The Presets and VS Code configuration added later call this same workflow and still require the same dependencies and toolchain.

## 4. Generate a template for a project without Presets

### 4.1 Generate the file

The original command already specifies the compiler, generator, and project options. Save them as Presets so you can invoke them from the editor or terminal. The practice project does not yet provide `CMakePresets.json`, so first create a template using the generator in `../tools`, outside the project.

Windows:

```powershell
cmake -DPLATFORM=windows -P "../tools/generate-cmake-presets.cmake"
```

Linux:

```bash
cmake -DPLATFORM=linux -P "../tools/generate-cmake-presets.cmake"
```

The output is `CMakePresets.json` in the current project root. When calling the guide's original tool from another directory, you can specify the output location explicitly:

```sh
cmake -DOUTPUT_DIR="<cmake-entry-directory>" -P "<guide-directory>/tools/generate-cmake-presets.cmake"
```

Place every `-D` argument before `-P`. Keep the script next to `CMakePresets.template.json`. Edit the generated file in the project afterward; the tools remain in the guide's companion directory.

For a project that already has `CMakePresets.json`, follow section 11. By default, the generator preserves the existing file and stops.

### 4.2 Migrate the project's existing options

The generated template includes common tool combinations, but you still need to add the original project's own options. Open `CMakePresets.json`, find the hidden `base` preset, and add `ACME_BUILD_TOOLS` and `ACME_APP_LABEL` from the earlier command to `cacheVariables`.

The updated **`base` object** is shown below. Retain the other Configure Presets and Build Presets in the file:

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

These two project options apply to all combinations, so they belong in `base`. Put options that apply to only one combination in that Configure Preset.

`CMAKE_EXPORT_COMPILE_COMMANDS=ON` generates the compilation database required by clangd. Include directories and compile definitions still come from the original CMake target definitions.

Compare the complete generated file with [Windows CMakePresets.json](02-vscode-windows/CMakePresets.json) or [Linux CMakePresets.json](03-vscode-linux/CMakePresets.json).

### 4.3 Check the presets and directories

After adding the project options, choose a build combination. The template gives each Configure Preset and its Build Preset the same name, making it easy to find the matching build entry after selecting a configuration.

| Name | Compiler | Generator | Build type |
| --- | --- | --- | --- |
| `gcc-ninja-debug` | `gcc` / `g++` | Ninja | Debug |
| `gcc-ninja-release` | `gcc` / `g++` | Ninja | Release |
| `clang-ninja-debug` | `clang` / `clang++` | Ninja | Debug |
| `clang-ninja-release` | `clang` / `clang++` | Ninja | Release |
| `gcc-make-debug` | `gcc` / `g++` | Windows: MinGW Makefiles; Linux: Unix Makefiles | Debug |
| `gcc-make-release` | `gcc` / `g++` | Same as above | Release |
| `clang-make-debug` | `clang` / `clang++` | Same as above | Debug |
| `clang-make-release` | `clang` / `clang++` | Same as above | Release |

Each preset uses a separate directory, for example:

```text
build/
├── gcc-ninja-debug/
├── clang-ninja-debug/
└── gcc-make-debug/
```

The original `build/cli-debug` can still be used for the original commands and does not share a cache with the new presets. Use the corresponding new directory when changing compiler or generator.

The Windows example uses GCC/Clang from MSYS2 MINGW64. If you use UCRT64, consistently use tools from that environment. Make sure `clang++` does not accidentally resolve to a different Clang installation that generates MSVC targets by default. Linux Make presets require `make`, while Windows MinGW Makefiles presets require `mingw32-make`.

## 5. Merge VS Code workspace settings

### 5.1 Open the project and install extensions

Once Presets preserve the original build command, connect the project to VS Code. Open the **`my-existing-project` root directory** and install or enable the following extensions so the editor can invoke CMake, start the language server, and launch a debugger.

| Extension | ID | Purpose |
| --- | --- | --- |
| CMake Tools | `ms-vscode.cmake-tools` | Configure, Build, and target selection |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | C/C++ language services and formatting |
| C/C++ | `ms-vscode.cpptools` | GDB debugging |
| CodeLLDB | `vadimcn.vscode-lldb` | LLDB debugging |

Install C/C++ or CodeLLDB for the debugger you use. Install both if you retain both debugging configurations. The clangd executable must also be available for the extension to start.

With Remote-SSH, open the remote project in the remote window and install CMake Tools, clangd, and your debugging extension on the remote host. Prepare the compiler, build tools, clangd, and GDB there as well. The following project configuration uses remote file paths.

### 5.2 `.vscode/settings.json`

After enabling the extensions, specify how they work together in `.vscode/settings.json`. The complete Windows settings are shown below:

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

On Linux, use the same settings and replace the `--query-driver` argument in `clangd.arguments` with:

```text
--query-driver=/usr/bin/gcc,/usr/bin/g++,/usr/bin/clang,/usr/bin/clang++
```

For the complete Linux file, see [Linux settings.json](03-vscode-linux/.vscode/settings.json).

Keep only the toolchain paths you use in `--query-driver`. On Windows with UCRT64, change `mingw64` in the patterns to `ucrt64`. Adjust custom installation locations or versioned compiler names to match their actual paths. Separate multiple patterns with commas within a single argument.

| Setting | Reason |
| --- | --- |
| `cmake.useCMakePresets` | Configure and Build use the Presets containing the migrated project options |
| Three automatic configuration switches set to `false` | Run Configure explicitly so it is clear when build rules are updated |
| `cmake.buildBeforeRun` | Build before debugging when dynamically resolving the launch target |
| `C_Cpp.intelliSenseEngine=disabled` | clangd provides language services, while the C/C++ extension continues to provide GDB debugging |
| `--compile-commands-dir` | Read the database from the current Configure Preset's build directory |
| `--query-driver` | Allow clangd to query matching drivers for the toolchain's standard-library search paths |
| C/C++ formatting settings | Use clangd and retain the project's existing `.clang-format` |

If the project already has `settings.json`, merge the corresponding keys into its existing content. Retain other child keys in `[cpp]` and `[c]`, keep any still-needed `clangd.arguments`, and ensure the database-directory argument uses only the current approach. If existing `cmake.configureArgs`, `cmake.configureSettings`, or similar settings duplicate or conflict with the Preset, migrate any required build options before removing the old values.

With this language-service configuration, C++17, compile definitions, and library include paths all come from CMake's compilation commands. There is no need to add `c_cpp_properties.json` to repeat them.

## 6. Merge GDB / CodeLLDB debugging configuration

The workspace settings determine how to configure and build; `launch.json` determines how to start debugging. Create `.vscode/launch.json` with the following content. If the file already exists, add the required entries to its existing `configurations` array and retain other configurations and `compounds`.

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

CMake Tools returns the selected executable target's actual path for `program`. You do not need to guess which subdirectory contains the executable or manually add the Windows `.exe` suffix.

GDB is located through the PATH inherited by VS Code. Use your own debugging entry if you need a personal path. CodeLLDB uses the extension's bundled LLDB backend by default. The debugger must support the target architecture and the debugging information generated by the compiler.

The current dynamic `program` and `cmake.buildBeforeRun=true` already arrange a build before debugging, so do not add a duplicate `preLaunchTask` to these entries. Retain existing build tasks for other debugging entries according to their purpose.

Both example programs can run with no arguments and the project root as their working directory. For a real project's arguments, working directory, and environment variables, follow [section 10](#10-programs-that-need-arguments-resource-directories-or-environment-variables).

## 7. Complete the first Configure and Build in VS Code

The project's build options, language services, and debugging entry points are now connected. Press `Ctrl+Shift+P` to open the Command Palette, then complete the first Configure and Build so these settings produce actual build rules and programs.

1. Run **`CMake: Select Configure Preset`** and select `gcc-ninja-debug`.
2. Run **`CMake: Configure`** and wait for it to succeed.
3. Run **`clangd: Restart language server`** so clangd uses the selected build directory.
4. Run **`CMake: Select Build Preset`** and select `gcc-ninja-debug`.
5. Run **`CMake: Build`** and wait for both programs to finish building.

After Configure succeeds, the directory should contain:

```text
build/gcc-ninja-debug/
├── CMakeCache.txt
├── compile_commands.json
├── build.ninja
└── ...
```

After Build succeeds, this example also contains `acme_calc.exe` and `acme_info.exe`, or `acme_calc` and `acme_info` on Linux. Other projects may use output-directory settings to place programs in `bin` or deeper subdirectories; the dynamic `program` reads the path supplied by CMake.

`compile_commands.json` is generated after Configure succeeds; clangd does not need to wait for Build. Open `app/calc.cpp`: the include path for `acme/math.hpp` is propagated through the `acme_math` CMake target dependency. When you open the library source, the `ACME_APP_LABEL` definition comes from the original `target_compile_definitions()`. You do not need to maintain these details again in the editor.

Keep the database in the build directory rather than copying it to the project root. Command variables are resolved when clangd starts, so restart the language server after the first configuration and after changing the Configure Preset.

You can still invoke the same Presets from the terminal. To reproduce a configuration or build problem, use these equivalent commands from the project root:

```sh
cmake --list-presets
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

Terminal commands do not update CMake Tools' preset selections for you. On returning to VS Code, select the same Configure/Build Presets and restart clangd when the language-service configuration needs updating.

## 8. Debug the two executables one at a time

After both programs build, launch each with the same debugging configuration. The debugging configuration selects GDB or CodeLLDB; the target-selection command determines which program runs this time.

### 8.1 Debug `acme_calc` first

1. Run **`CMake: Set Launch/Debug Target`** and select **`acme_calc`**.
2. Open `app/calc.cpp` and set a breakpoint on the `std::cout` line that prints the result.
3. Select **`CMake / GDB`** or **`CMake / LLDB (CodeLLDB)`** from the dropdown in the Run and Debug panel.
4. Save your changes and press **F5**.
5. When the program stops at the breakpoint, inspect `left`, `right`, and `result`. Their default values are `2`, `3`, and `5`, respectively.
6. Continue execution. The output should be `team-demo: 2 + 3 = 5`.

After this debugging session, CMake Tools retains the current valid `acme_calc` selection. Press F5 again to keep debugging the same program.

### 8.2 Debug `acme_info` next

To switch to the information tool, keep the current preset and debugger and change only the launch target.

1. Stop the current debugging session.
2. Run **`CMake: Set Launch/Debug Target`** and select **`acme_info`**.
3. Open `app/info.cpp` and set a breakpoint on the first `std::cout` line.
4. Keep the same GDB or CodeLLDB debugging configuration and press **F5**.
5. Inspect `check` at the breakpoint; its value should be `13`. Continuing execution prints the application label and library-check result.

Switching between existing launch targets does not require another Configure or a clangd restart. CMake Tools remembers the current valid selection. Confirm it again if the target disappears, the editor state is reset, or the environment changes.

### 8.3 Distinguish the three selections

The preceding steps involve three selections, controlling the build environment, program to run, and debugging method. Understanding their roles lets you change only the relevant parts later.

| Selection | Value in this example | What it determines |
| --- | --- | --- |
| Configure/Build Preset | `gcc-ninja-debug` and others | Toolchain, build type, and build directory |
| Launch/Debug Target | `acme_calc` or `acme_info` | Which program to launch |
| Run and Debug configuration | GDB or CodeLLDB | Debug adapter, arguments, and working directory |

`acme_math` is a library and cannot be launched as a program. If `acme_info` does not appear, check that `ACME_BUILD_TOOLS` is `ON` in the current Configure Preset, then run Configure again.

The Build Presets in this guide do not set `targets`, and no other build scope has been selected: a normal Build builds the default targets, including both programs. In the current preset mode, the build before F5 builds the selected launch target and its dependencies. If a real project sets another build target or a Build Preset's `targets`, check that scope too. Selecting a launch target does not change the scope of a normal Build.

### 8.4 Optional: configure custom keyboard shortcuts

If you frequently switch programs, restart language services, or adjust personal settings, add these five user bindings. Run `Preferences: Open Keyboard Shortcuts (JSON)` and merge the entries into the existing array in your user `keybindings.json`.

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

| Custom shortcut | Command | Command ID |
| --- | --- | --- |
| F6 | `CMake: Delete Cache and Reconfigure` | `cmake.cleanConfigure` |
| Alt+S | `clangd: Restart language server` | `clangd.restart` |
| Ctrl+K, Ctrl+A | `Preferences: Open User Settings (JSON)` | `workbench.action.openSettingsJson` |
| Ctrl+K, Ctrl+Z | `Preferences: Open Keyboard Shortcuts (JSON)` | `workbench.action.openGlobalKeybindingsFile` |
| F4 | `CMake: Set Launch/Debug Target` | `cmake.selectLaunchTarget` |

`ctrl+k ctrl+a` and `ctrl+k ctrl+z` are two-step key chords: press Ctrl+K first, then Ctrl+A or Ctrl+Z. For the initial setup, open the keyboard-shortcut JSON from the Command Palette. After saving it, use Ctrl+K, Ctrl+Z to open the file again.

F4 invokes **`CMake: Set Launch/Debug Target`**. Press F4 to switch programs, then use F5 to debug. To continue debugging the same program, press F5 directly. Alt+S restarts clangd. After changing the Configure Preset, run Configure first, then press Alt+S so the language server reads the database in the new directory.

F6 clears the CMake configuration cache in the current build directory and runs Configure again. For a normal configuration, use `CMake: Configure` from the Command Palette. After saving source changes, update the program with Build or F5. F6 itself configures the project; build the target afterward as needed.

Ctrl+K, Ctrl+A opens the user `settings.json`; project settings remain in `.vscode/settings.json`. Ctrl+K, Ctrl+Z opens the user `keybindings.json`. Keyboard shortcuts are user configuration, so the project does not need `.vscode/keybindings.json`. When setting up F4 for the first time, check its `when` condition for conflicts with other bindings. The Command Palette can perform the same operations.

## 9. Try incremental builds and switch Presets

After the first debugging session, modify the source and switch presets to see how configuration participates in everyday development. Source changes mainly trigger incremental builds; toolchain or build-rule changes require updated CMake configuration.

### 9.1 Modify source code

Change the default `right` value in `app/calc.cpp` from `3` to `4`, save, and press F5. The build tool recompiles the affected source file and links `acme_calc`. The output becomes `team-demo: 2 + 4 = 6`.

After practicing, you can restore the default to `3` and save. Other changes also update outputs according to their scope of impact. Common cases are listed below.

| Change | Action | Build behavior |
| --- | --- | --- |
| Source, dependencies, and build rules are unchanged | F5 | Check build status; no recompilation or linking is needed |
| Saved a `.cpp` change | F5 | Recompile affected source and link the relevant targets |
| Saved a header change | F5 | Update compilation units that depend on the header and the relevant targets |
| Changed `CMakeLists.txt`, project options, definitions, or compiler/linker options | Configure → restart clangd if needed → F5 | Update build rules first, then update outputs based on dependencies and the new rules |
| Changed compiler, generator, or build type | Select and configure the matching Configure/Build Presets | Use that combination's separate directory; existing valid outputs can be reused |

Reconfiguring does not mean a full rebuild. Everyday changes are updated incrementally according to their impact. Adjust project build rules through CMake files and Presets.

### 9.2 Switch to Clang, Make, or Release

Besides modifying source code, you can reuse the existing configuration to switch tool combinations. For `clang-ninja-debug`, for example, select the new preset, update the build rules and language services, then continue debugging.

1. Prepare `clang`, `clang++`, and Ninja for the platform, and ensure `--query-driver` matches the actual drivers.
2. Run `CMake: Select Configure Preset` and select `clang-ninja-debug`.
3. Run `CMake: Configure`.
4. Run `clangd: Restart language server`.
5. Run `CMake: Select Build Preset` and select `clang-ninja-debug`.
6. Confirm that the Launch/Debug Target is still the program you want to debug, then press F5.

Follow the same sequence for `gcc-make-debug` or `clang-make-debug`, using the platform's Make tool. Release presets are mainly for release builds. Use Debug when practicing breakpoints and variable inspection.

## 10. Programs that need arguments, resource directories, or environment variables

The two programs above can run with their default arguments and the project root as their working directory. A real project may need command-line arguments, resource files, or a runtime environment. Configure these requirements in the debugging entry.

### 10.1 Pass arguments

Use the calculator to practice passing arguments. Stop debugging and change `args` in the currently used `launch.json` entry to:

```json
"args": ["12", "30"]
```

Select `acme_calc` with `CMake: Set Launch/Debug Target` and press F5. The output should be `team-demo: 12 + 30 = 42`.

`args` is an array of strings. Settings in a debugging configuration apply whenever that entry is used. Switching launch targets does not automatically change the arguments to those required by another program.

### 10.2 Adjust the working directory and environment variables

If a program reads resources through relative paths, set `cwd` according to the project's runtime instructions. For example, use `${workspaceFolder}` when resource paths are relative to the project root, or `${workspaceFolder}/runtime` when they are relative to the project's `runtime` directory. This example has no external runtime resources, so the project root is sufficient.

The two adapters use different environment-variable syntax. The following fragments are fields to merge into the selected debugging entry. Replace the example paths with resource directories that actually exist.

GDB:

```json
"cwd": "${workspaceFolder}",
"args": ["12", "30"],
"environment": [
  { "name": "ACME_DATA_DIR", "value": "${workspaceFolder}/resources" }
]
```

CodeLLDB:

```json
"cwd": "${workspaceFolder}",
"args": ["12", "30"],
"env": {
  "ACME_DATA_DIR": "${workspaceFolder}/resources"
}
```

Put build-time dependency locations in the CMake configuration, and runtime arguments and environment settings in the debugging configuration.

When programs have many differing runtime requirements, copy the required debugging entry and give it a descriptive name, such as `Calculator / GDB` or `Information / GDB`. If you retain the dynamic `program`, you still need to confirm the launch target each time; the entry's name does not fix it to a specific program. To debug multiple programs at once, define separate program paths and builds before debugging for each, then organize them into `compounds`.

## 11. Integrate a project that already has Presets

The preceding exercise starts with a project without Presets. If a real project already provides them, reuse them directly during integration. Those presets often also configure third-party dependencies, toolchains, or cross-compilation environments. Select a Debug preset provided by the project, then connect VS Code using sections 5–8 to preserve the project's existing build conventions.

### 11.1 Shared change: enable the compilation database

For a setting shared by the team, add the following to an appropriate existing Configure Preset or common parent preset:

```json
"CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
```

Retain other `cacheVariables` keys, `toolchainFile`, `environment`, and inheritance relationships. Ninja and Make generators can generate the database using this guide's approach. If an existing preset uses Visual Studio or another generator, add a compatible preset according to the project's supported setup, or use the project's language-service approach.

### 11.2 Personal change: inherit existing presets

Suppose the project already has a Configure Preset and a Build Preset both named `gcc-ninja-debug`. Create `CMakeUserPresets.json` in the **same CMake entry directory**:

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

After saving, CMake Tools lists `local-gcc-ninja-debug`. Select it and follow section 7 to Configure, restart clangd, and select the Build Preset. You can then use the inherited project configuration.

Make the following adjustments when using this example:

- Change `inherits` to the project's actual preset names. Configure and Build presets each inherit an existing preset of their own type. If the project has no corresponding Build Preset, remove `inherits` from the Build Preset above and retain `configurePreset`.
- Keep the file's `version` compatible with the project's Presets format and required CMake version. If the project uses a newer format, follow its existing format.
- The example `binaryDir` separates the personal cache. If the project has a specific build-directory convention, follow it.
- Add differences such as personal dependency locations to the local preset's `cacheVariables`. Do not remove required project options inherited from the parent.
- The inherited Build Preset retains settings such as `targets`. Confirm that its scope meets the project's build requirements, and select a program with the launch-target command.

`CMakeUserPresets.json` implicitly includes `CMakePresets.json` from the same directory, so it can inherit public presets without an additional `include`. Add the personal file to `.gitignore`.

## 12. Map the example steps to a real project

Whether using a newly generated template or presets already provided by the project, preserve the original build requirements when integrating a real project. The following sections map command arguments and directory layouts to their configuration locations.

### 12.1 Migrate the original build command

Use the original build command as the reference and migrate the generator, build directory, project options, and dependency locations together. Common arguments and their corresponding locations in Presets or debugging configuration are listed below.

| Original command or requirement | Location in Presets |
| --- | --- |
| `-G Ninja` / Make generator | Configure Preset's `generator` |
| `-B <build-directory>` | `binaryDir` |
| `-DCMAKE_BUILD_TYPE=Debug` | `cacheVariables.CMAKE_BUILD_TYPE` for the single-configuration generators in this guide |
| `-DCMAKE_C_COMPILER`, `-DCMAKE_CXX_COMPILER` | Corresponding `cacheVariables`; follow project conventions when using a toolchain file |
| Project `-D` options and definition-configuration variables | Project variables of the same name in `cacheVariables` |
| `-DCMAKE_PREFIX_PATH=...` or dependency-package directory | Same-named `cacheVariables`; personal paths belong in User Presets |
| `-DCMAKE_TOOLCHAIN_FILE=...` | Existing toolchain configuration, or the equivalent Configure Preset `toolchainFile` |
| `cmake --build ... --target <target>` | Build Preset's `targets` or an explicit Build command |
| Program arguments, runtime working directory, and environment variables | Corresponding adapter fields in `launch.json` |

CMake continues to maintain the project's `find_package()`, `target_link_libraries()`, `target_include_directories()`, and target C++ standards. Add build logic for the project's actual needs only if the original project does not correctly declare its dependencies.

If a real project generates only libraries, enable test, example, or tool executables through its project options, or use an executable project that depends on the library, before selecting a local launch/debug target.

### 12.2 The CMake entry point is in a workspace subdirectory

Some repositories contain several types of components, so the directory opened in VS Code differs from the CMake entry point. Suppose the workspace is `repository` and the required project's top-level CMake entry point is in `components/acme`:

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

1. Open `repository` in VS Code.
2. Put Presets in `components/acme`, the top-level CMake entry directory you are using.
3. Add the following to the workspace `.vscode/settings.json`:

```json
"cmake.sourceDirectory": "${workspaceFolder}/components/acme"
```

4. Continue with sections 7–8. Keep `--compile-commands-dir=${command:cmake.buildDirectory}` and the dynamic `program` unchanged.
5. If the program requires the entry directory as its working directory, change the debugging entry's `cwd` to `${workspaceFolder}/components/acme`; otherwise, follow the program's own runtime conventions.

Here, `${sourceDir}` points to `components/acme` and the example database path is `components/acme/build/gcc-ninja-debug/compile_commands.json`. `${workspaceFolder}` still points to `repository`.

When using Presets from the terminal, switch to the entry directory first:

```sh
cd components/acme
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug
```

This integrates one existing top-level CMake project. Library directories already managed by `add_subdirectory()` remain its subprojects rather than being split into separate VS Code projects. In a multi-root workspace, confirm CMake Tools' active project before using target commands.

## 13. Organize project configuration

After integration, organize the files added during this process. This example keeps the source code and CMake target definitions unchanged; the additions are concentrated in Presets and `.vscode`.

```text
my-existing-project/
├── CMakePresets.json
└── .vscode/
    ├── settings.json
    ├── launch.json
    └── extensions.json        # Optional: extension recommendations
```

To recommend extensions when other team members open the project, you can also add `.vscode/extensions.json`:

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

Extension recommendations do not install extensions automatically. If the file already exists, merge the required IDs into its existing `recommendations` array.

Append the required rules to the project's existing `.gitignore`:

```gitignore
/build/
/CMakeUserPresets.json
/CMakePresets.json.bak-*
/.cache/
```

This example already had `/build/`, so append only the other three entries. If the build directory is in a workspace subdirectory, maintain the rules in the corresponding project directory or specify the actual relative path in the repository-root `.gitignore`.

Commit shared Presets and required `.vscode` configuration according to the project's conventions, retaining the original source code, CMake files, and `.clang-format`. Personal Presets, build outputs, and user keyboard shortcuts do not belong in shared project configuration. Maintain the generator tools with the guide; the project keeps only the generated configuration adjusted to its needs.

## 14. Diagnose integration problems by symptom

When a problem occurs, compare the original build command with the current Preset first, then check the VS Code workspace, tool paths, and target selections. The following table lists checks for common symptoms during integration.

| Symptom | Checks and action |
| --- | --- |
| Preset cannot be found | Check the directory opened in VS Code, `cmake.sourceDirectory`, and the Presets file's location and format version |
| Command-line build works, but VS Code cannot find tools | Check the PATH inherited by VS Code, the remote execution location, and whether the project needs a toolchain environment; restart VS Code after updating the environment |
| Configuration reports missing third-party dependencies | Compare with the original build command and check that dependency-installation steps, project options, package directories, and toolchain files have been migrated |
| `compile_commands.json` cannot be found | Check the current Preset's database setting and generator, complete Configure first, then inspect the actual build directory |
| Standard-library headers show errors | Check that `--query-driver` matches the compiler and its toolchain in the database |
| clangd still uses the old configuration after switching Presets | Run `clangd: Restart language server` after Configure succeeds |
| A program is missing from the target list | Check the project option controlling that target; the example requires `ACME_BUILD_TOOLS=ON`, followed by Configure after the change |
| F5 launches another program | Run `CMake: Set Launch/Debug Target`, confirm the target, then check the selected debugging entry's arguments |
| An existing debugging entry does not trigger a build | Check whether its `program` uses the current dynamic approach, or whether its original build-before-debugging task is correct |
| Breakpoint is not hit or variables are unavailable | Use matching Debug outputs and a compatible debugger, set a breakpoint on a source line that actually executes, and check that source and binary correspond |
| Program cannot find resources at relative paths | Set `cwd` and runtime environment variables according to the project's runtime requirements |
