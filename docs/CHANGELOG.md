# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2026-07-19
### Added
- Native C++ JNI implementation of Windows Pseudo-Console (ConPTY) APIs (`CreatePseudoConsole`, `ResizePseudoConsole`, `ClosePseudoConsole`).
- Safe process spawning inside PTY using C++ managed `STARTUPINFOEX` and `PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE` attributes list to prevent JVM crashes.
- Direct non-blocking named pipe E/A read and write mappings.
- Fully interactive `cmd.exe` keyboard/output loop demo.
- Integrated `FastCore` JNI dynamic loader.
- Automated Visual Studio compile script `compile.bat` and demo launcher `run-demo.bat`.
