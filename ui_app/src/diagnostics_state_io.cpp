#include "diagnostics_state_io.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"
#include "json_min.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

bool ReadTextFile(const std::filesystem::path& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open state file: " + path.string();
        return false;
    }

    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read state file: " + path.string();
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool GetRequiredObject(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_object()) {
        if (outError) *outError = std::string("Missing or invalid object field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredArray(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_array()) {
        if (outError) *outError = std::string("Missing or invalid array field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool GetRequiredBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_bool()) {
        if (outError) *outError = std::string("Missing or invalid bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    return true;
}

bool GetRequiredString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) *outError = std::string("Missing or invalid string field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool TryGetOptionalString(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) return false;
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetOptionalNumber(const json_min::Value& object, const char* key, double* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid optional number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    if (outPresent) *outPresent = true;
    return true;
}

bool GetOptionalBool(const json_min::Value& object, const char* key, bool* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_bool()) {
        if (outError) *outError = std::string("Invalid optional bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    if (outPresent) *outPresent = true;
    return true;
}

bool ParseFractalType(const std::string& text, FractalType* outType) {
    if (text == "newton") { if (outType) *outType = FractalType::newton; return true; }
    if (text == "nova") { if (outType) *outType = FractalType::nova; return true; }
    if (text == "mandelbrot") { if (outType) *outType = FractalType::mandelbrot; return true; }
    if (text == "julia") { if (outType) *outType = FractalType::julia; return true; }
    if (text == "burning_ship") { if (outType) *outType = FractalType::burning_ship; return true; }
    if (text == "multibrot") { if (outType) *outType = FractalType::multibrot; return true; }
    if (text == "phoenix") { if (outType) *outType = FractalType::phoenix; return true; }
    if (text == "explaino") { if (outType) *outType = FractalType::explaino; return true; }
    if (text == "explaino_y") { if (outType) *outType = FractalType::explaino_y; return true; }
    if (text == "explaino_fp") { if (outType) *outType = FractalType::explaino_fp; return true; }
    if (text == "explaino_nova") { if (outType) *outType = FractalType::explaino_nova; return true; }
    if (text == "explaino_halley") { if (outType) *outType = FractalType::explaino_halley; return true; }
    if (text == "explaino_dual") { if (outType) *outType = FractalType::explaino_dual; return true; }
    if (text == "explaino_mult") { if (outType) *outType = FractalType::explaino_mult; return true; }
    if (text == "explaino_phoenix") { if (outType) *outType = FractalType::explaino_phoenix; return true; }
    if (text == "explaino_transcendental") { if (outType) *outType = FractalType::explaino_transcendental; return true; }
    if (text == "explaino_inertial") { if (outType) *outType = FractalType::explaino_inertial; return true; }
    return false;
}

bool ParsePolyKind(int rawValue, PolyKind* outKind) {
    if (rawValue == static_cast<int>(PolyKind::z3_minus_1)) { if (outKind) *outKind = PolyKind::z3_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::z4_minus_1)) { if (outKind) *outKind = PolyKind::z4_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::custom)) { if (outKind) *outKind = PolyKind::custom; return true; }
    return false;
}

bool ParseColoringMode(const std::string& text, ColoringMode* outMode) {
    if (text == "root_basin") { if (outMode) *outMode = ColoringMode::root_basin; return true; }
    if (text == "iteration_count") { if (outMode) *outMode = ColoringMode::iteration_count; return true; }
    if (text == "smooth_escape") { if (outMode) *outMode = ColoringMode::smooth_escape; return true; }
    if (text == "joy_basins") { if (outMode) *outMode = ColoringMode::joy_basins; return true; }
    return false;
}

bool ParseIntField(const json_min::Value& object, const char* key, int* outValue, std::string* outError) {
    double rawValue = 0.0;
    if (!GetRequiredNumber(object, key, &rawValue, outError)) return false;
    if (rawValue < static_cast<double>(INT_MIN) || rawValue > static_cast<double>(INT_MAX)) {
        if (outError) *outError = std::string("Out-of-range integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(rawValue);
    return true;
}

} // namespace

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    if (outError) outError->clear();
    if (!ioView || !ioParams || !ioRender) {
        if (outError) *outError = "LoadDiagnosticsStateJson requires non-null output pointers";
        return false;
    }

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "State JSON root must be an object";
        return false;
    }

    const json_min::Value& root = parseResult.value;
    int stateVersion = 0;
    if (!ParseIntField(root, "state_version", &stateVersion, outError)) return false;
    if (stateVersion != 1 && stateVersion != 2 && stateVersion != 3) {
        if (outError) *outError = "Unsupported state_version: " + std::to_string(stateVersion);
        return false;
    }

    std::string fractalTypeId;
    if (!GetRequiredString(root, "fractal_type", &fractalTypeId, outError)) return false;

    ViewState nextView = *ioView;
    KernelParams nextParams = *ioParams;
    RenderSettings nextRender = *ioRender;

    if (!ParseFractalType(fractalTypeId, &nextView.fractal_type)) {
        if (outError) *outError = "Unknown fractal_type: " + fractalTypeId;
        return false;
    }

    const json_min::Value* viewObject = nullptr;
    const json_min::Value* paramsObject = nullptr;
    const json_min::Value* renderObject = nullptr;
    if (!GetRequiredObject(root, "view", &viewObject, outError)) return false;
    if (!GetRequiredObject(root, "params", &paramsObject, outError)) return false;
    if (!GetRequiredObject(root, "render", &renderObject, outError)) return false;

    double centerX = 0.0;
    double centerY = 0.0;
    double zoom = 0.0;
    double rotationDegrees = 0.0;
    double centerHpX = 0.0;
    double centerHpY = 0.0;
    double log2Zoom = 0.0;
    double explainoPhase = 0.0;
    double explainoSeedDrift = 0.0;
    bool explainoSeedTween = true;
    bool autoIncrementSeed = nextView.auto_increment_seed;
    double explainoSeedRate = nextView.explaino_seed_rate;
    if (!GetRequiredNumber(*viewObject, "center_x", &centerX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_y", &centerY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "zoom", &zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "rotation_degrees", &rotationDegrees, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_x", &centerHpX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_y", &centerHpY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "log2_zoom", &log2Zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_phase", &explainoPhase, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_seed_drift", &explainoSeedDrift, outError)) return false;
    if (!GetRequiredBool(*viewObject, "explaino_seed_tween", &explainoSeedTween, outError)) return false;
    if (!GetOptionalBool(*viewObject, "auto_increment_seed", &autoIncrementSeed, nullptr, outError)) return false;
    if (!GetOptionalNumber(*viewObject, "explaino_seed_rate", &explainoSeedRate, nullptr, outError)) return false;

    nextView.center.x = static_cast<float>(centerX);
    nextView.center.y = static_cast<float>(centerY);
    nextView.zoom = static_cast<float>(zoom);
    nextView.rotation_degrees = static_cast<float>(rotationDegrees);
    nextView.center_hp_x = centerHpX;
    nextView.center_hp_y = centerHpY;
    nextView.log2_zoom = log2Zoom;
    nextView.explaino_phase = static_cast<float>(explainoPhase);
    nextView.explaino_seed_drift = static_cast<float>(explainoSeedDrift);
    nextView.explaino_seed_tween = explainoSeedTween;
    nextView.auto_increment_seed = autoIncrementSeed;
    nextView.explaino_seed_rate = static_cast<float>(explainoSeedRate);

    int maxIter = 0;
    int rawPolyKind = 0;
    double epsilon = 0.0;
    double exposure = 0.0;
    std::string coloringModeId;
    double novaAlpha = 0.0;
    double phoenixPReal = 0.0;
    double phoenixPImag = 0.0;
    int multibrotPower = 0;
    double explainoSeed = 0.0;
    double explainoSeedB = nextParams.explaino_seed_b;
    double explainoMix = nextParams.explaino_mix;
    double explainoWarpStrength = 0.0;
    int explainoRootCount = 0;
    const json_min::Value* polyCoeffsArray = nullptr;
    if (!ParseIntField(*paramsObject, "max_iter", &maxIter, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "epsilon", &epsilon, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "exposure", &exposure, outError)) return false;
    if (!ParseIntField(*paramsObject, "poly_kind", &rawPolyKind, outError)) return false;
    if (stateVersion >= 2) {
        if (!GetRequiredString(*paramsObject, "coloring_mode", &coloringModeId, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "nova_alpha", &novaAlpha, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_real", &phoenixPReal, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_imag", &phoenixPImag, outError)) return false;
        if (!ParseIntField(*paramsObject, "multibrot_power", &multibrotPower, outError)) return false;
    }
    if (!GetRequiredNumber(*paramsObject, "explaino_seed", &explainoSeed, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_seed_b", &explainoSeedB, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_mix", &explainoMix, nullptr, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "explaino_warp_strength", &explainoWarpStrength, outError)) return false;
    if (!ParseIntField(*paramsObject, "explaino_root_count", &explainoRootCount, outError)) return false;
    if (!GetRequiredArray(*paramsObject, "poly_coeffs", &polyCoeffsArray, outError)) return false;
    if (polyCoeffsArray->as_array().size() != 5) {
        if (outError) *outError = "poly_coeffs must contain exactly 5 numbers";
        return false;
    }

    if (!ParsePolyKind(rawPolyKind, &nextParams.poly_kind)) {
        if (outError) *outError = "Unknown poly_kind: " + std::to_string(rawPolyKind);
        return false;
    }
    if (IsExplainoFamily(nextView.fractal_type) && nextParams.poly_kind != PolyKind::custom) {
        if (outError) *outError = "poly_kind must be custom for fractal_type " + fractalTypeId;
        return false;
    }
    if (stateVersion >= 2) {
        if (!ParseColoringMode(coloringModeId, &nextParams.coloring_mode)) {
            if (outError) *outError = "Unknown coloring_mode: " + coloringModeId;
            return false;
        }
        nextParams.nova_alpha = static_cast<float>(novaAlpha);
        nextParams.phoenix_p_real = static_cast<float>(phoenixPReal);
        nextParams.phoenix_p_imag = static_cast<float>(phoenixPImag);
        nextParams.multibrot_power = multibrotPower;
    } else {
        nextParams.coloring_mode = DefaultColoringModeForFractal(nextView.fractal_type);
    }
    if (!IsColoringModeAllowedForFractal(nextView.fractal_type, nextParams.coloring_mode)) {
        if (outError) {
            std::string coloringModeName = "unknown";
            switch (nextParams.coloring_mode) {
            case ColoringMode::root_basin: coloringModeName = "root_basin"; break;
            case ColoringMode::iteration_count: coloringModeName = "iteration_count"; break;
            case ColoringMode::smooth_escape: coloringModeName = "smooth_escape"; break;
            case ColoringMode::joy_basins: coloringModeName = "joy_basins"; break;
            }
            *outError = "coloring_mode " + coloringModeName + " is not allowed for fractal_type " + fractalTypeId;
        }
        return false;
    }
    nextParams.max_iter = maxIter;
    nextParams.epsilon = static_cast<float>(epsilon);
    nextParams.exposure = static_cast<float>(exposure);
    nextParams.explaino_seed = explainoSeed;
    nextParams.explaino_seed_b = explainoSeedB;
    nextParams.explaino_mix = static_cast<float>(explainoMix);
    nextParams.explaino_warp_strength = static_cast<float>(explainoWarpStrength);
    nextParams.explaino_root_count = explainoRootCount;
    if (IsExplainoFamily(nextView.fractal_type)) {
        ExplainoSeedNormalize(nextView, nextParams);
    }

    // Color grading (v3+, optional for backward compat)
    double colorSaturation = nextParams.color_saturation;
    double colorContrast = nextParams.color_contrast;
    double colorTintR = nextParams.color_tint_r;
    double colorTintG = nextParams.color_tint_g;
    double colorTintB = nextParams.color_tint_b;
    if (!GetOptionalNumber(*paramsObject, "color_saturation", &colorSaturation, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast", &colorContrast, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_r", &colorTintR, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_g", &colorTintG, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_b", &colorTintB, nullptr, outError)) return false;
    nextParams.color_saturation = static_cast<float>(colorSaturation);
    nextParams.color_contrast = static_cast<float>(colorContrast);
    nextParams.color_tint_r = static_cast<float>(colorTintR);
    nextParams.color_tint_g = static_cast<float>(colorTintG);
    nextParams.color_tint_b = static_cast<float>(colorTintB);

    // explaino_cluster_radius (optional for backward compat)
    double explainoClusterRadius = nextParams.explaino_cluster_radius;
    if (!GetOptionalNumber(*paramsObject, "explaino_cluster_radius", &explainoClusterRadius, nullptr, outError)) return false;
    nextParams.explaino_cluster_radius = static_cast<float>(explainoClusterRadius);

    // transcendental_func (optional for backward compat)
    {
        std::string tfStr;
        const json_min::Value* tfVal = paramsObject->get("transcendental_func");
        if (tfVal && tfVal->is_string()) {
            tfStr = tfVal->as_string();
            if (tfStr == "f_sin") nextParams.transcendental_func = TranscendentalFunc::f_sin;
            else if (tfStr == "f_exp_minus_1") nextParams.transcendental_func = TranscendentalFunc::f_exp_minus_1;
            else if (tfStr == "f_cosh") nextParams.transcendental_func = TranscendentalFunc::f_cosh;
        }
    }

    // momentum_beta (optional for backward compat)
    {
        const json_min::Value* mbVal = paramsObject->get("momentum_beta");
        if (mbVal && mbVal->is_number()) {
            nextParams.momentum_beta = static_cast<float>(mbVal->as_number());
        }
    }

    for (size_t index = 0; index < 5; ++index) {
        const json_min::Value& coeff = polyCoeffsArray->as_array()[index];
        if (!coeff.is_number()) {
            if (outError) *outError = "poly_coeffs must contain only numbers";
            return false;
        }
        nextParams.poly_coeffs[index] = static_cast<float>(coeff.as_number());
    }

    int width = 0;
    int height = 0;
    int blockSize = 0;
    int deviceId = 0;
    if (!ParseIntField(*renderObject, "width", &width, outError)) return false;
    if (!ParseIntField(*renderObject, "height", &height, outError)) return false;
    if (!ParseIntField(*renderObject, "block_size", &blockSize, outError)) return false;
    if (!ParseIntField(*renderObject, "device_id", &deviceId, outError)) return false;
    nextRender.resolution.x = width;
    nextRender.resolution.y = height;
    nextRender.block_size = blockSize;
    nextRender.device_id = deviceId;

    *ioView = nextView;
    *ioParams = nextParams;
    *ioRender = nextRender;
    return true;
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    if (outError) outError->clear();
    std::string text;
    if (!ReadTextFile(std::filesystem::path(path), &text, outError)) return false;
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, outError);
}

bool ResolveFindingStateJsonPath(const std::string& selectedPath,
    std::string* outStateJsonPath,
    std::string* outError) {
    if (outError) outError->clear();

    const std::filesystem::path inputPath = std::filesystem::path(selectedPath).lexically_normal();
    if (!std::filesystem::exists(inputPath)) {
        if (outError) *outError = "Selected path does not exist: " + inputPath.string();
        return false;
    }

    if (inputPath.filename() == "state.json") {
        if (outStateJsonPath) *outStateJsonPath = inputPath.string();
        return true;
    }

    if (inputPath.filename() != "finding.json") {
        if (outError) *outError = "Select either state.json or finding.json";
        return false;
    }

    std::string text;
    if (!ReadTextFile(inputPath, &text, outError)) return false;

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "Finding metadata must be a JSON object";
        return false;
    }

    std::string stateFileName;
    if (!GetRequiredString(parseResult.value, "state_file", &stateFileName, outError)) return false;
    const std::filesystem::path statePath = inputPath.parent_path() / stateFileName;
    if (!std::filesystem::exists(statePath)) {
        if (outError) *outError = "Finding metadata points to missing state file: " + statePath.string();
        return false;
    }
    if (outStateJsonPath) *outStateJsonPath = statePath.string();
    return true;
}