@echo off
REM ==================================================================
REM  run_tests.bat - Bien dich + chay tat ca test vector,
REM                  ghi ket qua TUNG test vector vao thu muc results\
REM ==================================================================
cd /d "%~dp0"

echo Dang bien dich...
g++ -std=c++17 -Wall -O2 -static -Iinclude src\main.cpp src\utils.cpp src\des.cpp src\aes.cpp -o des_aes.exe
if errorlevel 1 (
    echo [LOI] Bien dich that bai. Xem huong dan trong file HUONG_DAN.md
    pause
    exit /b 1
)

echo Dang chay tat ca test vector...
if not exist results mkdir results
.\des_aes.exe export

echo.
echo Ket qua nam trong thu muc results\ :
echo   DES_1.txt, DES_2.txt, ...  moi trang thai cua tung test vector DES
echo   AES_1.txt, AES_2.txt, ...  moi trang thai cua tung test vector AES-128
echo   summary.txt                bang PASS/FAIL
pause
