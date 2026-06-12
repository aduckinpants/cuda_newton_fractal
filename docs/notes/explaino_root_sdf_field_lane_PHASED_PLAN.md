# ExplainO Root SDF Field Lane

## Current Phase

Phase 6 - hostile audit, focused validation, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active viewer-first slice.
- [x] Phase 2 - record deferred non-v1 ideas and branch-hygiene proof.
- [x] Phase 3 - add RED tests for enum/catalog/schema/binding, field math, state/report/capture authority, Color Pipeline fail-closed behavior, and explorer capability.
- [x] Phase 4 - implement `explaino_root_sdf` as a field-primary SDF producer lane with CPU/CUDA parity.
- [x] Phase 5 - wire reports, Capture Finding `fractal-state.json`, runtime no-mouse automation, and performance witness.
- [x] Phase 6 - hostile audit, focused validation, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Add a new selectable field-primary lane `explaino_root_sdf`.
- [done] Render current ExplainO root layout as root circles plus explicit index-pair bridge capsules.
- [done] Preserve ExplainO root authority; effective roots must not overwrite generated/custom roots.
- [done] Include removable/extensible deterministic `h_source=phase_sine` modulation.
- [done] Color through the existing SDF Color Pipeline and fail closed for unsupported non-SDF Source rows.
- [done] Keep generalized SDF field-generation/downsample optimization deferred until after this lane closes.
- [done] Record non-v1 ideas in deferred docs: ExplainO SDF orbit trap, root-homotopy Newton, slime-trace fractal, root-metric Color Pipeline sources, and SDF gradient-force iteration.

## V1 Contract Lock

`explaino_root_sdf` is a field-primary SDF lane, not an escape/iteration fractal.

Authoritative inputs:

- selected lane id;
- `params.explaino_root_authority`, `params.explaino_root_count`, generated-root inputs, or custom root coordinates;
- `explaino_root_sdf_radius`, `explaino_root_sdf_bridge_width`, and `explaino_root_sdf_smooth_blend`;
- `explaino_root_sdf_h_source`, `explaino_root_sdf_h_amplitude`, and `explaino_root_sdf_h_frequency`;
- SDF Color Pipeline state.

Derived/report-only values:

- base roots resolved from existing ExplainO root authority;
- effective roots after local h-mode modulation;
- base/effective root hashes;
- bridge count and field dimensions/downsample;
- backend/probe diagnostics and unsupported-source reasons.

Routing:

- The lane must not enter `RenderFractalCUDA`.
- It must enter the field-primary SDF producer path.
- It must report `SdfFieldProducerKind::explaino_root_sdf`.
- Unsupported non-SDF Source rows must fail closed with `unsupported_source_for_producer`, producer kind, row index, and source id.
- No silent substitution or visually attractive fallback may claim an unsupported source is active.

Root/bridge semantics:

- Root order is semantic.
- Four roots bridge `(0,1)` and `(2,3)`.
- Three roots bridge `(0,1)` only.
- V1 does not auto-pair, sort, nearest-neighbor match, or infer conjugates.

SDF math:

- `circle_sdf(p, c, r) = length(p - c) - r`.
- `capsule_sdf(p, a, b, w) = distance from p to segment a-b minus w`.
- `smooth_blend == 0` uses exact min union.
- `bridge_width == 0` omits bridge primitives entirely.
- Duplicate roots and degenerate bridge capsules must remain finite and deterministic.
- Sign convention is negative inside, positive outside.

Phase modulation:

- `h_source=none` leaves effective roots equal to base roots.
- `phase_sine` is deterministic modulation driven by `view.explaino_phase`, not wall-clock time.
- `centroid = mean(base_roots)`.
- `dir_i = normalize(base_root_i - centroid)`, with fallback direction `tau * i / root_count` when the vector is zero.
- `effective_root_i = base_root_i + dir_i * h_amplitude * sin(tau * h_frequency * explaino_phase + tau * i / root_count)`.
- `h_amplitude` and `h_frequency` are inert when `h_source=none`.

Explorer contract:

- `explaino_root_sdf` must be discoverable by the ExplainO slime/explorer capability surface.
- Explorer-visible controls must include root authority/count/custom roots when active, ExplainO seed/phase, root-SDF shape controls, and h-mode controls.
- Active controls must mutate through no-mouse automation and produce deterministic frame/measurement effects when expected.
- Inactive controls must be reported inert and must not produce false-positive utility.
- Future selectable visual lanes must either implement the minimum explorer contract or declare an explicit exemption.

## Branch Hygiene Proof

- Bootstrap before branch work: clean `codex/explaino-slime-trace-runner` at `571dba4`, rearward review `ok`.
- Pre-merge `master`: `d111a8c7c65009420172bcc998bd2108d2af3fb3`.
- Pre-merge `origin/master`: `d111a8c7c65009420172bcc998bd2108d2af3fb3`.
- Closed branch head: `571dba4085f54f45f9fd7ca7751643176293e906`.
- Ancestry: `master` was ancestor of `codex/explaino-slime-trace-runner`; closed branch was not ancestor of `master`.
- Backup refs: `viewer-host/pre-root-sdf-master-20260612-122010` and `viewer-host/pre-root-sdf-source-20260612-122010`.
- Merge-up: `master` fast-forwarded to `571dba4` and was pushed to `origin/master`.
- Feature branch: `codex/explaino-root-sdf-field-lane` created from `571dba4`.
- Release refs: none matched `release*` in this checkout during preflight.

## Proof Ledger

- Manual RED: implementation started from missing enum/schema/catalog/field producer/runtime lane coverage for `explaino_root_sdf`; the first failing checks were the new focused C++ and runtime tests added in this slice.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ExplainO Root SDF field lane" --profile runtime --plan docs/notes/explaino_root_sdf_field_lane_PHASED_PLAN.md --contract docs/contracts/explaino_root_sdf_field_lane.contract.json` appended `ck:ceff5143` and locked `global_active_contract`.
- Checked-in regression RED: `test_explaino_root_sdf_field`, `test_fractal_runtime_validation`, `test_fractal_family_rules`, `test_safe_mode_schema`, and `tests/test_fractal_runtime_explaino_root_sdf.py` now cover the failures found during hostile review.
- First GREEN: focused native and published runtime rails passed after the field-primary dispatch, root authority, safe-mode, validation, and capture/replay fixes landed.
- Native schema/catalog/binding proof: `cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema`; `test_schema_binding`; `test_fractal_types`; `test_fractal_catalog_authority`.
- Native field math proof: `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_sdf_field` passed with `pass=31 fail=0`.
- Native state/report/capture proof: `test_viewer_ui_automation_report`; `test_diagnostics_capture`; `test_diagnostics_state_io`; `test_fractal_parameter_surface_descriptor`.
- Native validation/family/safe-mode proof: `test_fractal_runtime_validation`; `test_fractal_family_rules`; `test_fractal_derived_fields`; `test_safe_mode_schema`.
- Native SDF/Color Pipeline preservation proof: `test_color_pipeline_sdf_field_groups`; `test_color_pipeline_sdf_postprocess`; `test_color_pipeline_sdf_postprocess_cuda`; `test_sdf_pack_field_producer`; `test_sdf_pack_field_producer_cuda`; `test_color_pipeline_core`; `test_color_pipeline_window`.
- Deferred-doc proof: non-v1 ideas are parked in `DEFERRED_THREADS.md`; `docs/notes/sdf_field_pack_near_term_TODO.md` now marks `explaino_root_sdf` as a bounded interrupt before returning to field-generation/downsample optimization.
- Runtime publish proof: `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\explaino_root_sdf_field_lane\build_vsdevcmd_after_validation.log 2>&1` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Final runtime publish proof: `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\explaino_root_sdf_field_lane\build_vsdevcmd_final.log 2>&1` passed after the code-quality factoring.
- Published no-mouse proof: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_root_sdf.py -q` passed `3 passed`; `py -3.14 -m pytest tests/test_fractal_runtime_sdf_performance_witness.py -q` passed `1 passed`; final combined proof `py -3.14 -m pytest tests/test_fractal_runtime_explaino_root_sdf.py tests/test_fractal_runtime_sdf_performance_witness.py -q` passed `4 passed`; `py -3.14 -m pytest tests/test_function_descriptor_cli.py tests/test_callable_engine_adversarial_cli.py -q` passed `7 passed`.
- Full native helper proof: `cmd /c ui_app\build_tests_vsdevcmd.cmd > artifacts\explaino_root_sdf_field_lane\build_tests_full.log 2>&1` passed; log tail reports `All helper tests passed`.
- Code-quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_root_sdf_code_quality.json` passed after factoring root-SDF binding and safe-mode helpers.
- Contract receipt proof repair: the first contract command list had granular helper commands that did not match the final proof topology and included a nonexistent focused CUDA target. The contract was corrected to the actual receipt-verifiable rails: plan sync, full native helper sweep, runtime publish, published runtime proof, logged 1024/2048 performance witnesses, hostile audit, code quality, and diff check.
- Performance witness 1024: `artifacts/explaino_root_sdf_field_lane/sdf_performance_witness_1024.json` and `.md`; static roots field/post/total `3.159/1.067/4.226 ms`, phase_sine field/post/total `3.292/1.002/4.294 ms`, CUDA direct scalar postprocess, direct SDF pack grid evaluation, 4 roots, 2 bridges.
- Performance witness 2048: `artifacts/explaino_root_sdf_field_lane/sdf_performance_witness_2048.json` and `.md`; static roots field/post/total `9.219/4.279/13.498 ms`, phase_sine field/post/total `8.930/3.734/12.664 ms`, CUDA direct scalar postprocess, direct SDF pack grid evaluation, 4 roots, 2 bridges.
- Post-green hostile finding: real issues were found and fixed; see `Audit Findings`.

## Action Hostile Review

- Action ID: explaino_root_sdf_field_lane_initial_mutation_v1
- Suspected Failure Mode: adding a visible selector that either routes through normal fractal rendering, silently drops unsupported Color Pipeline sources, or bypasses root/slime explorer authority.
- Correct Owner/Action: mutate only the new field-primary lane, root-SDF producer, schema/binding/report/capture/tests, and deferred-doc truth needed by this slice.
- Proof Surface: native enum/schema/binding/field/state/report/capture/explorer tests plus published no-mouse selector/control/capture/replay/performance proof.
- Blocked Action: do not generalize SDF downsample/field-generation machinery, add new SDF ops, add SDF orbit traps, add root-homotopy Newton, add slime-trace live rendering, add root-metric Color Pipeline sources, or use physical mouse automation.

## Hostile Audit

- Status: complete

Required questions:

- Did the lane avoid `RenderFractalCUDA` and enter only the field-primary SDF producer path?
- Did root authority come from existing generated/custom root surfaces without overwriting base roots?
- Did index bridge ordering stay stable across generated roots, custom roots, state reload, and h-mode modulation?
- Did SDF formulas and edge cases match CPU/CUDA parity tests?
- Did unsupported non-SDF Source rows fail closed with structured diagnostics and no silent fallback?
- Did `state.json`, reports, Capture Finding, and `fractal-state.json` separate authoritative inputs from derived root summaries?
- Did the lane appear in the explorer capability surface and mutate active controls while marking inactive h controls inert?
- Did performance receipts include enough context to avoid broad FPS claims?
- Did the slice avoid reopening deferred SDF optimization or non-v1 feature ideas?

## Audit Passes

- [x] Pass 1 - hostile diff review found real defects in validation ordering, family selector assumptions, safe-mode coverage, replay proof shape, code-quality baseline regressions, and contract receipt command shape.
- [x] Pass 2 - re-read the repaired validation/family/safe-mode/runtime state; no additional real defect found in the touched selector, root authority, SDF producer, report, or capture seams.
- [x] Pass 3 - clean re-read of deferred scope, performance receipts, Color Pipeline fail-closed behavior, and factored binding/schema helpers confirmed the repaired state did not expose another workflow mistake before closure validation.

## Audit Findings

- [done] Validation guard ordering defect: `explaino_root_sdf` originally reported the live-render-path guard before malformed custom-root authority validation, hiding invalid captured/custom roots. Fixed by moving the final live-path guard after shared root-authority validation and adding `test_fractal_runtime_validation` coverage.
- [done] Family selector assumption defect: prefix-style ExplainO selector checks tried to treat `explaino_root_sdf` as a legacy ExplainO family selector. Fixed by making it explicit root-layout authority plus field-primary SDF, not `IsExplainoFamily`, and adding `test_fractal_family_rules` assertions.
- [done] Safe-mode coverage gap: safe-mode schema only had broad panel-count drift checks, so root-SDF controls could disappear without a focused failure. Fixed by adding explicit root-SDF safe-mode schema assertions.
- [done] Runtime replay proof defect: the first runtime test compared compact diagnostic seed capture against full-quality Capture Finding output. Fixed to compare replay of emitted finding state twice, preserving the actual Capture Finding replay contract.
- [done] Code-quality baseline regression: adding root-SDF bindings/controls lengthened existing catch-all `BindFloat` and safe-mode control builder functions. Fixed by factoring root-SDF float binding and safe-mode control construction into dedicated helpers.
- [done] Contract receipt command defect: the original contract listed granular helper commands that did not match the final proof topology and referenced `test_explaino_root_sdf_field_cuda`, which is not a valid focused helper target. Fixed by replacing the required command list with receipt-verifiable final rails and wrapping performance witnesses so the evidence mapper can record them.
- [done] Bounded duplication risk: `diagnostics_capture.cpp` resolves root-SDF derived summaries separately from the field producer to keep capture tests light. Sidecar tests now cover the emitted values, but future root-SDF semantics changes must keep this duplicate summary path in sync.

## Notes

- Expected enum id: `47`, only if current terminal id remains `46`.
- Expected branch: `codex/explaino-root-sdf-field-lane`.
- Expected contract: `docs/contracts/explaino_root_sdf_field_lane.contract.json`.
- Runtime proof must use no-mouse automation and published runtime, not helper-only proof.
