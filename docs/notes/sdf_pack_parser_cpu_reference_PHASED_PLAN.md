# SDF Pack Parser CPU Reference

## Current Phase

Phase 5 - checkpoint, receipts, merge-back, push, clean-tree, and stale-plan closeout.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in plan/contract
- [x] Phase 2 - write RED native tests for v1 SDF pack parsing, deterministic rejection, and CPU sample sensitivity
- [x] Phase 3 - implement the bounded parser and CPU reference evaluator
- [x] Phase 4 - validate focused native rails and hostile-audit the repaired seam
- [ ] Phase 5 - checkpoint, receipts, merge-back, push, clean-tree, and stale-plan closeout

## Explicit User Asks

- [open] Continue the next forward work item after the interior policy regression hardening.
- [open] Keep this as the next SDF roadmap slice, not a broad renderer or UI rewrite.
- [open] Build a simple, extensible foundation that can support later CUDA SDF packs, Color Pipeline/probe consumption, viewport overlay, and SDF-native fractal lanes.
- [open] Preserve existing Lens SDF and Generic Equation Pack behavior.

## Scope

In scope:

- Add a v1 authored `sdf.pack` JSON parser beside the existing generic equation-pack tooling.
- Add a CPU reference evaluator for bounded 2D SDF ASTs.
- Support a first useful op subset: `circle`, `box`, `capsule`, `union`, `intersect`, `subtract`, `smooth_union`, `translate`, `rotate`, `scale`, `repeat`, and `param`.
- Add deterministic native tests for malformed packs, unknown fields/ops, duplicate params, nonfinite values, oversized ASTs/params, bounds, sample values, and parameter override sensitivity.
- Add at least one checked-in example pack under `docs/examples/sdf_packs/`.

Out of scope:

- CUDA SDF pack evaluation.
- Live viewport rendering or a new `FractalType`.
- Color Pipeline SDF signals.
- Probe/report consumption.
- Lens overlay productization.
- Dynamic CUDA kernel registration.
- Salticid adapter work.
- Physical mouse automation.

## Proof Ledger

- Bootstrap authority: current `master` was clean/even after interior policy schema hardening pushed at `28f98f4`.
- Next-work authority: `docs/notes/sdf_field_pack_near_term_TODO.md` lists Slice 4 as "Authored SDF Pack Parser And CPU Reference" after Lens SDF truth cleanup, SDF field interface extraction, and Lens semantics authority.
- Boundary authority: `docs/notes/generic_cuda_equation_pack_PAUSE_README.md` confirms the analogous equation-pack pattern is AST-authoritative and warns that dynamic kernel registration/live renderer rewrite are later work.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "sdf pack parser cpu reference" --profile native --plan docs/notes/sdf_pack_parser_cpu_reference_PHASED_PLAN.md --contract docs/contracts/sdf_pack_parser_cpu_reference.contract.json` passed and produced checkpoint token `ck:10ed1176`.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_parser_cpu_reference_test_sdf_pack_red --log artifacts/logs/sdf_pack_parser_cpu_reference_test_sdf_pack_red.log --out-json artifacts/validation/sdf_pack_parser_cpu_reference_test_sdf_pack_red.json --heartbeat-seconds 30 --timeout-seconds 240 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack` failed because `sdf_pack.cpp` and `sdf_pack.h` did not exist.
- Green proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_parser_cpu_reference_test_sdf_pack --log artifacts/logs/sdf_pack_parser_cpu_reference_test_sdf_pack.log --out-json artifacts/validation/sdf_pack_parser_cpu_reference_test_sdf_pack.json --heartbeat-seconds 30 --timeout-seconds 240 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack` passed with `test_sdf_pack: pass=26 fail=0`.
- Lens preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_parser_cpu_reference_test_lens_sdf --log artifacts/logs/sdf_pack_parser_cpu_reference_test_lens_sdf.log --out-json artifacts/validation/sdf_pack_parser_cpu_reference_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` passed with `test_lens_sdf: all passed`.
- Harness hardening: `test_sdf_pack` is exposed as a focused target and wired into the full native helper suite so full helper runs do not skip the new parser rail.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_pack_parser_cpu_reference.contract.json --out-json artifacts/validation/sdf_pack_parser_cpu_reference_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_pack_parser_cpu_reference_PHASED_PLAN.md --out-json artifacts/validation/sdf_pack_parser_cpu_reference_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_pack_parser_cpu_reference_code_quality.json` passed with baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_parser_cpu_reference_diff_check --log artifacts/logs/sdf_pack_parser_cpu_reference_diff_check.log --out-json artifacts/validation/sdf_pack_parser_cpu_reference_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Generic Equation Pack preservation proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_pack_parser_cpu_reference_test_generic_equation_pack --log artifacts/logs/sdf_pack_parser_cpu_reference_test_generic_equation_pack.log --out-json artifacts/validation/sdf_pack_parser_cpu_reference_test_generic_equation_pack.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_generic_equation_pack` passed with CPU `pass=23 fail=0` and CUDA `pass=54 fail=0`.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the parser rejects unknown root keys, unknown AST ops, nonfinite params, duplicate params, out-of-range overrides, and oversized ASTs instead of falling back.
- [x] Pass 2 - verified the CPU reference evaluator is independent of Lens SDF RGBA presentation and `test_lens_sdf` still passes.
- [x] Pass 3 - clean re-read of the repaired diff confirmed no CUDA evaluator, live viewport lane, Color Pipeline SDF signal, selector/view preset, camera/dive, or smooth-escape tuning slipped into this slice.

## Audit Findings

- [x] Real finding - after the focused target went green, `test_sdf_pack` was not yet wired into the full native helper suite; the build script now compiles and runs it during full helper runs.
- [x] Clean re-read - the repaired state is parser/CPU-reference only and does not touch renderer, viewport, Color Pipeline, Lens SDF implementation, selector/view presets, camera/dive, or smooth-escape behavior.

## Action Hostile Review

- Action ID: sdf-pack-parser-cpu-reference-red
- Suspected failure mode: authored SDF pack work could blur into Lens SDF, dynamic CUDA kernels, or live renderer integration before a deterministic source-of-truth schema and CPU reference exist.
- Correct owner/action: add a bounded native parser and CPU reference evaluator with fail-closed tests first.
- Proof surface: focused `test_sdf_pack`, existing `test_lens_sdf`, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: CUDA SDF pack evaluator, viewport integration, Color Pipeline SDF signals, SDF-native fractal lanes, renderer rewrite, or physical mouse automation.
