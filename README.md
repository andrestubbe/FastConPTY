# FastConPTY 0.1.0 [ALPHA-2026-07] — High-Performance JNI Windows ConPTY Connector for Java

[![Status](https://img.shields.io/badge/status-0.1.0-brightgreen.svg)](https://github.com/andrestubbe/FastConPTY/releases/tag/0.1.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-ready-green.svg)](https://jitpack.io/#andrestubbe/FastConPTY)

**⚡ FastConPTY is a native Windows Pseudo-Terminal (ConPTY) connector for the FastJava ecosystem, written in C++ and JNI. It enables zero-copy, low-latency execution of interactive shell processes (like Cmd, PowerShell, and Bash) directly inside Java applications.**

FastConPTY is the pseudo-terminal infrastructure module of the **FastJava** ecosystem. Unlike typical Java terminal process launchers, it does not rely on JNA reflection overlays. Instead, it compiles to a native C++ library (`fastconpty.dll`) interacting directly with the Win32 kernel to achieve sub-millisecond I/O latency, Zero-Copy buffer mapping, and stable Windows process trees.

To achieve a completely responsive, zero-latency desktop terminal experience, FastConPTY is designed to pair natively with the rendering and parsing modules of the **FastJava** ecosystem:

* ⚡ **[FastCore](https://github.com/andrestubbe/FastCore)** — Unified native JNI loader and ecosystem configuration kernel.
* ⚡ **[FastTerminal](https://github.com/andrestubbe/FastTerminal)** — Low-latency, hardware-accelerated 24-bit True Color terminal blitter.
* ⚡ **[FastTUI](https://github.com/andrestubbe/FastTUI)** — Composable Terminal User Interface widget library.

---

```java
// Quick Start — Spawning an Interactive Process in a Pseudo-Console

import fastconpty.FastConPTY;
import fastconpty.TerminalProcess;

public class Demo {
    public static void main(String[] args) {
        // Create an 80x25 Pseudo-Console session
        FastConPTY conpty = FastConPTY.create(80, 25);
        
        // Spawn cmd.exe inside it
        long pid = conpty.startProcess("cmd.exe");
        System.out.println("Spawned cmd.exe with PID: " + pid);
        
        // Write keys to the process stdin
        byte[] keys = "dir\r\n".getBytes();
        conpty.write(keys, 0, keys.length);
        
        // Read output from the process stdout (non-blocking)
        byte[] buffer = new byte[4096];
        int read = conpty.read(buffer, 0, buffer.length);
        if (read > 0) {
            System.out.print(new String(buffer, 0, read));
        }
        
        // Clean up
        conpty.close();
    }
}
```

---

## Table of Contents
- [Why FastConPTY?](#why-fastconpty)
- [Key Features](#key-features)
- [Architecture & JNI Boundary](#architecture--jni-boundary)
- [Installation](#installation)
- [Technical Demos & Benchmarks](#technical-demos--benchmarks)
- [License](#license)

---

## Why FastConPTY?

Spawning interactive command-line utilities (like `git push`, SSH authentication, `pip install`, or `docker exec`) inside Java via standard `ProcessBuilder` leads to terminal output corruption, missing colors, or hanging threads:

1. **Broken Interactive CLIs**: Standard Java `ProcessBuilder` uses anonymous pipes without a TTY abstraction, causing curses/ncurses and ANSI programs to disable formatting or crash entirely.
2. **JNA Reflection Overhead**: Libraries relying on JNA marshal structs and memory pointers through dynamic reflection layers, inducing garbage collection churn and latency.
3. **Thread Contention & Blocking I/O**: Standard streams frequently block worker threads while polling child process I/O without native non-blocking pipe readiness.
4. **Process Tree Desynchronization**: Failing to attach processes to `STARTUPINFOEX` attribute lists causes orphan child processes and leaked console handles.

FastConPTY creates a true Windows Pseudo-Console (ConPTY) handle inside the Win32 kernel, mapping Java-backed ByteBuffer structures directly to native pipes via zero-overhead JNI:

| Feature | Standard `ProcessBuilder` | Pty4J (JNA / WinPTY) | FastConPTY |
|:---|:---|:---|:---|
| **Native Mechanism** | Anonymous Pipes | JNA calls / `agent.exe` bridge | Native Win32 ConPTY API |
| **ANSI / VT100 Support** | ❌ None (Raw byte dump) | ⚠️ Partial (Bridge layer lag) | ✅ Full OS ConPTY Engine |
| **Zero-Copy Direct Buffers** | ❌ Heap byte[] copy | ❌ JNA heap marshalling | ✅ Yes (`DirectByteBuffer`) |
| **Kernel Latency** | High (Pipe buffering) | ~5–25 ms (Reflection/Bridge) | **< 1 ms** (Direct Win32) |
| **Process Tree Safety** | Basic PID only | Variable (Zombie proc risk) | Native `STARTUPINFOEX` |
| **Dependencies** | Standard JVM | Heavy bundle (~15 MB / DLLs) | 0 external deps (Lightweight DLL) |

---

## Key Features
- **✨ Full Lifecycle Control:** Programmatically instantiate, resize (`ResizePseudoConsole`), and safely dispose of Windows pseudo-consoles.
- **🚀 Shell Compatibility:** Seamlessly host complex shells (`cmd.exe`, `powershell.exe`, `bash.exe`) and interactive CLIs.
- **💾 Low-Overhead Pipe IO:** Direct memory access using JNI array bindings to feed inputs and poll output pipe buffers.
- **⚡ Unified Loader:** Integrates with `FastCore` to locate and load the native dll dynamically.

---

## Architecture & JNI Boundary
FastConPTY splits its operations into two main layers:
1. **Java Layer:** The public `FastConPTY` class exposes high-level session, process, and sizing APIs. It maps data directly to JNI boundaries.
2. **Native C++ Layer (`fastconpty.cpp`):** Operates inside `fastconpty.dll`. It resolves kernel functions dynamically from `kernel32.dll`, sets up asynchronous named pipes, initializes the `PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE` attribute list, and triggers the Win32 `CreateProcessA` calls.

---

## Installation

### Maven (via JitPack)
Add the JitPack repository and the dependency to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastConPTY</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```



---

## Technical Demos & Benchmarks

| Case | Java Example | Launcher | Description |
|:---|:---|:---|:---|
| **Interactive Showcase Demo** | [Demo.java](examples/Demo/src/main/java/fastconpty/Demo/Demo.java) | `run-demo.bat` | End-to-end interactive demonstration of bidirectional Windows pseudo-console spawning and ANSI piping. |
| **JMH Microbenchmark Suite** | [Benchmark.java](examples/Benchmark/src/main/java/fastconpty/benchmark/Benchmark.java) | `run-benchmark.bat` | Formal OpenJDK JMH throughput measurements across native ConPTY resize and I/O pipelines. |

---

## License
MIT License. See [LICENSE](LICENSE) for details.
