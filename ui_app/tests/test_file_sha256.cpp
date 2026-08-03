#include "file_sha256.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void Check(bool condition, const char* name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << "\n";
        ++g_failures;
    }
}

void WriteBytes(const fs::path& path, const std::string& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

int main() {
    const fs::path root = fs::temp_directory_path() / "cuda_fractal_file_sha256_e2";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    std::string digest;
    std::string error;
    const fs::path empty = root / "empty.bin";
    WriteBytes(empty, "");
    Check(ComputeFileSha256Hex(empty.string(), &digest, &error), "empty file hashes");
    Check(digest == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "empty digest exact");

    const fs::path abc = root / "abc.bin";
    WriteBytes(abc, "abc");
    Check(ComputeFileSha256Hex(abc.string(), &digest, &error), "abc file hashes");
    Check(digest == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "abc digest exact");

    digest = "not-cleared";
    Check(!ComputeFileSha256Hex((root / "missing.bin").string(), &digest, &error),
          "missing file rejected");
    Check(digest.empty(), "failed digest cleared");
    Check(!error.empty(), "missing file reports error");

    fs::remove_all(root, ignored);
    if (g_failures == 0) std::cout << "test_file_sha256: PASS\n";
    return g_failures == 0 ? 0 : 1;
}
