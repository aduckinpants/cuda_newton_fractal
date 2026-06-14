# Viewer Host Tooling Hardening Phased Plan

## Explicit User Asks

- After the completed active-root-field goal, address the tooling issues observed during closure.
- Do a focused review/hardening/fix slice before returning to product/SDF planning.
- Keep this as tooling hardening, not a new product feature.

## Current Phase

Phase 5 - Hostile audit, validation, checkpoint, receipts, rearward review, push.

## Phase Checklist

- [x] Phase 0: Create this plan and contract on `codex/viewer-host-tooling-hardening`.
- [x] Phase 1: Add RED tests for the concrete tooling friction observed in the previous slice.
- [x] Phase 2: Harden patch application for CRLF patch files and quoted Windows/no-index paths.
- [x] Phase 3: Harden contract validation/receipt command handling for Windows build command escaping.
- [x] Phase 4: Harden logged-command guidance around long builds so wrapper timeouts are explicit and machine-readable.
- [ ] Phase 5: Hostile audit, validation, checkpoint, receipts, rearward review, push.

## Scope Lock

This slice may change only repo workflow/tooling surfaces and their tests/docs. It must not change fractal runtime behavior, UI product behavior, SDF math, root-field math, Color Pipeline semantics, or published runtime feature behavior.

## Concrete Issues From Previous Slice

1. `viewer_host_apply_repo_patch.py` rejected useful patch files because it only recognized literal `+++ b/` / `--- a/` targets and passed CRLF patch files directly to `git apply`.
2. Generated no-index diffs on Windows used quoted paths and backslashes, producing `patch contains no file targets` even when the target paths were in scope.
3. Contract JSON command strings containing `ui_app\build...` were valid JSON but parsed to a backspace escape in command text, which made receipt preflight impossible to satisfy cleanly.
4. Runtime/native build commands hit outer tool timeouts while child compiler processes continued, creating ambiguous logs and forcing manual process inspection.

## Desired Tooling Behavior

- Patch wrapper should normalize CRLF patch text into a temporary LF-only patch before calling `git apply`.
- Patch wrapper should extract and scope-check quoted git paths such as `+++ "b/ui_app\src\file.cpp"` and normalize backslashes to repo paths.
- Slice contract validation should reject required validation commands containing control characters or malformed Windows path escapes before the slice reaches receipt writing.
- Receipt preflight should report normalized command guidance when required/provided commands differ only by slash style or control-character escapes.
- Logged-command wrapper should make long-running command timeout use explicit through `--timeout-seconds` and structured JSON in docs/tests; this slice does not change heavy build defaults.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Bootstrap | done | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` on clean `b5847ff`. |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `b5847ff`. |
| RED tests | done | `py -3.14 -m pytest tests/test_viewer_host_tooling_hardening.py -q` initially failed for quoted Windows patch targets, missing LF normalizer, contract control-character validation, and timeout reporting. |
| Focused tooling tests | done | `py -3.14 -m pytest tests/test_viewer_host_tooling_hardening.py -q` passed: 5 tests. |
| Adjacent logged-command tests | done | `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q` passed: 6 tests. |
| Adjacent contract payload tests | done | `py -3.14 -m pytest tests/test_viewer_host_contract_tools.py::test_validate_slice_contract_payload_rejects_string_assertions tests/test_viewer_host_contract_tools.py::test_validate_slice_contract_payload_rejects_unknown_assertion_evidence_kind -q` passed: 2 tests. |
| Adjacent write-receipts tests | done | `py -3.14 -m pytest tests/test_agent_workflow_tools.py -k write_receipts -q` passed: 2 passed, 41 deselected. |
| Known unrelated red | noted | Full `tests/test_viewer_host_contract_tools.py tests/test_viewer_host_run_logged_command.py` hit existing FITS contract validator failure about binding workbench controls; not caused by this slice. |
| Contract validation | done | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/viewer_host_tooling_hardening.contract.json --out-json artifacts/validation/viewer_host_tooling_hardening_contract.json` passed. |
| Plan sync | done | `py -3.14 tools/viewer_host_run_logged_command.py --label viewer_host_tooling_hardening_plan_sync --log artifacts/viewer_host_tooling_hardening/plan_sync.log -- py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed. |
| Code-quality baseline | done | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/viewer_host_tooling_hardening_code_quality.json` passed with baseline score 93/100. |
| Hostile-audit validator | done | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/viewer_host_tooling_hardening_PHASED_PLAN.md --out-json artifacts/validation/viewer_host_tooling_hardening_hostile_audit.json` passed. |
| Diff check | done | `git diff --check > artifacts/viewer_host_tooling_hardening/diff_check.log 2>&1` passed. |
| Contract proof | pending | Checkpoint commit and receipts remain. |
| Hostile audit | done | Real audit finding recorded and repaired; clean re-read passes recorded below. |
| Receipts/rearward/push | pending | Required before closeout. |

## Test Targets

- Existing or new focused Python tests for:
  - patch target extraction and CRLF-safe apply path;
  - contract command validation for bad backslash/control-character cases;
  - checkpoint receipt missing-command diagnostics;
  - logged-command timeout JSON behavior.
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/viewer_host_tooling_hardening.contract.json --out-json artifacts/validation/viewer_host_tooling_hardening_contract.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label viewer_host_tooling_hardening_plan_sync --log artifacts/viewer_host_tooling_hardening/plan_sync.log -- py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/viewer_host_tooling_hardening_code_quality.json`
- `git diff --check > artifacts/viewer_host_tooling_hardening/diff_check.log 2>&1`

## Hostile Audit

- Status: complete

Required questions:

- Did the patch wrapper really handle the CRLF/quoted-path cases that burned time?
- Did contract validation catch bad command escapes before receipt writing?
- Did receipt diagnostics become more actionable instead of just moving the failure around?
- Did logged-command handling become clearer without hiding failed child processes?
- Did this slice avoid product/runtime behavior changes?

## Audit Passes

- [x] Pass 1 found a real gap after the first implementation: receipt preflight still lacked near-match command guidance, so a slash-style mismatch would remain harder to diagnose.
- [x] Pass 2 re-read the repaired state after adding receipt command hints; no additional real defect found in the touched tooling seams.
- [x] Pass 3 re-read the focused tests, logged-command output, and contract validation behavior; no additional workflow mistake found.

## Audit Findings

- [x] Receipt preflight hinting was still missing after the initial patch/contract/logged-command implementation; fixed with `_build_validation_command_hints(...)` and covered by `test_receipt_preflight_hints_near_matching_validation_commands`.
- [x] Clean re-read evidence: focused hardening tests and adjacent workflow tests proved cleanly after the receipt-hint repair.

## Stop Point

Stop after tooling hardening is validated, checkpointed, receipted, rearward-reviewed, pushed, and the next product direction is left for user review.
