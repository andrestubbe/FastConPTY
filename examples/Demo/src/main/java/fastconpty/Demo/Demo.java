package fastconpty.Demo;

import fastconpty.FastConPTY;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Demo {

    public static void main(String[] args) {
        System.out.println("=== FastConPTY Interactive JNI Terminal Demo ===");
        System.out.println("Starting cmd.exe inside Windows Pseudo-Console...");
        System.out.println("Type 'exit' to quit.\n");

        try {
            // Create ConPTY session (80 columns, 25 rows)
            FastConPTY conpty = FastConPTY.create(80, 25);

            // Start cmd.exe
            long pid = conpty.startProcess("cmd.exe");
            System.out.println("✓ Process spawned successfully. PID: " + pid);

            // Spawn background thread to continuously read and print output
            Thread readerThread = new Thread(() -> {
                try {
                    byte[] buffer = new byte[8192];
                    while (true) {
                        int readBytes = conpty.read(buffer, 0, buffer.length);
                        if (readBytes > 0) {
                            System.out.print(new String(buffer, 0, readBytes, StandardCharsets.UTF_8));
                            System.out.flush();
                        } else if (readBytes < 0) {
                            break; // Process output closed
                        } else {
                            Thread.sleep(10); // No data, back off
                        }
                    }
                } catch (Exception e) {
                    System.err.println("\nReader thread error: " + e.getMessage());
                }
            });
            readerThread.setDaemon(true);
            readerThread.start();

            // Read keyboard input from the user's Java console and write it to the terminal process
            InputStream in = System.in;
            byte[] inputBuffer = new byte[1024];
            while (conpty.waitFor(50) == -1) {
                if (in.available() > 0) {
                    int bytesRead = in.read(inputBuffer);
                    if (bytesRead > 0) {
                        conpty.write(inputBuffer, 0, bytesRead);
                    }
                } else {
                    Thread.sleep(10);
                }
            }

            int exitCode = conpty.waitFor(2000);
            System.out.println("\n=== Process exited with code: " + exitCode + " ===");

            // Cleanup
            conpty.close();
            System.out.println("FastConPTY JNI Session cleaned up.");

        } catch (Exception e) {
            System.err.println("Demo execution failed: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
