# FastConPTY API Reference

## Class: `fastconpty.FastConPTY`

### Methods

#### `create`
```java
public static FastConPTY create(int cols, int rows)
```
Creates a new Windows Pseudo-Console session.
- **Parameters:**
  - `cols`: Width of the console viewport in character cells.
  - `rows`: Height of the console viewport in character cells.
- **Returns:** An instance of `FastConPTY`.
- **Throws:** `IllegalStateException` if the pseudo-console allocation fails.

#### `startProcess`
```java
public long startProcess(String commandLine)
```
Spawns a shell or interactive utility inside the pseudo-console.
- **Parameters:**
  - `commandLine`: The exact command-line string (e.g. `"cmd.exe"`, `"powershell.exe"`).
- **Returns:** The PID (Process Identifier) of the spawned process.

#### `resize`
```java
public void resize(int cols, int rows)
```
Resizes the pseudo-console dimensions.
- **Parameters:**
  - `cols`: New width.
  - `rows`: New height.

#### `write`
```java
public void write(byte[] data, int off, int len)
```
Writes input bytes (keystrokes) into the pseudo-console input stream.
- **Parameters:**
  - `data`: Byte array containing the inputs.
  - `off`: Offset in the array.
  - `len`: Length of bytes to write.

#### `read`
```java
public int read(byte[] buffer, int off, int len)
```
Reads available output bytes from the pseudo-console output stream.
- **Parameters:**
  - `buffer`: Target byte array.
  - `off`: Offset.
  - `len`: Maximum bytes to read.
- **Returns:** Number of bytes read, `0` if no data is available (non-blocking), or `-1` if the pipe is closed.

#### `waitFor`
```java
public int waitFor(int timeoutMs)
```
Waits for the spawned process to terminate.
- **Parameters:**
  - `timeoutMs`: Milliseconds to wait.
- **Returns:** The process exit code, `-1` if still running, or `-2` on errors.

#### `close`
```java
public void close()
```
Disposes of the pseudo-console, closes open named pipes, terminates the child process, and frees native memory allocations.
