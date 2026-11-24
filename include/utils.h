// =====================================================================
//  utils.h - Các hàm tiện ích dùng chung cho DES và AES
//  (chuyển đổi hex <-> số, in số dạng nhị phân, kẻ đường phân cách...)
// =====================================================================
#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>

// Đổi một số (tối đa 64 bit) thành chuỗi hex có đúng `numHexDigits` ký tự.
// Ví dụ: toHex(0xAB, 4) -> "00AB"
std::string toHex(uint64_t value, int numHexDigits);

// Đổi một số thành chuỗi nhị phân `numBits` bit, cứ `groupSize` bit thì cách 1 dấu cách.
// Ví dụ: toBin(0xF0, 8, 4) -> "1111 0000"
std::string toBin(uint64_t value, int numBits, int groupSize);

// Đổi một mảng byte thành chuỗi hex liền nhau (chữ thường, giống FIPS-197).
// Ví dụ: {0x32, 0x43} -> "3243"
std::string bytesToHex(const uint8_t* data, int length);

// Xoá dấu cách và tiền tố "0x" khỏi chuỗi người dùng nhập.
std::string cleanHexInput(const std::string& s);

// Đọc chuỗi hex đúng 16 ký tự (64 bit) thành số uint64_t.
// Trả về false nếu chuỗi sai độ dài hoặc có ký tự không phải hex.
bool parseHex64(const std::string& hex, uint64_t& out);

// Đọc chuỗi hex đúng 2*length ký tự thành mảng byte.
bool parseHexBytes(const std::string& hex, uint8_t* out, int length);

// In một đường kẻ ngang để dễ nhìn.
void printLine(char c = '=', int length = 78);

#endif
