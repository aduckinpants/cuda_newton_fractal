# PFC Explaino Collatz Direct

## Explicit User Asks

- Continue the parameter functionality campaign from the pushed holder branch.
- Add an Explaino Collatz direct variant after standalone Collatz control behavior is proven.
- Preserve the existing `explaino_collatz` Newton/fixed-point lane.
- Keep `collatz_transition_strength` owner-lane scoped; do not add it to `explaino_all`.
- Preserve Explaino selector identity and existing Explaino-all registry/common-axis behavior.
- Use no-mouse persistent runtime proof and avoid repeated viewer relaunch loops.
- Do not touch Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broad engine redesign in this slice.

## Current Phase

Phase 1 is complete: implementation, validation, and hostile audit are complete for this slice.

## Phase Checklist

- [x] Bootstrap and confirm the campaign holder was clean before this feature branch.
- [x] Create feature branch `codex/pfc-explaino-collatz-direct` from `codex/parameter-functionality-campaign`.
- [x] Inspect the existing standalone Collatz, existing Explaino Collatz Newton lane, selector identity, schema, binding, validation, probe, and sample seams.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this Explaino Collatz direct contract.
- [x] RED: prove `explaino_collatz_direct` is absent from the public selector/runtime surface.
- [x] RED: prove native schema/sample/probe rails do not yet cover a direct Explaino Collatz lane.
- [x] Implement `explaino_collatz_direct` as a new explicit selector without changing the existing `explaino_collatz` Newton semantics.
- [x] Make `collatz_transition_strength` visible and authoritative on `collatz` and `explaino_collatz_direct`, but not on `explaino_all`.
- [x] Wire selector identity, schema/binding, safe-mode schema, validation, descriptor/catalog, native sample/probe, and runtime math.
- [x] Validate focused schema/native/sample/probe/function-descriptor rails.
- [x] Publish runtime and run one persistent no-mouse viewer proof for selector identity plus frame-hash sensitivity.
- [x] Run full native helper suite if focused rails pass.
- [x] Hostile audit the diff and repair real findings.
- [x] Prepare checkpoint, validation receipts, contract-proof receipt, push, and clean-tree closure.

## Owner Seams

- UI schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Fractal enum/string authority: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`.
- Family/selector/coloring authority: `ui_app/src/fractal_family_rules.h`, `ui_app/src/fractal_derived_fields.cpp`.
- Runtime validation: `ui_app/src/fractal_runtime_validation.h`.
- Direct Collatz formula authority: `ui_app/src/escape_time_specialized_formulas.h`.
- Device runtime: `ui_app/src/fractal_sample_device.inl`.
- Host/probe runtime: `ui_app/src/fractal_probe_runner.cpp`.
- Descriptor/catalog visibility: `ui_app/src/function_descriptor.cpp`.
- Safe-mode schema: `ui_app/src/safe_mode_schema.cpp`.
- Native proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, `ui_app/tests/test_fractal_family_rules.cpp`, `ui_app/tests/test_fractal_sample_kernel.cu`, and probe/descriptor tests.
- Runtime proof: `tests/test_fractal_runtime_explaino_collatz_direct.py` and related published-runtime descriptor checks.

## Design Boundary

- Add a new selector, `explaino_collatz_direct`, rather than repurposing the existing `explaino_collatz` Newton/fixed-point selector.
- The direct selector uses the standalone Collatz escape-time step with Explaino start/warp authority.
- `collatz_transition_strength` remains an owner-lane formula control: visible on `collatz` and `explaino_collatz_direct`, hidden on `explaino_all`.
- `explaino_collatz` keeps its current Newton/fixed-point controls and semantics.
- Common Explaino controls must be visible only when they are actually consumed by the direct path.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-explaino-collatz-direct`.
- Starting head: `6ff4ef9`.
- Standalone Collatz prerequisite: Step 1 closed on the holder; `collatz_transition_strength` already has native and no-mouse runtime proof on the standalone `collatz` lane.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "parameter functionality campaign slice 7 Explaino Collatz direct variant" --profile runtime --plan docs/contracts/pfc_explaino_collatz_direct_PHASED_PLAN.md --contract docs/contracts/pfc_explaino_collatz_direct.contract.json` produced `ck:160ff1c0`.
- RED native coverage proof: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_red_schema --log artifacts/logs/pfc_explaino_collatz_direct_red_schema.log --out-json artifacts/validation/pfc_explaino_collatz_direct_red_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed before implementation because `FractalType::explaino_collatz_direct` did not exist.
- Implementation fact: `FractalType::explaino_collatz_direct` is append-only ordinal `45`, has enum/string/schema/safe-mode/catalog support, is an Explaino selector, is an escape-time family, and is not a basin-coloring family.
- Implementation fact: existing `FractalType::explaino_collatz` remains the Newton/fixed-point lane; the new direct branch is separate in both `fractal_sample_device.inl` and `fractal_probe_runner.cpp`.
- Implementation fact: `collatz_transition_strength` is visible and authoritative on `collatz` and `explaino_collatz_direct`, and descriptor/schema tests prove it is absent from `explaino_all`.
- Hostile-audit finding repair: the direct selector preset now sets `explaino_warp_strength = 0.25f` so visible seed/phase controls are active by default; a derived-fields regression locks this.
- Focused schema rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_schema --log artifacts/logs/pfc_explaino_collatz_direct_schema.log --out-json artifacts/validation/pfc_explaino_collatz_direct_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Focused sample rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_sample --log artifacts/logs/pfc_explaino_collatz_direct_sample.log --out-json artifacts/validation/pfc_explaino_collatz_direct_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` passed with `test_fractal_sample_kernel: passed=1026 failed=0`.
- Focused descriptor rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_descriptor --log artifacts/logs/pfc_explaino_collatz_direct_descriptor.log --out-json artifacts/validation/pfc_explaino_collatz_direct_descriptor.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` passed.
- Focused probe rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_generic_probe --log artifacts/logs/pfc_explaino_collatz_direct_generic_probe.log --out-json artifacts/validation/pfc_explaino_collatz_direct_generic_probe.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_generic_probe` passed.
- Safe-mode rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_safe_mode --log artifacts/logs/pfc_explaino_collatz_direct_safe_mode.log --out-json artifacts/validation/pfc_explaino_collatz_direct_safe_mode.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_safe_mode_schema` passed.
- Sample-tier rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_sample_tier --log artifacts/logs/pfc_explaino_collatz_direct_sample_tier.log --out-json artifacts/validation/pfc_explaino_collatz_direct_sample_tier.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_sample_tier_resolver` passed.
- Final full native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_full_native_final --log artifacts/logs/pfc_explaino_collatz_direct_full_native_final.log --out-json artifacts/validation/pfc_explaino_collatz_direct_full_native_final.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed`.
- Final runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_runtime_publish_final --log artifacts/logs/pfc_explaino_collatz_direct_runtime_publish_final.log --out-json artifacts/validation/pfc_explaino_collatz_direct_runtime_publish_final.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Final no-mouse/catalog runtime proof: `py -3.14 -m pytest tests/test_function_descriptor_cli.py tests/test_callable_engine_adversarial_cli.py tests/test_fractal_parameter_surface_descriptor_cli.py tests/test_fractal_runtime_explaino_collatz_direct.py -q --junitxml artifacts/pytest/pfc_explaino_collatz_direct_runtime_final.junit.xml` passed with `10 passed`.
- Code-quality rail: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_explaino_collatz_direct_code_quality.json` passed after repairing a one-line `sample_tier_resolver.cpp` max-function-lines regression.
- Diff-check rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_explaino_collatz_direct_diff_check --log artifacts/logs/pfc_explaino_collatz_direct_diff_check.log --out-json artifacts/validation/pfc_explaino_collatz_direct_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually add a direct Explaino Collatz selector instead of mutating the existing Newton `explaino_collatz` lane? Yes. The new enum/string/schema selector is `explaino_collatz_direct`, and the existing `explaino_collatz` Newton branch remains separate.
- Did I keep `collatz_transition_strength` off `explaino_all`? Yes. Schema/descriptor tests prove the control is visible on `collatz` and `explaino_collatz_direct`, not `explaino_all`.
- Did I make every visible control on `explaino_collatz_direct` authoritative through the shipped path? Yes for the direct-specific control and common controls used by the branch. The direct branch consumes `collatz_transition_strength`, `explaino_warp_strength`, `explaino_seed`, `explaino_seed_drift`, `explaino_phase`, and `explaino_phase_strength`; epsilon/root-spread/damping stay hidden on this selector.
- Did I preserve standalone Collatz default behavior and current Explaino-all registry/common-axis behavior? Yes. The standalone Collatz branch is unchanged, and `explaino_all` registry-axis behavior was not edited.
- Did I prove selector identity and frame sensitivity through no-mouse published-runtime automation? Yes. The final pytest kept `current_fractal_type == "explaino_collatz_direct"` and proved frame hash changes through in-process set-value commands.
- Did I avoid Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, or broad engine drift? Yes. The touched files are limited to selector/schema/catalog/runtime/test surfaces for this bounded selector; Color Pipeline focused checks under the schema rail remained green.
- Did I close with receipts, push, clean tree, and no stale plan text? This plan is prepared for wrapper checkpoint, receipt, push, and clean-tree closure with the stale closeout text removed.

## Audit Passes

- [x] Pass 1: diff review found the direct branch was additive and did not rewrite the existing Newton `explaino_collatz` branch.
- [x] Pass 2: real finding found and repaired: code-quality detected a `sample_tier_resolver.cpp` max-function-lines regression from a new switch case.
- [x] Pass 3: real finding found and repaired: default warp was zero on the new direct selector, which would make visible seed/phase common controls inert until another slider moved.
- [x] Pass 4: clean re-read confirmed `collatz_transition_strength` is not visible on `explaino_all`, direct runtime consumes the visible controls, no physical mouse test was added, final native/runtime/code-quality/diff rails are green, and scope did not drift.

## Audit Findings

- [x] Finding 1: the first attempted RED used an unsupported focused build target name. Repaired by running the valid `advanced_color_grading_owner` rail, which failed on the absent enum as intended.
- [x] Finding 2: code-quality failed because adding `explaino_collatz_direct` as a separate switch case line regressed `sample_tier_resolver.cpp` max function length. Repaired by collapsing the fallthrough case onto the existing Collatz line and rerunning code-quality plus the focused sample-tier rail.
- [x] Finding 3: `explaino_collatz_direct` exposed common seed/phase controls but initially defaulted warp to zero, making those controls inert at the default preset. Repaired by setting the direct preset warp to `0.25f`, adding a derived-fields regression, and rerunning final full-native plus published-runtime proof.
- [x] Clean final pass: no additional real issue found after final validation; the slice remains bounded to the new direct selector and its proof surfaces.

## Out Of Scope

- Rewriting the existing `explaino_collatz` Newton lane.
- Adding `collatz_transition_strength` to `explaino_all`.
- Explaino Julia direct constants.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
