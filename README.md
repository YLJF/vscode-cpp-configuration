# VS Code C++ Development Environment Configuration

This repository provides two sets of development environment guides, based on CMake Tools and Makefile Tools. Both cover Windows, Linux, and Linux development over Remote-SSH. Choose the guide that matches your project's build system.

| Project build system | Configuration guide |
| --- | --- |
| CMake | [CMake Tools guides and examples](cmake-tools-vscode-cpp-development-environment/README.md) |
| GNU Make / Makefile | [Makefile Tools guides and examples](makefile-tools-vscode-cpp-development-environment/README.md) |

Both approaches use clangd for language services and support debugging with GDB or CodeLLDB. Each directory contains platform-specific instructions and example projects. See its README for installation and usage details.

## Repository Structure

```text
vscode-configuration/
├── cmake-tools-vscode-cpp-development-environment/
├── makefile-tools-vscode-cpp-development-environment/
└── README.md
```

## Getting Started

Open the directory for your build system and read its README, then choose the guide for your operating system. To run an example, open that individual example project in VS Code and follow the guide to configure, build, and debug it.
