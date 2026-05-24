# SDF Pack CUDA Evaluator

## Current Phase

Phase 5 - checkpoint, receipts, merge-back, push, clean-tree, and stale-plan closeout.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in plan/contract
- [x] Phase 2 - write RED native CUDA parity tests for primitive, transform, combinator, and bounded nested SDF packs
- [x] Phase 3 - implement bounded runtime lowering plus CUDA sampling while preserving the CPU reference path
- [x] Phase 4 - validate focused native preservation rails and hostile-audit the repaired seam
- [ ] Phase 5 - checkpoint, receipts, merge-back, push, clean-tree, and stale-plan closeout

## Explicit User Asks

- [closed] Continue the next planned SDF field-pack work item.
- [closed] Keep the work simple, modular, and extensible instead of creating a renderer monolith.
- [closed] Prove behavior with deterministic tests and no physical mouse automation.
- [closed] Preserve Generic Equation Pack, Lens SDF, and Color Pipeline behavior while adding the CUDA SDF evaluator substrate.

## Scope

In scope:

- Lower the existing v1 authored SDF pack AST into a bounded device-friendly runtime descriptor.
- Add a CUDA sampling path for SDF packs.
- Add CPU/CUDA parity tests for primitives, transforms, combinators, parameter overrides, and bounded nested compositions.
- Add one bounded CUDA performance witness that samples a representative grid and reports a checksum/timing without claiming product FPS.
- Keep `SampleSdfPackCpu(...)` as the CPU reference authority.
- Keep this as a headless native substrate slice.

Out of scope:

- Live viewport integration.
- New `FractalType` registration.
- Color Pipeline SDF signals.
- Probe/report consumption.
- Lens overlay productization.
- Salticid adapter work.
- Dynamic CUDA kernel registration.
- Physical mouse automation.

## Proof Ledger

- Bootstrap authority: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-pack-cuda-evaluator` at `9d858eb`; tree was clean and the prior active contract was the already-closed SDF parser slice.
- Roadmap authority: `docs/notes/sdf_field_pack_near_term_TODO.md` lists Slice 5 as "CUDA SDF Pack Evaluator" after the authored SDF pack parser and CPU reference.
- Boundary authority: the previous SDF parser CPU reference plan closed with CUDA evaluation explicitly out of scope, so this slice starts that next bounded layer without changing viewport/product surfaces.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "sdf pack cuda evaluator" --profile native --plan docs/notes/sdf_pack_cuda_evaluator_PHASED_PLAN.md --contract docs/contracts/sdf_pack_cuda_evaluator.contract.json` passed and produced checkpoint token `ck:4e31f8cb`.
- Contract preflight: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_pack_cuda_evaluator.contract.json --out-json artifacts/validation/sdf_pack_cuda_evaluator_contract_preflight.json` passed.
- Plan preflight: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for this dirty slice plan.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_red --log artifacts/logs/sdf_pack_cuda_evaluator_red.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack_cuda` failed because `ui_app/src/sdf_pack_cuda.cu` does not exist yet.
- CUDA parity proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_test_sdf_pack_cuda --log artifacts/logs/sdf_pack_cuda_evaluator_test_sdf_pack_cuda.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_test_sdf_pack_cuda.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack_cuda` passed after audit hardening with `test_sdf_pack_cuda: pass=19293 fail=0`; it covers primitive/combinator, transform/parameter overrides, nested smooth-repeat composition, invalid descriptor fail-fast, nonfinite descriptor fail-fast, and a 19,200-point CUDA performance witness (`elapsed_ms=0.524`, checksum `1190.354039027` in this run).
- CPU reference preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_test_sdf_pack --log artifacts/logs/sdf_pack_cuda_evaluator_test_sdf_pack.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_test_sdf_pack.json --heartbeat-seconds 30 --timeout-seconds 240 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack` passed with `test_sdf_pack: pass=26 fail=0`.
- Lens SDF preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_test_lens_sdf --log artifacts/logs/sdf_pack_cuda_evaluator_test_lens_sdf.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` passed with `test_lens_sdf: all passed`.
- Generic Equation Pack preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_test_generic_equation_pack --log artifacts/logs/sdf_pack_cuda_evaluator_test_generic_equation_pack.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_test_generic_equation_pack.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_generic_equation_pack` passed with CPU `pass=23 fail=0` and CUDA `pass=54 fail=0`.
- Color Pipeline preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_color_pipeline_owner --log artifacts/logs/sdf_pack_cuda_evaluator_color_pipeline_owner.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_color_pipeline_owner.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed with `test_color_pipeline_core`, `test_color_pipeline_window`, `test_schema_binding`, `test_escape_time_coloring`, `test_diagnostics_state_io`, `test_finding_archive_actions`, and `test_runtime_reset`.
- Full native helper proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_cuda_evaluator_full_native --log artifacts/logs/sdf_pack_cuda_evaluator_full_native.log --out-json artifacts/validation/sdf_pack_cuda_evaluator_full_native.json --heartbeat-seconds 30 --timeout-seconds 1800 -- cmd /c ui_app\build_tests_vsdevcmd.cmd` passed; the log shows `test_sdf_pack_cuda: pass=19293 fail=0` and ends with `All helper tests passed`.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - found the new `test_sdf_pack_cuda` focused rail was not wired into the full helper suite; repaired the full helper compile/run path and proved it in `sdf_pack_cuda_evaluator_full_native`.
- [x] Pass 2 - found `SampleSdfPackCuda(...)` could return host-level success for a structurally invalid manual descriptor while only marking per-point errors; added invalid/nonfinite descriptor regressions and tightened runtime descriptor validation.
- [x] Pass 3 - clean re-read of the repaired state confirmed the slice adds only SDF pack runtime lowering/evaluation/tests and does not change Generic Equation Pack behavior, Lens SDF behavior, Color Pipeline behavior, viewport integration, renderer routing, selector defaults, camera/dive, or smooth-escape tuning.

## Audit Findings

- [x] Real finding - the focused CUDA SDF pack rail was callable directly but absent from the full native helper suite, so a broad helper run could have skipped the new CUDA substrate proof.
- [x] Real finding - the CUDA sample API needed host-boundary descriptor validation for invalid child topology and nonfinite descriptor constants instead of relying on per-point device errors.
- [x] Clean re-read - after the repairs, focused CUDA proof, full native helper proof, and preservation rails did not expose another SDF, Generic Equation Pack, Lens SDF, Color Pipeline, renderer, selector, camera, or smooth-escape issue in this slice.

## Action Hostile Review

- Action ID: sdf-pack-cuda-evaluator-start
- Suspected failure mode: a CUDA substrate can accidentally become a docs-only stub, share no authority with the CPU parser, or widen into live viewport/renderer work before parity exists.
- Correct owner/action: add a bounded runtime descriptor and CUDA sample path beside the existing SDF pack parser, with CPU reference parity as the proof rail.
- Proof surface: focused `test_sdf_pack_cuda`, existing `test_sdf_pack`, `test_generic_equation_pack`, Color Pipeline focused rails, `test_lens_sdf`, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: live viewport integration, new fractal type, Color Pipeline SDF signals, probe consumption, renderer rewrite, dynamic kernel registration, or physical mouse automation.
