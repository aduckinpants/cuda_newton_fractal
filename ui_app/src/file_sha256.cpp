#include "file_sha256.h"

#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

bool Fail(const std::string& message, std::string* outError) {
    if (outError) *outError = message;
    return false;
}

class AlgorithmHandle {
public:
    ~AlgorithmHandle() {
        if (value_) BCryptCloseAlgorithmProvider(value_, 0);
    }
    BCRYPT_ALG_HANDLE* Out() { return &value_; }
    BCRYPT_ALG_HANDLE Get() const { return value_; }
private:
    BCRYPT_ALG_HANDLE value_{nullptr};
};

class HashHandle {
public:
    ~HashHandle() {
        if (value_) BCryptDestroyHash(value_);
    }
    BCRYPT_HASH_HANDLE* Out() { return &value_; }
    BCRYPT_HASH_HANDLE Get() const { return value_; }
private:
    BCRYPT_HASH_HANDLE value_{nullptr};
};

} // namespace

bool ComputeFileSha256Hex(
    const std::string& path,
    std::string* outHex,
    std::string* outError) {
    if (outHex) outHex->clear();
    if (outError) outError->clear();
    if (!outHex) return Fail("SHA-256 output pointer is required", outError);

    std::ifstream input(path, std::ios::binary);
    if (!input) return Fail("Failed to open file for SHA-256: " + path, outError);

    AlgorithmHandle algorithm;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        algorithm.Out(), BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) return Fail("BCryptOpenAlgorithmProvider(SHA-256) failed", outError);

    DWORD objectLength = 0;
    DWORD bytesWritten = 0;
    status = BCryptGetProperty(
        algorithm.Get(), BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength), &bytesWritten, 0);
    if (!BCRYPT_SUCCESS(status) || objectLength == 0) {
        return Fail("BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed", outError);
    }

    DWORD digestLength = 0;
    status = BCryptGetProperty(
        algorithm.Get(), BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&digestLength), sizeof(digestLength), &bytesWritten, 0);
    if (!BCRYPT_SUCCESS(status) || digestLength != 32) {
        return Fail("BCryptGetProperty(BCRYPT_HASH_LENGTH) returned an invalid SHA-256 size", outError);
    }

    std::vector<UCHAR> object(objectLength);
    HashHandle hash;
    status = BCryptCreateHash(
        algorithm.Get(), hash.Out(), object.data(), objectLength, nullptr, 0, 0);
    if (!BCRYPT_SUCCESS(status)) return Fail("BCryptCreateHash(SHA-256) failed", outError);

    std::array<char, 64 * 1024> buffer{};
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize count = input.gcount();
        if (count > 0) {
            status = BCryptHashData(
                hash.Get(), reinterpret_cast<PUCHAR>(buffer.data()), static_cast<ULONG>(count), 0);
            if (!BCRYPT_SUCCESS(status)) return Fail("BCryptHashData(SHA-256) failed", outError);
        }
    }
    if (!input.eof()) return Fail("Failed while reading file for SHA-256: " + path, outError);

    std::array<UCHAR, 32> digest{};
    status = BCryptFinishHash(hash.Get(), digest.data(), static_cast<ULONG>(digest.size()), 0);
    if (!BCRYPT_SUCCESS(status)) return Fail("BCryptFinishHash(SHA-256) failed", outError);

    std::ostringstream hex;
    hex << std::hex << std::setfill('0');
    for (const UCHAR byte : digest) hex << std::setw(2) << static_cast<unsigned int>(byte);
    *outHex = hex.str();
    return true;
}
