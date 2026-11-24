// utils.cpp - Các hàm tiện ích
#include "utils.h"

#include <cctype>
#include <iostream>

using namespace std;

string toHex(uint64_t value, int digits) {
    const char* d = "0123456789ABCDEF";
    string s(digits, '0');
    for (int i = digits - 1; i >= 0; i--) {  // lấy từng 4 bit, từ phải sang trái
        s[i] = d[value & 0xF];
        value >>= 4;
    }
    return s;
}

string toBin(uint64_t value, int bits, int group) {
    string s;
    for (int i = bits - 1; i >= 0; i--) {
        s += ((value >> i) & 1) ? '1' : '0';
        if (group > 0 && i > 0 && (bits - i) % group == 0) s += ' ';  // cách nhóm
    }
    return s;
}

string bytesToHex(const uint8_t* data, int length) {
    const char* d = "0123456789abcdef";
    string s;
    for (int i = 0; i < length; i++) {
        s += d[data[i] >> 4];
        s += d[data[i] & 0xF];
    }
    return s;
}

string cleanHexInput(const string& s) {
    string t;
    for (char c : s)
        if (!isspace((unsigned char)c)) t += c;
    if (t.size() >= 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) t = t.substr(2);
    return t;
}

// Ký tự hex -> giá trị 0..15 (trả về -1 nếu sai)
static int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool parseHex64(const string& hex, uint64_t& out) {
    string h = cleanHexInput(hex);
    if (h.size() != 16) return false;
    out = 0;
    for (char c : h) {
        int v = hexValue(c);
        if (v < 0) return false;
        out = (out << 4) | v;
    }
    return true;
}

bool parseHexBytes(const string& hex, uint8_t* out, int length) {
    string h = cleanHexInput(hex);
    if ((int)h.size() != 2 * length) return false;
    for (int i = 0; i < length; i++) {
        int hi = hexValue(h[2 * i]);
        int lo = hexValue(h[2 * i + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (uint8_t)(hi * 16 + lo);
    }
    return true;
}

void printLine(char c, int length) {
    cout << string(length, c) << "\n";
}
