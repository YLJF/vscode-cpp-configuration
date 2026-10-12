# VS Code C++ Development Environment with CMake Tools

These guides cover VS Code C++ development on Windows and Linux, including toolchain installation, CMake project configuration, language services, debugging, environment synchronization, and complete examples of integrating the configuration into an existing CMake project.

CMake Tools and CMake Presets manage configuration and builds, clangd provides completion, diagnostics, and formatting, and GDB or CodeLLDB provides debugging. The Windows guide uses an MSYS2 native MinGW toolchain; the Linux guide covers both local development and Remote-SSH.

## Guide Navigation

Choose an entry point for your task. Each guide provides the relevant configuration files and instructions.

| Task | Guide |
| --- | --- |
| Set up a development environment on Windows | [Windows development environment guide](cmake-tools-configuration-guide/vsconfig/windows-development-environment-guide.md) |
| Set up a Linux environment or develop over SSH | [Linux development environment guide](cmake-tools-configuration-guide/vsconfig/linux-development-environment-guide.md) |
| Integrate the configuration into an existing CMake project | [Integration tutorial and complete examples](cmake-tools-configuration-guide/example/README.md) |
| Restore the environment on another Windows computer | [Windows environment synchronization guide](cmake-tools-configuration-guide/vssync/windows-environment-sync-guide.md) |
| Restore configuration in a local or remote Linux environment | [Linux environment synchronization guide](cmake-tools-configuration-guide/vssync/linux-environment-sync-guide.md) |

For a new environment, start with the guide for your operating system. Once the tools are ready, try the complete example below. When integrating your own project, follow the integration tutorial and retain its existing targets, dependencies, C++ standard, and build options while adding the required configuration. When moving to another computer or development host, use the environment synchronization guide.

## Configuration Overview

| Feature | Configuration |
| --- | --- |
| Compiler | GCC / G++ or Clang / Clang++ |
| Build generator | Ninja; MinGW Makefiles on Windows; Unix Makefiles on Linux |
| Build combinations | Presets distinguish the compiler, generator, and Debug / Release configuration; each combination has its own build directory |
| Language services | clangd reads `compile_commands.json` from the current Configure Preset's build directory |
| Debugging | GDB with the C/C++ extension, or LLDB with the CodeLLDB extension |
| Multiple executables | Select the launch target through CMake Tools; F5 starts the selected program |
| Remote development | Connect to a Linux host with Remote-SSH and configure, build, and debug on the remote side |
| Environment synchronization | Settings Sync synchronizes editor configuration; Git synchronizes source and shared project configuration; install the toolchain on the destination host |

The main guides use C++20. The existing-project examples in `cmake-tools-configuration-guide/example/` use C++17 to demonstrate integration while retaining the project's original standard.

## Run the Complete Example

### 1. Prepare the Tools

Follow the guide for your operating system to install VS Code, Git, CMake, clangd, and your chosen compiler, build tools, and debugger. The complete integration example requires CMake 3.21 or later.

For the first run, use **GCC + Ninja + Debug + GDB**. Install CMake Tools, clangd, and the C/C++ extension in VS Code; install CodeLLDB if you choose LLDB. On Windows, use a compatible MSYS2 toolchain environment and ensure the tools can be found from the terminal environment used by VS Code.

### 2. Clone the Repository and Open a Project

Run the following in PowerShell or a Linux terminal:

```sh
git clone https://github.com/YLJF/vscode-configuration.git
cd vscode-configuration/cmake-tools-vscode-cpp-development-environment
```

In VS Code, use **File → Open Folder** and select the directory for your operating system. The following paths are relative to this CMake Tools directory:

| Operating system | Project directory to open |
| --- | --- |
| Windows | `cmake-tools-configuration-guide/example/02-vscode-windows` |
| Linux | `cmake-tools-configuration-guide/example/03-vscode-linux` |

Each directory contains its own `CMakeLists.txt`, `CMakePresets.json`, and `.vscode/` configuration. Before the first run, check clangd's `--query-driver` patterns and the debugger paths against your actual toolchain installation.

### 3. Configure and Build

Run these commands in order from the Command Palette (Ctrl+Shift+P):

1. `CMake: Select Configure Preset`: select `gcc-ninja-debug`.
2. `CMake: Configure`: generate the build rules and compilation database.
3. `clangd: Restart language server`: let the language server read the newly generated database.
4. `CMake: Select Build Preset`: select the matching `gcc-ninja-debug` preset.
5. `CMake: Build`: build `acme_calc` and `acme_info`.

Generated files are stored in the project's `build/gcc-ninja-debug/` directory. For Clang or Make, select the corresponding preset and prepare the required tools. After switching Configure Presets, configure again, restart clangd, and select the matching Build Preset.

### 4. Select a Program and Debug

Set a breakpoint in `app/calc.cpp`, run **`CMake: Set Launch/Debug Target`**, and select `acme_calc`. In the Run and Debug view, select `CMake / GDB` and press F5. For CodeLLDB, select `CMake / LLDB (CodeLLDB)`.

To debug the other program, end the current session, run the target-selection command again, select `acme_info`, and press F5. Later F5 runs reuse the current valid target selection.

The example enables `cmake.buildBeforeRun`, so F5 triggers a build before launching. Ninja or Make performs an incremental build based on dependencies: normally, only affected files are recompiled and targets requiring updates are relinked. If nothing has changed, recompilation is unnecessary.

To work through integration from the original project, follow the [integration tutorial](cmake-tools-configuration-guide/example/README.md), copy `cmake-tools-configuration-guide/example/01-existing-project`, and add the configuration step by step.

## Generate a CMake Presets Template

Run this command from this CMake Tools directory, where this README is located, to generate the template in a project of your choice:

```sh
cmake -DOUTPUT_DIR="<project-root>" -P "./cmake-tools-configuration-guide/vsconfig/tools/generate-cmake-presets.cmake"
```

Replace `<project-root>`, including the angle brackets, with the actual path. The script chooses the Windows or Linux Make generator for the host and creates eight preset combinations: GCC / Clang, Ninja / Make, and Debug / Release. Preset names include the generator, for example `gcc-ninja-debug` and `gcc-make-debug`.

The script generates `CMakePresets.json`; set up project targets, dependencies, and editor configuration according to the guide. Adjust compilers, build options, and preset combinations as the project requires. By default, the script refuses to overwrite an existing `CMakePresets.json`; for an existing project, extend its original presets first. See the [Windows guide](cmake-tools-configuration-guide/vsconfig/windows-development-environment-guide.md) or [Linux guide](cmake-tools-configuration-guide/vsconfig/linux-development-environment-guide.md) for arguments and file maintenance.

## Custom Keyboard Shortcuts

These shortcuts take effect after you merge them into VS Code's **user `keybindings.json`** according to the guide. The same operations are available from the Command Palette.

| Shortcut | Action |
| --- | --- |
| F4 | Select the launch/debug target: `CMake: Set Launch/Debug Target` (`cmake.selectLaunchTarget`) |
| F6 | Delete the CMake configuration cache and configure again; build afterward as needed |
| Alt+S | Restart clangd; after switching presets, configure before restarting |
| Ctrl+K, Ctrl+A | Open user settings as JSON |
| Ctrl+K, Ctrl+Z | Open user keyboard shortcuts as JSON |

Press each Ctrl+K sequence in two steps: Ctrl+K first, then Ctrl+A or Ctrl+Z. The F4 condition and complete JSON are included in the final configuration reference of both platform guides. VS Code manages user shortcuts; project configuration belongs in `.vscode/`.

## Directory Structure

Configuration guides, environment synchronization instructions, and complete integration examples are grouped under `cmake-tools-configuration-guide/` for easy reference.

```text
cmake-tools-vscode-cpp-development-environment/
├── README.md
├── cmake-tools-configuration-guide/
│   ├── vsconfig/                 # Windows / Linux development environment guides
│   │   └── tools/                # Presets generator script and template
│   ├── vssync/                   # Windows / Linux environment synchronization guides
│   └── example/
│       ├── README.md             # Existing CMake project integration tutorial
│       ├── 01-existing-project/  # Original project before integration
│       ├── 02-vscode-windows/    # Complete Windows integration
│       ├── 03-vscode-linux/      # Complete Linux integration
│       └── tools/               # Generator tools included with the examples
├── .vscode/                      # VS Code configuration for the single-target example
├── CMakeLists.txt                # Single-target example; target name: test
├── CMakePresets.json             # Presets for the single-target example
└── src/                         # Source for the single-target example
```

This directory also retains a single-target `test` example. To try the complete integration workflow and debugging multiple executables, open the individual project for your operating system under `cmake-tools-configuration-guide/example/` as described above.

Shared source, CMake configuration, and editor project configuration are tracked in Git. Build directories, compilation databases, local `CMakeUserPresets.json`, validation records, and backups are excluded by `.gitignore`. Set personal tool paths and SSH connection details for your actual environment.
