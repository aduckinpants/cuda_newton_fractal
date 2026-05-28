# SDF Row Field Resolution Authority REDs

## Current Phase

Closed - Step 3A authority model and RED matrix checkpointed for Step 3B handoff.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in Step 3A plan and contract.
- [x] Phase 2 - inspect current source-row, state, diagnostics, and runtime-report authority seams.
- [x] Phase 3 - document the proposed row-local field-resolution authority model.
- [x] Phase 4 - add a durable RED matrix proving current surfaces cannot express row-local SDF field downsample or multi-field grouping.
- [x] Phase 5 - validate contract, plan sync, RED matrix, hostile audit, code quality, and diff hygiene.
- [x] Phase 6 - checkpoint, write receipts, run rearward review, push, and stop before Step 3B implementation.

## Explicit User Asks

- [done] Start Step 3A from the planned SDF next-five campaign.
- [done] Keep the SDF performance/foundation work moving without building on an unclear field-resolution base.
- [done] Carry numbers and proof for hot-path work; for this Step 3A design slice, produce machine-readable RED evidence instead of claiming a performance improvement.

## Scope

In scope:

- Step 3A only: authority model and RED matrix for row-local SDF field resolution.
- Documentation of the proposed model:
  - default `inherit_shared` uses current `LensSettings::downsample`;
  - explicit row policy is a field downsample choice, not the existing postprocess sample step;
  - enabled SDF rows group by effective field downsample;
  - disabled rows request no field;
  - distinct live field groups are capped, initially at `4`.
- A headless/static RED matrix proving current shipped code cannot express the model yet.

Out of scope:

- Step 3B multi-field producer/cache implementation.
- Step 3C visible row-local UI/runtime productization.
- Any product behavior change to SDF postprocess, renderer, Lens SDF, capture/replay, or pacing.
- Runtime publish or no-mouse proof that would imply the feature is shipped.

## Proof Ledger

- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_row_field_resolution_authority_reds.contract.json --out-json artifacts/validation/sdf_row_field_resolution_authority_reds_contract.json` passed.
- Focused RED matrix direct run: `py -3.14 tools/sdf_row_field_resolution_red_matrix.py --repo-root . --out artifacts/validation/sdf_row_field_resolution_authority_red_matrix.json` reported `status=expected_red`, `red_count=7`, and `all_expected_gaps_present=true`.
- Focused pytest: `py -3.14 -m pytest tests/test_sdf_row_field_resolution_authority_reds.py -q` passed, `2 passed`.
- Logged plan sync: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_row_field_resolution_authority_reds_plan_sync --log artifacts/logs/sdf_row_field_resolution_authority_reds_plan_sync.log --out-json artifacts/validation/sdf_row_field_resolution_authority_reds_plan_sync.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/sdf_row_field_resolution_authority_reds_PHASED_PLAN.md` passed.
- Receipt-backed pytest JUnit rail: `py -3.14 -m pytest tests/test_sdf_row_field_resolution_authority_reds.py -q --junitxml artifacts/pytest/sdf_row_field_resolution_authority_reds.junit.xml` passed, `2 passed`.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_row_field_resolution_authority_reds_code_quality.json` passed at baseline score `93/100`.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_row_field_resolution_authority_reds_diff_check --log artifacts/logs/sdf_row_field_resolution_authority_reds_diff_check.log --out-json artifacts/validation/sdf_row_field_resolution_authority_reds_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_row_field_resolution_authority_reds_PHASED_PLAN.md --out-json artifacts/validation/sdf_row_field_resolution_authority_reds_hostile_audit.json` passed after the final status marker.
- Closeout discipline: checkpoint, receipts, rearward review, and push are required before final response; this plan closes only the Step 3A work product, not Step 3B implementation.

## Authority Model Candidate

This slice will document and test the first candidate model, not implement it:

- Add a source-row-local field-resolution policy to `ColorPipelineSourceRuntimeParams`.
- The default policy is `inherit_shared`, preserving the current single shared `LensSettings::downsample` authority and old state behavior.
- Explicit row policy uses field downsample values `1`, `2`, `4`, `8`, or `16`.
- Runtime field planning groups enabled SDF source rows by effective downsample.
- Non-SDF rows and disabled rows do not request SDF fields.
- Mixed SDF/non-SDF stacks keep the existing fail-closed behavior.
- Live frames cap distinct SDF field groups at `4` and report a fail-closed reason if the cap is exceeded.

## RED Matrix Targets

The RED matrix must distinguish current row-local postprocess sampling from the missing field-resolution authority:

- Current `signal.sdf_sample_step` is a per-row postprocess sampling/coarsening control, not a field producer downsample.
- Current state IO serializes row source params but has no row-local `sdf_field_downsample` or equivalent field policy.
- Current capture/state summaries serialize one shared `lens.downsample`, not per-row field groups.
- Current runtime reports expose requested/effective shared Lens SDF downsample, not row-local effective groups, field count, or per-group timings.
- Current UI exposes one shared `color_pipeline.source.sdf_field.downsample.primary`, not a row-local field policy.

## Hostile Audit

- Status: done

Required questions:

- Did this slice actually prove the current system cannot express row-local SDF field resolution?
- Did it avoid confusing `signal.sdf_sample_step` with true field downsample authority?
- Did it preserve the current shared Lens downsample default?
- Did it avoid Step 3B/3C product implementation?
- Did it leave capture/replay, Lens Field v2, SDF rows, pacing, and runtime behavior untouched?
- Did it checkpoint and push only after receipts and rearward review?

## Audit Passes

- [done] Pass 1 - found and repaired a real contract-shape/workflow defect: the first contract used an unsupported evidence kind and referenced future files before they existed.
- [done] Pass 2 - found and repaired a real proof defect: the initial helper was only a stub and did not prove the current field-authority gap.
- [done] Pass 3 - clean re-read of the repaired state found no additional real defect: no product C++ behavior mutation, no Step 3B/3C implementation, and no wording that claims row-local field resolution is shipped.

## Audit Findings

- [done] Finding 1 - real contract-shape defect: `viewer_host_begin_work_slice.py` rejected the first contract because `logged_command_json` is unsupported and the allowed mutation scope named missing future files. Repaired by creating the files before lock and using a supported assertion evidence kind.
- [done] Finding 2 - real proof defect: the initial RED helper only emitted `status=stub`, which could not prove the current shipped surfaces lacked row-local field authority. Repaired by scanning current runtime/state/UI/report seams and asserting seven expected-red rows plus two preservation rows.
- [done] Finding 3 - real receipt-shape defect: the first receipt attempt recorded the logged pytest rail as `text_log`, so the contract-proof assertion could not bind the RED pytest result. Repaired by switching the contract proof to direct pytest JUnit cases.
- [done] Clean re-read after final validation found no additional real issue: the touched files are the Step 3A contract, plan, authority model doc, RED helper, pytest wrapper, and handoff breadcrumb only; product SDF runtime files were inspected but not modified.
