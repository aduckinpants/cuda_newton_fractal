#pragma once

#include <string>

// Computes SHA-256 over the exact file bytes and returns lowercase hexadecimal.
bool ComputeFileSha256Hex(
    const std::string& path,
    std::string* outHex,
    std::string* outError);
