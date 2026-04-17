#pragma once

#include "fractal_types.h"
#include "runtime_walk.h"

#include <map>
#include <string>
#include <vector>

struct RuntimeWalkFitsMappingBinding {
    std::string target_selector;
    std::string source_signal;
    std::string target_path;
    double input_min = 0.0;
    double input_max = 1.0;
    double scale = 1.0;
    double offset = 0.0;
    double weight = 1.0;
    bool enabled = true;
    bool has_clamp = false;
    double clamp_min = 0.0;
    double clamp_max = 0.0;
};

struct RuntimeWalkFitsMappingProfile {
    std::string id;
    std::string target_selector;
    FractalType base_fractal_type = FractalType::explaino_fp;
    std::vector<RuntimeWalkFitsMappingBinding> bindings;
};

struct RuntimeWalkFitsMappingCatalog {
    std::vector<RuntimeWalkFitsMappingProfile> profiles;
};

struct RuntimeWalkFitsOrientationInputs {
    std::string fits_path;
    std::map<std::string, double> signals;
};

struct RuntimeWalkTransportSynthesisOptions {
    std::size_t sample_count = 33;
    double motion_scale = 0.75;
    double warp_scale = 0.10;
};

bool ParseRuntimeWalkFitsMappingCatalogJson(const std::string& jsonText,
    RuntimeWalkFitsMappingCatalog* outCatalog,
    std::string* outError);

bool LoadRuntimeWalkFitsMappingCatalogFile(const std::string& path,
    RuntimeWalkFitsMappingCatalog* outCatalog,
    std::string* outError);

bool ParseRuntimeWalkFitsOrientationInputsJson(const std::string& jsonText,
    RuntimeWalkFitsOrientationInputs* outInputs,
    std::string* outError);

bool LoadRuntimeWalkFitsOrientationInputsFile(const std::string& path,
    RuntimeWalkFitsOrientationInputs* outInputs,
    std::string* outError);

bool ResolveDefaultRuntimeWalkFitsMappingProfilePath(const std::string& exeDir,
    std::string* outPath,
    std::string* outError);

bool ExtractRuntimeWalkFitsOrientationInputs(const std::string& exeDir,
    const std::string& fitsPath,
    const std::string& outJsonPath,
    std::string* outError);

bool SynthesizeRuntimeWalkBaseState(const RuntimeWalkFitsMappingCatalog& catalog,
    const std::string& profileId,
    const RuntimeWalkFitsOrientationInputs& inputs,
    ViewState* outView,
    KernelParams* outParams,
    RenderSettings* outRender,
    std::string* outError);

bool SynthesizeRuntimeWalkTransportBundle(const RuntimeWalkFitsMappingCatalog& catalog,
    const std::string& profileId,
    const RuntimeWalkFitsOrientationInputs& inputs,
    const RuntimeWalkTransportSynthesisOptions& options,
    RuntimeWalkBundle* outBundle,
    std::string* outError);

bool WriteRuntimeWalkBundleJsonFile(const std::string& path,
    const RuntimeWalkBundle& bundle,
    std::string* outError);

bool WriteRuntimeWalkSynthesizedStateJson(const std::string& path,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::string* outError);
