// des.h - Thuật toán DES
//   Khối 64 bit, khoá 64 bit (dùng 56 bit), 16 vòng.
//   verbose = true: in ra giá trị của từng trạng thái.
#ifndef DES_H
#define DES_H

#include <cstdint>

void desGenerateSubkeys(uint64_t key, uint64_t subkeys[16], bool verbose);  // sinh K1..K16
uint64_t desEncrypt(uint64_t plaintext, uint64_t key, bool verbose);        // mã hoá
uint64_t desDecrypt(uint64_t ciphertext, uint64_t key, bool verbose);       // giải mã

#endif
