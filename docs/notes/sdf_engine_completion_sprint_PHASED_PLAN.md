# SDF Engine Completion Sprint

## Current Phase

Complete - SDF engine hardening sprint closed; next SDF feature growth requires a fresh replan.

## Phase Checklist

- [x] Phase 0 - merge Capture Finding fractal-state sidecar into `master`, push, and verify rearward review `ok`
- [x] Phase 1 - open this sprint plan/contract and reconcile roadmap truth for recent closed work
- [x] Phase 2 - repair published-runtime SDF built-in pack resolution and missing-pack diagnostics
- [x] Phase 3 - add SDF field capability reporting for current field producers
- [x] Phase 4 - harden field-primary lane Source-stack honesty for unsupported non-SDF signals
- [x] Phase 5 - harden SDF field cache keys and performance witnesses
- [x] Phase 6 - plan the first SDF operator-expansion gate without implementing new ops
- [x] Phase 7 - post-sprint replan before broad SDF feature growth

## Explicit User Asks

- [closed] Start with a looking-back hostile review of recent closed work.
- [closed] Return to the earlier SDF engine topic in dependency order.
- [closed] Merge the Capture Finding fractal-state sidecar branch into `master` before opening new SDF mutation.
- [closed] Treat `fractal-state.json` as review authority only; keep `state.json` as replay authority.
- [closed] Verify native-helper optimization, function picker/layout, mixed Source rows, SDF pack scene/catalog, and Capture Finding sidecar as suspect before new product claims.
- [closed] Stabilize published-runtime pack resolution, field/source capability reporting, capture/replay/report authority, and measured performance before adding new SDF ops or lanes.
- [closed] Keep unsupported mixed Source rows fail-closed instead of inventing fake support.
- [closed] Avoid physical mouse automation and avoid broad Color Pipeline redesign.

## Scope

In scope:

- Roadmap truth sync for recent closed work.
- Published-runtime SDF built-in pack staging/loading/reporting.
- Internal SDF field capability/reporting surfaces.
- Mixed-source honesty for field-primary lanes such as `sdf_pack_scene` and `generic_equation_pack`.
- SDF field cache-key review and compact performance witnesses.
- A planning-only gate for future SDF operator expansion.

Out of scope:

- New SDF ops before phases 1-5 are green.
- Recursive/apollonian packs, IFS, Flame/attractor density, 3D DE/raymarch, or perturbation work.
- Full authored-pack catalog/authoring UX.
- Broad Color Pipeline workflow redesign.
- Salticid runtime dependency or adapter/removal campaign.
- Physical mouse automation.

## Current Repo Truth

- `codex/capture-finding-fractal-state-sidecar` closed at `c3d6d3a`, was fast-forwarded into `master`, pushed to `origin/master`, and rearward review for `c3d6d3a` is `ok`.
- `state.json` remains Capture Finding replay authority; `fractal-state.json` is review-focused active-state authority.
- Native helper build optimization is landed on `master`, but follow-up validation should use the grouped helper topology rather than reintroducing repeated heavy setup.
- Color Pipeline function picker/layout is landed with focused native/runtime proof; the earlier broad native helper sweep was explicitly unproven because it timed out before the helper optimization.
- Mixed SDF/non-SDF Source rows are supported only on renderer-backed lanes. `generic_equation_pack` and `sdf_pack_scene` remain intentionally fail-closed for mixed Source rows until real non-SDF source-signal producers exist, and runtime proof now covers that denial.
- The SDF substrate has Lens SDF, Lens Field v2, authored pack field production, row-local downsample, SDF source rows, overlay/capture/replay, `sdf_pack_scene`, and a curated built-in catalog seed. Published-runtime built-in pack JSON is now staged beside the runtime and wins over repo-root metadata; repo-root lookup is only a dev fallback.
- Runtime reports now expose `lens_sdf_field_producer_kind`, `lens_sdf_supported_signals`, and `lens_sdf_field_capability_fail_closed_reason` in addition to the existing source-stack/backend/downsample/timing fields.
- The latest compact performance witness records Lens SDF, Lens Field v2, and `sdf_pack_scene` rows. At 640x480 in this run, recommendation remains `field_generation_or_downsample_candidate`; no global FPS improvement claim is made.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported clean branch `codex/capture-finding-fractal-state-sidecar`, `HEAD=c3d6d3a`, active prior contract `capture_finding_fractal_state_sidecar`, and code-quality baseline OK.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `c3d6d3a`.
- Merge preflight: `master` fast-forwarded from `32a6fe0` to `c3d6d3a`, pushed to `origin/master`, then `codex/sdf-engine-completion-sprint` was created.
- RED proof: `sdf_engine_pack_resolution_native_red` failed because `fractal_ui_repo_root.txt` could still override a conflicting runtime-staged built-in pack.
- Native pack resolution proof: `sdf_engine_pack_resolution_native_green` passed `test_sdf_pack_viewer_ui` with 142 assertions after loader priority changed to runtime-staged pack first.
- Runtime publish proof: `sdf_engine_pack_resolution_runtime_publish` and `sdf_engine_capability_runtime_publish` both rebuilt the published viewer at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime pack resolution proof: `sdf_engine_pack_resolution_runtime_pytest` passed `tests/test_fractal_runtime_sdf_pack_scene_lane.py` with 3 tests, including staged built-in pack file checks.
- Native capability proof: `sdf_engine_capability_report_native` passed `test_viewer_ui_automation_report` and `test_sdf_pack_viewer_ui`.
- Runtime capability proof: `sdf_engine_capability_runtime_pytest` passed SDF pack scene plus Lens Field v2 focused runtime checks with 4 tests.
- Field-primary honesty proof: `sdf_engine_field_primary_honesty_runtime_pytest` passed 4 tests, including fail-closed mixed Source rows for `generic_equation_pack` and `sdf_pack_scene`.
- Performance/cache proof: `sdf_engine_performance_witness_tool_tests` passed the witness unit and compact published-runtime witness tests; `sdf_engine_completion_performance_witness` wrote `artifacts/sdf_engine_completion_sprint/sdf_performance_witness.json` and `.md`; `sdf_engine_field_cache_native` passed `test_lens_sdf`.
- Receipt hardening proof: `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py::test_validation_evidence_spec_for_command_prefers_logged_command_out_json -q` passed and proves logged runtime commands with `--out-json` are recorded as validator JSON evidence for contract-proof assertions; the contract now asserts the logged-command artifact's `exit_code` field. A full `tests/test_viewer_host_contract_proof.py` pass then exposed and repaired stale 46-to-47 fractal-count drift in the parameter-surface contract, inventory, and test guardrail.

## SDF Operator Expansion Gate

No new SDF operators were implemented in this sprint. The first operator-expansion slice may start only after a fresh plan/contract and should use this gate:

- Candidate first ops: mirror/fold, polar repeat, inversion, rounded primitives, and elongated primitives.
- Each op must add parser validation, CPU runtime evaluation, CUDA runtime evaluation, nonfinite/bounds rejection, CPU/CUDA parity, and at least one curated built-in pack using the op.
- Each op must keep `sdf_pack_scene` and authored-pack field reports honest about backend, pack id, field dimensions/downsample, and fail-closed errors.
- Recursive/apollonian packs, IFS, Flame/attractor density, 3D DE/raymarch, Salticid adapter/removal, and hybrid non-SDF substrate work remain separate architecture campaigns.

## Post-Sprint Replan

Recommended next ordering, subject to user review:

1. Field generation/downsample optimization for authored SDF pack fields, because the compact witness still points there and `sdf_pack_scene_signed_distance` is the heaviest row in this run.
2. First small SDF operator expansion only after the optimization path is bounded or explicitly deferred.
3. Authored pack catalog/authoring UX after the engine can honestly package, report, replay, and measure the built-ins.
4. Broader Color Pipeline composition UI after field/source honesty and operator capability metadata stay stable across at least one new op slice.

## Hostile Audit

- Status: complete
- Required posture: assume recent sidecar, native-helper, picker/layout, mixed-source, and SDF pack-scene work each left at least one stale claim, missing runtime proof, hidden dev-only dependency, or unsupported lane overclaim.

Required questions:

- Did the sidecar branch actually merge to `master` and remain rearward-review `ok`?
- Did docs stop treating function picker/layout, helper optimization, or Capture Finding sidecar as unshipped?
- Did the plan avoid claiming mixed Source support for `sdf_pack_scene` or `generic_equation_pack` without source-signal producers?
- Did published-runtime SDF pack resolution stop depending on a dev-only repo-root fallback?
- Did capture/replay/report authority stay aligned across `state.json`, `fractal-state.json`, live runtime reports, and finding archives?
- Did every hot-path claim include measured before/after evidence?
- Did the sprint avoid new SDF ops, new lanes, Salticid adapter work, and physical mouse automation before the hardening phases are green?

## Audit Passes

- [x] Pass 1 - recent-work truth audit found stale status text still naming function picker/layout as the next step after it had shipped, and found the function-picker plan still showed final Phase 5 unchecked.
- [x] Pass 2 - SDF published-runtime packaging and capability-overclaim audit confirmed `generic_equation_pack` and `sdf_pack_scene` must remain fail-closed for mixed Source rows without renderer-backed non-SDF source-signal producers.
- [x] Pass 3 - clean re-read after truth sync found no remaining in-scope stale text in the touched SDF status surfaces for function picker/layout, Capture Finding sidecar, or mixed Source support boundaries.
- [x] Pass 4 - implementation audit found the runtime SDF pack loader still preferred repo-root metadata over staged runtime packs. Added RED/native proof, reversed priority, and staged pack JSON during runtime publish.
- [x] Pass 5 - report audit found existing fields did not identify field producer capability as a first-class surface. Added producer kind, supported signals, and fail-closed reason to the runtime report.
- [x] Pass 6 - field-primary source honesty audit found no runtime regression for supported mixed rows but added explicit published-runtime proof that `generic_equation_pack` and `sdf_pack_scene` mixed Source rows still fail closed.
- [x] Pass 7 - performance audit found the existing witness omitted Lens Field v2 and `sdf_pack_scene`. Added those rows to the witness and recorded fresh numbers without making an FPS-improvement claim.
- [x] Pass 8 - closure-receipt audit found `viewer_host_run_logged_command.py --out-json ...` commands were recorded only as text-log evidence, and the first contract assertions targeted a nonexistent `returncode` field instead of the logged-command `exit_code`. Added a focused workflow regression, changed receipt evidence mapping to prefer `--out-json`, and repaired the contract JSON paths while preserving text-log evidence for logged commands without JSON output.
- [x] Pass 9 - workflow-test audit found the older parameter-surface contract and inventory still expected 46 fractal lanes while schema/enum authority now expose 47, and the workflow test carried the same stale literal. Repaired the stale count and guardrail instead of leaving a known failing workflow test.

## Audit Findings

- [x] Finding: Color Pipeline function picker/layout was still described as the next selected product step in SDF status surfaces after it had landed. Updated `_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `sdf_field_pack_near_term_TODO.md`.
- [x] Finding: `docs/notes/color_pipeline_function_picker_layout_PHASED_PLAN.md` still had Phase 5 unchecked despite the handoff recording closure. Marked the plan complete.
- [x] Finding: the first sprint contract used an unsupported `logged_command_json` evidence kind. Repaired it to use validator JSON evidence and re-locked the contract.
- [x] Clean re-read: touched status surfaces now identify the active next SDF engine seam as published-runtime pack resolution and field/source honesty, while unsupported field-primary mixed Source lanes remain explicitly fail-closed.
- [x] Finding: published-runtime SDF built-ins were not staged and the loader used `fractal_ui_repo_root.txt` before checking the runtime folder. Fixed both and proved staged runtime packs win.
- [x] Finding: SDF report capability was implicit. Added explicit producer-kind/supported-signal/fail-closed report fields and runtime assertions for Lens Field v2 plus `sdf_pack_scene`.
- [x] Finding: field-primary mixed Source denial had no dedicated `sdf_pack_scene`/`generic_equation_pack` runtime test in the SDF pack lane file. Added one.
- [x] Finding: the performance witness did not include all current SDF producer classes. Extended it to cover Lens SDF, Lens Field v2, and `sdf_pack_scene`.
- [x] Finding: validation receipts could not prove required runtime publish/proof JSON assertions because logged commands with `--out-json` were recorded as log-only evidence and the sprint contract pointed at `returncode` instead of `exit_code`. Fixed the evidence mapper, repaired the contract assertion paths, and added a regression.
- [x] Finding: the full contract-proof workflow test found stale `fractal_parameter_surface_matrix` count drift after later catalog growth. Updated the contract, inventory, plan note, and test guardrail from the stale 46 assumption to the current schema/enum-consistency check with 47 lanes.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_engine_completion_sprint.contract.json --out-json artifacts/validation/sdf_engine_completion_sprint_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_engine_completion_sprint_PHASED_PLAN.md --out-json artifacts/validation/sdf_engine_completion_sprint_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_engine_completion_sprint_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_engine_completion_sprint_diff_check --log artifacts/logs/sdf_engine_completion_sprint_diff_check.log --out-json artifacts/validation/sdf_engine_completion_sprint_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_engine_completion_sprint_runtime_publish --log artifacts/logs/sdf_engine_completion_sprint_runtime_publish.log --out-json artifacts/validation/sdf_engine_completion_sprint_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- cmd /c ui_app\build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_engine_completion_sprint_runtime_proof --log artifacts/logs/sdf_engine_completion_sprint_runtime_proof.log --out-json artifacts/validation/sdf_engine_completion_sprint_runtime_proof.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_pack_scene_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`
- Product slices add focused native and published-runtime proofs before checkpoint.
