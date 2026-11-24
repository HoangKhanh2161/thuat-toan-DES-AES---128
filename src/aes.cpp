// =====================================================================
//  aes.cpp - Cài đặt thuật toán AES-128
//
//  Sơ đồ mã hoá (10 vòng):
//
//    Vòng 0      : AddRoundKey(K0)
//    Vòng 1 .. 9 : SubBytes -> ShiftRows -> MixColumns -> AddRoundKey(K_i)
//    Vòng 10     : SubBytes -> ShiftRows ->               AddRoundKey(K10)
//                  (vòng cuối KHÔNG có MixColumns)
//
//  Sơ đồ giải mã (làm ngược lại, dùng các phép biến đổi nghịch đảo):
//
//    Vòng 0      : AddRoundKey(K10)
//    Vòng 1 .. 9 : InvShiftRows -> InvSubBytes -> AddRoundKey(K_(10-i)) -> InvMixColumns
//    Vòng 10     : InvShiftRows -> InvSubBytes -> AddRoundKey(K0)
// =====================================================================
#include "aes.h"
#include "aes_tables.h"
#include "utils.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::string;
using std::vector;

// Ma trận trạng thái 4x4 byte: a[hàng][cột]
struct State {
    uint8_t a[4][4];
};

// ---------------------------------------------------------------------
//  Chuyển đổi giữa mảng 16 byte và ma trận State (xếp theo cột)
// ---------------------------------------------------------------------
static State bytesToState(const uint8_t in[16]) {
    State s;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++) s.a[r][c] = in[r + 4 * c];
    return s;
}

static void stateToBytes(const State& s, uint8_t out[16]) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++) out[r + 4 * c] = s.a[r][c];
}

static string stateToHex(const State& s) {
    uint8_t bytes[16];
    stateToBytes(s, bytes);
    return bytesToHex(bytes, 16);
}

// Lấy khoá vòng thứ `round` (4 word) và xếp thành ma trận 4x4:
// word w[4*round + c] là cột c của ma trận.
static State roundKeyToState(const uint32_t w[44], int round) {
    State k;
    for (int c = 0; c < 4; c++) {
        uint32_t word = w[4 * round + c];
        for (int r = 0; r < 4; r++) k.a[r][c] = static_cast<uint8_t>(word >> (24 - 8 * r));
    }
    return k;
}

// =====================================================================
//  CÁC PHÉP BIẾN ĐỔI CỦA AES
// =====================================================================

// SubBytes: thay mỗi byte bằng giá trị tương ứng trong S-box
static void subBytes(State& s) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] = AES_SBOX[s.a[r][c]];
}

static void invSubBytes(State& s) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] = AES_INV_SBOX[s.a[r][c]];
}

// ShiftRows: hàng r dịch vòng sang TRÁI r byte (hàng 0 giữ nguyên)
static void shiftRows(State& s) {
    for (int r = 1; r < 4; r++) {
        uint8_t tmp[4];
        for (int c = 0; c < 4; c++) tmp[c] = s.a[r][(c + r) % 4];
        for (int c = 0; c < 4; c++) s.a[r][c] = tmp[c];
    }
}

// InvShiftRows: hàng r dịch vòng sang PHẢI r byte
static void invShiftRows(State& s) {
    for (int r = 1; r < 4; r++) {
        uint8_t tmp[4];
        for (int c = 0; c < 4; c++) tmp[(c + r) % 4] = s.a[r][c];
        for (int c = 0; c < 4; c++) s.a[r][c] = tmp[c];
    }
}

// Nhân với 2 (tức là x) trong GF(2^8): dịch trái 1 bit,
// nếu bit cao nhất ban đầu là 1 thì XOR thêm 0x1b (đa thức x^8 + x^4 + x^3 + x + 1)
static uint8_t xtime(uint8_t x) {
    return static_cast<uint8_t>((x << 1) ^ ((x & 0x80) ? 0x1b : 0x00));
}

// Nhân 2 byte a * b trong GF(2^8) (phương pháp "nhân - cộng" dùng xtime)
static uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    while (b) {
        if (b & 1) result ^= a;  // cộng trong GF(2^8) chính là XOR
        a = xtime(a);
        b >>= 1;
    }
    return result;
}

// MixColumns: mỗi cột được nhân với ma trận cố định
//   | 02 03 01 01 |
//   | 01 02 03 01 |
//   | 01 01 02 03 |
//   | 03 01 01 02 |
static void mixColumns(State& s) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = s.a[0][c], a1 = s.a[1][c], a2 = s.a[2][c], a3 = s.a[3][c];
        s.a[0][c] = gmul(a0, 2) ^ gmul(a1, 3) ^ a2 ^ a3;
        s.a[1][c] = a0 ^ gmul(a1, 2) ^ gmul(a2, 3) ^ a3;
        s.a[2][c] = a0 ^ a1 ^ gmul(a2, 2) ^ gmul(a3, 3);
        s.a[3][c] = gmul(a0, 3) ^ a1 ^ a2 ^ gmul(a3, 2);
    }
}

// InvMixColumns: nhân với ma trận nghịch đảo
//   | 0e 0b 0d 09 |
//   | 09 0e 0b 0d |
//   | 0d 09 0e 0b |
//   | 0b 0d 09 0e |
static void invMixColumns(State& s) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = s.a[0][c], a1 = s.a[1][c], a2 = s.a[2][c], a3 = s.a[3][c];
        s.a[0][c] = gmul(a0, 0x0e) ^ gmul(a1, 0x0b) ^ gmul(a2, 0x0d) ^ gmul(a3, 0x09);
        s.a[1][c] = gmul(a0, 0x09) ^ gmul(a1, 0x0e) ^ gmul(a2, 0x0b) ^ gmul(a3, 0x0d);
        s.a[2][c] = gmul(a0, 0x0d) ^ gmul(a1, 0x09) ^ gmul(a2, 0x0e) ^ gmul(a3, 0x0b);
        s.a[3][c] = gmul(a0, 0x0b) ^ gmul(a1, 0x0d) ^ gmul(a2, 0x09) ^ gmul(a3, 0x0e);
    }
}

// AddRoundKey: XOR từng byte của State với khoá vòng
static void addRoundKey(State& s, const State& roundKey) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) s.a[r][c] ^= roundKey.a[r][c];
}

// =====================================================================
//  IN KẾT QUẢ
// =====================================================================

static string byteHex(uint8_t b) {
    const char* d = "0123456789abcdef";
    return string(1, d[b >> 4]) + d[b & 0xF];
}

static string wordHex(uint32_t w) {
    uint8_t b[4] = {static_cast<uint8_t>(w >> 24), static_cast<uint8_t>(w >> 16),
                    static_cast<uint8_t>(w >> 8), static_cast<uint8_t>(w)};
    return bytesToHex(b, 4);
}

// In nhiều ma trận State nằm cạnh nhau, mỗi ma trận có 1 tiêu đề,
// sau đó in lại mỗi trạng thái dưới dạng 1 dòng hex 32 ký tự (giống FIPS-197).
static void printStates(const vector<string>& labels, const vector<State>& states) {
    const string gap = "   ";
    vector<size_t> widths;
    for (const string& l : labels) widths.push_back(std::max<size_t>(11, l.size()));

    // Dòng tiêu đề
    string line = "  ";
    for (size_t i = 0; i < labels.size(); i++) {
        string cell = labels[i];
        cell.resize(widths[i], ' ');  // thêm dấu cách cho đủ độ rộng cột
        line += cell + gap;
    }
    cout << line.substr(0, line.find_last_not_of(' ') + 1) << "\n";  // bỏ dấu cách thừa cuối dòng

    // 4 hàng của các ma trận
    for (int r = 0; r < 4; r++) {
        line = "  ";
        for (size_t i = 0; i < states.size(); i++) {
            string cell;
            for (int c = 0; c < 4; c++) cell += byteHex(states[i].a[r][c]) + (c < 3 ? " " : "");
            cell.resize(widths[i], ' ');
            line += cell + gap;
        }
        cout << line.substr(0, line.find_last_not_of(' ') + 1) << "\n";
    }

    // Dạng 1 dòng hex
    for (size_t i = 0; i < states.size(); i++)
        cout << "    " << std::left << std::setw(14) << labels[i] << ": "
             << stateToHex(states[i]) << std::right << "\n";
}

static void printRoundHeader(const string& title) {
    cout << "\n";
    printLine('-');
    cout << " " << title << "\n";
    printLine('-');
}

// =====================================================================
//  MỞ RỘNG KHOÁ (Key Expansion)
//    w[0..3]  = 4 word đầu lấy thẳng từ khoá
//    w[i]     = w[i-4] XOR temp, trong đó:
//               temp = w[i-1]                                  nếu i không chia hết cho 4
//               temp = SubWord(RotWord(w[i-1])) XOR Rcon[i/4]  nếu i chia hết cho 4
// =====================================================================
static uint32_t rotWord(uint32_t w) {  // [a0 a1 a2 a3] -> [a1 a2 a3 a0]
    return (w << 8) | (w >> 24);
}

static uint32_t subWord(uint32_t w) {  // áp S-box cho từng byte của word
    uint32_t result = 0;
    for (int i = 3; i >= 0; i--) {
        uint8_t b = static_cast<uint8_t>(w >> (8 * i));
        result = (result << 8) | AES_SBOX[b];
    }
    return result;
}

void aesKeyExpansion(const uint8_t key[16], uint32_t w[44], bool verbose) {
    for (int i = 0; i < 4; i++) {
        w[i] = (static_cast<uint32_t>(key[4 * i]) << 24) | (static_cast<uint32_t>(key[4 * i + 1]) << 16) |
               (static_cast<uint32_t>(key[4 * i + 2]) << 8) | static_cast<uint32_t>(key[4 * i + 3]);
    }

    if (verbose) {
        printRoundHeader("KEY EXPANSION - mo rong khoa 128 bit thanh 44 word w[0..43]");
        for (int i = 0; i < 4; i++)
            cout << "  w[" << std::setw(2) << i << "] = " << wordHex(w[i]) << "   (lay truc tiep tu khoa)\n";
        cout << "\n   i  temp      RotWord   SubWord   Rcon      XOR Rcon  w[i-4]    w[i] = temp XOR w[i-4]\n";
    }

    for (int i = 4; i < 44; i++) {
        uint32_t temp = w[i - 1];
        if (i % 4 == 0) {
            uint32_t afterRot = rotWord(temp);
            uint32_t afterSub = subWord(afterRot);
            uint32_t rcon = AES_RCON[i / 4];
            uint32_t afterRcon = afterSub ^ rcon;
            w[i] = w[i - 4] ^ afterRcon;
            if (verbose) {
                cout << "  " << std::setw(2) << i << "  " << wordHex(temp) << "  " << wordHex(afterRot)
                     << "  " << wordHex(afterSub) << "  " << wordHex(rcon) << "  " << wordHex(afterRcon)
                     << "  " << wordHex(w[i - 4]) << "  " << wordHex(w[i]) << "\n";
            }
        } else {
            w[i] = w[i - 4] ^ temp;
            if (verbose) {
                cout << "  " << std::setw(2) << i << "  " << wordHex(temp) << string(42, ' ')
                     << wordHex(w[i - 4]) << "  " << wordHex(w[i]) << "\n";
            }
        }
    }

    if (verbose) {
        cout << "\n  Round keys (moi khoa vong = 4 word lien tiep):\n";
        for (int r = 0; r <= 10; r++) {
            cout << "  K" << std::left << std::setw(2) << r << std::right << " = w[" << std::setw(2) << 4 * r
                 << ".." << std::setw(2) << 4 * r + 3 << "] = " << wordHex(w[4 * r]) << wordHex(w[4 * r + 1])
                 << wordHex(w[4 * r + 2]) << wordHex(w[4 * r + 3]) << "\n";
        }
    }
}

// =====================================================================
//  MÃ HOÁ
// =====================================================================
void aesEncrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " AES-128 ENCRYPTION\n";
        printLine('=');
        cout << "  Key       = " << bytesToHex(key, 16) << "\n";
        cout << "  Plaintext = " << bytesToHex(in, 16) << "\n";
    }

    uint32_t w[44];
    aesKeyExpansion(key, w, verbose);

    State s = bytesToState(in);

    // ----- Vòng 0: chỉ AddRoundKey -----
    State input = s;
    State k0 = roundKeyToState(w, 0);
    addRoundKey(s, k0);
    if (verbose) {
        printRoundHeader("ROUND 0  (chi co AddRoundKey voi K0)");
        printStates({"Input", "RoundKey 0", "AddRoundKey"}, {input, k0, s});
    }

    // ----- Vòng 1..10 -----
    for (int round = 1; round <= 10; round++) {
        State start = s;
        subBytes(s);
        State afterSub = s;
        shiftRows(s);
        State afterShift = s;

        State afterMix = s;
        bool hasMix = (round != 10);  // vòng cuối không có MixColumns
        if (hasMix) {
            mixColumns(s);
            afterMix = s;
        }

        State k = roundKeyToState(w, round);
        addRoundKey(s, k);

        if (verbose) {
            string keyLabel = "RoundKey " + std::to_string(round);
            if (hasMix) {
                printRoundHeader("ROUND " + std::to_string(round));
                printStates({"Start", "SubBytes", "ShiftRows", "MixColumns", keyLabel, "AddRoundKey"},
                            {start, afterSub, afterShift, afterMix, k, s});
            } else {
                printRoundHeader("ROUND " + std::to_string(round) + "  (vong cuoi - KHONG co MixColumns)");
                printStates({"Start", "SubBytes", "ShiftRows", keyLabel, "AddRoundKey"},
                            {start, afterSub, afterShift, k, s});
            }
        }
    }

    stateToBytes(s, out);
    if (verbose) {
        printRoundHeader("OUTPUT");
        cout << "  CIPHERTEXT = " << bytesToHex(out, 16) << "\n";
    }
}

// =====================================================================
//  GIẢI MÃ
// =====================================================================
void aesDecrypt(const uint8_t in[16], const uint8_t key[16], uint8_t out[16], bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " AES-128 DECRYPTION  (dung khoa vong theo thu tu nguoc K10 -> K0)\n";
        printLine('=');
        cout << "  Key        = " << bytesToHex(key, 16) << "\n";
        cout << "  Ciphertext = " << bytesToHex(in, 16) << "\n";
    }

    uint32_t w[44];
    aesKeyExpansion(key, w, false);  // khoá vòng giống hệt lúc mã hoá

    State s = bytesToState(in);

    // ----- Vòng 0: AddRoundKey với K10 -----
    State input = s;
    State k10 = roundKeyToState(w, 10);
    addRoundKey(s, k10);
    if (verbose) {
        printRoundHeader("ROUND 0  (chi co AddRoundKey voi K10)");
        printStates({"Input", "RoundKey 10", "AddRoundKey"}, {input, k10, s});
    }

    // ----- Vòng 1..10 -----
    for (int round = 1; round <= 10; round++) {
        int keyIndex = 10 - round;  // vòng 1 dùng K9, ..., vòng 10 dùng K0
        State start = s;
        invShiftRows(s);
        State afterShift = s;
        invSubBytes(s);
        State afterSub = s;
        State k = roundKeyToState(w, keyIndex);
        addRoundKey(s, k);
        State afterAdd = s;

        bool hasMix = (round != 10);  // vòng cuối không có InvMixColumns
        if (hasMix) invMixColumns(s);

        if (verbose) {
            string keyLabel = "RoundKey " + std::to_string(keyIndex);
            if (hasMix) {
                printRoundHeader("ROUND " + std::to_string(round));
                printStates({"Start", "InvShiftRows", "InvSubBytes", keyLabel, "AddRoundKey", "InvMixColumns"},
                            {start, afterShift, afterSub, k, afterAdd, s});
            } else {
                printRoundHeader("ROUND " + std::to_string(round) + "  (vong cuoi - KHONG co InvMixColumns)");
                printStates({"Start", "InvShiftRows", "InvSubBytes", keyLabel, "AddRoundKey"},
                            {start, afterShift, afterSub, k, afterAdd});
            }
        }
    }

    stateToBytes(s, out);
    if (verbose) {
        printRoundHeader("OUTPUT");
        cout << "  PLAINTEXT = " << bytesToHex(out, 16) << "\n";
    }
}
