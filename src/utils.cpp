// =====================================================================
//  utils.cpp - Cài đặt các hàm tiện ích (xem giải thích trong utils.h)
// =====================================================================
#include "utils.h"

#include <cctype>
#include <iostream>

static const char HEX_DIGITS[] = "0123456789ABCDEF";

std::string toHex(uint64_t value, int numHexDigits) {
    std::string s(numHexDigits, '0');
    // Lấy từng nhóm 4 bit từ phải sang trái
    for (int i = numHexDigits - 1; i >= 0; i--) {
        s[i] = HEX_DIGITS[value & 0xF];
        value >>= 4;
    }
    return s;
}

std::string toBin(uint64_t value, int numBits, int groupSize) {
    std::string s;
    for (int i = numBits - 1; i >= 0; i--) {
        s += ((value >> i) & 1) ? '1' : '0';
        int bitsWritten = numBits - i;
        if (groupSize > 0 && bitsWritten % groupSize == 0 && i != 0) {
            s += ' ';
        }
    }
    return s;
}

std::string bytesToHex(const uint8_t* data, int length) {
    const char* lower = "0123456789abcdef";
    std::string s;
    for (int i = 0; i < length; i++) {
        s += lower[data[i] >> 4];
        s += lower[data[i] & 0xF];
    }
    return s;
}

std::string cleanHexInput(const std::string& s) {
    std::string t;
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) t += c;
    }
    if (t.size() >= 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) {
        t = t.substr(2);
    }
    return t;
}

// Đổi 1 ký tự hex thành giá trị 0..15, trả về -1 nếu không hợp lệ
static int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool parseHex64(const std::string& hex, uint64_t& out) {
    std::string h = cleanHexInput(hex);
    if (h.size() != 16) return false;
    out = 0;
    for (char c : h) {
        int v = hexValue(c);
        if (v < 0) return false;
        out = (out << 4) | static_cast<uint64_t>(v);
    }
    return true;
}

bool parseHexBytes(const std::string& hex, uint8_t* out, int length) {
    std::string h = cleanHexInput(hex);
    if (static_cast<int>(h.size()) != 2 * length) return false;
    for (int i = 0; i < length; i++) {
        int hi = hexValue(h[2 * i]);
        int lo = hexValue(h[2 * i + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = static_cast<uint8_t>((hi << 4) | lo);
    }
    return true;
}

void printLine(char c, int length) {
    std::cout << std::string(length, c) << "\n";
}
