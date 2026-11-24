@echo off
REM ==================================================================
REM  build.bat - Bien dich chuong trinh tren Windows (can co g++)
REM  Cach dung: nhay dup vao file nay, hoac go "build.bat" trong cmd
REM ==================================================================
cd /d "%~dp0"

echo Dang bien dich...
g++ -std=c++17 -Wall -O2 -static -Iinclude src\main.cpp src\utils.cpp src\des.cpp src\aes.cpp -o des_aes.exe

if errorlevel 1 (
    echo.
    echo [LOI] Bien dich that bai. Xem huong dan trong file HUONG_DAN.md
    pause
    exit /b 1
)

echo Bien dich thanh cong: des_aes.exe
echo Nhay dup vao des_aes.exe hoac go "des_aes.exe" de chay chuong trinh.
pause
