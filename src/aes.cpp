// aes.cpp - Cài đặt AES-128
//   Mã hoá:
//     Vòng 0    : AddRoundKey(K0)
//     Vòng 1..9 : SubBytes -> ShiftRows -> MixColumns -> AddRoundKey(K_i)
//     Vòng 10   : SubBytes -> ShiftRows -> AddRoundKey(K10)       (không có MixColumns)
//   Giải mã: làm ngược lại bằng các phép nghịch đảo, dùng khoá K10 -> K0
#include "aes.h"
#include "aes_tables.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Ma trận trạng thái 4x4: a[hàng][cột]
struct State {
    uint8_t a[4][4];
};

// 16 byte -> ma trận (xếp theo cột)
static State toState(const uint8_t in[16]) {
    State s;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++) s.a[r][c] = in[r + 4 * c];
    return s;
}

// ma trận -> 16 byte
static void fromState(const State& s, uint8_t out[16]) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++) out[r + 4 * c] = s.a[r][c];
}

static string stateHex(const State& s) {
    uint8_t b[16];
    fromState(s, b);
    return bytesToHex(b, 16);
}

// Khoá vòng thứ r (4 word w[4r..4r+3]) -> ma trận, mỗi word là 1 cột
static State roundKey(const uint32_t w[44], int r) {
    State k;
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++) k.a[row][c] = (w[4 * r + c] >> (24 - 8 * row)) & 0xFF;
    return k;
}

// ---------------- 4 phép biến đổi và nghịch đảo ----------------

// SubBytes: thay mỗi byte bằng giá trị trong S-box
static void subBytes(State& s) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] = AES_SBOX[s.a[r][c]];
}

static void invSubBytes(State& s) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] = AES_INV_SBOX[s.a[r][c]];
}

// ShiftRows: hàng r dịch vòng sang trái r byte
static void shiftRows(State& s) {
    for (int r = 1; r < 4; r++) {
        uint8_t t[4];
        for (int c = 0; c < 4; c++) t[c] = s.a[r][(c + r) % 4];
        for (int c = 0; c < 4; c++) s.a[r][c] = t[c];
    }
}

// InvShiftRows: hàng r dịch vòng sang phải r byte
static void invShiftRows(State& s) {
    for (int r = 1; r < 4; r++) {
        uint8_t t[4];
        for (int c = 0; c < 4; c++) t[(c + r) % 4] = s.a[r][c];
        for (int c = 0; c < 4; c++) s.a[r][c] = t[c];
    }
}

// Nhân 2 trong GF(2^8): dịch trái 1 bit, nếu tràn thì XOR 0x1b
static uint8_t xtime(uint8_t x) {
    return (uint8_t)((x << 1) ^ ((x & 0x80) ? 0x1b : 0));
}

// Nhân a * b trong GF(2^8)
static uint8_t mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    while (b) {
        if (b & 1) result ^= a;  // cộng trong GF(2^8) là XOR
        a = xtime(a);
        b >>= 1;
    }
    return result;
}

// MixColumns: nhân mỗi cột với ma trận [02 03 01 01; 01 02 03 01; 01 01 02 03; 03 01 01 02]
static void mixColumns(State& s) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = s.a[0][c], a1 = s.a[1][c], a2 = s.a[2][c], a3 = s.a[3][c];
        s.a[0][c] = mul(a0, 2) ^ mul(a1, 3) ^ a2 ^ a3;
        s.a[1][c] = a0 ^ mul(a1, 2) ^ mul(a2, 3) ^ a3;
        s.a[2][c] = a0 ^ a1 ^ mul(a2, 2) ^ mul(a3, 3);
        s.a[3][c] = mul(a0, 3) ^ a1 ^ a2 ^ mul(a3, 2);
    }
}

// InvMixColumns: nhân với ma trận [0e 0b 0d 09; 09 0e 0b 0d; 0d 09 0e 0b; 0b 0d 09 0e]
static void invMixColumns(State& s) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = s.a[0][c], a1 = s.a[1][c], a2 = s.a[2][c], a3 = s.a[3][c];
        s.a[0][c] = mul(a0, 0x0e) ^ mul(a1, 0x0b) ^ mul(a2, 0x0d) ^ mul(a3, 0x09);
        s.a[1][c] = mul(a0, 0x09) ^ mul(a1, 0x0e) ^ mul(a2, 0x0b) ^ mul(a3, 0x0d);
        s.a[2][c] = mul(a0, 0x0d) ^ mul(a1, 0x09) ^ mul(a2, 0x0e) ^ mul(a3, 0x0b);
        s.a[3][c] = mul(a0, 0x0b) ^ mul(a1, 0x0d) ^ mul(a2, 0x09) ^ mul(a3, 0x0e);
    }
}

// AddRoundKey: XOR với khoá vòng
static void addRoundKey(State& s, const State& k) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] ^= k.a[r][c];
}

// ---------------- In kết quả ----------------

static string hex2(uint8_t b) {
    const char* d = "0123456789abcdef";
    return string(1, d[b >> 4]) + d[b & 0xF];
}

static string wordHex(uint32_t w) {
    return hex2(w >> 24) + hex2(w >> 16) + hex2(w >> 8) + hex2(w);
}

static void title(const string& text) {
    cout << "\n";
    printLine('-');
    cout << " " << text << "\n";
    printLine('-');
}

// Bỏ dấu cách thừa ở cuối dòng rồi in
static void printTrimmed(const string& line) {
    cout << line.substr(0, line.find_last_not_of(' ') + 1) << "\n";
}

// In các ma trận cạnh nhau (mỗi ma trận có 1 tên), rồi in lại dạng 1 dòng hex
static void printStates(const vector<string>& names, const vector<State>& states) {
    vector<int> width;
    for (const string& n : names) width.push_back(n.size() > 11 ? n.size() : 11);

    string line = "  ";
    for (size_t i = 0; i < names.size(); i++) {
        string cell = names[i];
        cell.resize(width[i], ' ');
        line += cell + "   ";
    }
    printTrimmed(line);

    for (int r = 0; r < 4; r++) {
        line = "  ";
        for (size_t i = 0; i < states.size(); i++) {
            string cell;
            for (int c = 0; c < 4; c++) cell += hex2(states[i].a[r][c]) + (c < 3 ? " " : "");
            cell.resize(width[i], ' ');
            line += cell + "   ";
        }
        printTrimmed(line);
    }

    for (size_t i = 0; i < states.size(); i++)
        cout << "    " << left << setw(14) << names[i] << ": " << stateHex(states[i]) << right << "\n";
}

// ---------------- Mở rộng khoá ----------------
//   w[0..3] lấy từ khoá
//   w[i] = w[i-4] XOR temp, với temp = w[i-1]
//          (nếu i chia hết cho 4 thì temp = SubWord(RotWord(w[i-1])) XOR Rcon[i/4])

static uint32_t rotWord(uint32_t w) {  // [a0 a1 a2 a3] -> [a1 a2 a3 a0]
    return (w << 8) | (w >> 24);
}

static uint32_t subWord(uint32_t w) {  // S-box cho từng byte
    return ((uint32_t)AES_SBOX[(w >> 24) & 0xFF] << 24) | ((uint32_t)AES_SBOX[(w >> 16) & 0xFF] << 16) |
           ((uint32_t)AES_SBOX[(w >> 8) & 0xFF] << 8) | AES_SBOX[w & 0xFF];
}

void aesKeyExpansion(const uint8_t key[16], uint32_t w[44], bool verbose) {
    for (int i = 0; i < 4; i++)
        w[i] = ((uint32_t)key[4 * i] << 24) | ((uint32_t)key[4 * i + 1] << 16) | ((uint32_t)key[4 * i + 2] << 8) |
               key[4 * i + 3];

    if (verbose) {
        title("KEY EXPANSION");
        for (int i = 0; i < 4; i++) cout << "  w[" << setw(2) << i << "] = " << wordHex(w[i]) << "\n";
        cout << "\n   i  temp      RotWord   SubWord   Rcon      XOR Rcon  w[i-4]    w[i]\n";
    }

    for (int i = 4; i < 44; i++) {
        uint32_t temp = w[i - 1];
        if (i % 4 == 0) {
            uint32_t rot = rotWord(temp);
            uint32_t sub = subWord(rot);
            uint32_t withRcon = sub ^ AES_RCON[i / 4];
            w[i] = w[i - 4] ^ withRcon;
            if (verbose)
                cout << "  " << setw(2) << i << "  " << wordHex(temp) << "  " << wordHex(rot) << "  " << wordHex(sub)
                     << "  " << wordHex(AES_RCON[i / 4]) << "  " << wordHex(withRcon) << "  " << wordHex(w[i - 4])
                     << "  " << wordHex(w[i]) << "\n";
        } else {
            w[i] = w[i - 4] ^ temp;
            if (verbose)
                cout << "  " << setw(2) << i << "  " << wordHex(temp) << string(42, ' ') << wordHex(w[i - 4]) << "  "
                     << wordHex(w[i]) << "\n";
        }
    }

    if (verbose) {
        cout << "\n  Khoa vong:\n";
        for (int r = 0; r <= 10; r++)
            cout << "  K" << left << setw(2) << r << right << " = " << wordHex(w[4 * r]) << wordHex(w[4 * r + 1])
                 << wordHex(w[4 * r + 2]) << wordHex(w[4 * r + 3]) << "\n";
    }
}

// ---------------- Mã hoá ----------------
void aesEncrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " AES-128 - MA HOA\n";
        printLine('=');
        cout << "  Key       = " << bytesToHex(key, 16) << "\n";
        cout << "  Plaintext = " << bytesToHex(in, 16) << "\n";
    }

    uint32_t w[44];
    aesKeyExpansion(key, w, verbose);

    // Vòng 0
    State s = toState(in);
    State input = s;
    State k = roundKey(w, 0);
    addRoundKey(s, k);
    if (verbose) {
        title("ROUND 0");
        printStates({"Input", "RoundKey 0", "AddRoundKey"}, {input, k, s});
    }

    // Vòng 1..10
    for (int round = 1; round <= 10; round++) {
        State start = s;
        subBytes(s);
        State afterSub = s;
        shiftRows(s);
        State afterShift = s;
        if (round < 10) mixColumns(s);  // vòng 10 không có MixColumns
        State afterMix = s;
        k = roundKey(w, round);
        addRoundKey(s, k);

        if (verbose) {
            string keyName = "RoundKey " + to_string(round);
            if (round < 10) {
                title("ROUND " + to_string(round));
                printStates({"Start", "SubBytes", "ShiftRows", "MixColumns", keyName, "AddRoundKey"},
                            {start, afterSub, afterShift, afterMix, k, s});
            } else {
                title("ROUND 10  (khong co MixColumns)");
                printStates({"Start", "SubBytes", "ShiftRows", keyName, "AddRoundKey"},
                            {start, afterSub, afterShift, k, s});
            }
        }
    }

    fromState(s, out);
    if (verbose) {
        title("OUTPUT");
        cout << "  Ciphertext = " << bytesToHex(out, 16) << "\n";
    }
}

// ---------------- Giải mã ----------------
void aesDecrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " AES-128 - GIAI MA (khoa vong dung nguoc K10 -> K0)\n";
        printLine('=');
        cout << "  Key        = " << bytesToHex(key, 16) << "\n";
        cout << "  Ciphertext = " << bytesToHex(in, 16) << "\n";
    }

    uint32_t w[44];
    aesKeyExpansion(key, w, false);

    // Vòng 0: AddRoundKey với K10
    State s = toState(in);
    State input = s;
    State k = roundKey(w, 10);
    addRoundKey(s, k);
    if (verbose) {
        title("ROUND 0");
        printStates({"Input", "RoundKey 10", "AddRoundKey"}, {input, k, s});
    }

    // Vòng 1..10: dùng khoá K9 .. K0
    for (int round = 1; round <= 10; round++) {
        State start = s;
        invShiftRows(s);
        State afterShift = s;
        invSubBytes(s);
        State afterSub = s;
        k = roundKey(w, 10 - round);
        addRoundKey(s, k);
        State afterAdd = s;
        if (round < 10) invMixColumns(s);  // vòng 10 không có InvMixColumns

        if (verbose) {
            string keyName = "RoundKey " + to_string(10 - round);
            if (round < 10) {
                title("ROUND " + to_string(round));
                printStates({"Start", "InvShiftRows", "InvSubBytes", keyName, "AddRoundKey", "InvMixColumns"},
                            {start, afterShift, afterSub, k, afterAdd, s});
            } else {
                title("ROUND 10  (khong co InvMixColumns)");
                printStates({"Start", "InvShiftRows", "InvSubBytes", keyName, "AddRoundKey"},
                            {start, afterShift, afterSub, k, afterAdd});
            }
        }
    }

    fromState(s, out);
    if (verbose) {
        title("OUTPUT");
        cout << "  Plaintext = " << bytesToHex(out, 16) << "\n";
    }
}
