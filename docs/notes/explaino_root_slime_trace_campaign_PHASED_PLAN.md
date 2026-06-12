# ExplainO Root Authority And Slime Trace Campaign

## Current Phase

Closed for Slice 1 - captured-root analyzer authority repair is implemented and validated; later campaign slices require a fresh contract revision before mutation.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean `master` at `edf2bf5`, confirm rearward review `ok`, and create `codex/explaino-root-slime-trace-campaign`.
- [x] Phase 1 - open and lock this campaign contract.
- [x] Phase 2 - add RED tests proving analyzer root authority currently ignores captured roots.
- [x] Phase 3 - repair analyzer/chart output so captured roots are authoritative and fallback roots are labeled.
- [x] Phase 4 - validate focused Python tests, contract, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.

## Explicit User Asks

- [active] Start from clean `master` at `edf2bf5` on `codex/explaino-root-slime-trace-campaign`.
- [active] Implement the narrow captured-root analyzer authority repair first.
- [active] Create this campaign plan and `docs/contracts/explaino_root_slime_trace_campaign.contract.json`.
- [active] Do not start seed hunting, charts, GA, FITS/flashlight integration, or broader RTK trace tooling before authority proof.
- [active] Keep `state.json` as replay authority and make captured runtime roots analysis authority when present and valid.
- [active] Do not change render behavior.

## Scope

In scope for this slice:

- `tools/reality_toolkit/fractal_explorer/finding_analyzer.py` root source resolution.
- `tools/reality_toolkit/fractal_explorer/finding_charts.py` root authority/source reporting.
- Focused analyzer/chart tests.
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

- Status: deferred pending a fresh contract revision.
- Inventory current `sidecar_mutation_history`.
- Define the minimum v1 trace receipt fields: step index, previous value, applied value, pre/post state hash, measurement hash, root authority, roots at step, selection reason, scene id, and RNG/policy id where applicable.
- Do not implement a GA or new trace runner under Slice 2 unless the active contract is revised and locked.

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

## Hostile Audit

- Status: complete

Questions:

- Did captured runtime roots actually win over coefficient solving? Yes; `test_analyze_finding_uses_captured_explaino_roots_before_coefficients` monkeypatches the coefficient solver to fail if called and passes on captured roots.
- Did malformed captured roots fail clearly instead of silently falling back? Yes; count mismatch and nonfinite captured-root tests both fail with `ValueError` before fallback.
- Did legacy states without captured roots still use labeled coefficient fallback? Yes; legacy coefficient fallback is labeled `legacy_coefficients` / `fallback_legacy_derived`.
- Did report/JSON/CSV surfaces expose root source and authority? Yes; `FindingAnalysis` carries the fields, formatted reports include them, and roots CSV includes them.
- Did this slice avoid render, SDF, seed-hunter, FITS/flashlight, trace runner, and GA work? Yes; touched files are limited to this plan/contract, analyzer/chart tooling, focused tests, handoff, and artifacts.

## Audit Passes

- [x] Pass 1 - inspected analyzer and chart output after implementation; found the original product bug through RED failures.
- [x] Pass 2 - re-read the repaired state after the manual runner fix and no additional real defect found in the focused analyzer/chart surface.
- [x] Pass 3 - confirmed the repaired state after adding nonfinite-root coverage; pytest and direct script paths proved cleanly.

## Audit Findings

- [x] Original analyzer bug: captured `params.explaino_roots` were ignored and coefficient solving was used as the effective root authority.
- [x] Test harness bug introduced during this slice: a pytest `monkeypatch` fixture test was incorrectly listed in the direct script runner; the manual runner now excludes that fixture-only test and passes.
- [x] Initial malformed-root proof was too narrow; added explicit nonfinite captured-root rejection coverage.
- [x] Clean re-read the repaired state after the fixes; no additional real issue found in the bounded Slice 1 scope.
