# AcmeCalc

An existing CMake project with C++17, a static library, a main application, and an optional tool.

Run the original commands from the project root:

```sh
cmake -S . -B build/cli-debug -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DACME_BUILD_TOOLS=ON -DACME_APP_LABEL=team-demo
cmake --build build/cli-debug
```

Run the programs (Windows PowerShell):

```powershell
./build/cli-debug/acme_calc.exe 12 30
./build/cli-debug/acme_info.exe
```

Run the programs (Linux):

```bash
./build/cli-debug/acme_calc 12 30
./build/cli-debug/acme_info
```

The main application should print `team-demo: 12 + 30 = 42`.

`ACME_BUILD_TOOLS=ON` enables `acme_info`, and `ACME_APP_LABEL` sets the output label.
