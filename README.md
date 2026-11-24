# Thuật toán DES và AES-128 (C++)

Bài tập môn học: cài đặt hai thuật toán mã hoá khối **DES** và **AES-128**, kiểm tra bằng các test vector,
và **in ra giá trị của từng trạng thái** trong quá trình mã hoá / giải mã để đối chiếu với các ví dụ làm tay trên lớp.

> 👉 **Người mới học C++?** Đọc file [HUONG_DAN.md](HUONG_DAN.md) để biết cách biên dịch và chạy từng bước.

---

## 1. Cấu trúc thư mục

```
thuat-toan-DES-AES---128/
├── include/                 # Các file header (.h)
│   ├── des.h                #   khai báo hàm DES
│   ├── des_tables.h         #   các bảng của DES: IP, IP^-1, E, P, PC-1, PC-2, 8 hộp S
│   ├── aes.h                #   khai báo hàm AES-128
│   ├── aes_tables.h         #   các bảng của AES: S-box, S-box nghịch đảo, Rcon
│   ├── test_vectors.h       #   DANH SÁCH TEST VECTOR (thêm ví dụ mới ở đây)
│   └── utils.h              #   hàm tiện ích: đổi hex, in nhị phân...
├── src/                     # Các file cài đặt (.cpp)
│   ├── des.cpp              #   cài đặt DES (sinh khoá con, hàm f, 16 vòng Feistel)
│   ├── aes.cpp              #   cài đặt AES-128 (mở rộng khoá, SubBytes, ShiftRows, MixColumns...)
│   ├── utils.cpp            #   cài đặt hàm tiện ích
│   └── main.cpp             #   chương trình chính: menu, chạy test vector, so sánh PASS/FAIL
├── results/                 # KẾT QUẢ CHẠY THUẬT TOÁN
│   ├── des_result.txt       #   mọi trạng thái của DES cho từng test vector
│   ├── aes_result.txt       #   mọi trạng thái của AES-128 cho từng test vector
│   └── summary.txt          #   bảng tổng hợp PASS / FAIL
├── build.bat                # Windows: biên dịch
├── run_tests.bat            # Windows: biên dịch + chạy hết test vector + ghi vào results/
├── Makefile                 # Linux / macOS: make, make results
├── HUONG_DAN.md             # Hướng dẫn chạy cho người mới
└── README.md
```

Chỉ dùng thư viện chuẩn của C++ (`<iostream>`, `<string>`, `<vector>`, `<cstdint>`, `<iomanip>`), không cần cài thêm gì.

## 2. Chạy nhanh

```bash
# Windows: nhấp đúp run_tests.bat, hoặc:
g++ -std=c++17 -Wall -O2 -Iinclude src/main.cpp src/utils.cpp src/des.cpp src/aes.cpp -o des_aes.exe
des_aes.exe            # hiện menu
des_aes.exe des        # chạy tất cả test vector DES, in mọi trạng thái
des_aes.exe aes        # chạy tất cả test vector AES-128, in mọi trạng thái
des_aes.exe check      # chỉ in PASS / FAIL
des_aes.exe des 133457799BBCDFF1 0123456789ABCDEF                 # DES với khoá + bản rõ tự chọn
des_aes.exe aes 2b7e151628aed2a6abf7158809cf4f3c 3243f6a8885a308d313198a2e0370734
```

## 3. Kết quả kiểm định (test vector)

Mỗi test vector được **mã hoá** rồi **giải mã ngược lại**; chương trình so sánh bản mã với giá trị mong đợi
và kiểm tra giải mã có ra lại đúng bản rõ hay không. Toàn bộ kết quả nằm trong thư mục [`results/`](results/).

| # | Thuật toán | Nguồn ví dụ | Khoá | Bản rõ | Bản mã | Kết quả |
|---|---|---|---|---|---|---|
| 1 | DES | J. Orlin Grabbe – *The DES Algorithm Illustrated* | `133457799BBCDFF1` | `0123456789ABCDEF` | `85E813540F0AB405` | ✅ PASS |
| 2 | DES | Giáo trình Stallings | `0F1571C947D9E859` | `02468ACEECA86420` | `DA02CE3A89ECAC3B` | ✅ PASS |
| 3 | DES | FIPS 81 ("Now is t") | `0123456789ABCDEF` | `4E6F772069732074` | `3FA40E8A984D4815` | ✅ PASS |
| 4 | DES | Bản mã toàn số 0 | `0E329232EA6D0D73` | `8787878787878787` | `0000000000000000` | ✅ PASS |
| 5 | AES-128 | FIPS-197 Appendix B | `2b7e151628aed2a6abf7158809cf4f3c` | `3243f6a8885a308d313198a2e0370734` | `3925841d02dc09fbdc118597196a0b32` | ✅ PASS |
| 6 | AES-128 | FIPS-197 Appendix C.1 | `000102030405060708090a0b0c0d0e0f` | `00112233445566778899aabbccddeeff` | `69c4e0d86a7b0430d8cdb78070b4c55a` | ✅ PASS |
| 7 | AES-128 | Giáo trình Stallings | `0f1571c947d9e8590cb7add6af7f6798` | `0123456789abcdeffedcba9876543210` | `ff0b844a0853bf7c6934ab4364148fb9` | ✅ PASS |
| 8 | AES-128 | "Thats my Kung Fu" / "Two One Nine Two" | `5468617473206d79204b756e67204675` | `54776f204f6e65204e696e652054776f` | `29c3505f571420f6402299b31a02d73a` | ✅ PASS |

**Muốn kiểm tra ví dụ làm trên bảng?** Mở [`include/test_vectors.h`](include/test_vectors.h), thêm 1 dòng theo mẫu
rồi chạy lại `run_tests.bat` — hoặc chọn mục 3 / 4 trong menu để gõ trực tiếp khoá và bản rõ.

---

## 4. Giải thích đơn giản thuật toán DES

DES mã hoá **khối 64 bit** với **khoá 64 bit** (thật ra chỉ dùng 56 bit, 8 bit còn lại là bit chẵn lẻ), qua **16 vòng Feistel**.

### 4.1. Sinh 16 khoá con (`desGenerateSubkeys` trong `des.cpp`)
1. **PC-1**: chọn 56 bit từ khoá 64 bit → chia đôi thành `C0` (28 bit) và `D0` (28 bit).
2. Mỗi vòng `i`: dịch vòng trái `C` và `D` đi 1 hoặc 2 bit (bảng `SHIFTS`: vòng 1, 2, 9, 16 dịch 1 bit, còn lại dịch 2 bit).
3. **PC-2**: chọn 48 bit từ `C_i D_i` → được khoá con `K_i`.

### 4.2. Mã hoá (`desProcess` trong `des.cpp`)
1. **IP** (hoán vị khởi đầu) → chia đôi thành `L0`, `R0` (mỗi nửa 32 bit).
2. Lặp 16 vòng:  `L_i = R_(i-1)`  và  `R_i = L_(i-1) XOR f(R_(i-1), K_i)`
3. Đổi chỗ thành `R16 L16`, rồi **IP⁻¹** (hoán vị kết thúc) → bản mã.

### 4.3. Hàm f (`feistelF` trong `des.cpp`)
1. **E**: mở rộng `R` từ 32 bit lên 48 bit.
2. **XOR** với khoá con `K_i`.
3. **8 hộp S**: chia 48 bit thành 8 nhóm 6 bit. Với mỗi nhóm `b1b2b3b4b5b6`: **hàng** = `b1b6`, **cột** = `b2b3b4b5` → tra bảng ra 4 bit. Tổng cộng ra 32 bit.
4. **P**: hoán vị 32 bit → kết quả của hàm f.

### 4.4. Giải mã
Giống hệt mã hoá, chỉ dùng khoá con theo **thứ tự ngược**: K16, K15, ..., K1.

### 4.5. Chương trình in ra những gì (file `results/des_result.txt`)
- Khoá K, `K+ = PC-1(K)`, `C0`, `D0`, rồi bảng 16 dòng: số bit dịch, `C_i`, `D_i`, `K_i` (hex và nhị phân).
- `IP(M)`, `L0`, `R0`.
- Mỗi vòng: `E(R)`, `K_i`, `E(R) XOR K_i`, **từng hộp S** (6 bit vào → hàng, cột → giá trị ra), đầu ra S-box, `f = P(...)`, `L_i`, `R_i`.
- `R16 L16` và bản mã sau `IP⁻¹`.
- Toàn bộ quá trình giải mã tương tự.

---

## 5. Giải thích đơn giản thuật toán AES-128

AES-128 mã hoá **khối 128 bit (16 byte)** với **khoá 128 bit**, qua **10 vòng**.
16 byte được xếp thành **ma trận trạng thái (State) 4×4 theo cột**: byte 0..3 là cột 0, byte 4..7 là cột 1, ...

### 5.1. Mở rộng khoá (`aesKeyExpansion` trong `aes.cpp`)
Từ khoá 16 byte tạo ra 44 word `w[0..43]` (mỗi word 4 byte):
- `w[0..3]` lấy thẳng từ khoá.
- Với `i` chia hết cho 4: `temp = SubWord(RotWord(w[i-1])) XOR Rcon[i/4]`, còn lại `temp = w[i-1]`.
- `w[i] = w[i-4] XOR temp`.
- Khoá vòng `K_r` = 4 word `w[4r..4r+3]`.

### 5.2. Bốn phép biến đổi
| Phép biến đổi | Làm gì | Nghịch đảo (dùng khi giải mã) |
|---|---|---|
| **SubBytes** | Thay mỗi byte bằng giá trị trong S-box (byte `XY` → hàng X, cột Y) | InvSubBytes (S-box nghịch đảo) |
| **ShiftRows** | Hàng `r` dịch vòng trái `r` byte (hàng 0 giữ nguyên) | InvShiftRows (dịch phải) |
| **MixColumns** | Nhân mỗi cột với ma trận `[02 03 01 01; 01 02 03 01; 01 01 02 03; 03 01 01 02]` trong GF(2⁸) | InvMixColumns (ma trận `0e 0b 0d 09`) |
| **AddRoundKey** | XOR State với khoá vòng | Chính nó (XOR 2 lần = không đổi) |

Phép nhân trong GF(2⁸) dùng hàm `xtime` (nhân 2 = dịch trái 1 bit, nếu tràn thì XOR `0x1b`).

### 5.3. Thứ tự các vòng
- **Vòng 0**: AddRoundKey(K0)
- **Vòng 1 → 9**: SubBytes → ShiftRows → MixColumns → AddRoundKey(K_i)
- **Vòng 10**: SubBytes → ShiftRows → AddRoundKey(K10) *(không có MixColumns)*
- **Giải mã** làm ngược lại: InvShiftRows → InvSubBytes → AddRoundKey → InvMixColumns, dùng khoá K10 → K0.

### 5.4. Chương trình in ra những gì (file `results/aes_result.txt`)
- Bảng mở rộng khoá giống FIPS-197 Appendix A: `temp`, `RotWord`, `SubWord`, `Rcon`, `XOR Rcon`, `w[i-4]`, `w[i]`, và 11 khoá vòng `K0..K10`.
- Mỗi vòng: các **ma trận 4×4 đặt cạnh nhau** — Start, SubBytes, ShiftRows, MixColumns, RoundKey, AddRoundKey — kèm dạng 1 dòng hex 32 ký tự để dễ so sánh.
- Toàn bộ quá trình giải mã tương tự.

Ví dụ (vòng 1, FIPS-197 Appendix B):
```
  Start         SubBytes      ShiftRows     MixColumns    RoundKey 1    AddRoundKey
  19 a0 9a e9   d4 e0 b8 1e   d4 e0 b8 1e   04 e0 48 28   a0 88 23 2a   a4 68 6b 02
  3d f4 c6 f8   27 bf b4 41   bf b4 41 27   66 cb f8 06   fa 54 a3 6c   9c 9f 5b 6a
  e3 e2 8d 48   11 98 5d 52   5d 52 11 98   81 19 d3 26   fe 2c 39 76   7f 35 ea 50
  be 2b 2a 08   ae f1 e5 30   30 ae f1 e5   e5 9a 7a 4c   17 b1 39 05   f2 2b 43 49
```

---

## 6. Kiểm tra độ chính xác

Ngoài 8 test vector ở trên, khi phát triển code còn được kiểm tra thêm:
- S-box của AES được tính lại từ định nghĩa toán học (nghịch đảo trong GF(2⁸) + biến đổi affine) và khớp 100% với bảng.
- Mỗi hàng của 8 hộp S trong DES là một hoán vị của 0..15; IP và IP⁻¹ là nghịch đảo của nhau.
- Phép thử DES của Rivest (16 lần mã hoá/giải mã liên tiếp, kết quả cuối `1B1A2DDB4C642438`) cho kết quả đúng.
- 2000 bộ khoá / bản rõ AES ngẫu nhiên: giải mã luôn ra đúng bản rõ.

*Ghi chú: bài tập cho phép "vibe code"; code được viết với sự hỗ trợ của AI và đã được kiểm tra bằng các test vector chuẩn ở trên.*
