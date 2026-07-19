# Building from Source

## Prerequisites

- JDK 17+
- Maven 3.9+
- **Windows:** Visual Studio 2019+ or Build Tools (with C++ Desktop development workload)

## Build

### Windows

```bash
compile.bat
mvn clean package
```

The compile script triggers the Visual Studio C++ Compiler (`cl.exe`) to compile `native/fastconpty.cpp` into `fastconpty.dll`, and copies it to the Java resources directory. The Maven packaging task bundles it directly inside the output JAR.

## Run Examples

```bash
run-demo.bat
```

## Installation

### JitPack (Recommended)

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
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastCore</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

### Gradle (JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:FastConPTY:0.1.0'
    implementation 'com.github.andrestubbe:FastCore:0.1.0'
}
```
