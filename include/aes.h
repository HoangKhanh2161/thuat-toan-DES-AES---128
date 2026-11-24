// aes.h - Thuật toán AES-128
//   Khối 16 byte, khoá 16 byte, 10 vòng.
//   16 byte được xếp vào ma trận trạng thái 4x4 theo CỘT:
//       in[0] in[4] in[8]  in[12]
//       in[1] in[5] in[9]  in[13]
//       in[2] in[6] in[10] in[14]
//       in[3] in[7] in[11] in[15]
//   verbose = true: in ra ma trận trạng thái sau từng bước của từng vòng.
#ifndef AES_H
#define AES_H

#include <cstdint>

void aesKeyExpansion(const uint8_t key[16], uint32_t w[44], bool verbose);                  // khoá -> w[0..43]
void aesEncrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose);  // mã hoá
void aesDecrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose);  // giải mã

#endif
