# Deploy Diagnostics Disk Pressure

## Current Phase

Phase 5 complete - deploy publish filtering and safe diagnostics cleanup are implemented and validated

## Phase Checklist

- [x] Phase 1 - Audit deploy and disk-pressure surfaces
- [x] Phase 2 - Add RED manifest/cleanup proof for diagnostics and findings bulk
- [x] Phase 3 - Tighten deploy/publish copy rules and safe cleanup behavior
- [x] Phase 4 - Validate focused rails and run hostile audit
- [x] Phase 5 - Checkpoint, receipts, push, and rearward review

## Explicit User Asks

- [done] Stop deploy/publish from copying gigabytes of diagnostics and findings that are not needed for the runtime/test proof path.
- [done] Clean unnecessary old generated diagnostic data without deleting user-owned captures or D: findings.
- [done] Add regression coverage so this bloat cannot silently return.
- [done] Keep the currently-tested SDF feature behavior untouched.

## Proof Ledger

- Bootstrap/rearward review: current clean a57ccc6 rearward review returned ok before branch creation.
- RED deploy-manifest proof: py -3.14 -m pytest tests/test_viewer_host_deploy_disk_pressure.py -q failed before implementation with missing tools.viewer_host_deploy_disk_pressure.
- GREEN deploy-manifest proof: py -3.14 -m pytest tests/test_viewer_host_deploy_disk_pressure.py -q passed with 3 tests after adding the filtered manifest, cleanup helper, and publish-wrapper smoke.
- Publish dry-run proof: powershell -NoProfile -ExecutionPolicy Bypass -File tools\\publish_repo_artifacts.ps1 -PublishRoot $env:TEMP\\viewer_host_publish_test -Label disk_pressure_smoke -WhatIf returned a filtered publish path.
- Cleanup dry-run receipt: artifacts/validation/deploy_diagnostics_disk_pressure_cleanup_dry_run.json found 3,370 generated diagnostic bundle candidates totaling 8,002,910,878 bytes.
- Cleanup execution receipt: artifacts/validation/deploy_diagnostics_disk_pressure_cleanup_execute.json deleted those 3,370 generated timestamped diagnostic bundles.
- Post-cleanup size witness: D:\\salt-fractal\\cuda_newton_fractal_clone\\runtime\\diagnostics dropped from 7.70 GiB to about 0.25 GiB; D:\\salt-fractal\\cuda_newton_fractal_clone\\findings remained about 0.69 GiB and was not pruned.
- Manifest proof: artifacts/validation/deploy_diagnostics_disk_pressure_manifest.json has zero diagnostics/findings source entries.
- Contract validation: artifacts/validation/deploy_diagnostics_disk_pressure_contract.json passed.
- Plan sync: artifacts/validation/deploy_diagnostics_disk_pressure_plan_sync.json passed after the plan text repair.
- Logged focused pytest: artifacts/validation/deploy_diagnostics_disk_pressure_pytest.json passed with 3 tests.
- Code quality baseline: artifacts/validation/deploy_diagnostics_disk_pressure_code_quality.json passed.
- Diff check: artifacts/validation/deploy_diagnostics_disk_pressure_diff_check.json passed.
- Publish script hardening: tools/publish_repo_artifacts.ps1 now contains only the filtered helper shim; the historical broad Move-Item body was removed.
- Post-green hostile finding: found and repaired malformed plan text left from the bootstrap patch trailer.

## Hostile Audit

- Status: complete
- Required questions:
  - Did the deploy path actually stop copying diagnostics and findings bulk? Current answer: yes for the repo publish helper; filtered manifest includes no diagnostics/findings entries.
  - Did cleanup avoid deleting user-owned findings or manual captures? Current answer: yes; cleanup only targets timestamped runtime/diagnostics/<timestamp>__diagnostic_<pid> directories.
  - Did runtime publish still include all required viewer files? Current answer: helper preserves runtime exe/imgui/build/schema-adjacent selected outputs; publish wrapper dry-run and manifest proof passed.
  - Did a regression test or manifest audit prove the exclusion? Current answer: yes, focused pytest plus generated manifest proof.
  - Did this slice avoid changing SDF/fractal behavior? Current answer: yes; touched tooling/docs/tests only.

## Audit Passes

- [done] Pass 1 - found a real workflow defect: malformed bootstrap patch text leaked into the phased plan after the initial plan/contract creation.
- [done] Pass 2 - focused validations passed after repairing the plan text.
- [done] Pass 3 - reread the final diff and cleanup receipts before checkpoint; no SDF/fractal runtime behavior changes were introduced.

## Audit Findings

- [done] Real defect found and repaired: the plan file contained stray unified-diff trailer text after the initial contract bootstrap patch only applied the first file cleanly.
- [done] Publish-wrapper risk removed: tools/publish_repo_artifacts.ps1 no longer carries the historical broad Move-Item body; focused smoke proves the remaining shim routes through the filtered helper.
- [done] No additional real defect found after final validation and receipt review.

## Notes

### Scope

This is a workflow/tooling cleanup slice. It may touch deploy/publish manifests, cleanup helpers, tests, and continuity docs. It must not change fractal renderer behavior, SDF pack scene behavior, Color Pipeline behavior, or runtime UI semantics.

### Surfaces Found

- tools/publish_repo_artifacts.ps1 was the deploy/publish script with broad historical move specs for whole artifacts and ui/diagnostics trees.
- D:\\salt-fractal\\cuda_newton_fractal_clone\\runtime\\diagnostics held the actual 7.70 GiB pressure point, mostly generated timestamped diagnostic archives.
- D:\\salt-fractal\\cuda_newton_fractal_clone\\findings is user/product output and was intentionally not cleaned.
- The C: checkout had about 303 MiB in repo-local artifacts, not the 8 GiB pressure source.

### Acceptance Criteria

- Deploy/publish manifest excludes stale diagnostics archives and findings/manual-capture bulk by default.
- Required runtime files remain present after publish/build.
- A focused test or manifest audit fails if diagnostics/findings bulk re-enters the deploy package.
- Cleanup helper defaults to dry-run and only deletes allow-listed generated repo-local/runtime diagnostic data when explicitly asked.
- Any actual cleanup performed in this session is reported with before/after sizes and exact paths.
- D: captures and user findings are not deleted or pruned by this slice.

### Validation Targets

- py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/deploy_diagnostics_disk_pressure.contract.json --out-json artifacts/validation/deploy_diagnostics_disk_pressure_contract.json
- py -3.14 -m pytest tests/test_viewer_host_deploy_disk_pressure.py -q
- py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/deploy_diagnostics_disk_pressure_PHASED_PLAN.md
- powershell -NoProfile -ExecutionPolicy Bypass -File tools\\publish_repo_artifacts.ps1 -PublishRoot $env:TEMP\\viewer_host_publish_test -Label disk_pressure_smoke -WhatIf
- py -3.14 tools/code_quality_audit.py --check-baseline
- git diff --check

### Closure Notes

- This slice is workflow-only. It does not need viewer runtime proof because it does not change viewer behavior.
