#include "finding_capture_state.h"

#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"

bool PrepareFindingCaptureRuntimeState(ViewState& view, KernelParams& params) {
    bool invalidateCaches = false;
    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, &invalidateCaches);
    }
    if (view.auto_max_iter) {
        const int nextMaxIter = ComputeAutoMaxIter(view.log2_zoom, view.fractal_type);
        if (params.max_iter != nextMaxIter) {
            params.max_iter = nextMaxIter;
            invalidateCaches = true;
        }
    }
    return invalidateCaches;
}