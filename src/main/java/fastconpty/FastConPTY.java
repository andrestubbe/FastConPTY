package fastconpty;

/**
 * FastConPTY — Windows Pseudo-Terminal Backend for the FastJava Ecosystem
 * Kapselt die Windows-ConPTY-API in einer performanten JNI-Schnittstelle.
 */
public final class FastConPTY {

    static {
        fastcore.FastCore.loadLibrary("fastconpty");
    }

    // Zeiger auf das native C++ ConPTYInstance-Objekt
    private final long handle;

    private FastConPTY(long handle) {
        this.handle = handle;
    }

    /**
     * Erstellt eine neue ConPTY-Instanz mit den gegebenen Dimensionen (cols/rows)
     */
    public static FastConPTY create(int cols, int rows) {
        long h = nativeCreatePseudoConsole(cols, rows);
        if (h == 0L) {
            throw new IllegalStateException("FastConPTY: Failed to create PseudoConsole");
        }
        return new FastConPTY(h);
    }

    /**
     * Startet einen Shell-Prozess (z. B. cmd.exe, powershell.exe) in dieser ConPTY-Session
     * @return PID des gestarteten Prozesses
     */
    public long startProcess(String commandLine) {
        long pid = nativeStartProcess(handle, commandLine);
        if (pid == 0L) {
            throw new IllegalStateException("FastConPTY: Failed to start process in ConPTY");
        }
        return pid;
    }

    /**
     * Ändert die Größe des Pseudo-Terminals
     */
    public void resize(int cols, int rows) {
        nativeResizePseudoConsole(handle, cols, rows);
    }

    /**
     * Schreibt Eingaben in das Terminal (z. B. Tastatureingaben)
     */
    public void write(byte[] data, int off, int len) {
        nativeWrite(handle, data, off, len);
    }

    /**
     * Liest Ausgaben aus dem Terminal (non-blocking, gibt 0 zurück wenn keine Daten vorhanden sind)
     */
    public int read(byte[] buffer, int off, int len) {
        return nativeRead(handle, buffer, off, len);
    }

    /**
     * Wartet auf das Ende des Prozesses mit einem Timeout
     * @return Exit-Code, oder -1 wenn noch aktiv, oder -2 bei Fehlern
     */
    public int waitFor(int timeoutMs) {
        return nativeWaitFor(handle, timeoutMs);
    }

    /**
     * Schließt die ConPTY-Session und gibt native Ressourcen frei
     */
    public void close() {
        nativeClosePseudoConsole(handle);
    }

    // Native Methoden – Implementierung in fastconpty.dll (C/C++)
    private static native long nativeCreatePseudoConsole(int cols, int rows);
    private static native void nativeResizePseudoConsole(long handle, int cols, int rows);
    private static native void nativeClosePseudoConsole(long handle);
    private static native long nativeStartProcess(long handle, String commandLine);
    private static native void nativeWrite(long handle, byte[] data, int off, int len);
    private static native int nativeRead(long handle, byte[] buffer, int off, int len);
    private static native int nativeWaitFor(long handle, int timeoutMs);
}
