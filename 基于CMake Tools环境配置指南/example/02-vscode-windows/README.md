# AcmeCalc

一个已有的 CMake 工程：C++17、一个静态库、一个主程序和一个可选工具。

在项目根目录执行原有命令：

```sh
cmake -S . -B build/cli-debug -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DACME_BUILD_TOOLS=ON -DACME_APP_LABEL=team-demo
cmake --build build/cli-debug
```

运行主程序（Windows PowerShell）：

```powershell
./build/cli-debug/acme_calc.exe 12 30
./build/cli-debug/acme_info.exe
```

运行主程序（Linux）：

```bash
./build/cli-debug/acme_calc 12 30
./build/cli-debug/acme_info
```

主程序应输出 `team-demo: 12 + 30 = 42`。

`ACME_BUILD_TOOLS=ON` 启用 `acme_info`，`ACME_APP_LABEL` 设置输出标签。
