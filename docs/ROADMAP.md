# FastConPTY Roadmap

## Phase 1: Core JNI Substrate (Completed)
- [x] Native C++ JNI implementation of `CreatePseudoConsole`.
- [x] Win32 `CreateProcessA` spawning with `STARTUPINFOEX` and `PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE` attributes.
- [x] JNI-mapped non-blocking reading and writing.
- [x] Interactive loop console demo.

## Phase 2: Asynchronous Event Notification (Q3 2026)
- [ ] IOCP (Input/Output Completion Ports) JNI integration for event-driven reading.
- [ ] Native event loop threads to push output changes directly into Java callbacks.
- [ ] Resize listener hooks.

## Phase 3: Advanced Shell Control (Q4 2026)
- [ ] Environment variable mapping support in native process creation.
- [ ] UTF-16 character translation mappings.
- [ ] Integration with `FastTUI` and `CreamCLI` as a floating terminal bubble overlay.
- [ ] Multi-session PTY managers.
