# Capture Finding Fractal-State Sidecar

## Current Phase

Phase 4 - Runtime proof, hostile audit, checkpoint, receipts, and push

## Phase Checklist

- [x] Phase 1 - RED and contract setup
- [x] Phase 2 - Sidecar writer and archive metadata
- [x] Phase 3 - Authority hardening for Color Pipeline, Lens/SDF, and SDF packs
- [x] Phase 4 - Runtime proof, hostile audit, checkpoint, receipts, and push

## Explicit User Asks

- [closed] Keep Capture Finding `state.json` as the normal replay authority.
- [closed] Add `fractal-state.json` beside Capture Finding archives to show the exact values that mattered for that capture.
- [closed] Keep unrelated hidden/default parameter groups out of the review sidecar.
- [closed] Preserve existing diagnostic capture and Capture Finding replay behavior.
- [closed] Prove the new sidecar with native, Python archive, and published no-mouse runtime rails.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` showed branch `codex/native-helper-build-optimization`, `HEAD=32a6fe0`, clean tree, active contract `native_helper_build_optimization`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `32a6fe0`.
- Branch hygiene: `master` fast-forwarded from `bb987a6` to `32a6fe0`, pushed to `origin/master`, then `codex/capture-finding-fractal-state-sidecar` was created.
- RED: native and Python tests initially failed on missing `BuildFindingFractalStateJson(...)`, missing explicit archive sidecar path support, and missing `finding.json.fractal_state_file`.
- Implementation: Capture Finding now writes `fractal-state.json` into the diagnostics bundle, passes it explicitly to the archive helper, and archives it beside `state.json` without changing diagnostic/replay `state.json`.
- Authority hardening: sidecar records active fractal controls, active Color Pipeline Source/Shape/Palette/Grading stacks, Lens/SDF values when SDF rows affect output, and SDF pack viewer state when the pack append path runs.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_finding_fractal_state_sidecar_native_focused --log artifacts/logs/capture_finding_fractal_state_sidecar_native_focused.log --out-json artifacts/validation/capture_finding_fractal_state_sidecar_native_focused.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_finding_archive_actions test_diagnostics_capture test_diagnostics_state_io` passed.
- Python proof: `py -3.14 -m pytest tests/test_fractal_finding_capture.py -q --junitxml artifacts/pytest/capture_finding_fractal_state_sidecar_finding_capture.junit.xml` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_finding_fractal_state_sidecar_runtime_publish --log artifacts/logs/capture_finding_fractal_state_sidecar_runtime_publish.log --out-json artifacts/validation/capture_finding_fractal_state_sidecar_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed.
- Published runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_finding_fractal_state_sidecar_runtime_proof --log artifacts/logs/capture_finding_fractal_state_sidecar_runtime_proof.log --out-json artifacts/validation/capture_finding_fractal_state_sidecar_runtime_proof.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_sdf_rows.py -q` passed with 11 no-mouse runtime tests.
- Published runtime JUnit proof: `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_sdf_rows.py::test_capture_finding_preserves_sdf_source_row_pixels_no_mouse tests/test_fractal_runtime_color_pipeline_sdf_rows.py::test_capture_finding_replays_live_sdf_field_resolution_for_multi_row_stack_no_mouse -q --junitxml artifacts/pytest/capture_finding_fractal_state_sidecar_runtime_capture.junit.xml` passed.
- Code quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_fractal_state_sidecar_code_quality.json` passed after the oversized helper audit finding was fixed.
- Diff proof: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_finding_fractal_state_sidecar_diff_check --log artifacts/logs/capture_finding_fractal_state_sidecar_diff_check.log --out-json artifacts/validation/capture_finding_fractal_state_sidecar_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Questions:
  - Did this slice leave `state.json` replay-compatible? Yes: `state.json` is still produced by the existing broad serializer and `test_diagnostics_state_io` plus runtime replay rails stayed green.
  - Did `fractal-state.json` report live active Color Pipeline rows instead of stale top-level mirrors? Yes: native and runtime checks inspect Source-stack rows from live state, including SDF gates/downsample.
  - Did Lens/SDF and row-local SDF field settings match the captured output? Yes: runtime Capture Finding assertions compare sidecar Lens field dimensions to captured/replayed state.
  - Did SDF pack viewer state appear when it affected the capture? Yes: the existing SDF pack state append path now also updates sibling `fractal-state.json` when present.
  - Did the sidecar omit unrelated inactive family parameter groups? Yes: Julia/SDF and Explaino tests prove Magnet/Explaino inactive groups are omitted from active controls.
  - Did diagnostic capture remain unchanged? Yes: the broad `state.json` path is untouched; sidecar creation is only wired through Capture Finding archive flow.
  - Did the published runtime proof exercise Capture Finding without physical mouse automation? Yes: the proof uses the persistent no-mouse harness and in-process `capture_finding` control.

## Audit Passes

- [x] Pass 1 - Found code-quality regression: `WriteFindingActiveControlsJson(...)` was too large and lowered the quality baseline.
- [x] Pass 2 - Found sidecar authority gap: Explaino-family captures would have underreported shared seed/mix/root controls.
- [x] Pass 3 - Rechecked archive wiring, sidecar content, runtime proof, and code-quality baseline after both repairs.

## Audit Findings

- [x] Finding: oversized sidecar active-control writer split into smaller helpers; code-quality baseline passes.
- [x] Finding: Explaino-family sidecar active controls expanded to include shared Explaino controls, roots, and Explaino-all axis controls; native regression added.
- [x] Clean re-read: after both repairs, archive wiring, sidecar content, runtime proof, and code-quality baseline were revalidated with no remaining in-scope defect.

## Notes

Scope:
- Add a review-focused `fractal-state.json` sidecar to Capture Finding archives only.
- Keep `state.json` as complete replay authority and keep diagnostic capture broad.
- Include active fractal controls, camera/view, render context, active Color Pipeline stacks, Lens/SDF authority, and SDF pack authority where relevant.

Out of scope:
- Trimming or replacing `state.json`.
- Changing `state_version` or state loading semantics.
- Reworking presets, recipes, full export format, or finding analyzer UI.
- New fractal types, new SDF ops, or Color Pipeline composition semantics.
