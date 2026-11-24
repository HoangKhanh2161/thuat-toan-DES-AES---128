# Hướng dẫn chạy chương trình (dành cho người mới học C++)

Làm lần lượt từng bước dưới đây. Mỗi bước có giải thích ngắn gọn.

---

## Bước 1. Kiểm tra máy đã có trình biên dịch g++ chưa

Mở **Command Prompt** (bấm `Windows`, gõ `cmd`, Enter) rồi gõ:

```
g++ --version
```

- Nếu hiện ra dòng kiểu `g++ (Rev6, Built by MSYS2 project) 13.1.0` → **đã có**, chuyển sang Bước 2.
- Nếu báo `'g++' is not recognized...` → **chưa có**, cài như sau:
  1. Tải và cài **MSYS2** tại https://www.msys2.org (cứ bấm Next).
  2. Mở chương trình **MSYS2 UCRT64** vừa cài, gõ lệnh sau rồi Enter (gõ `Y` khi được hỏi):
     ```
     pacman -S mingw-w64-ucrt-x86_64-gcc
     ```
  3. Thêm thư mục `C:\msys64\ucrt64\bin` vào biến môi trường **Path**:
     bấm `Windows` → gõ `environment` → *Edit the system environment variables* → *Environment Variables...*
     → chọn `Path` → *Edit* → *New* → dán `C:\msys64\ucrt64\bin` → OK.
  4. **Đóng cmd và mở lại**, gõ lại `g++ --version` để kiểm tra.

> Linux: `sudo apt install g++ make`  •  macOS: `xcode-select --install`

---

## Bước 2. Tải code về máy

**Cách 1 (dễ nhất):** vào trang GitHub của repository → bấm nút xanh **Code** → **Download ZIP** → giải nén.

**Cách 2 (nếu đã cài git):**
```
git clone https://github.com/HoangKhanh2161/thuat-toan-DES-AES---128.git
cd thuat-toan-DES-AES---128
```

---

## Bước 3. Biên dịch (dịch code C++ thành file chạy .exe)

**Cách 1 – nhấp đúp file `build.bat`.** Nếu hiện `Bien dich thanh cong` là xong, trong thư mục sẽ có thêm file `des_aes.exe`.

**Cách 2 – tự gõ lệnh** trong cmd (đứng ở thư mục chứa code):
```
g++ -std=c++17 -Wall -O2 -static -Iinclude src/main.cpp src/utils.cpp src/des.cpp src/aes.cpp -o des_aes.exe
```
Giải thích lệnh:
| Phần | Ý nghĩa |
|---|---|
| `g++` | trình biên dịch C++ |
| `-std=c++17` | dùng chuẩn C++17 |
| `-Wall` | bật cảnh báo để phát hiện lỗi |
| `-O2` | tối ưu cho chạy nhanh |
| `-static` | gói luôn thư viện C++ vào file exe → chạy được trên mọi máy, tránh lỗi thiếu / nhầm file `.dll` |
| `-Iinclude` | tìm các file `.h` trong thư mục `include` |
| `src/....cpp` | **4 file** cần biên dịch cùng lúc (chương trình được tách thành nhiều file) |
| `-o des_aes.exe` | đặt tên file chạy là `des_aes.exe` |

> Linux / macOS: gõ `make`, rồi chạy bằng `./des_aes`

---

## Bước 4. Chạy chương trình

Nhấp đúp `des_aes.exe`, hoặc gõ trong cmd:
```
des_aes.exe
```
> Nếu dùng **PowerShell** (hoặc terminal trong VS Code) thì phải gõ `.\des_aes.exe`

Menu hiện ra:
```
==============================================================
      CHUONG TRINH DES & AES-128  (in tung trang thai)
==============================================================
  1. DES     - chay test vector co san
  2. AES-128 - chay test vector co san
  3. DES     - tu nhap khoa + ban ro
  4. AES-128 - tu nhap khoa + ban ro
  5. Kiem tra nhanh tat ca test vector (chi in PASS/FAIL)
  0. Thoat
Chon:
```

- **Chọn 1 hoặc 2** → chọn tiếp số thứ tự của ví dụ (hoặc 0 để chạy tất cả). Chương trình in toàn bộ các trạng thái.
- **Chọn 3 hoặc 4** → tự gõ khoá và bản rõ (dạng hex) – **dùng để kiểm tra ví dụ trên bảng**. Ví dụ với DES:
  ```
  Khoa   : 133457799BBCDFF1
  Ban ro : 0123456789ABCDEF
  Ban ma mong doi (Enter de bo qua): 85E813540F0AB405
  ```
  (DES cần **16** ký tự hex, AES-128 cần **32** ký tự hex; được phép gõ chữ hoa/thường và có dấu cách.)
- **Chọn 5** → chỉ in bảng PASS/FAIL cho nhanh.

Output rất dài (mỗi vòng in nhiều dòng). Nếu muốn đọc thoải mái, nên **ghi ra file** như Bước 5.

---

## Bước 5. Ghi kết quả ra file

**Cách 1:** nhấp đúp `run_tests.bat` → tự biên dịch, chạy hết các ví dụ và ghi **mỗi test vector ra 1 file** trong thư mục `results/`:
- `results/DES_1.txt`, `DES_2.txt`, ... – mọi trạng thái của từng test vector DES
- `results/AES_1.txt`, `AES_2.txt`, ... – mọi trạng thái của từng test vector AES-128
- `results/summary.txt` – bảng PASS / FAIL, cho biết file nào ứng với test vector nào

**Cách 2:** gõ `des_aes.exe export` (làm đúng như Cách 1, nhưng không biên dịch lại).

**Cách 3:** tự ghi 1 ví dụ bất kỳ ra file – dấu `>` nghĩa là "ghi output vào file thay vì in ra màn hình":
```
des_aes.exe des 133457799BBCDFF1 0123456789ABCDEF > vi_du_cua_toi.txt
des_aes.exe aes 2b7e151628aed2a6abf7158809cf4f3c 3243f6a8885a308d313198a2e0370734 > vi_du_aes.txt
```

---

## Bước 6. Thêm ví dụ đã làm trên bảng vào danh sách

Mở file `include/test_vectors.h` bằng Notepad / VS Code, thêm 1 dòng vào danh sách tương ứng:

```cpp
static const std::vector<TestVector> DES_TEST_VECTORS = {
    ...
    {"Vi du tren bang tuan 5", "KHOA_16_HEX", "BAN_RO_16_HEX", "BAN_MA_MONG_DOI"},   // <- thêm dòng này
};
```
- Nếu chưa biết bản mã đúng, để `""` ở cột cuối.
- Lưu file, chạy lại **Bước 3** (biên dịch) rồi **Bước 4/5**.

---

## Bước 7. Nên đọc code theo thứ tự nào?

1. `include/des.h`, `include/aes.h` – xem có những hàm gì (đọc phần chú thích ở đầu file).
2. `src/des.cpp` – đọc từ trên xuống: `permute` → `desGenerateSubkeys` → `feistelF` → `desProcess`.
3. `src/aes.cpp` – đọc các hàm `subBytes`, `shiftRows`, `mixColumns`, `addRoundKey` → `aesKeyExpansion` → `aesEncrypt` → `aesDecrypt`.
4. `src/main.cpp` – phần menu và so sánh kết quả.
5. `include/des_tables.h`, `include/aes_tables.h` – các bảng hằng số (chép từ chuẩn FIPS).

Phần giải thích thuật toán xem trong [README.md](README.md).

---

## Lỗi thường gặp

| Lỗi | Cách sửa |
|---|---|
| Chạy exe báo lỗi *entry point not found* / thiếu `libstdc++-6.dll` | Biên dịch lại bằng lệnh có `-static` (build.bat đã có sẵn) |
| `'g++' is not recognized...` | Chưa cài g++ hoặc chưa thêm vào Path → xem Bước 1, nhớ mở lại cmd |
| `fatal error: des.h: No such file or directory` | Thiếu `-Iinclude` trong lệnh, hoặc đang đứng sai thư mục (phải `cd` vào thư mục chứa `src` và `include`) |
| `undefined reference to ...` | Thiếu file `.cpp` trong lệnh biên dịch – phải có đủ cả 4 file trong `src/` |
| PowerShell báo `des_aes.exe is not recognized` | Gõ `.\des_aes.exe` thay vì `des_aes.exe` |
| `[LOI] DES can khoa va ban ro dung 16 ky tu hex` | Khoá / bản rõ nhập sai độ dài hoặc có ký tự không phải hex (chỉ được 0-9, A-F) |
| Cửa sổ chạy xong tắt ngay | Chạy từ cmd thay vì nhấp đúp, hoặc dùng menu (chọn 0 mới thoát) |
| Dùng Dev-C++ / Code::Blocks | Tạo **Project** mới, thêm cả 4 file trong `src/` vào project, và thêm thư mục `include` vào *Include directories* |
