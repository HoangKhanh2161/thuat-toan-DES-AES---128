// utils.h - Các hàm tiện ích: đổi số <-> chuỗi hex / nhị phân
#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>

std::string toHex(uint64_t value, int digits);           // vd: toHex(0xAB, 4)    = "00AB"
std::string toBin(uint64_t value, int bits, int group);  // vd: toBin(0xF0, 8, 4) = "1111 0000"
std::string bytesToHex(const uint8_t* data, int length); // vd: {0x32, 0x43}      -> "3243"
std::string cleanHexInput(const std::string& s);         // bỏ dấu cách và "0x"

bool parseHex64(const std::string& hex, uint64_t& out);               // 16 ký tự hex -> số 64 bit
bool parseHexBytes(const std::string& hex, uint8_t* out, int length); // chuỗi hex -> mảng byte

void printLine(char c = '=', int length = 78);  // in đường kẻ ngang

#endif
