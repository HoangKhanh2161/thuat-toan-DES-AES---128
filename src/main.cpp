// =====================================================================
//  main.cpp - Chương trình chính: menu chọn chạy DES / AES-128
//
//  Cách chạy:
//    des_aes.exe                       -> hiện menu để chọn
//    des_aes.exe des                   -> chạy tất cả test vector DES (in mọi trạng thái)
//    des_aes.exe aes                   -> chạy tất cả test vector AES (in mọi trạng thái)
//    des_aes.exe check                 -> kiểm tra nhanh, chỉ in PASS/FAIL
//    des_aes.exe des <khoa> <ban_ro>   -> chạy DES với khoá, bản rõ tự nhập (hex)
//    des_aes.exe aes <khoa> <ban_ro>   -> chạy AES với khoá, bản rõ tự nhập (hex)
// =====================================================================
#include "aes.h"
#include "des.h"
#include "test_vectors.h"
#include "utils.h"

#include <cctype>
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::string;

// Đổi chuỗi hex về chữ HOA, bỏ dấu cách -> để so sánh 2 chuỗi hex
static string normalizeHex(const string& s) {
    string t = cleanHexInput(s);
    for (char& c : t) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return t;
}

// In phần tổng kết của 1 lần chạy, trả về true nếu mọi kiểm tra đều đúng
static bool printSummary(const string& algo, const string& name, const string& key, const string& plaintext,
                         const string& ciphertext, const string& expected, const string& decrypted) {
    bool encryptOk = expected.empty() || normalizeHex(ciphertext) == normalizeHex(expected);
    bool decryptOk = normalizeHex(decrypted) == normalizeHex(plaintext);

    cout << "\n";
    printLine('#');
    cout << " RESULT - " << algo << ": " << name << "\n";
    printLine('#');
    cout << "  Key                   : " << key << "\n";
    cout << "  Plaintext             : " << plaintext << "\n";
    cout << "  Ciphertext (tinh duoc): " << ciphertext << "\n";
    if (expected.empty()) {
        cout << "  Ciphertext mong doi   : (khong co - bo qua so sanh)\n";
    } else {
        cout << "  Ciphertext mong doi   : " << expected << "   -> "
             << (encryptOk ? "PASS (khop)" : "FAIL (KHONG khop!)") << "\n";
    }
    cout << "  Decrypt(Ciphertext)   : " << decrypted << "   -> "
         << (decryptOk ? "PASS (giai ma ra dung ban ro)" : "FAIL (giai ma SAI!)") << "\n";
    printLine('#');
    cout << "\n";
    return encryptOk && decryptOk;
}

// ---------------------------------------------------------------------
//  Chạy 1 test vector DES. verbose = in toàn bộ trạng thái trung gian.
// ---------------------------------------------------------------------
static bool runDes(const TestVector& tv, bool verbose) {
    uint64_t key, plaintext;
    if (!parseHex64(tv.key, key) || !parseHex64(tv.plaintext, plaintext)) {
        cout << "[LOI] DES can khoa va ban ro dung 16 ky tu hex. Kiem tra lai: " << tv.name << "\n";
        return false;
    }

    if (verbose) {
        cout << "\n\n";
        printLine('*');
        cout << " TEST VECTOR: " << tv.name << "\n";
        printLine('*');
        cout << "\n";
    }

    uint64_t ciphertext = desEncrypt(plaintext, key, verbose);
    if (verbose) cout << "\n\n";
    uint64_t decrypted = desDecrypt(ciphertext, key, verbose);

    string expected = tv.expected.empty() ? "" : normalizeHex(tv.expected);
    if (verbose) {
        return printSummary("DES", tv.name, toHex(key, 16), toHex(plaintext, 16), toHex(ciphertext, 16),
                            expected, toHex(decrypted, 16));
    }
    bool ok = (expected.empty() || toHex(ciphertext, 16) == expected) && decrypted == plaintext;
    cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << tv.name << "  -> ciphertext = " << toHex(ciphertext, 16)
         << "\n";
    return ok;
}

// ---------------------------------------------------------------------
//  Chạy 1 test vector AES-128.
// ---------------------------------------------------------------------
static bool runAes(const TestVector& tv, bool verbose) {
    uint8_t key[16], plaintext[16], ciphertext[16], decrypted[16];
    if (!parseHexBytes(tv.key, key, 16) || !parseHexBytes(tv.plaintext, plaintext, 16)) {
        cout << "[LOI] AES-128 can khoa va ban ro dung 32 ky tu hex. Kiem tra lai: " << tv.name << "\n";
        return false;
    }

    if (verbose) {
        cout << "\n\n";
        printLine('*');
        cout << " TEST VECTOR: " << tv.name << "\n";
        printLine('*');
        cout << "\n";
    }

    aesEncrypt(plaintext, key, ciphertext, verbose);
    if (verbose) cout << "\n\n";
    aesDecrypt(ciphertext, key, decrypted, verbose);

    string ctHex = bytesToHex(ciphertext, 16);
    string expected = tv.expected.empty() ? "" : normalizeHex(tv.expected);
    if (verbose) {
        return printSummary("AES-128", tv.name, bytesToHex(key, 16), bytesToHex(plaintext, 16), ctHex,
                            tv.expected.empty() ? "" : cleanHexInput(tv.expected), bytesToHex(decrypted, 16));
    }
    bool ok = (expected.empty() || normalizeHex(ctHex) == expected) &&
              bytesToHex(decrypted, 16) == bytesToHex(plaintext, 16);
    cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << tv.name << "  -> ciphertext = " << ctHex << "\n";
    return ok;
}

// Chạy tất cả test vector của 1 thuật toán (có in trạng thái)
static bool runAllDes() {
    bool allOk = true;
    for (const TestVector& tv : DES_TEST_VECTORS) allOk = runDes(tv, true) && allOk;
    return allOk;
}

static bool runAllAes() {
    bool allOk = true;
    for (const TestVector& tv : AES_TEST_VECTORS) allOk = runAes(tv, true) && allOk;
    return allOk;
}

// Kiểm tra nhanh: chỉ in PASS / FAIL cho từng test vector
static bool quickCheck() {
    bool allOk = true;
    printLine('=');
    cout << " KIEM TRA NHANH TAT CA TEST VECTOR\n";
    printLine('=');
    cout << " DES:\n";
    for (const TestVector& tv : DES_TEST_VECTORS) allOk = runDes(tv, false) && allOk;
    cout << " AES-128:\n";
    for (const TestVector& tv : AES_TEST_VECTORS) allOk = runAes(tv, false) && allOk;
    printLine('=');
    cout << (allOk ? " TAT CA DEU PASS" : " CO TEST BI FAIL!") << "\n";
    printLine('=');
    return allOk;
}

// =====================================================================
//  PHẦN MENU (dùng khi chạy chương trình không có tham số)
// =====================================================================

// Đọc 1 dòng từ bàn phím, trả về false nếu hết dữ liệu vào
static bool readLine(const string& prompt, string& line) {
    cout << prompt;
    cout.flush();
    if (!std::getline(std::cin, line)) return false;
    return true;
}

static void waitEnter() {
    string dummy;
    readLine("\nNhan Enter de quay lai menu...", dummy);
}

// Cho người dùng chọn 1 test vector trong danh sách. Trả về -1 nếu chọn "tất cả".
static int chooseVector(const std::vector<TestVector>& list) {
    cout << "\n  Danh sach test vector:\n";
    for (size_t i = 0; i < list.size(); i++) cout << "    " << (i + 1) << ". " << list[i].name << "\n";
    cout << "    0. Chay TAT CA\n";
    string line;
    while (readLine("  Chon so: ", line)) {
        try {
            int n = std::stoi(line);
            if (n == 0) return -1;
            if (n >= 1 && n <= static_cast<int>(list.size())) return n - 1;
        } catch (...) {
        }
        cout << "  Lua chon khong hop le, nhap lai.\n";
    }
    return -2;  // hết dữ liệu vào
}

// Cho người dùng tự nhập khoá, bản rõ, (và bản mã mong đợi nếu có)
static void customInput(bool isDes) {
    int hexLen = isDes ? 16 : 32;
    TestVector tv;
    tv.name = "Du lieu tu nhap";
    cout << "\n  (Nhap dang hex, " << hexLen << " ky tu, duoc phep co dau cach)\n";
    if (!readLine("  Khoa   : ", tv.key)) return;
    if (!readLine("  Ban ro : ", tv.plaintext)) return;
    if (!readLine("  Ban ma mong doi (Enter de bo qua): ", tv.expected)) return;
    if (isDes)
        runDes(tv, true);
    else
        runAes(tv, true);
}

static void menu() {
    string line;
    while (true) {
        cout << "\n";
        printLine('=', 62);
        cout << "      CHUONG TRINH DES & AES-128  (in tung trang thai)\n";
        printLine('=', 62);
        cout << "  1. DES     - chay test vector co san\n";
        cout << "  2. AES-128 - chay test vector co san\n";
        cout << "  3. DES     - tu nhap khoa + ban ro\n";
        cout << "  4. AES-128 - tu nhap khoa + ban ro\n";
        cout << "  5. Kiem tra nhanh tat ca test vector (chi in PASS/FAIL)\n";
        cout << "  0. Thoat\n";
        if (!readLine("Chon: ", line)) return;

        string choice = cleanHexInput(line);
        if (choice == "0") return;

        if (choice == "1" || choice == "2") {
            bool isDes = (choice == "1");
            const std::vector<TestVector>& list = isDes ? DES_TEST_VECTORS : AES_TEST_VECTORS;
            int idx = chooseVector(list);
            if (idx == -2) return;
            if (idx == -1) {
                if (isDes) runAllDes(); else runAllAes();
            } else {
                if (isDes) runDes(list[idx], true); else runAes(list[idx], true);
            }
        } else if (choice == "3") {
            customInput(true);
        } else if (choice == "4") {
            customInput(false);
        } else if (choice == "5") {
            quickCheck();
        } else {
            cout << "Lua chon khong hop le!\n";
            continue;
        }
        waitEnter();
    }
}

static void printUsage() {
    cout << "Cach dung:\n"
         << "  des_aes                      (hien menu)\n"
         << "  des_aes des                  (chay tat ca test vector DES)\n"
         << "  des_aes aes                  (chay tat ca test vector AES-128)\n"
         << "  des_aes check                (kiem tra nhanh PASS/FAIL)\n"
         << "  des_aes des <khoa> <ban_ro>  (DES voi du lieu tu nhap, hex 16 ky tu)\n"
         << "  des_aes aes <khoa> <ban_ro>  (AES voi du lieu tu nhap, hex 32 ky tu)\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        menu();
        return 0;
    }

    string mode = argv[1];
    if (mode == "des" && argc == 2) return runAllDes() ? 0 : 1;
    if (mode == "aes" && argc == 2) return runAllAes() ? 0 : 1;
    if (mode == "check" && argc == 2) return quickCheck() ? 0 : 1;

    if ((mode == "des" || mode == "aes") && (argc == 4 || argc == 5)) {
        TestVector tv{"Du lieu tu nhap (tham so dong lenh)", argv[2], argv[3], argc == 5 ? argv[4] : ""};
        bool ok = (mode == "des") ? runDes(tv, true) : runAes(tv, true);
        return ok ? 0 : 1;
    }

    printUsage();
    return 1;
}
