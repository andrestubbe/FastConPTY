@echo off
chcp 65001 >nul

echo 🔨 Building FastConPTY...
call mvn clean install -DskipTests -q >nul 2>&1
if %ERRORLEVEL% NEQ 0 ( echo ❌ Build failed. & exit /b %ERRORLEVEL% )

cd examples\Demo
echo 🔨 Compiling Demo...
call mvn clean compile dependency:build-classpath -Dmdep.outputFile=cp.txt -DincludeScope=runtime -q >nul 2>&1
if %ERRORLEVEL% NEQ 0 ( echo ❌ Compile failed. & cd ..\.. & exit /b %ERRORLEVEL% )

set /p CP=<cp.txt
echo 🚀 Launching FastConPTY Demo...
echo.
java --enable-native-access=ALL-UNNAMED -cp "target\classes;%CP%" fastconpty.Demo.Demo %*

cd ..\..
