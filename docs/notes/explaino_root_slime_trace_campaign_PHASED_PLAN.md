# ExplainO Root Authority And Slime Trace Campaign

## Current Phase

Closed for Slice 3 - ExplainO capability atlas generator and manifests are implemented and validated; Slice 4 headless trace runner remains preplanned but requires a fresh contract revision before mutation.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean `master` at `edf2bf5`, confirm rearward review `ok`, and create `codex/explaino-root-slime-trace-campaign`.
- [x] Phase 1 - open and lock this campaign contract.
- [x] Phase 2 - add RED tests proving analyzer root authority currently ignores captured roots.
- [x] Phase 3 - repair analyzer/chart output so captured roots are authoritative and fallback roots are labeled.
- [x] Phase 4 - validate focused Python tests, contract, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.
- [x] Phase 5 - inventory legacy `sidecar_mutation_history` and define the minimum v1 trace receipt contract.
- [x] Phase 6 - validate the receipt contract with focused tests, contract validation, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.
- [x] Phase 7 - revise and lock the campaign contract for the ExplainO capability atlas slice.
- [x] Phase 8 - add RED tests proving the atlas must cover every current ExplainO fractal id and expose root/source capability fields.
- [x] Phase 9 - implement the repo-derived atlas generator and generated JSON/Markdown outputs.
- [x] Phase 10 - validate focused atlas tests, contract, plan sync, hostile audit, code quality, and diff check before checkpoint; receipts, rearward review, push, and clean-tree proof are handled after the checkpoint commit.

## Explicit User Asks

- [x] Start from clean `master` at `edf2bf5` on `codex/explaino-root-slime-trace-campaign`.
- [x] Implement the narrow captured-root analyzer authority repair first.
- [x] Create this campaign plan and `docs/contracts/explaino_root_slime_trace_campaign.contract.json`.
- [x] Do not start seed hunting, charts, GA, FITS/flashlight integration, or broader RTK trace tooling before authority proof.
- [x] Keep `state.json` as replay authority and make captured runtime roots analysis authority when present and valid.
- [x] Do not change render behavior.
- [x] Work the next preplanned slice: trace receipt hardening/inventory before trace runner or seed tooling.
- [x] Keep future trace receipt work separate from existing legacy `sidecar_mutation_history` replay authority.
- [x] Work the next preplanned slice: generate an ExplainO capability atlas from repo-derived authority surfaces before headless trace-runner work.

## Scope

In scope for this slice:

- Generate a repo-derived ExplainO capability atlas in JSON and Markdown.
- Include every current ExplainO fractal id discovered from the checked-in enum/type surfaces.
- Record root generation support, custom-root authority notes, captured-root expectations, conjugate/secondary-root hints, source evidence, and analyzer limitations.
- Add focused tests proving coverage and generated-output freshness.
- Campaign planning surfaces.

Out of scope for this slice:

- Render/runtime behavior.
- SDF work.
- Trace runner implementation.
- Seed hunting, root-history chart expansion beyond atlas authority fields, FITS/flashlight integration, runtime-walk preset packs, adaptive viewport presentation, slime stopping policy, or GA work.
- Hand-maintained archaeology prose as the atlas authority.

## Campaign Slice Backlog

Slice 1 - captured root analyzer authority:

- Status: implemented in this checkpoint.
- Analyzer root resolution uses captured `params.explaino_roots` when `params.explaino_root_count > 0`.
- Coefficient solving is fallback-only and labeled `legacy_coefficients` / `fallback_legacy_derived`.
- Malformed captured roots fail instead of silently re-solving.
- Analyzer JSON, formatted report, and root CSV expose root source and authority.

Slice 2 - trace receipt hardening plan:

- Status: implemented and merged to `master` at `a707daf`.
- Inventory current `sidecar_mutation_history`.
- Define the minimum v1 trace receipt fields: step index, path, type, previous value, target value, applied value, utility, selection reason, pre/post state hash, measurement hash, root authority, roots at step, scene id, RNG seed, and policy id.
- Do not implement a GA or new trace runner under Slice 2 unless the active contract is revised and locked.
- Document that legacy `sidecar_mutation_history` remains ordered replay input and is not promoted to rigorous trace authority by itself.

Slice 3 - ExplainO capability atlas:

- Status: implemented in this checkpoint.
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

- Slice 3 branch: `codex/explaino-capability-atlas` created from pushed `master` at `a707daf`.
- Slice 3 planned validation: `py -3.14 -m pytest tests/test_explaino_capability_atlas.py -q --junitxml artifacts/pytest/explaino_capability_atlas.junit.xml`.
- Slice 3 contract revision: `docs/contracts/explaino_root_slime_trace_campaign.contract.json` now scopes `explaino_capability_atlas` as a `headless_only` analysis-tooling slice.
- Slice 3 lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ExplainO capability atlas" --profile native --plan docs/notes/explaino_root_slime_trace_campaign_PHASED_PLAN.md --contract docs/contracts/explaino_root_slime_trace_campaign.contract.json` opened checkpoint `ck:9d60ef9e`.
- Slice 3 RED: `py -3.14 -m pytest tests/test_explaino_capability_atlas.py -q --junitxml artifacts/pytest/explaino_capability_atlas_red.junit.xml` failed at import time because `tools.reality_toolkit.fractal_explorer.explaino_capability_atlas` did not exist.
- Slice 3 GREEN: `py -3.14 -m pytest tests/test_explaino_capability_atlas.py -q --junitxml artifacts/pytest/explaino_capability_atlas.junit.xml` passed 4 tests.
- Slice 3 preservation: `py -3.14 -m pytest tests/test_fractal_finding_analyzer.py tests/test_explaino_capability_atlas.py -q --junitxml artifacts/pytest/explaino_capability_atlas_with_analyzer.junit.xml` passed 19 tests.
- Slice 3 whitespace audit for untracked files: `rg -n "[ \t]+$" docs\manifests\explaino_capability_atlas.json docs\manifests\explaino_capability_atlas.md tests\test_explaino_capability_atlas.py tools\reality_toolkit\fractal_explorer\explaino_capability_atlas.py` found no trailing-whitespace matches.

- Slice 3 hardening: hostile re-read found the atlas would have silently recorded missing lens-mask registry coverage as `unknown`; `build_atlas` now fails closed and the focused atlas test asserts no ExplainO row has unknown lens semantics.
- Slice 3 final focused test: `py -3.14 -m pytest tests/test_explaino_capability_atlas.py -q --junitxml artifacts/pytest/explaino_capability_atlas.junit.xml` passed 4 tests after hardening.
- Slice 3 contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_root_slime_trace_campaign.contract.json --out-json artifacts/validation/explaino_root_slime_trace_campaign_contract.json` passed.
- Slice 3 code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_root_slime_trace_campaign_code_quality.json` passed with baseline score 93/100.
- Slice 3 diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_slime_trace_campaign_diff_check --log artifacts/logs/explaino_root_slime_trace_campaign_diff_check.log --out-json artifacts/validation/explaino_root_slime_trace_campaign_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed with exit code 0.
- Slice 3 final untracked-file whitespace scan: `rg -n "[ \t]+$" docs\manifests\explaino_capability_atlas.json docs\manifests\explaino_capability_atlas.md tests\test_explaino_capability_atlas.py tools\reality_toolkit\fractal_explorer\explaino_capability_atlas.py` found no trailing-whitespace matches.

## Hostile Audit

- Status: complete

Questions:

- Did Slice 3 actually generate the atlas from repo-derived enum/family/derived-field surfaces instead of hand-maintained lore? Yes; the generator parses `fractal_types.h`, `fractal_family_rules.h`, `fractal_derived_fields.cpp`, and analyzer authority surfaces.
- Did Slice 3 cover every current ExplainO fractal id and fail if a future id is omitted? Yes; tests compare enum ids, selector registry ids, and generated atlas entries.
- Did Slice 3 keep runtime, SDF, trace runner, seed hunting, charts expansion, FITS/flashlight, stopping policy, and GA out of scope? Yes; touched files are plan/contract, atlas generator/manifests, focused tests, handoff, and artifacts only.
- Did Slice 3 expose analyzer limitations without claiming unproven root semantics? Yes; each entry records captured-root authority, legacy coefficient fallback, static-analysis limits, and conservative per-lane notes.

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
- [x] Pass 6 - audit generated atlas coverage against current ExplainO ids and derived-field evidence; focused tests compare enum ids, selector registry ids, and generated entries.
- [x] Pass 7 - re-read generated JSON/Markdown for overclaims, stale manual prose, and unsupported capability claims; found missing lens semantics would have been silently labeled `unknown`, then repaired it to fail closed.
- [x] Pass 8 - confirmed validation rails prove the atlas slice through focused pytest, analyzer preservation pytest, contract validation, code-quality baseline, diff check, and explicit untracked-file whitespace scan; no runtime/SDF collateral was used as proof.

## Audit Findings

- [x] Original analyzer bug: captured `params.explaino_roots` were ignored and coefficient solving was used as the effective root authority.
- [x] Test harness bug introduced during this slice: a pytest `monkeypatch` fixture test was incorrectly listed in the direct script runner; the manual runner now excludes that fixture-only test and passes.
- [x] Initial malformed-root proof was too narrow; added explicit nonfinite captured-root rejection coverage.
- [x] Clean re-read the repaired state after the fixes; no additional real issue found in the bounded Slice 1 scope.
- [x] Slice 2 plan wording initially kept a stale Slice 1 audit sentence saying "this slice" touched analyzer/chart tooling; repaired to explicitly distinguish Slice 1 from Slice 2.
- [x] Clean re-read the repaired Slice 2 state; no additional real issue found in the bounded receipt-contract scope.
- [x] Slice 3 workflow mistake: the first Slice 3 contract revision used invalid `workflow_type: analysis_tooling`; repaired to the allowed `headless_only` workflow type before implementation.
- [x] Slice 3 audit finding: `git diff --stat` and `git diff --check` did not include untracked atlas files before staging; added an explicit trailing-whitespace scan over the new files before closure.
- [x] Slice 3 atlas hardening finding: missing ExplainO lens-mask registry coverage would have been silently emitted as `unknown`; `build_atlas` now fails closed and focused tests assert no generated row has unknown lens semantics.
- [x] Clean re-read after the Slice 3 lens-semantics hardening found no additional real issue in the bounded atlas scope.
