// =====================================================================
//  des.h - Thuật toán DES (Data Encryption Standard)
//
//  - Khối dữ liệu: 64 bit   (lưu trong 1 biến uint64_t)
//  - Khoá:         64 bit   (thực chất dùng 56 bit, 8 bit còn lại là bit chẵn lẻ)
//  - Số vòng:      16 vòng Feistel
//
//  Nếu tham số verbose = true, hàm sẽ in ra TẤT CẢ các giá trị trung gian
//  (IP, L_i, R_i, E(R), XOR khoá, từng S-box, P, khoá con K_i, ...)
//  để so sánh với kết quả làm tay trên bảng.
// =====================================================================
#ifndef DES_H
#define DES_H

#include <cstdint>

// Sinh 16 khoá con K1..K16 (mỗi khoá 48 bit) từ khoá chính 64 bit.
// Kết quả lưu vào subkeys[0..15] (subkeys[0] là K1).
void desGenerateSubkeys(uint64_t key, uint64_t subkeys[16], bool verbose);

// Mã hoá 1 khối 64 bit.
uint64_t desEncrypt(uint64_t plaintext, uint64_t key, bool verbose);

// Giải mã 1 khối 64 bit (giống mã hoá nhưng dùng khoá con theo thứ tự ngược K16..K1).
uint64_t desDecrypt(uint64_t ciphertext, uint64_t key, bool verbose);

#endif
