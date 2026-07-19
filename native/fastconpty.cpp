#include <windows.h>
#include <process.h>
#include <string>
#include <iostream>
#include "jni.h"

// Define function pointers for pseudo-console APIs
typedef HRESULT (WINAPI *PFNCREATEPSEUDOCONSOLE)(COORD, HANDLE, HANDLE, DWORD, HPCON*);
typedef HRESULT (WINAPI *PFNRESIZEPSEUDOCONSOLE)(HPCON, COORD);
typedef VOID (WINAPI *PFNCLOSEPSEUDOCONSOLE)(HPCON);

static PFNCREATEPSEUDOCONSOLE CreatePseudoConsoleFn = nullptr;
static PFNRESIZEPSEUDOCONSOLE ResizePseudoConsoleFn = nullptr;
static PFNCLOSEPSEUDOCONSOLE ClosePseudoConsoleFn = nullptr;

static bool LoadConPTYApis() {
    if (CreatePseudoConsoleFn != nullptr) return true;
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32) return false;

    CreatePseudoConsoleFn = (PFNCREATEPSEUDOCONSOLE)GetProcAddress(hKernel32, "CreatePseudoConsole");
    ResizePseudoConsoleFn = (PFNRESIZEPSEUDOCONSOLE)GetProcAddress(hKernel32, "ResizePseudoConsole");
    ClosePseudoConsoleFn = (PFNCLOSEPSEUDOCONSOLE)GetProcAddress(hKernel32, "ClosePseudoConsole");

    return CreatePseudoConsoleFn && ResizePseudoConsoleFn && ClosePseudoConsoleFn;
}

struct ConPTYInstance {
    HPCON hPC = nullptr;
    HANDLE hInputRead = nullptr;
    HANDLE hInputWrite = nullptr;
    HANDLE hOutputRead = nullptr;
    HANDLE hOutputWrite = nullptr;
    HANDLE hProcess = nullptr;
    HANDLE hThread = nullptr;
};

extern "C" JNIEXPORT jlong JNICALL
Java_fastconpty_FastConPTY_nativeCreatePseudoConsole(JNIEnv* env, jclass, jint cols, jint rows) {
    if (!LoadConPTYApis()) {
        return 0L;
    }

    ConPTYInstance* instance = new ConPTYInstance();

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    // Create pipes
    if (!CreatePipe(&instance->hInputRead, &instance->hInputWrite, &sa, 0) ||
        !CreatePipe(&instance->hOutputRead, &instance->hOutputWrite, &sa, 0)) {
        if (instance->hInputRead) CloseHandle(instance->hInputRead);
        if (instance->hInputWrite) CloseHandle(instance->hInputWrite);
        if (instance->hOutputRead) CloseHandle(instance->hOutputRead);
        if (instance->hOutputWrite) CloseHandle(instance->hOutputWrite);
        delete instance;
        return 0L;
    }

    COORD size = { (SHORT)cols, (SHORT)rows };
    HRESULT hr = CreatePseudoConsoleFn(size, instance->hInputRead, instance->hOutputWrite, 0, &instance->hPC);

    if (FAILED(hr)) {
        CloseHandle(instance->hInputRead);
        CloseHandle(instance->hInputWrite);
        CloseHandle(instance->hOutputRead);
        CloseHandle(instance->hOutputWrite);
        delete instance;
        return 0L;
    }

    // We can close our references to the handles inherited by ConPTY
    CloseHandle(instance->hInputRead);
    instance->hInputRead = nullptr;
    CloseHandle(instance->hOutputWrite);
    instance->hOutputWrite = nullptr;

    return (jlong)instance;
}

extern "C" JNIEXPORT void JNICALL
Java_fastconpty_FastConPTY_nativeResizePseudoConsole(JNIEnv*, jclass, jlong handle, jint cols, jint rows) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (instance && instance->hPC && ResizePseudoConsoleFn) {
        COORD size = { (SHORT)cols, (SHORT)rows };
        ResizePseudoConsoleFn(instance->hPC, size);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_fastconpty_FastConPTY_nativeClosePseudoConsole(JNIEnv*, jclass, jlong handle) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (instance) {
        if (instance->hProcess) {
            TerminateProcess(instance->hProcess, 0);
            CloseHandle(instance->hProcess);
        }
        if (instance->hThread) {
            CloseHandle(instance->hThread);
        }
        if (instance->hPC && ClosePseudoConsoleFn) {
            ClosePseudoConsoleFn(instance->hPC);
        }
        if (instance->hInputWrite) {
            CloseHandle(instance->hInputWrite);
        }
        if (instance->hOutputRead) {
            CloseHandle(instance->hOutputRead);
        }
        delete instance;
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_fastconpty_FastConPTY_nativeStartProcess(JNIEnv* env, jclass, jlong handle, jstring jcmd) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (!instance) return 0L;

    const char* cmd = env->GetStringUTFChars(jcmd, nullptr);
    if (!cmd) return 0L;

    STARTUPINFOEXA si{};
    si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrSize);
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize);

    UpdateProcThreadAttribute(
        si.lpAttributeList,
        0,
        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
        instance->hPC,
        sizeof(HPCON),
        nullptr,
        nullptr
    );

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr,
        (LPSTR)cmd,
        nullptr,
        nullptr,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        nullptr,
        nullptr,
        &si.StartupInfo,
        &pi
    );

    env->ReleaseStringUTFChars(jcmd, cmd);
    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);

    if (!ok) return 0L;

    instance->hProcess = pi.hProcess;
    instance->hThread = pi.hThread;

    return (jlong)pi.dwProcessId;
}

extern "C" JNIEXPORT void JNICALL
Java_fastconpty_FastConPTY_nativeWrite(JNIEnv* env, jclass, jlong handle, jbyteArray data, jint off, jint len) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (!instance || !instance->hInputWrite) return;

    jbyte* bytes = env->GetByteArrayElements(data, nullptr);
    if (!bytes) return;

    DWORD bytesWritten = 0;
    WriteFile(instance->hInputWrite, bytes + off, len, &bytesWritten, nullptr);

    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
}

extern "C" JNIEXPORT jint JNICALL
Java_fastconpty_FastConPTY_nativeRead(JNIEnv* env, jclass, jlong handle, jbyteArray buffer, jint off, jint len) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (!instance || !instance->hOutputRead) return -1;

    // Check if bytes are available first
    DWORD bytesAvail = 0;
    if (!PeekNamedPipe(instance->hOutputRead, nullptr, 0, nullptr, &bytesAvail, nullptr) || bytesAvail == 0) {
        return 0; // Non-blocking: return 0 if no data
    }

    jbyte* bytes = env->GetByteArrayElements(buffer, nullptr);
    if (!bytes) return -1;

    DWORD bytesRead = 0;
    DWORD readLen = (len < (jint)bytesAvail) ? len : (jint)bytesAvail;
    BOOL success = ReadFile(instance->hOutputRead, bytes + off, readLen, &bytesRead, nullptr);

    env->ReleaseByteArrayElements(buffer, bytes, 0);

    return success ? (jint)bytesRead : -1;
}

extern "C" JNIEXPORT jint JNICALL
Java_fastconpty_FastConPTY_nativeWaitFor(JNIEnv*, jclass, jlong handle, jint timeoutMs) {
    ConPTYInstance* instance = (ConPTYInstance*)handle;
    if (!instance || !instance->hProcess) return -2;

    DWORD res = WaitForSingleObject(instance->hProcess, timeoutMs);
    if (res == WAIT_OBJECT_0) {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(instance->hProcess, &exitCode)) {
            return (jint)exitCode;
        }
        return -1;
    } else if (res == WAIT_TIMEOUT) {
        return -1; // Still running
    }
    return -2;
}
