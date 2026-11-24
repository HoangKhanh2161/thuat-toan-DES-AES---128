// test_vectors.h - Danh sách test vector
//   Thêm ví dụ mới: thêm 1 dòng {"Ten", "KHOA", "BAN RO", "BAN MA MONG DOI"},
//   - Chưa biết bản mã thì để "" ở cột cuối.
//   - DES: khoá và bản rõ 16 ký tự hex.   AES-128: khoá và bản rõ 32 ký tự hex.
#ifndef TEST_VECTORS_H
#define TEST_VECTORS_H

#include <string>
#include <vector>

struct TestVector {
    std::string name;       // tên ví dụ
    std::string key;        // khoá (hex)
    std::string plaintext;  // bản rõ (hex)
    std::string expected;   // bản mã mong đợi (hex)
};

// ------------------------------- DES ---------------------------------
static const std::vector<TestVector> DES_TEST_VECTORS = {
    // Ví dụ kinh điển trong bài "The DES Algorithm Illustrated" (J. Orlin Grabbe)
    {"DES vi du kinh dien (J. Orlin Grabbe)",
     "133457799BBCDFF1", "0123456789ABCDEF", "85E813540F0AB405"},

    // Ví dụ trong giáo trình Stallings - Cryptography and Network Security
    {"DES vi du giao trinh Stallings",
     "0F1571C947D9E859", "02468ACEECA86420", "DA02CE3A89ECAC3B"},

    // Khối đầu tiên của câu "Now is the time for all " (FIPS 81)
    {"DES 'Now is t' (FIPS 81)",
     "0123456789ABCDEF", "4E6F772069732074", "3FA40E8A984D4815"},

    // Ví dụ cho ra bản mã toàn số 0
    {"DES ban ma toan so 0",
     "0E329232EA6D0D73", "8787878787878787", "0000000000000000"},
};

// ------------------------------ AES-128 ------------------------------
static const std::vector<TestVector> AES_TEST_VECTORS = {
    // Ví dụ trong chuẩn FIPS-197, Appendix B (Cipher Example)
    {"AES FIPS-197 Appendix B",
     "2b7e151628aed2a6abf7158809cf4f3c", "3243f6a8885a308d313198a2e0370734",
     "3925841d02dc09fbdc118597196a0b32"},

    // Ví dụ trong chuẩn FIPS-197, Appendix C.1 (AES-128)
    {"AES FIPS-197 Appendix C.1",
     "000102030405060708090a0b0c0d0e0f", "00112233445566778899aabbccddeeff",
     "69c4e0d86a7b0430d8cdb78070b4c55a"},

    // Ví dụ trong giáo trình Stallings - Cryptography and Network Security
    {"AES vi du giao trinh Stallings",
     "0f1571c947d9e8590cb7add6af7f6798", "0123456789abcdeffedcba9876543210",
     "ff0b844a0853bf7c6934ab4364148fb9"},

    // Khoá "Thats my Kung Fu", bản rõ "Two One Nine Two" (ví dụ minh hoạ phổ biến)
    {"AES 'Two One Nine Two' / 'Thats my Kung Fu'",
     "5468617473206d79204b756e67204675", "54776f204f6e65204e696e652054776f",
     "29c3505f571420f6402299b31a02d73a"},
};

#endif
