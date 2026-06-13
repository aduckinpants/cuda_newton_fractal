# ExplainO Root SDF Generated N-Root Public Exposure Phased Plan

## Explicit User Asks

- Expose generated regular N-root layouts as an opt-in public mode for `explaino_root_sdf`.
- Preserve `legacy_quartic_v1` as the default generated Root SDF behavior.
- Add Root Layout and Root Count controls with correct visibility and no dead-control exposure.
- Keep custom roots four-slot only; generated N roots must not persist as custom authority.
- Support regular N roots for `N=2..16` with adjacent ring bridges and no duplicate bridge for `N=2`.
- Keep Root SDF field-primary; do not add escape-time N-root Newton behavior.
- Prove descriptor/schema/binding/field/runtime/capture behavior and include an `N=16` performance smoke.

## Current Phase

Phase 5 - Hostile review and closeout.

## Phase Checklist

- [x] Phase 0: create and lock this N-root plan/contract.
- [x] Phase 1: add focused RED tests for default legacy parity, generated layout/count visibility, N-root descriptor generation, bridge topology, and state/report authority.
- [x] Phase 2: implement generated layout/count authority and descriptor roots without changing custom root storage.
- [x] Phase 3: update Root SDF scene/field/report/capture surfaces to consume/report generated N roots.
- [x] Phase 4: run focused native rails, runtime no-mouse proof, capture replay proof, and N=16 performance smoke.
- [ ] Phase 5: hostile audit, repair findings, write receipts, rearward review, push, and clean-tree close.

## Scope Lock

Allowed:

- Root SDF generated root layout/count authority.
- Root-field descriptor, Root SDF scene builder, state/report/capture review surfaces.
- Schema/safe-mode/binding controls for Root SDF generated layout and count.
- Focused native and no-mouse runtime tests for generated N-root Root SDF.

Forbidden:

- Escape-time N-root Newton or renderer dispatch work.
- Arbitrary custom N-root coordinate editing.
- Reclassifying Root SDF as a broad ExplainO escape-time family.
- Broad SDF optimization, Color Pipeline redesign, or new SDF ops.
- Physical mouse automation.

## Phase 1 - RED Coverage

### Intent

Prove the current public surface cannot select a regular generated N-root layout and that the implementation must preserve legacy behavior while adding opt-in N-root behavior.

### Tasks

1. Add descriptor tests for `regular_ngon_v1` roots at `N=2,3,4,5,8,16`, invalid count rejection, and unchanged `legacy_quartic_v1` parity.
2. Add schema/binding tests for Root Layout visibility, Root Count visibility/enabled rules, and custom-root four-slot isolation.
3. Add Root SDF field tests for `N=2` bridge uniqueness, `N>=3` ring bridges, `bridge_width=0`, `smooth_blend=0`, and h-mode determinism.
4. Add report/capture expectations for layout kind, requested root count, base/effective hashes, bridge count, backend, and field dimensions.

### Exit Criteria

- RED tests fail on missing `regular_ngon_v1` public authority without weakening existing legacy/custom behavior.

## Phase 2 - Generated N-Root Authority

### Intent

Add input authority for generated regular N-root layouts without storing derived roots in the custom/captured four-root fields.

### Tasks

1. Add `ExplainoGeneratedRootLayout` with `legacy_quartic_v1` and `regular_ngon_v1`.
2. Add `explaino_generated_root_count` as the regular N-gon requested count, default `4`, range `2..16`.
3. Define `regular_ngon_v1` roots as:
   - `center = (0, 0)`
   - `radius = 0.85 + 0.95 * clamp(explaino_root_spread, 0, 1)`
   - `rotation = tau * (explaino_phase + fract(ExplainoSeedCombined(view, params) * 0.6180339887498949))`
   - `root_i = center + radius * (cos(rotation + tau*i/N), sin(rotation + tau*i/N))`
4. Preserve legacy generated Root SDF behavior by default.

### Exit Criteria

- Generated `regular_ngon_v1` roots are deterministic and descriptor-owned.
- Existing default Root SDF root/frame behavior remains unchanged.

## Phase 3 - Root SDF Consumption And Reporting

### Intent

Make Root SDF consume generated descriptor roots for `N=2..16` and report the authority clearly.

### Tasks

1. Update Root SDF scene resolution to accept descriptor counts `2..16`.
2. Keep legacy bridge behavior for `legacy_quartic_v1`.
3. For `regular_ngon_v1`, use one bridge for `N=2` and ring bridges for `N>=3`.
4. Extend runtime report and `fractal-state.json` review output with layout/count/hash/bridge/backend/field details.
5. Ensure `state.json` stores input controls only and never serializes generated N roots as custom authority.

### Exit Criteria

- Root SDF field output responds to generated root count and layout changes.
- Capture/replay stays deterministic.

## Phase 4 - Validation

### Native Rails

- `test_explaino_root_field`
- `test_explaino_root_sdf_field`
- `test_ui_schema`
- `test_safe_mode_schema`
- `test_schema_binding`
- `test_diagnostics_state_io`
- focused capture/report tests touched by the implementation

### Runtime Rails

- Publish runtime.
- No-mouse proof for selecting `explaino_root_sdf`, switching to `regular_ngon_v1`, and setting `N=2,5,8,16`.
- Capture Finding reload/replay proof for generated N-root Root SDF.
- Performance smoke for `1024` static `N=16`, `1024` h-mode `N=16`, and optional `2048` static `N=16`.

## Phase 5 - Hostile Review And Close

### Audit Questions

- Did default legacy Root SDF behavior stay unchanged?
- Did Root Count affect only `regular_ngon_v1` generated Root SDF?
- Did custom roots remain four-slot only?
- Did generated/effective N roots avoid custom authority persistence?
- Did `N=2` avoid duplicate bridges?
- Did Root SDF remain field-primary and outside escape-time N-root Newton work?
- Did runtime/capture/report evidence cover the public path?

### Exit Criteria

- Hostile audit records a real finding or clean re-audit passes.
- Validation and contract proof receipts are written.
- Rearward review is `ok`.
- Branch is pushed with a clean tree.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Source head | `master` clean at `78dbf8d` after AA/foundation merge and push. |
| Rearward review | `py -3.14 tools/viewer_host_rearward_review.py` reported `ok` for `78dbf8d`. |
| Active contract | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ExplainO Root SDF generated N-root public exposure" --profile runtime --plan docs/notes/explaino_root_sdf_generated_n_root_PHASED_PLAN.md --contract docs/contracts/explaino_root_sdf_generated_n_root.contract.json` locked token `ck:1ddd47f6`. |
| RED native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_field test_explaino_root_sdf_field test_ui_schema test_safe_mode_schema test_schema_binding` failed because `KernelParams` lacks `explaino_generated_root_layout` / `explaino_generated_root_count` and `ExplainoGeneratedRootLayout` does not exist. |
| Focused schema/core native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_field test_explaino_root_sdf_field test_ui_schema test_safe_mode_schema test_schema_binding` passed after implementation. |
| Persistence/report native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture test_diagnostics_state_io test_viewer_ui_automation_report` passed after implementation. |
| Broader focused native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_field test_explaino_root_sdf_field test_ui_schema test_safe_mode_schema test_schema_binding test_diagnostics_capture test_diagnostics_state_io test_viewer_ui_automation_report test_fractal_parameter_surface_descriptor test_fractal_runtime_validation` passed. |
| Runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd` passed and published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Published Root SDF runtime proof | `py -3.14 -m pytest tests/test_fractal_runtime_explaino_root_sdf.py -q` passed, including `regular_ngon_v1` counts `2`, `5`, `8`, and `16`, selector identity, root hash/frame hash changes, and Capture Finding sidecar checks. |
| Published SDF performance witness test | `py -3.14 -m pytest tests/test_fractal_runtime_sdf_performance_witness.py -q` passed. |
| Durable N=16 performance artifact | `py -3.14 tools\viewer_host_sdf_performance_witness.py --runtime-exe D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --out-json artifacts\explaino_root_sdf_generated_n_root\sdf_performance_witness.json --out-md artifacts\explaino_root_sdf_generated_n_root\sdf_performance_witness.md --work-dir artifacts\explaino_root_sdf_generated_n_root\witness_work --width 320 --height 240 --include-preview-sample` wrote N=16 static and phase-sine rows. Static N=16 reported about `1.4341 ms` field time; phase-sine N=16 reported about `1.4485 ms` field time on the compact witness. |
| Code quality | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality/explaino_root_sdf_generated_n_root.json` passed after the binding regression fix. |
| Repair revalidation | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture test_schema_binding` passed after hostile-audit cleanup. |

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - reviewed generated-root binding, capture, and helper-build seams; found real issues in helper dependencies, duplicated Root SDF capture resolution, and schema-binding size ratchet.
- [x] Pass 2 - re-read the repaired state after moving Capture Finding to the shared Root SDF resolver, fixing helper source lists, and shrinking `SetEnumId`; no additional real defect found in descriptor/root count/bridge/report authority.
- [x] Pass 3 - confirmed the repaired state against focused native, published runtime, performance witness, and code-quality rails; no additional workflow mistake found.

## Audit Findings

- [x] Focused helper build targets for diagnostics capture linked `diagnostics_capture.cpp` without the Root SDF resolver dependencies; fixed `ui_app/build_tests_vsdevcmd.cmd` so focused native tests compile the real resolver path.
- [x] Capture Finding had a local Root SDF root/bridge/h-mode resolver that would have drifted from generated N-root descriptor semantics; replaced it with `ResolveExplainoRootSdfScene(...)` and revalidated capture/state/report tests.
- [x] The new enum binding initially grew `schema_binding.cpp::SetEnumId()` past the code-quality ratchet; compacted the new assignment and reran the baseline check cleanly.
