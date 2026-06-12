# ExplainO Root Authority And Slime Trace Campaign

## Current Phase

Closed for Slice 2 - trace receipt hardening contract and inventory are implemented and validated; Slice 3 requires a fresh contract revision before mutation.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean `master` at `edf2bf5`, confirm rearward review `ok`, and create `codex/explaino-root-slime-trace-campaign`.
- [x] Phase 1 - open and lock this campaign contract.
- [x] Phase 2 - add RED tests proving analyzer root authority currently ignores captured roots.
- [x] Phase 3 - repair analyzer/chart output so captured roots are authoritative and fallback roots are labeled.
- [x] Phase 4 - validate focused Python tests, contract, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.
- [x] Phase 5 - inventory legacy `sidecar_mutation_history` and define the minimum v1 trace receipt contract.
- [x] Phase 6 - validate the receipt contract with focused tests, contract validation, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.

## Explicit User Asks

- [x] Start from clean `master` at `edf2bf5` on `codex/explaino-root-slime-trace-campaign`.
- [x] Implement the narrow captured-root analyzer authority repair first.
- [x] Create this campaign plan and `docs/contracts/explaino_root_slime_trace_campaign.contract.json`.
- [x] Do not start seed hunting, charts, GA, FITS/flashlight integration, or broader RTK trace tooling before authority proof.
- [x] Keep `state.json` as replay authority and make captured runtime roots analysis authority when present and valid.
- [x] Do not change render behavior.
- [x] Work the next preplanned slice: trace receipt hardening/inventory before trace runner or seed tooling.
- [x] Keep future trace receipt work separate from existing legacy `sidecar_mutation_history` replay authority.

## Scope

In scope for this slice:

- Inventory the existing `sidecar_mutation_history` shape and replay seams.
- Define a minimum v1 trace receipt contract for future parameter-space slime traces.
- Focused contract tests proving required fields and deferred boundaries.
- Campaign planning surfaces.

Out of scope for this slice:

- Render/runtime behavior.
- SDF work.
- Trace runner implementation.
- ExplainO capability atlas implementation.
- Seed hunting, root-history chart expansion, FITS/flashlight integration, runtime-walk preset packs, adaptive viewport presentation, slime stopping policy, or GA work.

## Campaign Slice Backlog

Slice 1 - captured root analyzer authority:

- Status: implemented in this checkpoint.
- Analyzer root resolution uses captured `params.explaino_roots` when `params.explaino_root_count > 0`.
- Coefficient solving is fallback-only and labeled `legacy_coefficients` / `fallback_legacy_derived`.
- Malformed captured roots fail instead of silently re-solving.
- Analyzer JSON, formatted report, and root CSV expose root source and authority.

Slice 2 - trace receipt hardening plan:

- Status: active in this checkpoint.
- Inventory current `sidecar_mutation_history`.
- Define the minimum v1 trace receipt fields: step index, path, type, previous value, target value, applied value, utility, selection reason, pre/post state hash, measurement hash, root authority, roots at step, scene id, RNG seed, and policy id.
- Do not implement a GA or new trace runner under Slice 2 unless the active contract is revised and locked.
- Document that legacy `sidecar_mutation_history` remains ordered replay input and is not promoted to rigorous trace authority by itself.

Slice 3 - ExplainO capability atlas:

- Status: deferred pending a fresh contract revision.
- Generate JSON/Markdown from repo-derived ExplainO capability surfaces rather than hand-maintained archaeology prose.
- Include fractal id, generated/custom root support, expected captured-root behavior, known conjugate semantics, second-polynomial/secondary root-family notes, and analyzer limitations.

Slice 4 - headless parameter-space slime trace runner:

- Status: deferred pending a fresh contract revision.
- Add a no-mouse/headless runner that loads a state, runs bounded ExplainO sidecar parameter traversal, and emits trace artifacts.
- Required outputs remain `slime_trace_manifest.json`, `initial_state.json`, `final_state.json`, `mutation_trace.jsonl`, `root_samples.jsonl`, `measurement_samples.jsonl`, and `trace_summary.json`.
- Stop and replan after this slice before seed hunting, root-history charts, FITS/flashlight reuse, stopping-policy work, or GA.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `master` at `edf2bf5`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `edf2bf5`.
- Branch: `codex/explaino-root-slime-trace-campaign`.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ExplainO captured root analyzer authority repair" --profile native --plan docs/notes/explaino_root_slime_trace_campaign_PHASED_PLAN.md --contract docs/contracts/explaino_root_slime_trace_campaign.contract.json` opened checkpoint `ck:abb9093f`.
- RED: `py -3.14 -m pytest tests/test_fractal_finding_analyzer.py -q --junitxml artifacts/pytest/explaino_root_slime_trace_campaign_finding_analyzer_red.junit.xml` failed 4 focused tests because analyzer still called coefficient solving, did not label fallback source, did not fail malformed captured roots, and root CSV lacked authority columns.
- GREEN: `py -3.14 -m pytest tests/test_fractal_finding_analyzer.py -q --junitxml artifacts/pytest/explaino_root_slime_trace_campaign_finding_analyzer.junit.xml` passed 15 tests.
- Harness check: `py -3.14 tests/test_fractal_finding_analyzer.py` passed 14 direct-run tests after removing the fixture-based test from the manual runner list.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_root_slime_trace_campaign.contract.json --out-json artifacts/validation/explaino_root_slime_trace_campaign_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/explaino_root_slime_trace_campaign_PHASED_PLAN.md --out-json artifacts/validation/explaino_root_slime_trace_campaign_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_root_slime_trace_campaign_code_quality.json` passed with baseline score 93/100.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_slime_trace_campaign_diff_check --log artifacts/logs/explaino_root_slime_trace_campaign_diff_check.log --out-json artifacts/validation/explaino_root_slime_trace_campaign_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed with exit code 0.
- Master merge: `codex/progress-visibility-protocol` and `codex/explaino-root-slime-trace-campaign` were merged to `master` at `f9bb54b`; exact ExplainO contract rails passed on the merged head and `master` was pushed.
- Slice 2 branch: `codex/explaino-trace-receipt-hardening` created from pushed `master` at `f9bb54b`.
- Slice 2 inventory found current `sidecar_mutation_history` stores `label`, `path`, `type`, `target_value`, and `utility`; it round-trips and replays ordered targets but does not carry previous/applied values, state hashes, measurement hash, root authority/roots, scene id, RNG seed, or policy identity.
- Slice 2 contract: `docs/contracts/explaino_slime_trace_receipt_v1.contract.json` records legacy inventory, v1 required fields, compatibility rules, and deferred non-goals.
- Slice 2 focused test: `py -3.14 -m pytest tests/test_explaino_trace_receipt_contract.py -q --junitxml artifacts/pytest/explaino_trace_receipt_hardening_contract.junit.xml` passed 4 tests.
- Slice 2 contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_root_slime_trace_campaign.contract.json --out-json artifacts/validation/explaino_root_slime_trace_campaign_contract.json` passed.
- Slice 2 plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Slice 2 hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/explaino_root_slime_trace_campaign_PHASED_PLAN.md --out-json artifacts/validation/explaino_root_slime_trace_campaign_hostile_audit.json` passed.
- Slice 2 code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_root_slime_trace_campaign_code_quality.json` passed with baseline score 93/100.
- Slice 2 diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_slime_trace_campaign_diff_check --log artifacts/logs/explaino_root_slime_trace_campaign_diff_check.log --out-json artifacts/validation/explaino_root_slime_trace_campaign_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed with exit code 0.

## Hostile Audit

- Status: complete

Questions:

- Did captured runtime roots actually win over coefficient solving? Yes; `test_analyze_finding_uses_captured_explaino_roots_before_coefficients` monkeypatches the coefficient solver to fail if called and passes on captured roots.
- Did malformed captured roots fail clearly instead of silently falling back? Yes; count mismatch and nonfinite captured-root tests both fail with `ValueError` before fallback.
- Did legacy states without captured roots still use labeled coefficient fallback? Yes; legacy coefficient fallback is labeled `legacy_coefficients` / `fallback_legacy_derived`.
- Did report/JSON/CSV surfaces expose root source and authority? Yes; `FindingAnalysis` carries the fields, formatted reports include them, and roots CSV includes them.
- Did Slice 1 avoid render, SDF, seed-hunter, FITS/flashlight, trace runner, and GA work? Yes; touched files were limited to the campaign plan/contract, analyzer/chart tooling, focused tests, handoff, and artifacts.
- Did Slice 2 keep legacy `sidecar_mutation_history` as replay input instead of overclaiming it as rigorous trace authority? Yes; the v1 contract marks it as legacy ordered replay input and says missing v1 fields must fail closed for future trace analysis.
- Did Slice 2 define enough v1 receipt fields for future golden-thread/root-aware trace analysis? Yes; the v1 contract requires step index, previous/applied values, state hashes, measurement hash, root authority/roots, scene id, RNG seed, and policy id.
- Did Slice 2 avoid runner, seed-hunter, charts, FITS/flashlight, stopping-policy, GA, runtime, and SDF mutation? Yes; touched files are limited to docs/contracts, the focused contract test, campaign plan, handoff, and artifacts.

## Audit Passes

- [x] Pass 1 - inspected analyzer and chart output after implementation; found the original product bug through RED failures.
- [x] Pass 2 - re-read the repaired state after the manual runner fix and no additional real defect found in the focused analyzer/chart surface.
- [x] Pass 3 - confirmed the repaired state after adding nonfinite-root coverage; pytest and direct script paths proved cleanly.
- [x] Pass 4 - audited Slice 2 receipt contract against current legacy mutation-history seams and found stale Slice 1 wording that over-described active scope.
- [x] Pass 5 - re-read deferred boundaries and confirmed trace runner, seed hunter, FITS/flashlight, stopping policy, and GA remain out of scope.

## Audit Findings

- [x] Original analyzer bug: captured `params.explaino_roots` were ignored and coefficient solving was used as the effective root authority.
- [x] Test harness bug introduced during this slice: a pytest `monkeypatch` fixture test was incorrectly listed in the direct script runner; the manual runner now excludes that fixture-only test and passes.
- [x] Initial malformed-root proof was too narrow; added explicit nonfinite captured-root rejection coverage.
- [x] Clean re-read the repaired state after the fixes; no additional real issue found in the bounded Slice 1 scope.
- [x] Slice 2 plan wording initially kept a stale Slice 1 audit sentence saying "this slice" touched analyzer/chart tooling; repaired to explicitly distinguish Slice 1 from Slice 2.
- [x] Clean re-read the repaired Slice 2 state; no additional real issue found in the bounded receipt-contract scope.
