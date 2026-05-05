# UI Polish Slice 1 - Schema Domains And Control Polish

## Current Phase

Phase 4 complete - duplicate-schema cleanup and Warp domain repair validated on the published runtime

## Phase Checklist

- [x] Phase 1 - wait for Phase 0 closeout and capture the schema-domain baseline
- [x] Phase 2 - define the target domains, widget hints, and grouping fixes in the schema/binding seam
- [x] Phase 3 - implement the schema, binding, and focused test updates
- [x] Phase 4 - hostile audit the resulting UI-domain behavior and checkpoint the slice

## Explicit User Asks

- [done] Fix slider values that are not covering the proper domains.
- [done] Treat the UI polish as a bounded slice instead of a broad refactor.
- [done] Re-audit the shipped control bindings in detail and correct the remaining misses instead of stopping at partial coverage.
- [done] Remove the stale duplicate schema JSON entirely from the runtime/deploy surface and the remaining repo references.
- [done] Fix Warp so the shipped UI, shared runtime validation, and deployed runtime all agree on the real supported domain.

## Presumption Loop

The most likely owner is the schema metadata and binding layer, not a pile of one-off ImGui widget overrides in `main.cpp`. The slice should start from the existing schema authority and only step into renderer-facing code if focused tests prove the metadata surface cannot express the required domains.

Hostile review assumes the current static ranges are too weak or too generic for several controls. Write the narrowest regression that proves a wrong domain or missing widget hint before broadening the schema surface.

## Presumption Evidence

- Owner Proof: current UI review and the live render path show domain/range behavior is centralized in `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/ui_schema.h`, `ui_app/src/ui_schema.cpp`, and `ui_app/src/schema_binding.cpp`.
- RED Witness: the current render path passes the schema's single `min`/`max` pair directly into both slider and drag controls, so drag controls cannot distinguish a hard engine clamp from a UI-only suggested range.
- RED Witness: focused headless regressions now pin the missing metadata and range-resolution seam in `ui_app/tests/test_ui_schema.cpp` and `ui_app/tests/test_schema_binding.cpp`.
- Workflow Proof: this slice changes user-facing viewer controls, so helper-only validation is insufficient; the user-tested build is the published runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, which means closure requires runtime publish and deployed-runtime proof in addition to the headless seams.
- Fix Proof: `UISchemaControl` now parses `ui_min`/`ui_max`, `ResolveNumericControlRange(...)` separates widget hints from hard clamps, float/double/int render paths add direct numeric entry with post-edit hard clamping, the special Explaino seed path uses the same clamp/ID seam, and the checked-in schema now applies the split range model across the MVP sweep for centers, zoom, rotation, iteration/epsilon controls, seed/animation rates, Explaino seeds, dive speed, polynomial coefficients, and Explaino phase.
- Hostile Review Pass 1: the first green audit found that the new inline numeric editors were reusing a duplicate hidden ImGui ID; the render path now uses a control-specific hidden input label instead.
- Hostile Review Pass 2: the active slice contract omitted `ui_app/src/schema_binding.h` from the allowed mutation scope even though the range seam needs a public resolver declaration; the contract was repaired before proceeding.
- Hostile Review Pass 3 Prep: safe-mode fallback is part of the visible control surface for this slice, so `ui_app/src/safe_mode_schema.cpp` now mirrors the new range model for the overlapping view/fractal controls instead of leaving the fallback on stale single-range behavior.
- Hostile Review Pass 3: `fractal.view.explaino_phase_strength` still used a hard `min`/`max` pair even though the owning root-shape math consumes it as an unclamped modulation multiplier; the schema and regression now treat its `[-20,20]` range as UI-only metadata, and a follow-up audit pass did not find another nearby control with equally strong runtime-authority evidence for conversion.
- Hostile Review Reopen: the reopened user review showed the first sweep still left several Explaino-family controls on legacy hard `min`/`max` metadata even though the shipped experimental-family reference only defines their intended widget spans, not engine clamps. The same reopen found that `fractal.view.explaino_phase` was still hidden for `explaino_dual` even though the shared runtime path uses the phase/warp start for DualSeed as well.
- Range Evidence: `docs/EXPLAINO_EXPERIMENTAL_FAMILY_REFERENCE.md` defines the shipped UI spans for `joy_coupling`, `fold_coupling`, `bell_coupling`, `ripple_amplitude`, `splice_offset`, `vortex_strength`, and `tension_strength`; those values belong in `ui_min`/`ui_max` unless the runtime itself proves a hard clamp.
- Runtime Evidence: the inertial runtime uses `momentum_beta` as a signed scalar multiplier on the previous-step momentum vector in both the host probe path and the device sampler, so the prior `[0,1]` hard clamp was suppressing a real half of the control domain.
- Duplicate-Schema Evidence: the deployed D runtime still carried a stale extra schema payload beside the live schema even though `ui_app/src/viewer_schema_load.cpp` only searches for the live `fractal_binding_surface_v1.ui_schema.json` file. The stale duplicate was therefore dead weight at best and a user-hostile confusion source at worst, because it still advertised old control metadata that no longer matched the live schema.
- Warp Authority Evidence: `ui_app/src/fractal_device_math.cuh` and `ui_app/src/fractal_probe_runner.cpp` both clamp `explaino_warp_strength` to `[0,1]` before applying the warp-start transform, while the live schema and `ui_app/src/fractal_runtime_validation.h` still advertise `[0,5]`. The real runtime owner is therefore already the stricter `[0,1]` contract.

## Proof Ledger

- Manual RED: current `drag_float` controls such as `center_x` and `center_y` only have one schema range model even though the slice needs separate UI-domain hints and hard clamp semantics.
- Checked-in regression RED: `cmd /c ui_app\build_tests_vsdevcmd.cmd` originally failed on missing `ui_min`/`ui_max` schema fields plus the missing `NumericControlRange` / `ResolveNumericControlRange(...)` seam.
- First GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed after the schema metadata, range resolver, and checked-in center control updates landed.
- Post-green hostile finding: hostile re-audit found two local defects after the first green: repeated hidden input labels on inline numeric editors, and the special Explaino seed control path bypassing the new hard-clamp / unique-ID seam. Both were repaired and revalidated with the same native helper lane.
- Broadened MVP GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed again after extending the same range model to int controls, safe-mode fallback parity, and the larger checked-in schema sweep including zoom, rotation, max-iter, epsilon, seed/animation rates, Explaino seeds, dive speed, polynomial coefficients, and Explaino phase.
- Hostile-audit repair GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed again after converting `explaino_phase_strength` from a hard clamp to UI-only range metadata and updating the pre-existing negative-capable Explaino schema regression.
- Published-runtime GREEN: `verify: runtime publish` completed with active runtime `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, and the deployed executable passed `--validate-ui` with exit code 0.
- Reopen RED: after the user re-audited the shipped controls, `cmd /c ui_app\build_tests_vsdevcmd.cmd` failed on the new live-schema regression because the schema still used hard `min`/`max` metadata for `explaino_damping`, `momentum_beta`, and the experimental-family variant controls, and `explaino_phase` was still missing `explaino_dual` from its visibility list.
- Reopen GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed after converting the remaining non-runtime-clamped Explaino-family sliders to `ui_min`/`ui_max`, widening `momentum_beta` to a signed UI range, and restoring `explaino_phase` visibility for `explaino_dual`.
- Duplicate-Schema RED: the user then verified that D still contained a stale duplicate schema payload; repo search showed the duplicate file itself was already gone from the workspace, but stale deploy/runtime docs still referenced it.
- Warp RED: the deployed runtime still exposes Warp as `[0,5]` even though the actual host/device math clamps at `1.0`, so the first control the user checked remained visibly wrong after the earlier publish.
- Duplicate-Schema GREEN: precise repo search across the active ui/spec/docs surfaces found no remaining duplicate-schema references, the stale deployed duplicate payload was deleted, and the published runtime no longer carries any extra schema copy beside the live schema.
- Warp GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed after aligning the live schema plus `ui_app/src/fractal_runtime_validation.h` to the actual `[0,1]` Warp domain, `verify: runtime publish` republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, the deployed executable passed `--validate-ui`, and a direct deployed-schema probe confirmed `explaino_warp_strength` now ships with `min=0.0` and `max=1.0`.
- Closeout GREEN: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed at 95/100, and `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` stayed green after the final cleanup edits.

## Notes

- Phase 1 completion snapshot:
  - Phase 0 is checkpointed and pushed, and slice 1 now starts from `feature/ui-polish-schema-domains`
  - the current local owner hypothesis is a schema metadata gap, not a `main.cpp` one-off widget bug
- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/ui_schema.cpp`
  - `ui_app/src/ui_schema.h`
  - `ui_app/src/schema_binding.h`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/safe_mode_schema.cpp`
- Non-goals:
  - do not redesign the color-mode authority here
  - do not change render-resolution defaults or pacing policy here
- Validation target:
  - focused workflow or headless tests for schema/binding behavior
  - relevant native helper validation before checkpoint

## Resume Point

Phase 4 is complete. The published runtime now ships only the live schema payload, and Warp is aligned end-to-end to `[0,1]` across the live schema, shared runtime validation, and deployed D:\ runtime.