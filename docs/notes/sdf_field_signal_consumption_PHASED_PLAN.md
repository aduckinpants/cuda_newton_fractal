# SDF Field Signal Consumption

## Current Phase

Closed - source-neutral SDF field signals now feed the flashlight probe/report seam, runtime publish links the new helper, and preservation rails passed.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED native tests for source-neutral SDF signal sampling
- [x] Phase 3 - implement the minimal reusable SDF signal helper without changing Lens SDF field authority
- [x] Phase 4 - route the flashlight probe/report JSON through the helper and prove the output carries the new signals
- [x] Phase 5 - run focused Lens SDF, flashlight, Color Pipeline preservation, hostile audit, checkpoint, receipts, push, and clean-tree closeout

## Explicit User Asks

- [done] Continue from the hardened SDF CUDA substrate into the next best measured seam.
- [done] Let probe/report surfaces consume SDF field signals without caring about producer source.
- [done] Preserve existing Color Pipeline behavior while preparing a reusable SDF signal substrate.
- [done] Do not use OS mouse automation.
- [done] Keep this modular and avoid a viewport or renderer monolith.

## Scope

In scope:

- Add a source-neutral helper for SDF field samples.
- Cover signed distance, inside/outside, boundary band, approximate normal angle, and curvature estimate.
- Use existing `SdfFieldView` as the field authority.
- Route flashlight probe/report SDF samples through the helper.
- Preserve the current Lens SDF scalar field and RGBA paths.
- Preserve existing Color Pipeline rails while this seam remains headless.

Out of scope:

- Live viewport overlay productization.
- New `FractalType` registration.
- Renderer replacement.
- Authored SDF pack viewport rendering.
- New Color Pipeline UI rows until the signal helper is proven.
- Physical mouse automation.

## Proof Ledger

- Bootstrap authority: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-pack-cuda-hardening` at `21d99a3`, then the hardening branch was re-audited, corrected, merged to `master`, and pushed at `d96e34a`.
- Roadmap authority: `docs/notes/sdf_field_pack_near_term_TODO.md` lists Slice 6 as "Color Pipeline And Probe Consumption"; this plan starts with the probe/report signal seam before user-visible Color Pipeline runtime wiring.
- Boundary authority: SDF CUDA hardening is closed; this slice does not reopen descriptor validation except as a preservation rail.
- RED authority: `sdf_field_signal_consumption_red` failed before implementation because `../src/sdf_field_signal.h` did not exist.
- Lens SDF helper proof: `sdf_field_signal_consumption_test_lens_sdf` passed after adding `SdfFieldSignalSample` and `SampleSdfFieldSignals(...)`.
- Flashlight probe/report proof: `sdf_field_signal_consumption_test_flashlight_probe` passed with formatter checks for `inside_outside`, `boundary_band`, `normal_angle_radians`, and `curvature_estimate`.
- Runtime publish proof: `sdf_field_signal_consumption_runtime_publish` passed after adding `sdf_field_signal.cpp` to compile and link surfaces; log scan found no `error`, `fatal`, `LNK`, `unresolved`, or `failed` text.
- Color Pipeline preservation proof: `sdf_field_signal_consumption_color_pipeline_owner` passed, including `test_color_pipeline_core`, `test_color_pipeline_window`, `test_schema_binding`, `test_escape_time_coloring`, `test_diagnostics_state_io`, `test_finding_archive_actions`, and `test_runtime_reset`.
- Quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_signal_consumption_code_quality.json` passed with baseline score 96/100.
- Diff hygiene proof: `sdf_field_signal_consumption_diff_check` passed.
- Contract proof: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_signal_consumption.contract.json --out-json artifacts/validation/sdf_field_signal_consumption_contract.json` passed.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - re-read the source-neutral signal helper and tests as if the signal math is wrong; found no Lens SDF authority change, but confirmed the helper needed production build coverage.
- [x] Pass 2 - re-read the flashlight probe/report integration as if it still bypasses the helper; found the runtime publish link omitted `sdf_field_signal.obj`, then fixed the compile/link surfaces and reran publish cleanly.
- [x] Pass 3 - clean re-read of the repaired publish harness and preservation rails found no additional real defect after the link object and error-log checks were added.

## Audit Findings

- [x] Production publish build omitted the new SDF signal object, so `flashlight_probe.cpp` could compile in tests while the published runtime hit `LNK2019`/`LNK1120`. Fixed `ui_app/build_vsdevcmd.cmd` compile and link lists and reran runtime publish cleanly.
- [x] Publish validation initially reported success while the link log contained `LNK2019` and `fatal error LNK1120`. Hardened `:link_runtime` to fail if the link log contains `: error ` or `fatal error`, then reran runtime publish cleanly.
- [x] Clean re-read confirmed the repaired state keeps Lens SDF scalar/RGBA semantics unchanged, routes flashlight probe/report SDF payload through the helper, preserves Color Pipeline rails, and does not add viewport, renderer, new fractal type, or physical mouse work.

## Action Hostile Review

- Action ID: sdf-field-signal-consumption-closeout
- Suspected failure mode: source-specific Lens SDF sampling gets copied into probe/report code again, making later Color Pipeline consumption depend on the producer instead of `SdfFieldView`.
- Correct owner/action: add one reusable SDF signal helper and route flashlight probe/report sampling through it.
- Proof surface: focused native `test_lens_sdf`, focused native `test_flashlight_probe`, Color Pipeline preservation rail, runtime publish link, contract validation, plan sync, hostile audit, code-quality baseline, and diff hygiene.
- Blocked action: viewport overlay, renderer integration, new fractal type, authored-pack UI, or physical mouse automation.
