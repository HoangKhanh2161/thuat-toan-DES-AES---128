// des.cpp - Cài đặt DES
//   Mã hoá : IP -> 16 vòng -> đổi chỗ (R16 L16) -> IP^-1
//   Mỗi vòng: L_i = R_(i-1)
//             R_i = L_(i-1) XOR f(R_(i-1), K_i)
//   Hàm f  : E (32 -> 48 bit) -> XOR khoá con -> 8 hộp S (48 -> 32 bit) -> P
//   Giải mã: giống mã hoá nhưng dùng khoá con ngược lại K16 -> K1
#include "des.h"
#include "des_tables.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Hoán vị bit theo bảng: bit thứ i của kết quả = bit thứ table[i] của đầu vào
static uint64_t permute(uint64_t in, const int* table, int outBits, int inBits) {
    uint64_t out = 0;
    for (int i = 0; i < outBits; i++) {
        uint64_t bit = (in >> (inBits - table[i])) & 1;
        out = (out << 1) | bit;
    }
    return out;
}

// Dịch vòng trái số 28 bit đi n bit
static uint32_t rotateLeft28(uint32_t x, int n) {
    return ((x << n) | (x >> (28 - n))) & 0x0FFFFFFF;
}

// In 1 dòng:  tên = hex = nhị phân
static void show(const string& name, const string& hex, const string& bin = "") {
    cout << "  " << left << setw(20) << name << "= " << setw(16) << hex;
    if (bin != "") cout << " = " << bin;
    cout << right << "\n";
}

static void title(const string& text) {
    cout << "\n";
    printLine('-');
    cout << " " << text << "\n";
    printLine('-');
}

// ---------------- Sinh 16 khoá con ----------------
void desGenerateSubkeys(uint64_t key, uint64_t subkeys[16], bool verbose) {
    uint64_t kPlus = permute(key, PC1, 56, 64);  // PC-1: 64 -> 56 bit
    uint32_t C = (kPlus >> 28) & 0x0FFFFFFF;     // 28 bit trái
    uint32_t D = kPlus & 0x0FFFFFFF;             // 28 bit phải

    if (verbose) {
        title("KEY SCHEDULE");
        show("Key K", toHex(key, 16), toBin(key, 64, 8));
        show("PC-1(K)", toHex(kPlus, 14), toBin(kPlus, 56, 7));
        show("C0", toHex(C, 7), toBin(C, 28, 7));
        show("D0", toHex(D, 7), toBin(D, 28, 7));
        cout << "\n  Round Shift  C_i      D_i      K_i = PC-2(C_i D_i)\n";
    }

    for (int i = 0; i < 16; i++) {
        C = rotateLeft28(C, SHIFTS[i]);
        D = rotateLeft28(D, SHIFTS[i]);
        uint64_t CD = ((uint64_t)C << 28) | D;
        subkeys[i] = permute(CD, PC2, 48, 56);  // PC-2: 56 -> 48 bit

        if (verbose) {
            cout << "  " << setw(5) << i + 1 << setw(6) << SHIFTS[i] << "  " << toHex(C, 7) << "  "
                 << toHex(D, 7) << "  K" << left << setw(2) << i + 1 << right << " = "
                 << toHex(subkeys[i], 12) << " | " << toBin(subkeys[i], 48, 6) << "\n";
        }
    }
}

// ---------------- Hàm f(R, K) ----------------
static uint32_t f(uint32_t R, uint64_t K, int round, int keyNo, bool verbose) {
    string r = "R" + to_string(round - 1);
    string k = "K" + to_string(keyNo);

    uint64_t e = permute(R, E, 48, 32);  // 1. mở rộng E: 32 -> 48 bit
    uint64_t x = e ^ K;                  // 2. XOR với khoá con

    if (verbose) {
        show("E(" + r + ")", toHex(e, 12), toBin(e, 48, 6));
        show(k, toHex(K, 12), toBin(K, 48, 6));
        show("E(" + r + ") XOR " + k, toHex(x, 12), toBin(x, 48, 6));
        cout << "  S-box (hang = bit 1,6; cot = bit 2..5):\n";
    }

    uint32_t s = 0;  // 3. qua 8 hộp S, mỗi hộp: 6 bit vào -> 4 bit ra
    for (int i = 0; i < 8; i++) {
        int six = (x >> (42 - 6 * i)) & 0x3F;    // nhóm 6 bit thứ i
        int row = ((six >> 4) & 2) | (six & 1);  // bit đầu + bit cuối
        int col = (six >> 1) & 0xF;              // 4 bit giữa
        int val = SBOX[i][row][col];
        s = (s << 4) | val;

        if (verbose) {
            cout << "     S" << i + 1 << ": " << toBin(six, 6, 0) << " -> hang " << row << ", cot "
                 << setw(2) << col << " -> " << setw(2) << val << " = " << toBin(val, 4, 0) << "\n";
        }
    }

    uint32_t p = permute(s, P, 32, 32);  // 4. hoán vị P

    if (verbose) {
        show("S-box output", toHex(s, 8), toBin(s, 32, 4));
        show("f = P(S-box output)", toHex(p, 8), toBin(p, 32, 4));
    }
    return p;
}

// ---------------- Phần chung của mã hoá và giải mã ----------------
static uint64_t desProcess(uint64_t input, const uint64_t subkeys[16], bool decrypt, bool verbose) {
    uint64_t ip = permute(input, IP, 64, 64);  // hoán vị khởi đầu
    uint32_t L = ip >> 32;
    uint32_t R = ip & 0xFFFFFFFF;

    if (verbose) {
        title("INITIAL PERMUTATION (IP)");
        show("Input", toHex(input, 16), toBin(input, 64, 8));
        show("IP(Input)", toHex(ip, 16), toBin(ip, 64, 8));
        show("L0", toHex(L, 8), toBin(L, 32, 4));
        show("R0", toHex(R, 8), toBin(R, 32, 4));
    }

    for (int round = 1; round <= 16; round++) {
        int keyIndex = decrypt ? 16 - round : round - 1;  // giải mã: dùng khoá ngược
        if (verbose) title("ROUND " + to_string(round) + "  (khoa K" + to_string(keyIndex + 1) + ")");

        uint32_t fOut = f(R, subkeys[keyIndex], round, keyIndex + 1, verbose);
        uint32_t newL = R;
        uint32_t newR = L ^ fOut;

        if (verbose) {
            string i = to_string(round), p = to_string(round - 1);
            show("L" + i + " = R" + p, toHex(newL, 8), toBin(newL, 32, 4));
            show("R" + i + " = L" + p + " XOR f", toHex(newR, 8), toBin(newR, 32, 4));
        }
        L = newL;
        R = newR;
    }

    uint64_t swapped = ((uint64_t)R << 32) | L;            // đổi chỗ: R16 L16
    uint64_t output = permute(swapped, IP_INV, 64, 64);  // hoán vị kết thúc

    if (verbose) {
        title("FINAL PERMUTATION (IP^-1)");
        show("R16 L16", toHex(swapped, 16), toBin(swapped, 64, 8));
        show("Output = IP^-1", toHex(output, 16), toBin(output, 64, 8));
    }
    return output;
}

uint64_t desEncrypt(uint64_t plaintext, uint64_t key, bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " DES - MA HOA\n";
        printLine('=');
        show("Key", toHex(key, 16));
        show("Plaintext", toHex(plaintext, 16));
    }
    uint64_t subkeys[16];
    desGenerateSubkeys(key, subkeys, verbose);
    return desProcess(plaintext, subkeys, false, verbose);
}

uint64_t desDecrypt(uint64_t ciphertext, uint64_t key, bool verbose) {
    if (verbose) {
        printLine('=');
        cout << " DES - GIAI MA (khoa con dung nguoc K16 -> K1)\n";
        printLine('=');
        show("Key", toHex(key, 16));
        show("Ciphertext", toHex(ciphertext, 16));
    }
    uint64_t subkeys[16];
    desGenerateSubkeys(key, subkeys, false);
    return desProcess(ciphertext, subkeys, true, verbose);
}
