# C/C++/Linux/音频调试速成实验

这些实验对应招聘图片中反复出现的要求：C/C++ 调试、GDB/core dump、内存与并发、Linux/网络排障、性能分析，以及音频波形/FFT/时序定位。

## 环境

建议在 Ubuntu/WSL/Docker 中执行 Linux 实验；Windows 也可以先用 MSYS2 编译基础程序。`echo_server`、`strace`、`perf`、`valgrind` 这组实验需要 Linux/WSL。Linux 命令：

```bash
sudo apt update
sudo apt install -y build-essential cmake gdb valgrind linux-tools-generic python3 python3-numpy
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## 实验顺序

1. `gdb ./build/crash_demo`：练 `break/run/bt/frame/list/p/x/watch/continue`。
2. `ASAN_OPTIONS=detect_leaks=1 ./build/crash_demo uaf`：练 ASan/UBSan 根因定位。
3. `valgrind --leak-check=full ./build/crash_demo leak`：练泄漏报告。
4. `./build/race_demo` 配合 `-fsanitize=thread` 重编：练竞态证据。
5. `./build/deadlock_demo` 卡住后 `gdb -p PID`，执行 `thread apply all bt`：练死锁定位。
6. `perf record -g ./build/perf_demo && perf report`：练热点函数和调用栈。
7. `./build/echo_server`，另一个终端 `printf 'hi' | nc 127.0.0.1 9000`，同时执行 `ss -ltnp`、`strace -ff -tt -p PID`：练连接、系统调用和超时定位。
8. `python3 scripts/audio_debug.py`：练 peak/RMS/DC/削波/FFT 证据链。

## 复盘模板

每次实验写 5 行：现象、复现命令、第一条有价值证据、根因、修复后验证。面试时按这 5 行回答，比罗列工具名更有说服力。
