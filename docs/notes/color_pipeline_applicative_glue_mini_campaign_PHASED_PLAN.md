# Color Pipeline Applicative Glue Mini-Campaign

## Current Phase

Closed - applicative glue implementation and validation are complete; the post-sprint replan is deferred to the next slice.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice.
- [x] Phase 2 - repair stale SDF and Color Pipeline roadmap truth after the built-in pack catalog seed and drag repair merged to `master`.
- [x] Phase 3 - extend UI-Salt materialized metadata with Source-row applicator records and RED/native validation.
- [x] Phase 4 - route Source-row contribution through the applicator before `blend_weight` while preserving current `none` and `boundary_band` behavior.
- [x] Phase 5 - expose `sdf_inside` and `sdf_outside` through the existing Source-row gate control area and no-mouse automation/report surfaces.
- [x] Phase 6 - publish runtime, prove applicator behavior/capture replay/preset preservation, hostile-audit the diff, and record the external receipt/rearward-review/push closure gates.

## Explicit User Asks

- [x] Implement the Color Pipeline Applicative Glue Mini-Sprint.
- [x] Start with docs truth sync so stale status does not hide already-shipped SDF work.
- [x] Build typed Source-row applicator glue instead of adding broad new function entries or redesigning the Color Pipeline UI.
- [x] Keep current persisted Source-row gate fields as v1 storage authority.
- [x] Add `none`, `sdf_boundary_band`, `sdf_inside`, and `sdf_outside` applicators for Source rows only.
- [x] Preserve existing SDF Normal Angle boundary-gate behavior exactly.
- [x] Keep disabled rows inactive for compatibility, runtime, reports, capture, and replay.
- [x] Prove behavior with native tests and published no-mouse runtime tests; no physical mouse automation.
- [x] Stop after applicator glue is proven and re-rank function picker/layout, more applicators, SDF pack authoring/catalog UX, new SDF ops, and SDF-native lanes in the next planning slice.

## Scope

In scope:

- Roadmap truth sync for shipped SDF pack catalog seed, SDF drag mapping repair, row-local field downsample, UI-Salt backend authority, preset workflow truth, and SDF source-stack runtime authority.
- UI-Salt metadata support for v1 Source-row applicators.
- Runtime applicator semantics for `none`, `sdf_boundary_band`, `sdf_inside`, and `sdf_outside`.
- Existing Source-row gate serialization/import/export, UI controls, and automation surfaces extended without a second persisted state authority.
- Focused native and published-runtime proof.

Out of scope:

- Full authored SDF pack catalog/authoring UX.
- New SDF pack ops, recursive/apollonian packs, or additional SDF-native lanes.
- Broad Factorio-style Color Pipeline UI redesign.
- Salticid adapter/removal campaign or Salticid runtime dependency.
- Perturbation zoom or unrelated fractal work.
- Physical mouse automation.

## Proof Ledger

- Start authority: `master` at `bb987a6`, clean, pushed, rearward review `ok`.
- Known stale truth: active SDF roadmap/status text still named the built-in SDF pack catalog seed as next even though `bb987a6` merged that work and the SDF drag camera mapping repair.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Color Pipeline applicative glue mini-campaign" --profile runtime --plan docs/notes/color_pipeline_applicative_glue_mini_campaign_PHASED_PLAN.md --contract docs/contracts/color_pipeline_applicative_glue_mini_campaign.contract.json` appended `ck:14881cdb` and locked `global_active_contract`.
- Roadmap truth sync: `_STATUS.md`, `KNOWN_ISSUES.md`, `DEFERRED_THREADS.md`, and `sdf_field_pack_near_term_TODO.md` now mark the built-in pack catalog seed and SDF drag repair shipped, and name applicative glue as the next bounded composition seam.
- RED proof: `color_pipeline_applicative_glue_materializer_red` failed before implementation because `row_applicator` was unknown and the generated contract lacked `row_applicators`.
- Materializer proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed as `12 passed in 1.51s`.
- Metadata/core proof: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core` passed as `passed=2702 failed=0`, including the required `artifacts/validation/color_pipeline_applicative_glue_core.json` run.
- Window proof: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window` passed as `passed=231 failed=0`.
- CPU postprocess proof: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess` passed as `passed=117 failed=0`.
- CUDA postprocess proof: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda` passed as `passed=130 failed=0`.
- State proof: `ui_app/build_tests_vsdevcmd.cmd test_diagnostics_state_io` passed as `all passed`.
- Runtime publish proof: `ui_app/build_vsdevcmd.cmd` passed and refreshed the published runtime under `D:\salt-fractal\cuda_newton_fractal_clone\runtime`.
- Published no-mouse runtime proof: `tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_capture_replay_authority.py tests/test_fractal_runtime_color_pipeline_presets.py` passed as `13 passed in 33.99s`.
- Contract validation proof: `viewer_host_validate_slice_contract.py` passed with `checks.contract_schema_valid=true` in `artifacts/validation/color_pipeline_applicative_glue_contract.json`.
- Plan sync proof: `viewer_host_assert_phased_plan_sync.py` passed for this plan.
- Code-quality proof: `tools/code_quality_audit.py --check-baseline` passed and wrote `artifacts/validation/color_pipeline_applicative_glue_code_quality.json`.
- Diff hygiene proof: `git diff --check` passed with only an LF-to-CRLF warning for `tests/test_ui_salt_materializer.py`.

## Action Hostile Review

- Action ID: applicative_glue_metadata_reds_v1
- Suspected Failure Mode: row applicators could become another hardcoded UI enum with no materialized metadata authority, no fail-closed legality, and no regression proving current generated contracts lack the new surface.
- Correct Owner/Action: add RED/native materializer and contract-loader tests plus the minimal UI-Salt/generated metadata shape for Source-row applicators.
- Proof Surface: `tests/test_ui_salt_materializer.py`, `test_color_pipeline_core`, and generated `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json` diffs.
- Blocked Action: runtime postprocess behavior changes or UI control expansion before metadata/applicator contract proof exists.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation changes current SDF pixels, treats disabled rows as active, creates a second Source-row gate state authority, silently permits unsupported metadata, breaks capture/replay, or hides stale roadmap text until tests prove otherwise.

Required questions:

- Did docs truth sync actually remove the stale built-in-pack-catalog next-step wording? Yes; the stale next-step wording was replaced with applicative glue and deferred follow-up text.
- Did metadata define applicator legality instead of another hardcoded UI-only enum? Yes; UI-Salt materializes `row_applicators`, and the C++ loader validates target lane, signal-kind requirement, SDF requirement, and fail-closed reason.
- Did `none` preserve current pixels? Yes; CPU/CUDA postprocess tests keep `none` as pass-through before blend.
- Did `sdf_boundary_band` preserve current SDF Normal Angle Beauty behavior? Yes; the existing boundary-band mask helper remains the boundary applicator and the published preset proof stayed green.
- Did `sdf_inside` and `sdf_outside` act as row masks before `blend_weight`? Yes; CPU/CUDA tests and the published no-mouse applicator hash proof cover this.
- Did disabled rows stay inactive for compatibility, runtime, reports, capture, and replay? Yes; disabled rows are skipped by runtime selection and now report `fail_closed_reason="row disabled"` in the no-mouse report.
- Did no-mouse runtime proof exercise the published viewer path? Yes; runtime proof used `tools/viewer_host_runtime_pytest_lane.py` against the published runtime.
- Did the slice avoid new SDF ops, new fractal lanes, broad UI redesign, and Salticid runtime dependency? Yes; changes stayed to Source-row applicator metadata, masks, reports, tests, and docs truth sync.

## Audit Passes

- [x] Pass 1 - roadmap truth sync audit found the already-shipped SDF pack catalog seed and drag camera repair still described as future work; docs now name applicative glue as the active bounded seam.
- [x] Pass 2 - metadata/materializer and generated-contract audit found row-applicator metadata initially had no C++ contract validator; `test_color_pipeline_core` caught the compile/loader gap and the validator now rejects bad applicators.
- [x] Pass 3 - runtime applicator semantic audit found disabled-row report coverage was too weak for this slice; the report now exposes Source-row applicator mode, field downsample group, blend weight, and row-disabled fail-closed reason.
- [x] Pass 4 - UI/no-mouse/capture-replay audit proved the published viewer path for inside/outside/boundary applicators, disabled rows, Capture Finding replay, and existing SDF presets.
- [x] Pass 5 - clean final re-read after validation found no additional real defect; contract validation, plan sync, code-quality baseline, exact core artifact, and diff-check proved cleanly.

## Audit Findings

- [x] Real finding: stale roadmap text still treated the built-in SDF pack catalog seed and SDF drag camera mapping repair as future work. Fixed in `_STATUS.md`, `KNOWN_ISSUES.md`, `DEFERRED_THREADS.md`, and `sdf_field_pack_near_term_TODO.md`.
- [x] Real finding: the first C++ metadata contract pass added `row_applicators` storage but missed the validator seam, which would have let bad generated metadata load without focused rejection proof. Fixed with `ValidateMaterializedRowApplicators` and `test_color_pipeline_core` coverage.
- [x] Real finding: disabled Source-row runtime reporting was not strong enough to prove disabled rows were inactive for compatibility/runtime/report surfaces. Fixed by reporting `applicator_mode`, `effective_field_downsample_group`, `blend_weight`, and `fail_closed_reason`, with no-mouse runtime assertions.
- [x] Clean re-audit: final validation and repaired-state re-read found no additional real issue and no additional workflow mistake.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_applicative_glue_mini_campaign.contract.json --out-json artifacts/validation/color_pipeline_applicative_glue_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_applicative_glue_mini_campaign_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_applicative_glue_hostile_audit.json`
- `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess`
- `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda`
- `ui_app/build_tests_vsdevcmd.cmd test_diagnostics_state_io`
- `ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_capture_replay_authority.py tests/test_fractal_runtime_color_pipeline_presets.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_applicative_glue_code_quality.json`
- `git diff --check`
