package fastconpty.benchmark;

import fastconpty.FastConPTY;
import org.openjdk.jmh.annotations.*;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.MILLISECONDS)
@State(Scope.Benchmark)
@Warmup(iterations = 2, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 3, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(1)
public class Benchmark {

    private FastConPTY conPty;

    @Setup
    public void setup() {
        try {
            conPty = FastConPTY.create(80, 25);
        } catch (Throwable t) {
            conPty = null;
        }
    }

    @TearDown
    public void tearDown() {
        if (conPty != null) {
            try {
                conPty.close();
            } catch (Throwable ignored) {}
        }
    }

    @org.openjdk.jmh.annotations.Benchmark
    public void benchmarkResize() {
        if (conPty != null) {
            conPty.resize(100, 30);
            conPty.resize(80, 25);
        }
    }
}
