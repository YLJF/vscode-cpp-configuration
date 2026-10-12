# VS Code C++ 开发环境配置

本仓库提供基于 CMake Tools 和 Makefile Tools 的两套配置指南，覆盖 Windows、Linux，以及 Linux Remote-SSH 开发。根据项目采用的构建方式，选择对应入口。

| 项目构建方式                      | 配置入口                                                     |
| --------------------------------- | ------------------------------------------------------------ |
| 使用 CMake 管理项目               | [CMake Tools 配置指南与示例](<基于CMake Tools的VSCode C++开发环境配置/README.md>) |
| 使用 GNU Make / Makefile 管理项目 | [Makefile Tools 配置指南与示例](<基于Makefile Tools的VSCode C++开发环境配置/README.md>) |

两套方案均使用 clangd 提供语言服务，并支持通过 GDB 或 CodeLLDB 调试。各目录包含对应系统的配置说明和示例工程，具体安装与操作步骤见各自的 README。

## 仓库结构

```text
vscode-configuration/
├── 基于CMake Tools的VSCode C++开发环境配置/
├── 基于Makefile Tools的VSCode C++开发环境配置/
└── README.md
```

## 开始使用

先进入对应方案的目录，阅读其中的 README，再按操作系统选择配置指南。运行示例时，在 VS Code 中单独打开具体的示例工程目录，按照指南完成配置、构建和调试。