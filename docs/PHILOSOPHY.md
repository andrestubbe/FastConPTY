# The Philosophy of FastConPTY

> [!IMPORTANT]
> **"Zero Copy. Native IOCP. Zero Marshalling Overhead. Windows Kernel ConPTY Integration."**

FastConPTY is built on the principle that modern Java shell integrations and interactive terminal applications require a **JNI-first, low-overhead** pseudo-terminal connector that standard JNA wrappers and bloated thread pollers fail to deliver.

## Core Tenets

### 1. JNI-First Architecture
Eschew reflection-based runtime JNA lookups. Instead of introducing layers of method lookup overhead, structures marshaling, and garbage collection pressure, FastConPTY uses JNI to compile direct Win32 kernel calls, keeping latency under 1ms.

### 2. Zero-Copy Communication
Passing large terminal outputs and keystroke arrays between Java and C++ should not trigger Java heap copies. FastConPTY maps memory directly using JNI byte array bindings, enabling raw pipeline transfers with zero GC footprints.

### 3. Kernel-Managed Pseudo-Consoles
On Windows, standard console processes (like `cmd.exe` or `git`) depend on the OS console subsystem to render formatting and handle prompts. FastConPTY integrates natively with Windows ConPTY handles, ensuring progress bars, password entries, and colors look exactly as they do in native terminal windows.

### 4. Correct Attribute Spawning
spawning processes inside pseudo-consoles requires allocating Win32 thread attributes lists. FastConPTY manages `STARTUPINFOEX` and `PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE` structures directly inside the native C++ layer, avoiding unaligned pointer allocations and preventing JVM crash events.

---

**⚡ FastConPTY — Powering native terminal execution inside the FastJava ecosystem.**
