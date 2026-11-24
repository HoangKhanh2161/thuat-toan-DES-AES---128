// =====================================================================
//  aes.h - Thuật toán AES-128 (Advanced Encryption Standard)
//
//  - Khối dữ liệu: 128 bit = 16 byte, xếp thành ma trận trạng thái 4x4 (State)
//  - Khoá:         128 bit = 16 byte
//  - Số vòng:      10 vòng
//
//  Cách xếp 16 byte vào ma trận State (theo CỘT):
//      in[0]  in[4]  in[8]  in[12]
//      in[1]  in[5]  in[9]  in[13]
//      in[2]  in[6]  in[10] in[14]
//      in[3]  in[7]  in[11] in[15]
//
//  Nếu verbose = true, chương trình in ra ma trận State sau MỖI bước
//  (SubBytes, ShiftRows, MixColumns, AddRoundKey) của MỖI vòng.
// =====================================================================
#ifndef AES_H
#define AES_H

#include <cstdint>

// Mở rộng khoá 16 byte thành 44 word (w[0..43]), mỗi word 32 bit.
// Khoá vòng thứ r gồm 4 word: w[4r], w[4r+1], w[4r+2], w[4r+3].
void aesKeyExpansion(const uint8_t key[16], uint32_t w[44], bool verbose);

// Mã hoá 1 khối 16 byte: in (bản rõ) -> out (bản mã)
void aesEncrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose);

// Giải mã 1 khối 16 byte: in (bản mã) -> out (bản rõ)
void aesDecrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose);

#endif
