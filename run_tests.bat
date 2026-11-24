@echo off
REM ==================================================================
REM  run_tests.bat - Bien dich + chay tat ca test vector,
REM                  ghi ket qua vao thu muc results\
REM ==================================================================
cd /d "%~dp0"

echo Dang bien dich...
g++ -std=c++17 -Wall -O2 -Iinclude src\main.cpp src\utils.cpp src\des.cpp src\aes.cpp -o des_aes.exe
if errorlevel 1 (
    echo [LOI] Bien dich that bai. Xem huong dan trong file HUONG_DAN.md
    pause
    exit /b 1
)

if not exist results mkdir results

echo Dang chay DES...
.\des_aes.exe des > results\des_result.txt

echo Dang chay AES-128...
.\des_aes.exe aes > results\aes_result.txt

echo Dang kiem tra nhanh...
.\des_aes.exe check > results\summary.txt

type results\summary.txt
echo.
echo Da ghi ket qua vao:
echo   results\des_result.txt  (moi trang thai cua DES)
echo   results\aes_result.txt  (moi trang thai cua AES-128)
echo   results\summary.txt     (bang PASS/FAIL)
pause
