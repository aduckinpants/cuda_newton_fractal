# Root Proximity Backend Precision Policy

## Current Phase

Phase 4 - record hostile audit, write receipts, and checkpoint cleanly

## Phase Checklist

- [x] Phase 1 - classify root_proximity precision failure with a focused RED witness
- [x] Phase 2 - repair auto-tier policy with the smallest runtime-authoritative change
- [x] Phase 3 - prove native and published-runtime behavior for the referenced finding class
- [ ] Phase 4 - record hostile audit, write receipts, and checkpoint cleanly

## Explicit User Asks

- [done] Address the root_proximity precision issue shown by `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-11\135303_469__explaino` as a small bounded slice.
- [done] Preserve the forward-TDD and integration-TDD discipline used for the smooth_escape repair.
- [done] Capture the longer-term need for a programmable/metadata-owned backend policy so future precision flips do not require special-casing every fractal/color function/zoom combination.
- [done] Keep working forward without reopening unrelated Capture, Fractal Lab, Reality Toolkit, or broader advanced-color catalog work in this slice.

## Presumption Loop

The referenced finding is an ExplainO advanced-color state using `color_signal: root_proximity`, `color_palette: cyclic_escape`, and an explicit `fast`/`float32` capture. The local hypothesis is that root_proximity is another basin-family signal whose visible output depends on a logarithmic distance-to-root metric and therefore has a material fast-vs-standard delta like smooth_escape. Phase 1 must prove that with a native fast/standard/auto witness before changing policy. The immediate bounded repair may add root_proximity to the render-context auto policy, but the durable design target is descriptor-owned backend requirements, not an endless chain of one-off predicates.

## Presumption Evidence

- Finding: `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-11\135303_469__explaino` contains `color_signal: root_proximity`, `sample_tier: fast`, and `resolved_backend: float32`; the frame shows noisy basin-edge shimmer.
- Owner Proof: native and published-runtime probes agree that root_proximity is a render-context backend routing issue: explicit fast is `float32`, explicit standard is `float64`, and current auto resolves to `float32`.
- RED Witness: `artifacts/root_proximity_red_native_rerun.log` fails as expected with `fast_standard_diffs=7900`, `auto_fast_diffs=0`, `auto_standard_diffs=7900`, and `auto backend=float32`.
- Published Runtime Probe: the active `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` showed material fast/standard deltas at 64 and 512 pixels while auto matched fast exactly.
- Fix Proof: `ResolveSampleEvalModeForRender` now gates backend-sensitive basin auto promotion through `BasinColorSignalNeedsStandard`, covering `smooth_escape` and `root_proximity` while leaving `root_index` unpromoted and preserving explicit `fast`.
- Integration Witness: native helper tests, runtime publish, and focused published-runtime pytest all passed against the repaired policy.
- Long-term Policy Note: descriptor/catalog-owned backend sensitivity remains the follow-up target; this slice may add the second signal to the existing policy but must avoid implying this is the durable end state.
- Hostile Review Pass 1: complete; re-read the repaired native witness and no additional real defect found in the backend-owner proof.
- Hostile Review Pass 2: complete; re-read the repaired resolver gate and no broad float64/root_index/explicit-fast drift was found.
- Hostile Review Pass 3: complete; confirmed the repaired state against the published runtime proof and no stale-runtime or native-only closure issue was found.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported clean at `970edef` before this new slice.
- Finding inspection: done. The manual finding state and frame were inspected before planning.
- Native RED: `py -3.14 tools/viewer_host_run_logged_command.py --label "root_proximity red native helper tests rerun" --log artifacts/root_proximity_red_native_rerun.log -- ui_app\build_tests_vsdevcmd.cmd` failed for the intended policy gap with `auto backend=float32`, `fast_standard_diffs=7900`, `auto_fast_diffs=0`, and `auto_standard_diffs=7900`.
- Runtime classifier: temp published-runtime probe confirmed the same owner class before implementation: auto matched fast while standard differed materially.
- Native GREEN: `py -3.14 tools/viewer_host_run_logged_command.py --label "root_proximity native helper tests" --log artifacts/root_proximity_native_green.log -- ui_app\build_tests_vsdevcmd.cmd` passed; helper suite reported `All helper tests passed`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label "root_proximity runtime publish" --log artifacts/root_proximity_runtime_publish.log -- ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "root_proximity runtime backend pytest" --log artifacts/root_proximity_runtime_backend_pytest.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k root_proximity_auto_records_float64_backend` passed `1 passed, 22 deselected` against the active runtime.
- Code quality audit: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed with score 97/100.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/root_proximity_backend_precision_policy.contract.json --out-json artifacts/validation/root_proximity_backend_precision_policy_contract.json` passed with `ok: true`.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for `docs/notes/root_proximity_backend_precision_policy_PHASED_PLAN.md`.
- Hostile audit validator: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/root_proximity_backend_precision_policy_PHASED_PLAN.md --out-json artifacts/validation/root_proximity_backend_precision_policy_hostile_audit.json` passed with `ok: true`, three completed passes, and no open audit passes.
- Checkpoint: pending.

## Hostile Audit

- Status: closed
- Required posture: completed. The repaired state was re-read for witness validity, resolver scope, explicit-fast preservation, root_index non-promotion, published-runtime proof, and long-term descriptor-policy continuity.

## Audit Passes

- [x] Pass 1 - re-read the repaired native witness; the fast/standard delta remains material, auto now resolves through the policy gate, and no additional real defect found in the owner proof.
- [x] Pass 2 - re-read the repaired resolver state; promotion is still gated by `tier_auto`, basin support, `smooth_escape` coloring, and backend-sensitive signals only, with no additional real issue found for explicit `fast` or root_index.
- [x] Pass 3 - confirmed the repaired state against `artifacts/root_proximity_runtime_publish.log` and `artifacts/root_proximity_runtime_backend_pytest.log`; no additional workflow mistake found around stale runtime or native-only closure.

## Audit Findings

- [x] No additional real defect found during repaired-state hostile audit; the earlier invalid witness setup was corrected before implementation, and the final native plus published-runtime proofs exercise the repaired path.

## Action Hostile Review

- Action ID: action-20260511-root-proximity-closure-audit
- Suspected Failure Mode: closure may omit exact contract validators, receipts, or clean worktree proof even after native and runtime behavior are green.
- Expected Proof: contract validation, plan sync, hostile audit validator, validation receipt, contract proof receipt, checkpoint commit, and final `git status --short` clean output.
- Blocked Action: final response before receipts/checkpoint and clean worktree proof.

## Notes

- Immediate bounded target: render-context auto-tier policy for basin-family root_proximity, preserving explicit `fast` and `standard` semantics.
- Long-term target: move backend sensitivity into a programmatic policy surface, ideally descriptor/catalog metadata consumed by `ResolveSampleEvalModeForRender`, so future source functions can declare backend requirements without adding custom predicates per signal/family/zoom case.
- Non-goals:
  - do not override explicit `fast` in this slice
  - do not change root proximity math or palette semantics unless the RED witness proves backend policy is not the owner
  - do not reopen unrelated advanced-color catalog completeness, Capture/Export, Fractal Lab, Reality Toolkit, or generic.sample work
- Exit criteria:
  - a real root_proximity owner is proven by RED
  - minimal repair is runtime-authoritative
  - native and published-runtime proofs pass
  - hostile audit records three repaired-state passes or a real finding plus clean re-audit evidence
  - validation and contract-proof receipts exist for final committed HEAD
