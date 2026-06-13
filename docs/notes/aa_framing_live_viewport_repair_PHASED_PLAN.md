# AA Framing And Live Viewport Repair Phased Plan

## Explicit User Asks

- Repair the visual AA issue found by side-by-side comparison: `ssaa_2x2` must not shift/framing-change the camera relative to AA off.
- Consider live viewport AA control now that the FPS budget concern is visible.
- Add a conservative live viewport AA enable checkbox and mode dropdown, default off.
- Keep this as a bounded repair slice, not a broad renderer or pacing redesign.
- Do not start deferred SDF work or new fractal idea passes under this repair.

## Current Phase

Phase 5 - Hostile Review And Close.

## Phase Checklist

- [x] Phase 0: open and lock this repair plan/contract.
- [x] Phase 1: add a focused RED that catches AA camera/framing drift.
- [x] Phase 2: repair `ssaa_2x2` sampling so framing is preserved.
- [x] Phase 3: add live Render-panel AA controls with default-off behavior.
- [x] Phase 4: validate native schema/binding/renderer/state rails and published runtime proof.
- [x] Phase 5: run hostile review, write receipts, rearward review, push, and stop.

## Scope Lock

Allowed:

- AA sampling/framing math.
- Render AA schema controls and binding support.
- AA state/capture/preset authority tests.
- Focused runtime no-mouse proof that AA controls are visible, default off, and settable.

Forbidden:

- SDF optimization work.
- New AA modes beyond `off` and `ssaa_2x2`.
- Claims that live AA is fast enough generally.
- Reworking render pacing or preview debounce policy.
- Physical mouse automation.

## Phase 1 - AA Framing Regression

### Intent

Prove the visual bug with a deterministic native/fixture check before changing renderer math.

### Tasks

1. Add a renderer regression that compares AA-off and `ssaa_2x2` edge location or center/framing on the same camera.
2. Keep the existing AA-off exact parity test.
3. Require `ssaa_2x2` to change edge-rich pixels without translating the field.

### Exit Criteria

- The new regression fails on the current implementation or explicitly captures the visual framing invariant.

## Phase 2 - Sampling Repair

### Intent

Make `ssaa_2x2` sample four subpixel offsets around the original pixel center instead of delegating to a high-resolution render whose camera mapping can drift.

### Tasks

1. Repair the renderer path with a framing-preserving sample strategy.
2. Keep AA default off and source-signal sidecar fail-closed in V1.
3. Re-run renderer and diagnostics state IO rails.

### Exit Criteria

- AA-off remains exact.
- `ssaa_2x2` is deterministic.
- `ssaa_2x2` changes edge-rich samples but preserves camera/framing.

## Phase 3 - Live Viewport Controls

### Intent

Expose AA for live viewport exploration without pretending it is free.

### Tasks

1. Add a Render-panel checkbox bound to `fractal.render.aa_enabled`, default `false`.
2. Add a Render-panel combo bound to `fractal.render.aa_mode`, visible only when AA is enabled.
3. Keep `RenderSettings::aa_mode` as the single runtime/saved-state authority:
   - unchecked checkbox writes/reads effective `off`;
   - checked checkbox enables the selected mode, defaulting to `ssaa_2x2`.
4. Add schema/binding tests for visibility, defaults, enum options, and automation control ids.

### Exit Criteria

- AA controls are discoverable in the existing Render panel.
- Default live viewport remains AA off.
- Setting the AA mode through bindings updates `RenderSettings::aa_mode`.

## Phase 4 - Validation

### Native Rails

- `test_fractal_renderer`
- `test_diagnostics_state_io`
- `test_fractal_preset_core`
- `test_ui_schema`
- `test_schema_binding`

### Runtime Rails

- Publish runtime.
- Run a focused no-mouse runtime proof that AA control visibility and set-value automation work in the published viewer.
- Regenerate a side-by-side visual comparison and keep it under artifacts.

## Phase 5 - Hostile Review And Close

### Audit Questions

- Did `ssaa_2x2` preserve camera/framing instead of only smoothing a shifted render?
- Did AA default remain off?
- Did live AA controls stay bounded to checkbox + mode dropdown without a broad pacing redesign?
- Did capture/state/preset authority still round-trip AA mode?
- Did runtime proof use no physical mouse?
- Did this slice stop before SDF and new-fractal work?

### Exit Criteria

- Hostile audit records a real finding or clean re-audit passes.
- Validation and contract proof receipts are written.
- Rearward review is `ok`.
- Branch is pushed with a clean tree.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Bootstrap | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `e8a448b`. |
| Repo status | `py -3.14 tools/viewer_host_repo_status.py` reported clean tree. |
| Rearward review | `py -3.14 tools/viewer_host_rearward_review.py` reported `ok` for `e8a448b`. |
| Repair contract | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/aa_framing_live_viewport_repair.contract.json --out-json artifacts/validation/aa_framing_live_viewport_repair_contract.json` passed; begin-slice token `ck:73801a4e`. |
| AA framing proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer` passed after replacing the high-resolution downsample path with four same-camera subpixel samples. |
| Live viewport AA control proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_schema_binding test_fractal_renderer` passed; schema/binding tests require default-off checkbox authority and gated `ssaa_2x2` dropdown. |
| State/preset proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_state_io test_fractal_preset_core` passed. |
| Focused native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_schema_binding test_fractal_renderer test_diagnostics_state_io test_fractal_preset_core` passed after hostile-review repair. |
| Runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Published runtime proof | `py -3.14 -m pytest tests/test_fractal_runtime_aa_controls.py -q` passed against the published runtime. |
| Visual artifact | `artifacts/aa_framing_live_viewport_repair/aa_visual_compare/multibrot_aa_side_by_side_boundary_crop.png` and `summary.json` regenerated from published runtime captures. |
| Contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/aa_framing_live_viewport_repair.contract.json --out-json artifacts/validation/aa_framing_live_viewport_repair_contract.json` passed. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed. |
| Code quality | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/aa_framing_live_viewport_repair_code_quality.json` passed with score 94, CRITICAL=0, ERROR=0. |
| Hostile audit validation | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/aa_framing_live_viewport_repair_PHASED_PLAN.md --out-json artifacts/validation/aa_framing_live_viewport_repair_hostile_audit.json` passed. |
| Diff check | `git diff --check > artifacts\aa_framing_live_viewport_repair\diff_check.log 2>&1` passed with line-ending warnings only. |

## Hostile Audit

- Status: closed

## Audit Passes

- [x] Pass 1 found a computed-bool authority gap outside the immediate UI path: descriptor/probe helper code still assumed every bool binding had raw pointer storage.
- [x] Pass 2 re-read the repaired state and found no additional real defect in remaining `BindBool` callers; remaining direct uses are raw-storage checks or the `GetBoolValue` fallback itself.
- [x] Pass 3 clean re-read confirmed the repaired state stayed within scope; no SDF work, pacing redesign, or new AA modes were added.

## Audit Findings

- [x] Finding 1: `fractal.render.aa_enabled` is intentionally computed from `RenderSettings::aa_mode`, but `fractal_parameter_surface_descriptor.cpp`, the schema-binding inventory helper, and `fractal_probe_runner.cpp` still treated bools as pointer-only bindings. Repaired by routing bool resolution through `GetBoolValue` and bool edits through `SetBoolValue`; focused native and published runtime proof passed after the repair.
