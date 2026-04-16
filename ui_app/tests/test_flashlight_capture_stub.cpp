#include "diagnostics_capture.h"

#include <string>

bool CaptureDiagnosticsLastBundle(const std::string&,
    const ViewState&,
    const KernelParams&,
    const RenderSettings&,
    const RenderStats&,
    const uint32_t*,
    std::size_t,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    if (outResult) {
        outResult->output_dir = ".";
        outResult->frame_bmp_path = ".\\frame.bmp";
        outResult->state_json_path = ".\\state.json";
    }
    if (outError) outError->clear();
    return true;
}
