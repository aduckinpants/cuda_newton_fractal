#pragma once

#include "fractal_catalog.h"

#include <cstddef>
#include <string>

struct FractalDescriptionSentence {
    const char* claim_id;
    const char* text;
};

struct FractalDescriptionField {
    const char* field_name;
    const FractalDescriptionSentence* sentences;
    std::size_t sentence_count;
};

struct FractalReviewedDescription {
    FractalType fractal_type;
    const char* selector_id;
    const FractalDescriptionField* fields;
    std::size_t field_count;
    const char* const* source_refs;
    std::size_t source_ref_count;
};

std::size_t FractalReviewedDescriptionCount();

const FractalReviewedDescription& FractalReviewedDescriptionAt(std::size_t index);

const FractalReviewedDescription* FindFractalReviewedDescription(FractalType fractalType);

// Deterministic UTF-8 JSON with a single trailing newline.
std::string BuildFractalDescriptiveCatalogJson();

// Writes through the established same-directory .tmp replacement convention.
// Parent directories are never created. On failure, only the owned .tmp path is cleaned.
bool WriteFractalDescriptiveCatalogJsonFile(
    const std::string& path,
    std::string* outError);
