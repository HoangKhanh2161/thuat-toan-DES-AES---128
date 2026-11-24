# Thuật toán DES và AES-128 (C++)

Cài đặt DES và AES-128, in giá trị từng trạng thái (từng vòng, từng bước) khi mã hoá / giải mã và kiểm tra bằng test vector.

## Cách chạy

Cần có **g++** (kiểm tra bằng lệnh `g++ --version`; nếu chưa có thì cài [MSYS2](https://www.msys2.org) rồi chạy `pacman -S mingw-w64-ucrt-x86_64-gcc` và thêm `C:\msys64\ucrt64\bin` vào biến môi trường Path).

### Cách 1: VS Code + extension Code Runner

1. Cài extension **Code Runner** trong VS Code.
2. **File → Open Folder...** → chọn cả thư mục project này (không mở lẻ 1 file).
3. Mở `src/main.cpp` → bấm nút **▷ Run Code** ở góc trên bên phải (hoặc `Ctrl + Alt + N`).
4. Menu hiện ra trong Terminal, gõ số để chọn.

> File `.vscode/settings.json` đã cấu hình sẵn để Code Runner biên dịch cả 4 file `.cpp` và chạy trong Terminal (nhập được từ bàn phím).

### Cách 2: Dòng lệnh

```
g++ -std=c++17 -O2 -static -Iinclude src/main.cpp src/utils.cpp src/des.cpp src/aes.cpp -o des_aes
.\des_aes
```

(Linux / macOS: chạy bằng `./des_aes`, hoặc dùng `make`.)

### Menu

```
  1. DES - chay test vector co san
  2. AES - chay test vector co san
  3. DES - tu nhap khoa, ban ro
  4. AES - tu nhap khoa, ban ro
  5. Kiem tra nhanh (PASS/FAIL)
  6. Ghi ket qua vao thu muc results/
  0. Thoat
```

Muốn thêm ví dụ trên lớp: thêm 1 dòng vào [`include/test_vectors.h`](include/test_vectors.h), hoặc chọn mục 3 / 4 để gõ trực tiếp.

## Kết quả chạy

Mỗi test vector được mã hoá rồi giải mã lại. **Bấm vào tên file** để xem giá trị của từng trạng thái.

| File | Thuật toán | Nguồn | Khoá | Bản rõ | Bản mã | Kết quả |
|---|---|---|---|---|---|---|
| [DES_1.txt](results/DES_1.txt) | DES | J. Orlin Grabbe | `133457799BBCDFF1` | `0123456789ABCDEF` | `85E813540F0AB405` | ✅ PASS |
| [DES_2.txt](results/DES_2.txt) | DES | Stallings | `0F1571C947D9E859` | `02468ACEECA86420` | `DA02CE3A89ECAC3B` | ✅ PASS |
| [DES_3.txt](results/DES_3.txt) | DES | FIPS 81 | `0123456789ABCDEF` | `4E6F772069732074` | `3FA40E8A984D4815` | ✅ PASS |
| [DES_4.txt](results/DES_4.txt) | DES | Bản mã toàn số 0 | `0E329232EA6D0D73` | `8787878787878787` | `0000000000000000` | ✅ PASS |
| [AES_1.txt](results/AES_1.txt) | AES-128 | FIPS-197 Appendix B | `2b7e151628aed2a6abf7158809cf4f3c` | `3243f6a8885a308d313198a2e0370734` | `3925841d02dc09fbdc118597196a0b32` | ✅ PASS |
| [AES_2.txt](results/AES_2.txt) | AES-128 | FIPS-197 Appendix C.1 | `000102030405060708090a0b0c0d0e0f` | `00112233445566778899aabbccddeeff` | `69c4e0d86a7b0430d8cdb78070b4c55a` | ✅ PASS |
| [AES_3.txt](results/AES_3.txt) | AES-128 | Stallings | `0f1571c947d9e8590cb7add6af7f6798` | `0123456789abcdeffedcba9876543210` | `ff0b844a0853bf7c6934ab4364148fb9` | ✅ PASS |
| [AES_4.txt](results/AES_4.txt) | AES-128 | "Thats my Kung Fu" | `5468617473206d79204b756e67204675` | `54776f204f6e65204e696e652054776f` | `29c3505f571420f6402299b31a02d73a` | ✅ PASS |

Bảng tổng hợp: [results/summary.txt](results/summary.txt)

## Cấu trúc thư mục

```
include/   des.h, aes.h, utils.h, test_vectors.h, des_tables.h, aes_tables.h
src/       main.cpp (menu), des.cpp, aes.cpp, utils.cpp
results/   kết quả chạy từng test vector
```
