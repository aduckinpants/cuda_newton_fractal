# Lens And Flashlight Writeup

## Current Phase

Phase 14 - Hostile-review the landed flashlight slice, fix at least one real gap if found, and checkpoint the recovery

## Phase Checklist

- [x] Phase 1 - Trace runtime lens SDF implementation and call path
- [x] Phase 2 - Correct the earlier flashlight misidentification and trace the real `FlashlightProbe` runtime
- [x] Phase 3 - Write the first evidence-backed flashlight review note
- [x] Phase 4 - Extend the slice into a full-feature report covering bridge interaction, drift/coherence steering intent, regional hypotheses, and external reusable-engine composition
- [x] Phase 5 - Verify plan/doc sync and diff hygiene
- [x] Phase 6 - Trace the published runtime regression to the active non-flashlight build line
- [x] Phase 7 - Prove the flashlight-bearing branch lineage inside this repo and map the split from the current viewer-host line
- [x] Phase 8 - Run or stage a branch-isolated flashlight-capable runtime to verify whether fresh bridge/probe/STL artifacts are still reproducible
- [x] Phase 9 - Land a forensic note that explains the split, current failure mode, and next reintegration options
- [x] Phase 10 - Add branch donor audit outputs and a seam-level recovery matrix covering every non-merged branch/ref reviewed
- [x] Phase 11 - Recover headless flashlight CLI/core/report seams in the current architecture without importing legacy schema/UI debt
- [x] Phase 12 - Harden build/runtime prerequisites so flashlight recovery is reproducible on the current machine and published runtime
- [x] Phase 13 - Add regression coverage and runtime validation rails for the recovered headless flashlight path
- [x] Phase 14 - Hostile-review the landed flashlight slice, fix at least one real gap if found, and checkpoint the recovery

## Notes

- User request in the expanded thread:
  - document the full flashlight feature, not just the easy surface
  - cover the original orientation/drift idea, the bridge interaction, and the compositional external-tool framing
- Writable work stays in this repo.
- `C:\code\salticid-cuda` is treated as read-only implementation evidence while other agent work is active there.
- Current report surfaces for this thread:
  - [lens_and_flashlight_writeup.md](/C:/code/cuda_newton_fractal_clone/docs/notes/lens_and_flashlight_writeup.md)
  - [flashlight_probe_full_feature_report.md](/C:/code/cuda_newton_fractal_clone/docs/notes/flashlight_probe_full_feature_report.md)

## Seams Reviewed For The Full Report

- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\main.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\flashlight_probe_init.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\flashlight_tuning_cli.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\conversation_seed_spectrum.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf_chamfer.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\flashlight_bridge_state.h`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\flashlight_bridge_prefill_ingest.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\flashlight_view_prefill.cpp`
- `ui_app/src/explaino_sidecar_model.h`
- `ui_app/src/explaino_sidecar_divergence.cpp`
- `docs/callable_engine_surface.md`
- `docs/callable_engine_transport_session_cheatsheet.md`

## Findings To Preserve

- `FlashlightProbe` is real, explicitly named, and uses the runtime lens/SDF path as part of its measured signal.
- The runtime lens SDF and the flashlight probe are coupled; the old sidecar-only interpretation was wrong.
- There is a real bridge surface:
  - writes `flashlight_bridge_request.json`
  - optionally consumes `flashlight_bridge_prefill.json`
  - supports one-shot `pendingAutoEmit`
  - can apply view overrides and seed modulation for headless probe runs
- The callable engine/session transport already provides the right compositional external-control pattern for a future reusable flashlight engine/tool.
- The sidecar orientation/divergence machinery is not flashlight itself, but it is the closest existing in-repo formalization of orientation drift, divergence, and machine-readable steering-adjacent signals.

## Verification

- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `git diff --check`
- `cmd /c ui_app\\build_tests_vsdevcmd.cmd`
- `cmd /c ui_app\\build_vsdevcmd.cmd`
- `py -3.14 tools\\viewer_host_runtime_pytest_lane.py`

## Provenance And Regression Notes

- The active deployment path `D:\salt-fractal\cuda_newton_fractal_clone\runtime` is published from this repo's current viewer-host line via `ui_app/build_vsdevcmd.cmd`.
- The flashlight-bearing lineage is still present in this repo history as `origin/feature/seed-refactor` and the local object graph includes the flashlight commits and artifacts.
- The current bug is therefore not "flashlight never lived here"; it is that the published runtime follows a different descendant line from the same root commit and never merged the flashlight branch.
- Recovery run update:
  - a branch-isolated seed-refactor rebuild using a working 2022 Build Tools environment and an added `sm_86` CUDA target successfully regenerated `flashlight_probe.json`, `frame.bmp`, `lens_sdf.bmp`, and `state.json`
  - the historical `flashlight_trace.stl` and `flashlight_trace_overlay.bmp` did not regenerate, which narrows the remaining blocker to the external bridge/watcher export seam rather than the headless probe itself
- Current implementation stance for the recovery slice:
  - donor branches are reference surfaces, not merge targets
  - the current schema remains authoritative
  - only the headless flashlight core will be recovered in the first code slice
  - bridge/live/export recovery remains a separate follow-on after the donor audit proves where those seams actually live
- Recovery/audit completion notes:
  - landed `docs/notes/flashlight_donor_branch_audit.md` with the branch inventory and recovery matrix
  - recovered headless `--flashlight-probe` into current `viewer_cli` / `main.cpp` / `flashlight_probe.*` seams
  - hardened MSVC toolchain discovery through `tools/call_vsdevcmd.cmd` and reused it from the public build scripts
  - added pure helper coverage plus runtime regression coverage for fresh flashlight artifact regeneration
  - hostile review found and fixed a real headless-verb routing bug: `--describe-functions` previously won over `--flashlight-probe` instead of erroring as a conflict
