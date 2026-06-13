# Fractal Docs Refresh Wave 1 Foundation Phased Plan

## Explicit User Asks

- Merge the completed Root SDF branch into `master` if rearward review remains `ok`.
- Open a fresh Wave 1 foundation branch from the merged Root SDF head.
- Import and reconcile `D:/salt-output/explaino_novelty_analysis/20260612_000000_viewer_host_fractal_math_refresh_packet_doc_update_working/fractal_docs_bootstrap_bundle` as an intake candidate, not as repo authority.
- Execute only foundation slices through Hardening Pass 1:
  - bundle drift report and docs/status truth sync;
  - N-root/root-field authority descriptor with legacy four-root parity;
  - minimal preset-core authority foundation;
  - deterministic AA V1 with AA-off parity;
  - hardening receipt and explicit foundation decision.
- Stop after Hardening Pass 1 before low-hanging fractal ideas, medium enablers, new feature families, SDF-op expansion, or broad UI replacement.

## Current Phase

Phase 4 - Hardening Pass 1 And Pause.

## Phase Checklist

- [x] Verify clean Root SDF branch head and rearward review before merge.
- [x] Fast-forward `master` to completed Root SDF head `eebc0e3` and push it.
- [x] Create fresh branch `codex/fractal-docs-refresh-wave1-foundation` from merged `master`.
- [x] Lock this Wave 1 plan and contract as the active slice.
- [x] Phase 0: read the bootstrap bundle, compare it to live repo state, and write `docs/notes/fractal_docs_refresh_wave1_bundle_drift_report.md`.
- [x] Phase 0: sync status/deferred/known-issue docs only where live repo evidence proves the bundle is stale or useful.
- [x] Phase 1: add N-root/root-field authority descriptor skeleton without changing existing four-root renderer behavior.
- [x] Phase 1: prove `legacy_quartic_v1` parity for current ExplainO lanes and Root SDF compatibility.
- [x] Phase 2: add minimal `preset_core` schema and round-trip authority for root layout, SDF field-primary state, Color Pipeline tuple, and future AA fields.
- [x] Phase 3: add deterministic AA V1 with `off` preserving exact current output.
- [x] Phase 4: run Hardening Pass 1, record the foundation decision, write receipts, and pause.

## Foundation Stop Rule

Low-hanging fractal idea passes remain deferred until after the Hardening Pass 1 decision.

Valid Hardening Pass 1 decisions are:

- `FOUNDATION_READY`
- `FOUNDATION_READY_WITH_CAVEATS`
- `FOUNDATION_BLOCKED`
- `ROLLBACK_REQUIRED`

Any decision other than `FOUNDATION_READY` must name the blocker, evidence, and next bounded repair seam before further feature work.

## Bundle Authority Rules

- The side-folder bundle is an import candidate and planning source, not repo authority.
- Current code, checked-in docs, tests, runtime behavior, and git state win over bundle text.
- The bundle is stamped before the live `eebc0e3` Root SDF seed-dynamics repair, so Root SDF volatile/active-repair claims must be reclassified during Phase 0.
- No generated or copied bundle content may replace checked-in docs without a drift note explaining the source, live verification, and conflicts.

## Phase 0 - Bundle Intake And Drift Report

### Intent

Create a truthful repo-local intake surface before product mutation.

### Tasks

1. Read the bundle README, manifest, current-state docs, sprint execution packet, deterministic sync tooling spec, and handoff notes.
2. Compare bundle branch/head stamps against live `master`/branch state.
3. Identify stale bundle claims, useful plan content, contradictions, and show-stoppers.
4. Write `docs/notes/fractal_docs_refresh_wave1_bundle_drift_report.md`.
5. Update status docs only for evidence-backed truth sync.

### Exit Criteria

- Drift report exists and names live branch/head, bundle stamp, stale claims, accepted imports, rejected imports, and first implementation risk list.
- Contract validation, plan sync, code-quality baseline, hostile-audit validation, and diff check pass for the docs/intake slice.

## Phase 1 - N-Root Root-Field Authority Descriptor

### Intent

Prepare a generalized root authority descriptor beside existing ExplainO behavior without changing visible math.

### Required Boundaries

- Preserve existing four-root behavior as `legacy_quartic_v1`.
- Do not rewrite normal ExplainO renderer dispatch.
- Do not overwrite captured/custom roots with derived effective roots.
- Do not route Root SDF into escape/iteration `IsExplainoFamily` semantics.

### Tasks

1. Inventory current root authority seams in code and docs.
2. Add descriptor types for root layout kind, source kind, active count, max count, base roots, effective roots, and diagnostic hashes.
3. Add an adapter that maps current ExplainO four-root state into `legacy_quartic_v1` descriptors.
4. Add tests proving descriptor parity with existing roots and Root SDF compatibility.
5. Add fail-closed validation for malformed or unsupported descriptor state.

### Exit Criteria

- Legacy four-root ExplainO lanes produce descriptor roots matching current behavior.
- Generated and custom authority remain distinct.
- Root SDF consumes descriptor output without broad ExplainO-family membership.

## Phase 2 - Minimal Preset Core Authority

### Intent

Add a versioned, deterministic preset authority core before AA and before new fractal idea passes.

### Required Boundaries

- `preset_core` is not a full gallery UX.
- Presets are curated authority tuples, not broad replay `state.json` dumps.
- Unsupported fields fail closed or report a clear omission reason.

### Tasks

1. Define the minimal preset schema for current fractal selector, root layout/source, SDF field-primary state, Color Pipeline tuple, and AA setting placeholder.
2. Add loader/exporter tests for old/simple states and current Root SDF/SDF-pack/ExplainO cases.
3. Prove preset load preserves generated/custom root authority and Color Pipeline row stack authority.

### Exit Criteria

- Preset core round-trips representative legacy, ExplainO, Root SDF, and SDF field-primary states without silently dropping authority-bearing values.

## Phase 3 - Deterministic AA V1

### Intent

Add deterministic supersampling support only after the authority surfaces can record it.

### Required Boundaries

- AA default is off.
- AA off must preserve exact current output on tested paths.
- AA V1 is not a realtime performance fix.
- SDF field-primary policy must be explicit and tested.

### Tasks

1. Add AA setting schema/state support with deterministic modes.
2. Implement `off` exact path and one bounded supersampling mode.
3. Record AA mode in capture/finding/preset metadata where relevant.
4. Add timing witness for AA-on cost and no regression proof for AA-off.

### Exit Criteria

- AA-off hashes match baseline.
- AA-on is deterministic and visibly/evidentially changes edge-rich samples.
- Capture/finding/preset metadata records AA mode.

## Phase 4 - Hardening Pass 1 And Pause

### Intent

Close Wave 1 foundation with hostile review and a foundation decision before any feature-growth pass.

### Required Checks

- N-root descriptor parity and fail-closed validation.
- Preset core authority round-trip.
- AA-off parity and AA-on determinism.
- Root SDF seed/control repair remains intact.
- SDF and Color Pipeline authority paths touched by this campaign still replay correctly.
- Docs/status/deferred surfaces match the shipped foundation state.

### Exit Criteria

- Hostile audit records at least one real finding or three clean passes.
- Validation receipt and contract proof receipt are written for the committed head.
- Rearward review is `ok`.
- The plan records one explicit Hardening Pass 1 decision and then pauses.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Root SDF branch merged | `master` fast-forwarded and pushed from `571dba4` to `eebc0e3`. |
| New branch created | `codex/fractal-docs-refresh-wave1-foundation` from `eebc0e3`. |
| Active contract locked | `viewer_host_begin_work_slice.py` locked `fractal_docs_refresh_wave1_foundation` with checkpoint token `ck:4b39b5a6`. |
| Bundle drift report | `docs/notes/fractal_docs_refresh_wave1_bundle_drift_report.md` records live `eebc0e3` authority, bundle `24d9d0e` drift, accepted imports, rejected imports, and Wave 1 risks. |
| Contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/fractal_docs_refresh_wave1_foundation.contract.json --out-json artifacts/validation/fractal_docs_refresh_wave1_contract.json` passed. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed after Phase 0 plan edits. |
| Code-quality baseline | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/fractal_docs_refresh_wave1_code_quality.json` passed with score 94/100. |
| Hostile audit validation | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/fractal_docs_refresh_wave1_foundation_PHASED_PLAN.md --out-json artifacts/validation/fractal_docs_refresh_wave1_hostile_audit.json` passed. |
| Diff check | `git diff --check > artifacts/fractal_docs_refresh_wave1_foundation/diff_check.log 2>&1` passed. |
| N-root parity proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_field` passed with 92 checks; `cmd /c ui_app\build_tests_vsdevcmd.cmd test_explaino_root_sdf_field` passed with 31 checks. |
| Preset core proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_preset_core` passed with 36 checks; `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\fractal_docs_refresh_wave1_foundation\phase2_build_vsdevcmd.log 2>&1` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| AA parity proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer > artifacts\fractal_docs_refresh_wave1_foundation\phase3_test_fractal_renderer.log 2>&1` passed with 90 checks, including AA-off exact parity, deterministic `ssaa_2x2`, and fail-closed source-signal sidecar behavior. |
| AA state IO proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_state_io > artifacts\fractal_docs_refresh_wave1_foundation\phase3_test_diagnostics_state_io.log 2>&1` passed and covers explicit `ssaa_2x2`, legacy default `off`, and unknown-mode rejection. |
| AA runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\fractal_docs_refresh_wave1_foundation\phase3_build_vsdevcmd.log 2>&1` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| AA timing witness | `artifacts/fractal_docs_refresh_wave1_foundation/phase3_aa_timing_witness.json` records compact 256x192 Multibrot `aa_mode=off` at 2.6368 ms and `aa_mode=ssaa_2x2` at 4.2107 ms with changed frame hash; this is a bounded cost witness, not a realtime performance claim. |
| Hardening native rails | Serial focused rails passed: `test_explaino_root_field` 92 checks, `test_explaino_root_sdf_field` 31 checks, `test_fractal_preset_core` 44 checks after AA repair, `test_fractal_renderer` 90 checks, and `test_diagnostics_state_io` all passed. |
| Published runtime proof | `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_root_sdf.py > artifacts\fractal_docs_refresh_wave1_foundation\hardening_repair_runtime_explaino_root_sdf.log 2>&1` passed with 4 tests, 0 skips/errors against the final rebuilt `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Final runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\fractal_docs_refresh_wave1_foundation\hardening_repair_build_vsdevcmd.log 2>&1` passed and staged the final runtime after the preset-core AA repair. |
| Final workflow validation | Contract validation, contract re-lock, plan sync, hostile-audit validation, code-quality baseline, and diff check passed after hardening repairs. |
| Hardening decision | `FOUNDATION_READY`: Wave 1 foundation has a drift report, descriptor parity, preset-core authority including AA mode, deterministic AA V1 with AA-off parity and bounded AA-on timing witness, focused native rails, and published-runtime Root SDF proof. Stop here before low-hanging ideas/enablers/new families. |

## Hostile Audit

- Status: complete

Audit questions:

- Did the bundle get treated as an import candidate rather than authority?
- Did Phase 0 record stale bundle claims instead of silently importing them?
- Did N-root descriptor work preserve `legacy_quartic_v1` behavior?
- Did presets preserve authority-bearing root/SDF/Color Pipeline state instead of becoming broad state dumps?
- Did AA-off preserve exact current output?
- Did the campaign stop after Hardening Pass 1 before new idea passes?

## Audit Passes

- [x] Pass 1 found a contract-proof defect: the first Wave 1 contract draft used unsupported `file_exists` and `plan_text` assertion evidence kinds.
- [x] Pass 2 re-read the repaired state and found no additional real defect in the machine-checkable contract assertions.
- [x] Pass 3 clean re-read of the drift report and status sync found no additional workflow mistake or bundle-authority leak.
- [x] Pass 4 found a Phase 1 implementation defect: the first descriptor cut exposed only one root array/hash and did not make Root SDF consume the descriptor.
- [x] Pass 5 re-ran focused descriptor and Root SDF rails after repair and found no additional Phase 1 descriptor defect.
- [x] Pass 6 found a Phase 2 authority defect: the first preset-core source-stack serializer assumed a runtime `enabled` member that exists on draft rows, not `KernelParams` source-stack rows.
- [x] Pass 7 found a workflow/test-harness misuse: two native helper builds were run in parallel against the same `build_tests/obj` directory, causing a transient compiler permission-denied collision.
- [x] Pass 8 re-ran affected native rails serially and found no additional Phase 2 preset-core defect.
- [x] Pass 9 found a Phase 3 validation friction defect: the default 120s command wrapper timed out the CUDA renderer helper and left an `nvcc` process briefly orphaned even though the focused rail passed when rerun logged with an appropriate timeout.
- [x] Pass 10 re-read the AA implementation boundaries and found no additional Phase 3 code defect: AA-off is exact by test, `ssaa_2x2` is deterministic by test, and source-signal sidecar use fails closed in V1 instead of silently inventing mixed semantics.
- [x] Pass 11 found a Phase 4 hardening defect: `preset_core` still wrote `"aa":{"mode":"off"}` unconditionally because Phase 2 had only a placeholder and no render-settings input.
- [x] Pass 12 re-ran preset-core after adding render-aware overloads and found no additional preset AA authority defect.
- [x] Pass 13 found a closure-proof defect: the contract proof assertion for code quality expected an `ok` field that `tools/code_quality_audit.py` does not write.
- [x] Pass 14 revalidated and re-locked the repaired contract with code-quality assertions against the actual severity-count fields.

## Audit Findings

- [x] Finding 1: unsupported contract evidence kinds would have blocked `viewer_host_begin_work_slice.py` and later contract-proof receipt validation. Repaired by replacing prose/file assertions with supported `validator_json` assertions for contract validation, hostile audit validation, and code-quality baseline evidence.
- [x] Finding 2: initial Phase 1 descriptor work underrepresented the plan by omitting base/effective root arrays and leaving Root SDF on direct `KernelParams.explaino_roots` reads. Repaired by adding base/effective arrays and hashes, preserving effective roots as local derived state, routing Root SDF scene resolution through the descriptor, and adding focused parity tests.
- [x] Finding 3: preset-core initially serialized a false `enabled` source-stack field from the live runtime stack. Repaired by removing that field from `preset_core`; enabled/disabled remains draft/window authority, while `preset_core` preserves the live source-stack rows that actually exist in `KernelParams`.
- [x] Finding 4: native helper builds that share `D:/salt-fractal/cuda_newton_fractal_clone/build_tests/obj` cannot be safely parallelized. Repaired operationally by rerunning the affected rail serially; do not parallelize `ui_app/build_tests_vsdevcmd.cmd` invocations in later phases.
- [x] Finding 5: the CUDA renderer helper can exceed the generic 120s shell timeout and produce a misleading timeout without test output. Repaired operationally for this slice by rerunning the rail with artifact logging and a longer timeout; future CUDA helper rails should use logged execution and avoid assuming the default wrapper timeout is evidence of failure.
- [x] Finding 6: `preset_core` AA metadata was a hardcoded `off` placeholder after real AA modes were added. Repaired by adding render-aware preset-core build/apply overloads, preserving old callers, parsing AA modes fail-closed, and adding tests for `ssaa_2x2` round-trip plus invalid-mode rejection.
- [x] Finding 7: the Wave 1 contract's code-quality acceptance assertion pointed at a nonexistent `ok` field in `artifacts/validation/fractal_docs_refresh_wave1_code_quality.json`. Repaired by asserting `severity_counts.CRITICAL == 0` and `severity_counts.ERROR == 0`, then re-locking the active contract with `viewer_host_revise_contract.py`.
