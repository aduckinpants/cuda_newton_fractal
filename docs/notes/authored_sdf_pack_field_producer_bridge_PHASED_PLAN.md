# Authored SDF Pack Field Producer Bridge

## Current Phase

Phase 6 - validation is complete; checkpoint, receipts, rearward review, push, and clean-tree proof remain.

## Phase Checklist

- [x] Phase 1 - open the checked-in Step 5A plan/contract and lock the active slice.
- [x] Phase 2 - inspect existing SDF pack, CUDA sampler, Lens SDF field, and field-signal seams; add RED producer tests.
- [x] Phase 3 - implement the CPU authored-pack field producer with explicit region mapping and field-pixel distance conversion.
- [x] Phase 4 - implement the explicit CUDA backend and CPU/CUDA parity rails.
- [x] Phase 5 - prove existing SDF pack and Lens SDF behavior is preserved.
- [ ] Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [active] Continue down the planned SDF list after merging the SDF Normal Angle repair.
- [active] Start the authored SDF pack field-producer bridge as the next forward slice.
- [active] Keep this foundational: no UI churn, no new fractal lane, and no Color Pipeline redesign in this step.

## Scope

In scope:

- Add an authored SDF pack producer that emits the existing `SdfFieldResult`/`SdfFieldView` style field interface.
- Keep `mask_derived` Lens SDF as the existing shipped producer.
- Add an explicit field source kind for authored SDF packs.
- Keep CPU reference behavior available for tests and fallback.
- Add an explicit CUDA backend and parity proof through the existing `SampleSdfPackCuda` evaluator.
- Convert pack world-distance samples into field-pixel signed distances so existing SDF signal consumers keep their current units.

Out of scope:

- Viewer UI for loading/authored packs.
- Color Pipeline or overlay consumption of authored fields.
- SDF-native fractal lanes.
- Per-source downsample behavior.
- Renderer replacement or live viewport integration.

## Owner Seams

- `ui_app/src/sdf_pack.h` and `ui_app/src/sdf_pack.cpp`: existing pack parser, CPU sampler, and lowerer.
- `ui_app/src/sdf_pack_cuda.h` and `ui_app/src/sdf_pack_cuda.cu`: existing CUDA sampler for lowered packs.
- `ui_app/src/lens_sdf.h`: existing shared SDF field result/view surface.
- New producer seam: `ui_app/src/sdf_pack_field_producer.h` plus CPU/CUDA implementation files.
- Test/build seam: `ui_app/build_tests_vsdevcmd.cmd` and focused native producer tests.

## Proof Ledger

- Started slice under checkpoint `ck:7c4a4255` on branch `codex/authored-sdf-pack-field-producer-bridge`.
- RED rail: `authored_sdf_pack_field_producer_bridge_red_cpu` failed before implementation because `sdf_pack_field_producer.cpp` and `sdf_pack_field_producer.h` did not exist.
- CPU field-producer rail: `authored_sdf_pack_field_producer_bridge_native_cpu` passed (`test_sdf_pack_field_producer: pass=42 fail=0`). It proves pack region mapping, field-pixel unit conversion, sign/source authority, param overrides, CPU-only dispatcher behavior, auto fallback, explicit-CUDA fail-closed behavior without a registered backend, and fail-closed invalid requests.
- CUDA parity rail: `authored_sdf_pack_field_producer_bridge_native_cuda` passed (`test_sdf_pack_field_producer_cuda: pass=600 fail=0`). It proves CPU/CUDA field parity, explicit CUDA backend reporting, auto backend CUDA preference, and dispatcher CPU path.
- Existing SDF pack preservation: `authored_sdf_pack_field_producer_bridge_pack_preservation` passed (`test_sdf_pack_cuda: pass=19315 fail=0`).
- Existing Lens SDF preservation: `authored_sdf_pack_field_producer_bridge_lens_preservation` passed (`test_lens_sdf_cuda: all passed`).
- Contract validation: `authored_sdf_pack_field_producer_bridge_contract` passed.
- Plan sync: `authored_sdf_pack_field_producer_bridge_plan_sync` passed.
- Hostile audit: complete with two real findings and one clean re-read.
- Hostile-audit validation: `authored_sdf_pack_field_producer_bridge_hostile_audit` passed.
- Code-quality baseline: `authored_sdf_pack_field_producer_bridge_code_quality` passed (`Score: 93/100`, `CRITICAL: 0`, `ERROR: 0`).
- Diff check: `authored_sdf_pack_field_producer_bridge_diff_check` passed.
- Closure workflow: checkpoint commit, receipts, rearward review, push, and clean-tree proof are the remaining repo-command epilogue after this committed plan text.

## Hostile Audit

- Status: complete

Required questions:

- Did this actually add a reusable authored-pack field producer, or only another one-off test helper?
- Does the producer preserve existing SDF field units and sign convention?
- Does CUDA parity prove the same field values as CPU on representative authored packs?
- Did the existing mask-derived Lens SDF producer remain untouched in behavior?
- Did this slice accidentally add UI, Color Pipeline consumption, or SDF-native lane work?

## Audit Passes

- [x] Pass 1 - Found a real API/linkage defect: the backend dispatcher originally lived only in the CUDA translation unit, so CPU-only headless callers could not safely use the public backend API. Fixed by moving the dispatcher and backend registry into C++ and registering the CUDA producer from the CUDA translation unit.
- [x] Pass 2 - Found a real coverage gap: CPU-only builds proved auto fallback but did not prove explicit CUDA requests fail closed when no CUDA backend is registered. Fixed with a native test that explicit `cuda_sample` fails and reports missing backend registration.
- [x] Pass 3 - Clean re-read found no additional real defect in this slice: authored packs now emit shared SDF fields, CPU and CUDA agree, field distances use field-pixel units, existing mask-derived Lens SDF behavior is preserved, and no UI/Color Pipeline/SDF-native lane work was added.

## Audit Findings

- [x] Real finding: public backend dispatcher was tied to the CUDA object. Fixed with a C++ registry/dispatcher and static CUDA backend registration.
- [x] Real finding: explicit CUDA fail-closed behavior was untested in CPU-only builds. Fixed with native dispatcher coverage.
- [x] Clean re-read: no additional defect found after rerunning producer and preservation rails.
