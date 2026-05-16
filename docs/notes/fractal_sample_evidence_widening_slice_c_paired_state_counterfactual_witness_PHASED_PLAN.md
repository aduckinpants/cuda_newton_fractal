# Fractal Sample Evidence Widening Slice C - Paired-State Counterfactual Witness

## Current Phase

Complete - slice C now has one real paired-state/counterfactual widened consumer, the hostile-audit defects are repaired, the restart-facing docs are truth-repaired, the focused touched-seam proofs are green, and this plan is the final closed-state surface for the checkpoint head. Machine receipts for this head are written after the commit rather than into the plan text itself.

## Phase Checklist

- [x] Phase 1 - open this checked-in slice-C plan and contract, replace the closed slice-B lock, and re-prove the RED head still has no real widened consumer while the sidecar and runtime-walk seams remain legacy-only.
- [x] Phase 2 - land one real widened consumer in the bounded sidecar paired-state or counterfactual lane while preserving `SampleFractalPoints(...)` and current legacy caller semantics.
- [x] Phase 3 - run the narrow validation ladder, complete hostile audit, checkpoint, write receipts, repair any stale closeout text, and leave the repo clean.

## Explicit User Asks

- [done] Open and execute sample-evidence widening slice C only: one real first widened consumer proof.
- [done] Prefer the bounded paired-state or counterfactual witness lane unless live repo proof forces a narrower truthful first consumer.
- [done] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [done] Do not widen into projection-and-flow, ExplainO-BalanceVoid, renderer replacement, generic engine rewrite, meta-basin, operator-itinerary, DSL/program-space, generic advanced-color follow-up, or historical explaino_inertial archaeology.
- [done] Close only with commit, validation receipt, contract proof receipt, clean tree, and a committed-head stale-plan reread.

## Proof Ledger

- Bootstrap this session re-proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=dbda4ae`, clean tree, and an active locked contract still pointing at the already closed slice-B contract `fractal_sample_evidence_widening_slice_b_dual_return_api_default_legacy_evidence`.
- Source-of-truth reread re-proved the live repo stop point from slice B: `ui_app/src/fractal_sample_result.h` carries `FractalSampleEvidence`, `ui_app/src/fractal_types.h` plus `ui_app/src/fractal_sample_core.cu` carry `SampleFractalEvidencePoints(...)`, and `SampleFractalPoints(...)` remains the shipped legacy projection path.
- Current single-orbit consumer authority on the RED head was still legacy-only: `ui_app/src/explaino_sidecar_cuda_sample_host.cpp` still called `SampleFractalPoints(...)`, `ui_app/src/explaino_sidecar_measurement.cpp` still aggregated only `FractalSampleResult`, and `ui_app/src/runtime_walk_field_slime.cpp` still consumed only legacy residual or iteration fields from positional result batches.
- Repo-wide search before slice-C mutation found `FractalSampleEvidence` and `SampleFractalEvidencePoints(...)` only in the sample core plus sample tests; no live sidecar, runtime-walk, renderer, or headless owner read widened evidence on the RED head.
- The sidecar measurement lane was the narrowest truthful first consumer on the RED head: `BuildSidecarMeasurementBatch(...)` already owned baseline versus minus/plus counterfactual sweeps over a fixed coordinate set, while renderer and runtime-walk stayed purely legacy and projection-and-flow or ExplainO-BalanceVoid would have widened scope further.
- Current widened payload sufficiency on the RED head was already enough for the chosen first consumer: the sidecar paired-state/counterfactual witness can be built from `sample_coord` plus nested `legacy_result`, so no wider evidence field was proven necessary.
- Checked-in RED landed in `artifacts/validation/fractal_sample_evidence_slice_c_red_host_compile.log`: the first focused compile failed on missing `CudaSidecarMeasurementHost::SupportsWidenedEvidence()` and `CudaSidecarMeasurementHost::SampleEvidence(...)`, which proved the widened-consumer host seam did not exist on the RED head.
- First GREEN now lives in `ui_app/src/explaino_sidecar_cuda_sample_host.h`, `ui_app/src/explaino_sidecar_cuda_sample_host.cpp`, `ui_app/src/explaino_sidecar_measurement.h`, and `ui_app/src/explaino_sidecar_measurement.cpp`: the CUDA sidecar host now exposes widened sampling, `BuildSidecarMeasurementBatch(...)` now consumes widened evidence when available, and each measurement row records paired `minus_counterfactual` / `plus_counterfactual` witness metrics without changing `SampleFractalPoints(...)` or `FractalSampleResult` behavior.
- Focused first-consumer proof is green in the touched tests: `ui_app/tests/test_explaino_sidecar_cuda_sample_host.cpp` proves the widened host path exists and keeps the legacy path separate, and `ui_app/tests/test_explaino_sidecar_measurement.cpp` proves the first real consumer reads widened evidence meaningfully, rejects `sample_coord` pairing drift, and does not force legacy callers onto the widened path.
- Continuity truth repair landed after the GREEN implementation: `docs/notes/fractal_sample_evidence_widening_launch_anchor.md`, `docs/notes/fractal_sample_evidence_widening_staging_PHASED_PLAN.md`, `docs/notes/fractal_sample_evidence_widening_slice_a_contract_skeleton_PHASED_PLAN.md`, and `docs/notes/fractal_sample_evidence_widening_slice_b_dual_return_api_default_legacy_evidence_PHASED_PLAN.md` now describe the live branch truth instead of still claiming the first widened consumer is deferred, and the temporary slice-C scope broadening was removed from `docs/contracts/fractal_sample_evidence_widening_staging.contract.json`.
- Hostile audit then found a real caller-compatibility defect in the broader helper lane: `ui_app/build_tests_vsdevcmd.cmd` initially failed in `artifacts/validation/fractal_sample_evidence_slice_c_build_tests.log` because `ui_app/tests/test_runtime_walk_headless.cpp` defined a local `CudaSidecarMeasurementHost` stub that no longer satisfied the class vtable after slice C added `SupportsWidenedEvidence()` and `SampleEvidence(...)`.
- The repaired compatibility witness now lives in `ui_app/tests/test_runtime_walk_headless.cpp`, which adds bounded stub definitions for the new widened virtuals without widening runtime-walk behavior itself.
- Repaired-state native proof is green on the touched seams: `ui_app/build_tests_vsdevcmd.cmd` now rebuilds and reruns `test_runtime_walk_headless`, `test_runtime_walk_field_slime`, `test_explaino_sidecar_measurement`, `test_explaino_sidecar_window`, `test_explaino_exploration_advisor`, and `test_explaino_sidecar_cuda_sample_host` successfully before later stopping at the unrelated pre-existing `test_safe_mode_schema` failure `TestSafeModeSchemaKeepsGroupedDefaults_DefaultExplaino`.
- Narrow direct reruns on the rebuilt executables are green from the `ui_app` working directory: `test_explaino_sidecar_cuda_sample_host` reports `24 passed, 0 failed`, `test_explaino_sidecar_measurement` reports `all passed`, `test_runtime_walk_headless` reports `passed=36 failed=0`, `test_runtime_walk_field_slime` reports `31 passed, 0 failed`, `test_explaino_sidecar_window` reports `all passed`, and `test_explaino_exploration_advisor` reports `all passed`.
- Machine validators are green on the repaired state before checkpoint closure: `artifacts/validation/fractal_sample_evidence_widening_slice_c_paired_state_counterfactual_witness_contract.json` has `contract_schema_valid=true`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` reports `ok=true`, and `artifacts/validation/fractal_sample_evidence_widening_slice_c_paired_state_counterfactual_witness_hostile_audit.json` reports `ok=true`.
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` is not part of the slice-C acceptance proof: it remains red on untouched `ui_app/src/escape_time_coloring.h` and `ui_app/src/fractal_family_rules.h`, so this slice closes on the narrower touched-seam proof ladder instead of falsely claiming a full native-profile green.
- Post-green hostile finding: a real compatibility defect and a real continuity-truth defect were found and repaired before closure; no new evidence fields were added beyond `sample_coord + legacy_result`.

## Hostile Audit

- Status: complete
- Required posture: assume the first widened consumer is fake, only renames helpers, accidentally replaces `SampleFractalPoints(...)`, silently widens into projection-and-flow or ExplainO-BalanceVoid, breaks existing caller proofs, or stops with stale closeout text until hostile proof disproves each failure mode.
- Current hostile result: two real defects were found and repaired during the slice, and the clean re-read of the repaired state did not find an additional real defect.

## Audit Passes

- [x] Pass 1 - re-proved the RED head had no real widened consumer and that the sidecar paired-state/counterfactual lane was still only aggregate-plus-positional legacy logic.
- [x] Pass 2 - landed the first GREEN implementation and re-read it as if it only renamed helpers or added speculative fields; the widened consumer stayed bounded to `sample_coord + legacy_result` and preserved `SampleFractalPoints(...)` as the shipped legacy projection path.
- [x] Pass 3 - hostile reread found and repaired the first real continuity defect: the launch anchor, staging packet, and closed slice-A/B headers still described the live branch as pre-consumer/deferred after slice C landed.
- [x] Pass 4 - broader helper rebuild found and repaired the second real defect: `test_runtime_walk_headless` no longer linked because its local `CudaSidecarMeasurementHost` stub did not implement the new widened virtuals.
- [x] Pass 5 - reran the repaired state through the broader helper build plus the narrow direct executables, re-read the touched seams for scope drift, and found no additional real defect.

## Audit Findings

- [x] Real continuity defect fixed: the restart-facing launch/staging packet plus the closed slice-A/B plan headers still claimed the live branch had no first widened consumer or still treated that consumer as deferred. Those authority surfaces now name the bounded sidecar paired-state/counterfactual consumer truthfully.
- [x] Real caller-compatibility defect fixed: `ui_app/tests/test_runtime_walk_headless.cpp` supplied only the old `CudaSidecarMeasurementHost::Sample(...)` stub, so the broader helper build started failing at link time after slice C added widened virtuals. The test now stubs `SupportsWidenedEvidence()` and `SampleEvidence(...)` explicitly and the repaired helper build passes that caller again.
- [x] Clean re-read evidence: the first widened consumer is real and callable, the chosen fields remain bounded to `sample_coord + legacy_result`, `SampleFractalPoints(...)` and `FractalSampleResult` remain the shipped legacy projection surfaces, projection-and-flow and `ExplainO-BalanceVoid` remain deferred, and no additional real defect was found.

## Notes

- Current single-orbit authority for shipped callers still remains in `ui_app/src/fractal_sample_result.h`, `ui_app/src/fractal_types.h`, `ui_app/src/fractal_sample_core.cu`, `ui_app/src/fractal_renderer.cu`, `ui_app/src/explaino_sidecar_cuda_sample_host.cpp`, `ui_app/src/explaino_sidecar_measurement.cpp`, and `ui_app/src/runtime_walk_field_slime.cpp`.
- No current internal consumer already required richer evidence before this slice: sidecar measurement and runtime-walk both consumed `FractalSampleResult` only, and counterfactual pairing was positional and aggregate-only rather than evidence-backed.
- Chosen first consumer: `BuildSidecarMeasurementBatch(...)` in the sidecar measurement lane because it already owns baseline versus plus/minus counterfactual sweep semantics and can justify widened evidence without touching renderer ownership or generic sample ownership.
- Exact widened fields required beyond `sample_coord + legacy_result`: none. Slice C reuses the bounded slice-A/B payload and proves that payload is sufficient for paired-state/counterfactual witness metrics.
- `SampleFractalPoints(...)` stays behaviorally unchanged in this slice because the widened consumer routes through `SampleFractalEvidencePoints(...)` only inside the bounded sidecar measurement host path while existing legacy callers remain on the current projection API.
- Focused validation ladder used before checkpoint closure: `artifacts/validation/fractal_sample_evidence_slice_c_red_host_compile.log`, `artifacts/validation/fractal_sample_evidence_slice_c_host_compile.log`, `artifacts/validation/fractal_sample_evidence_slice_c_measurement_compile.log`, `artifacts/validation/fractal_sample_evidence_slice_c_runtime_walk_compile.log`, `artifacts/validation/fractal_sample_evidence_slice_c_build_tests.log`, and the rebuilt direct test reruns for sidecar host/measurement, runtime-walk headless/field slime, and sidecar window/advisor.
