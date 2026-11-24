// =====================================================================
//  des.cpp - Cài đặt thuật toán DES
//
//  Sơ đồ tổng quát (mã hoá):
//
//    Bản rõ 64 bit
//         |
//        IP                      (hoán vị khởi đầu)
//         |
//    L0 (32 bit) | R0 (32 bit)
//         |
//    Lặp 16 vòng (i = 1..16):
//         L_i = R_(i-1)
//         R_i = L_(i-1) XOR f(R_(i-1), K_i)
//         |
//    Đổi chỗ: R16 L16
//         |
//       IP^-1                    (hoán vị kết thúc)
//         |
//    Bản mã 64 bit
//
//  Hàm f(R, K):
//    1. Mở rộng E:   R (32 bit) -> 48 bit
//    2. XOR với khoá con K_i (48 bit)
//    3. Chia thành 8 nhóm 6 bit, mỗi nhóm qua 1 hộp S -> 4 bit  (tổng 32 bit)
//    4. Hoán vị P:   32 bit -> 32 bit
// =====================================================================
#include "des.h"
#include "des_tables.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <string>

using std::cout;
using std::string;

// ---------------------------------------------------------------------
// Hàm hoán vị bit tổng quát.
//   input   : số cần hoán vị, có inBits bit (nằm ở các bit thấp)
//   table   : bảng hoán vị, table[i] = vị trí bit nguồn (đánh số từ 1, tính từ trái)
//   outBits : số bit của kết quả
// Ví dụ với IP: bit thứ 1 của kết quả = bit thứ 58 của input, ...
// ---------------------------------------------------------------------
static uint64_t permute(uint64_t input, const int* table, int outBits, int inBits) {
    uint64_t output = 0;
    for (int i = 0; i < outBits; i++) {
        int sourcePos = table[i];                               // 1..inBits
        uint64_t bit = (input >> (inBits - sourcePos)) & 1ULL;  // lấy bit đó ra
        output = (output << 1) | bit;                           // ghép vào kết quả
    }
    return output;
}

// Dịch vòng trái một số 28 bit đi n bit (dùng cho C_i, D_i khi sinh khoá).
static uint32_t rotateLeft28(uint32_t x, int n) {
    return ((x << n) | (x >> (28 - n))) & 0x0FFFFFFF;
}

// In 1 dòng dạng:   "  <nhãn>  = <hex>   = <nhị phân>"
static void printRow(const string& label, const string& hex, const string& bin = "") {
    cout << "  " << std::left << std::setw(20) << label << "= " << std::setw(16) << hex;
    if (!bin.empty()) cout << " = " << bin;
    cout << std::right << "\n";
}

// =====================================================================
//  SINH KHOÁ CON
//    K (64 bit) --PC-1--> K+ (56 bit) = C0 (28 bit) | D0 (28 bit)
//    Vòng i: C_i = C_(i-1) dịch vòng trái SHIFTS[i] bit, D_i tương tự
//            K_i = PC-2(C_i D_i)   (48 bit)
// =====================================================================
void desGenerateSubkeys(uint64_t key, uint64_t subkeys[16], bool verbose) {
    uint64_t kPlus = permute(key, PC1, 56, 64);
    uint32_t C = static_cast<uint32_t>(kPlus >> 28) & 0x0FFFFFFF;  // 28 bit trái
    uint32_t D = static_cast<uint32_t>(kPlus) & 0x0FFFFFFF;        // 28 bit phải

    if (verbose) {
        cout << "\n";
        printLine('-');
        cout << " KEY SCHEDULE - sinh 16 khoa con K1..K16\n";
        printLine('-');
        printRow("Key K (64 bit)", toHex(key, 16), toBin(key, 64, 8));
        printRow("K+ = PC-1(K)", toHex(kPlus, 14), toBin(kPlus, 56, 7));
        printRow("C0", toHex(C, 7), toBin(C, 28, 7));
        printRow("D0", toHex(D, 7), toBin(D, 28, 7));
        cout << "\n  Round Shift  C_i      D_i      K_i = PC-2(C_i D_i)  (hex | binary 48 bit)\n";
    }

    for (int i = 0; i < 16; i++) {
        C = rotateLeft28(C, SHIFTS[i]);
        D = rotateLeft28(D, SHIFTS[i]);
        uint64_t CD = (static_cast<uint64_t>(C) << 28) | D;  // ghép lại 56 bit
        subkeys[i] = permute(CD, PC2, 48, 56);

        if (verbose) {
            cout << "  " << std::setw(5) << (i + 1) << std::setw(6) << SHIFTS[i] << "  "
                 << toHex(C, 7) << "  " << toHex(D, 7) << "  K" << std::left << std::setw(2)
                 << (i + 1) << std::right << " = " << toHex(subkeys[i], 12) << " | "
                 << toBin(subkeys[i], 48, 6) << "\n";
        }
    }
}

// =====================================================================
//  HÀM f(R, K) - trái tim của DES
// =====================================================================
static uint32_t feistelF(uint32_t R, uint64_t K, int round, int keyNumber, bool verbose) {
    string rName = "R" + std::to_string(round - 1);
    string kName = "K" + std::to_string(keyNumber);

    // Bước 1: mở rộng R từ 32 bit lên 48 bit bằng bảng E
    uint64_t expanded = permute(R, E, 48, 32);

    // Bước 2: XOR với khoá con
    uint64_t x = expanded ^ K;

    if (verbose) {
        printRow("E(" + rName + ")", toHex(expanded, 12), toBin(expanded, 48, 6));
        printRow(kName, toHex(K, 12), toBin(K, 48, 6));
        printRow("E(" + rName + ") XOR " + kName, toHex(x, 12), toBin(x, 48, 6));
        cout << "  S-boxes (input 6 bit -> row = bit1 bit6, col = bit2..bit5):\n";
    }

    // Bước 3: 8 hộp S, mỗi hộp nhận 6 bit, trả ra 4 bit
    uint32_t sOutput = 0;
    for (int i = 0; i < 8; i++) {
        int six = static_cast<int>((x >> (42 - 6 * i)) & 0x3F);  // nhóm 6 bit thứ i (tính từ trái)
        int row = ((six >> 4) & 0x2) | (six & 0x1);              // bit đầu + bit cuối
        int col = (six >> 1) & 0xF;                              // 4 bit giữa
        int value = SBOX[i][row][col];
        sOutput = (sOutput << 4) | static_cast<uint32_t>(value);

        if (verbose) {
            cout << "     S" << (i + 1) << ": " << toBin(six, 6, 0) << " -> row " << row
                 << ", col " << std::setw(2) << col << " -> " << std::setw(2) << value
                 << " = " << toBin(value, 4, 0) << "\n";
        }
    }

    // Bước 4: hoán vị P
    uint32_t f = static_cast<uint32_t>(permute(sOutput, P, 32, 32));

    if (verbose) {
        printRow("S-box output", toHex(sOutput, 8), toBin(sOutput, 32, 4));
        printRow("f = P(S-box output)", toHex(f, 8), toBin(f, 32, 4));
    }
    return f;
}

// =====================================================================
//  Phần chung của mã hoá và giải mã.
//  Giải mã DES = mã hoá nhưng dùng khoá con theo thứ tự ngược (K16 -> K1).
// =====================================================================
static uint64_t desProcess(uint64_t input, const uint64_t subkeys[16], bool decrypt, bool verbose) {
    // Hoán vị khởi đầu IP và chia đôi
    uint64_t ip = permute(input, IP, 64, 64);
    uint32_t L = static_cast<uint32_t>(ip >> 32);
    uint32_t R = static_cast<uint32_t>(ip & 0xFFFFFFFF);

    if (verbose) {
        cout << "\n";
        printLine('-');
        cout << " INITIAL PERMUTATION (IP)\n";
        printLine('-');
        printRow(decrypt ? "Input C (ciphertext)" : "Input M (plaintext)", toHex(input, 16),
                 toBin(input, 64, 8));
        printRow("IP(input)", toHex(ip, 16), toBin(ip, 64, 8));
        printRow("L0", toHex(L, 8), toBin(L, 32, 4));
        printRow("R0", toHex(R, 8), toBin(R, 32, 4));
    }

    // 16 vòng Feistel
    for (int round = 1; round <= 16; round++) {
        int keyIndex = decrypt ? (16 - round) : (round - 1);  // chỉ số trong mảng subkeys
        int keyNumber = keyIndex + 1;                        // tên khoá: K1..K16

        if (verbose) {
            cout << "\n";
            printLine('-');
            cout << " ROUND " << round << "   (subkey K" << keyNumber << ")\n";
            printLine('-');
        }

        uint32_t f = feistelF(R, subkeys[keyIndex], round, keyNumber, verbose);
        uint32_t newL = R;      // L_i = R_(i-1)
        uint32_t newR = L ^ f;  // R_i = L_(i-1) XOR f(R_(i-1), K_i)

        if (verbose) {
            string i = std::to_string(round);
            string p = std::to_string(round - 1);
            printRow("L" + i + " = R" + p, toHex(newL, 8), toBin(newL, 32, 4));
            printRow("R" + i + " = L" + p + " XOR f", toHex(newR, 8), toBin(newR, 32, 4));
        }
        L = newL;
        R = newR;
    }

    // Sau vòng 16: đổi chỗ thành R16 L16 rồi hoán vị IP^-1
    uint64_t preOutput = (static_cast<uint64_t>(R) << 32) | L;
    uint64_t output = permute(preOutput, IP_INV, 64, 64);

    if (verbose) {
        cout << "\n";
        printLine('-');
        cout << " FINAL PERMUTATION (IP^-1)\n";
        printLine('-');
        printRow("R16 L16 (swap)", toHex(preOutput, 16), toBin(preOutput, 64, 8));
        printRow(decrypt ? "IP^-1 -> PLAINTEXT" : "IP^-1 -> CIPHERTEXT", toHex(output, 16),
                 toBin(output, 64, 8));
    }
    return output;
}

uint64_t desEncrypt(uint64_t plaintext, uint64_t key, bool verbose) {
    uint64_t subkeys[16];
    if (verbose) {
        printLine('=');
        cout << " DES ENCRYPTION\n";
        printLine('=');
        printRow("Key", toHex(key, 16));
        printRow("Plaintext", toHex(plaintext, 16));
    }
    desGenerateSubkeys(key, subkeys, verbose);
    return desProcess(plaintext, subkeys, false, verbose);
}

uint64_t desDecrypt(uint64_t ciphertext, uint64_t key, bool verbose) {
    uint64_t subkeys[16];
    if (verbose) {
        printLine('=');
        cout << " DES DECRYPTION  (dung khoa con theo thu tu nguoc K16 -> K1)\n";
        printLine('=');
        printRow("Key", toHex(key, 16));
        printRow("Ciphertext", toHex(ciphertext, 16));
    }
    desGenerateSubkeys(key, subkeys, false);  // khoá con giống hệt lúc mã hoá
    return desProcess(ciphertext, subkeys, true, verbose);
}
