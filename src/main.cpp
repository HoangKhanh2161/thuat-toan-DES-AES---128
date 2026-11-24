// main.cpp - Chương trình chính
//   des_aes                  : hiện menu
//   des_aes export           : ghi kết quả mọi test vector vào thư mục results/
//   des_aes check            : chỉ in PASS / FAIL
//   des_aes des KHOA BAN_RO  : chạy DES với dữ liệu tự nhập (hex)
//   des_aes aes KHOA BAN_RO  : chạy AES-128 với dữ liệu tự nhập (hex)
#include "aes.h"
#include "des.h"
#include "test_vectors.h"
#include "utils.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Chữ hoa + bỏ dấu cách, để so sánh 2 chuỗi hex
static string normalize(const string& s) {
    string t = cleanHexInput(s);
    for (char& c : t) c = toupper((unsigned char)c);
    return t;
}

// So sánh với bản mã mong đợi + kiểm tra giải mã, rồi in kết quả
static bool finish(const TestVector& tv, const string& key, const string& plain, const string& cipher,
                   const string& decrypted, bool verbose) {
    bool encOk = tv.expected == "" || normalize(cipher) == normalize(tv.expected);
    bool decOk = normalize(decrypted) == normalize(plain);

    if (!verbose) {  // chỉ in 1 dòng
        cout << "[" << (encOk && decOk ? "PASS" : "FAIL") << "]  " << cipher << "  " << tv.name << "\n";
        return encOk && decOk;
    }

    cout << "\n";
    printLine('#');
    cout << " KET QUA: " << tv.name << "\n";
    printLine('#');
    cout << "  Key        : " << key << "\n";
    cout << "  Plaintext  : " << plain << "\n";
    cout << "  Ciphertext : " << cipher << "\n";
    if (tv.expected != "")
        cout << "  Expected   : " << cleanHexInput(tv.expected) << "  -> " << (encOk ? "PASS" : "FAIL") << "\n";
    cout << "  Decrypt    : " << decrypted << "  -> " << (decOk ? "PASS" : "FAIL") << "\n";
    printLine('#');
    return encOk && decOk;
}

static void printVectorTitle(const string& name) {
    printLine('*');
    cout << " TEST VECTOR: " << name << "\n";
    printLine('*');
}

// Chạy 1 test vector DES (mã hoá rồi giải mã lại)
static bool runDes(const TestVector& tv, bool verbose) {
    uint64_t key, plain;
    if (!parseHex64(tv.key, key) || !parseHex64(tv.plaintext, plain)) {
        cout << "Loi: DES can khoa va ban ro dung 16 ky tu hex (" << tv.name << ")\n";
        return false;
    }
    if (verbose) printVectorTitle(tv.name);

    uint64_t cipher = desEncrypt(plain, key, verbose);
    if (verbose) cout << "\n\n";
    uint64_t decrypted = desDecrypt(cipher, key, verbose);

    return finish(tv, toHex(key, 16), toHex(plain, 16), toHex(cipher, 16), toHex(decrypted, 16), verbose);
}

// Chạy 1 test vector AES-128 (mã hoá rồi giải mã lại)
static bool runAes(const TestVector& tv, bool verbose) {
    uint8_t key[16], plain[16], cipher[16], decrypted[16];
    if (!parseHexBytes(tv.key, key, 16) || !parseHexBytes(tv.plaintext, plain, 16)) {
        cout << "Loi: AES-128 can khoa va ban ro dung 32 ky tu hex (" << tv.name << ")\n";
        return false;
    }
    if (verbose) printVectorTitle(tv.name);

    aesEncrypt(plain, key, cipher, verbose);
    if (verbose) cout << "\n\n";
    aesDecrypt(cipher, key, decrypted, verbose);

    return finish(tv, bytesToHex(key, 16), bytesToHex(plain, 16), bytesToHex(cipher, 16), bytesToHex(decrypted, 16),
                  verbose);
}

// Kiểm tra nhanh tất cả test vector (mỗi vector 1 dòng)
static bool quickCheck() {
    bool allOk = true;
    printLine('=');
    cout << " KIEM TRA NHANH\n";
    printLine('=');
    cout << " DES:\n";
    for (size_t i = 0; i < DES_TEST_VECTORS.size(); i++) {
        cout << "  DES_" << i + 1 << ".txt  ";
        if (!runDes(DES_TEST_VECTORS[i], false)) allOk = false;
    }
    cout << "\n AES-128:\n";
    for (size_t i = 0; i < AES_TEST_VECTORS.size(); i++) {
        cout << "  AES_" << i + 1 << ".txt  ";
        if (!runAes(AES_TEST_VECTORS[i], false)) allOk = false;
    }
    printLine('=');
    cout << (allOk ? " TAT CA DEU PASS" : " CO TEST BI FAIL") << "\n";
    printLine('=');
    return allOk;
}

// Chạy 1 test vector, ghi output vào file (tạm chuyển cout sang file)
static bool runToFile(const string& path, bool isDes, const TestVector& tv) {
    ofstream file(path);
    if (!file) {
        cout << "Loi: khong tao duoc file " << path << " (thu muc results/ co ton tai khong?)\n";
        return false;
    }
    streambuf* screen = cout.rdbuf(file.rdbuf());  // cout -> file
    bool ok = isDes ? runDes(tv, true) : runAes(tv, true);
    cout.rdbuf(screen);                            // cout -> màn hình
    cout << "  Da ghi " << path << "\n";
    return ok;
}

// Ghi kết quả từng test vector: results/DES_1.txt, ..., results/AES_1.txt, ..., results/summary.txt
static bool exportResults() {
    bool allOk = true;
    for (size_t i = 0; i < DES_TEST_VECTORS.size(); i++)
        if (!runToFile("results/DES_" + to_string(i + 1) + ".txt", true, DES_TEST_VECTORS[i])) allOk = false;
    for (size_t i = 0; i < AES_TEST_VECTORS.size(); i++)
        if (!runToFile("results/AES_" + to_string(i + 1) + ".txt", false, AES_TEST_VECTORS[i])) allOk = false;

    ofstream summary("results/summary.txt");
    streambuf* screen = cout.rdbuf(summary.rdbuf());
    quickCheck();
    cout.rdbuf(screen);
    cout << "  Da ghi results/summary.txt\n\n";

    quickCheck();  // in bảng ra màn hình
    return allOk;
}

// ---------------- Menu ----------------

static bool readLine(const string& prompt, string& line) {
    cout << prompt;
    return (bool)getline(cin, line);
}

// Chọn 1 test vector trong danh sách. Trả về chỉ số, -1 = tất cả, -2 = không đọc được
static int chooseVector(const vector<TestVector>& list) {
    cout << "\n";
    for (size_t i = 0; i < list.size(); i++) cout << "  " << i + 1 << ". " << list[i].name << "\n";
    cout << "  0. Tat ca\n";
    string line;
    while (readLine("Chon: ", line)) {
        int n = -1;
        try {
            n = stoi(line);
        } catch (...) {
        }
        if (n == 0) return -1;
        if (n >= 1 && n <= (int)list.size()) return n - 1;
        cout << "Khong hop le, chon lai.\n";
    }
    return -2;
}

// Tự nhập khoá, bản rõ (và bản mã mong đợi nếu có)
static void customInput(bool isDes) {
    TestVector tv;
    tv.name = "Du lieu tu nhap";
    cout << "\nNhap hex " << (isDes ? 16 : 32) << " ky tu (co the co dau cach)\n";
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
        printLine('=', 44);
        cout << "  DES & AES-128\n";
        printLine('=', 44);
        cout << "  1. DES - chay test vector co san\n";
        cout << "  2. AES - chay test vector co san\n";
        cout << "  3. DES - tu nhap khoa, ban ro\n";
        cout << "  4. AES - tu nhap khoa, ban ro\n";
        cout << "  5. Kiem tra nhanh (PASS/FAIL)\n";
        cout << "  6. Ghi ket qua vao thu muc results/\n";
        cout << "  0. Thoat\n";
        if (!readLine("Chon: ", line)) return;
        string c = cleanHexInput(line);

        if (c == "0") return;
        if (c == "1" || c == "2") {
            bool isDes = (c == "1");
            const vector<TestVector>& list = isDes ? DES_TEST_VECTORS : AES_TEST_VECTORS;
            int idx = chooseVector(list);
            if (idx == -2) return;
            for (size_t i = 0; i < list.size(); i++) {
                if (idx != -1 && (int)i != idx) continue;
                if (isDes)
                    runDes(list[i], true);
                else
                    runAes(list[i], true);
            }
        } else if (c == "3" || c == "4") {
            customInput(c == "3");
        } else if (c == "5") {
            quickCheck();
        } else if (c == "6") {
            exportResults();
        } else {
            cout << "Khong hop le!\n";
            continue;
        }
        if (!readLine("\nNhan Enter de quay lai menu...", line)) return;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        menu();
        return 0;
    }

    string mode = argv[1];
    if (mode == "export") return exportResults() ? 0 : 1;
    if (mode == "check") return quickCheck() ? 0 : 1;
    if ((mode == "des" || mode == "aes") && (argc == 4 || argc == 5)) {
        TestVector tv = {"Du lieu tu nhap", argv[2], argv[3], argc == 5 ? argv[4] : ""};
        bool ok = (mode == "des") ? runDes(tv, true) : runAes(tv, true);
        return ok ? 0 : 1;
    }

    cout << "Cach dung:\n"
         << "  des_aes                    (menu)\n"
         << "  des_aes export             (ghi ket qua vao thu muc results/)\n"
         << "  des_aes check              (chi in PASS/FAIL)\n"
         << "  des_aes des KHOA BAN_RO    (DES, hex 16 ky tu)\n"
         << "  des_aes aes KHOA BAN_RO    (AES-128, hex 32 ky tu)\n";
    return 1;
}
