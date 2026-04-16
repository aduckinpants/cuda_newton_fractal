# FlashlightProbe Review And Extraction Plan

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - Re-audit the real `flashlight` implementation seam and correct the earlier misidentification
- [x] Phase 2 - Trace how runtime lens/SDF participates in `FlashlightProbe` and collect current vs. historical evidence
- [x] Phase 3 - Write the replacement review covering original intent, coastline behavior, current viability, and separate external-tool extraction plan
- [x] Phase 4 - Verify plan/doc sync and basic diff hygiene

## Notes

- Writable work stays in this repo.
- `C:\code\salticid-cuda` is read-only implementation evidence for the current flashlight runtime.
- The earlier draft was wrong because it mapped "flashlight" to the Explaino sidecar lens/controller stack.
- The replacement doc keeps the existing path `docs/notes/lens_and_flashlight_writeup.md` so downstream references do not break, but the content now documents the real `FlashlightProbe` feature.

## Evidence Summary

- Code seams reviewed in mainline:
  - `ide_ui_dx11/ui_app/src/main.cpp`
  - `ide_ui_dx11/ui_app/src/flashlight_probe_init.cpp`
  - `ide_ui_dx11/ui_app/src/flashlight_tuning_cli.cpp`
  - `ide_ui_dx11/ui_app/src/conversation_seed_spectrum.cpp`
  - `ide_ui_dx11/ui_app/src/lens_sdf.cpp`
  - `ide_ui_dx11/ui_app/src/lens_sdf_chamfer.cpp`
- Architecture/context reviewed in this repo:
  - `spec_intake/CliBridgeV2_GpuSampleFn_SpecIntake.md`
  - `ui_app/tests/test_fractal_sample_pipeline.cpp`
- Historical / artifact evidence reviewed:
  - `C:\Users\Adam\Desktop\cuda_newton_fractal\ui\diagnostics\last\flashlight_probe.json`
  - `C:\Users\Adam\Downloads\flashlight_trace.stl`
  - `C:\Users\Adam\Downloads\flashlight_trace_frame.bmp`
  - `C:\Users\Adam\Downloads\flashlight_trace_overlay.bmp`
  - `C:\Users\Adam\Downloads\suspect_forensic_manifest\diffs\directives__Probe_-_FlashlightProbe_(CUDA_Runtime_Sampling).md.diff`

## Current Findings

- `FlashlightProbe` is real, explicitly named, and uses the runtime lens/SDF as part of the probe signal.
- The strongest defensible interpretation is geometric drift/coherence probing over an Explaino working map, not literal latent-space access.
- The coastline / island complexity behavior should be treated as a real observed property from the original work.
- Current mainline still contains the manifold, render, lens, and live-trace machinery, but present-day headless artifact emission could not be fully re-proven from the obvious diagnostics path in this slice.

## Verification

- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `git diff --check`
