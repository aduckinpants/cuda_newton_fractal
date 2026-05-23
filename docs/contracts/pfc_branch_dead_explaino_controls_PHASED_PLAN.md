# PFC Branch-Dead Explaino Controls

## Explicit User Asks

- Continue the parameter functionality campaign from the sprint holder branch.
- Work Step 6: `codex/pfc-branch-dead-explaino-controls`.
- Audit hidden or previously public Explaino controls whose owning runtime branch does not authoritatively consume the parameter.
- Repair only bounded, default-preserving controls with a clear runtime authority seam.
- Keep ambiguous controls honestly classified instead of re-exposing sliders that still do nothing.
- Preserve the current working parameter API baseline, Explaino-all registry/common-axis behavior, Color Pipeline behavior, capture finding, FPS pacing, equation-pack integration, perturbation zoom, and generated editor boundaries.
- Use no-mouse runtime proof in a persistent viewer process.

## Current Phase

Closed: the Explaino Nova warp/damping repair is landed, checkpointed, receipted, pushed on the feature branch, integrated into `codex/parameter-functionality-campaign`, and covered by focused native, full native, published no-mouse runtime, descriptor CLI, code-quality, contract, plan-sync, whitespace, and hostile-audit evidence.

## Phase Checklist

- [x] Bootstrap and confirm repo branch, head, upstream, clean state, and closed prior Step 5 contract.
- [x] Create feature branch `codex/pfc-branch-dead-explaino-controls` from `codex/parameter-functionality-campaign`.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this branch-dead Explaino controls contract.
- [x] RED/classification: prove the currently hidden branch-dead Explaino control set and record which rows are repairable now versus deferred.
- [x] RED: prove `explaino_nova` currently hides `explaino_warp_strength` and `explaino_damping` while the runtime branch does not consume either parameter.
- [x] Implement the default-preserving `explaino_nova` repair so `explaino_warp_strength` affects the Nova `c` coordinate transform and `explaino_damping` affects the Newton step scale only on the Explaino Nova lane.
- [x] Mirror the same authority semantics in the host probe path.
- [x] Add native schema/descriptor/sample tests for Explaino Nova visibility, selector ownership, default parity, and non-default sensitivity.
- [x] Add a no-mouse published-runtime proof that sets the repaired controls in one viewer process, keeps `explaino_nova` selected, and changes the rendered frame.
- [x] Validate full/focused native/runtime rails, code quality, contract, plan sync, and hostile audit.
- [x] Checkpoint, write receipts, push feature branch, fast-forward sprint holder, push sprint holder, stale-plan grep, and verify clean tree.

## Owner Seams

- Schema visibility: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Binding and descriptor authority: `ui_app/src/schema_binding.cpp`, `ui_app/src/fractal_parameter_surface_descriptor.cpp`, and their tests.
- Runtime math: `ui_app/src/fractal_sample_device.inl`.
- Host probe math: `ui_app/src/fractal_probe_runner.cpp`.
- Native proof: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_fractal_parameter_surface_descriptor.cpp`, and `ui_app/tests/test_fractal_sample_kernel.cu`.
- Published runtime proof: `tests/test_fractal_runtime_branch_dead_explaino_controls.py`.
- Prior classification references: `docs/notes/fractal_control_surface_audit_inventory.md` and `docs/notes/fractal_parameter_surface_matrix_PHASED_PLAN.md`.

## Classification Boundary

- Repair now: `explaino_nova::explaino_warp_strength` and `explaino_nova::explaino_damping`, because both can be default-preserving and map onto existing Explaino warp/damping semantics without inventing a new family.
- Classify/defer unless code proves a narrow safe authority seam: `explaino_julia::epsilon`, `explaino_lambda::epsilon`, `explaino_rational_escape::epsilon`, `explaino_transcendental::explaino_root_spread`, `explaino_lambda::explaino_root_spread`, `explaino_julia::explaino_damping`, `explaino_lambda::explaino_damping`, and `explaino_rational_escape::explaino_damping`.
- Do not re-expose a control only because the parameter exists in `KernelParams`; a repaired slider must materially change the owning runtime branch through the shipped public path.

## Proof Ledger

- Sprint holder source head: `b462dc8`.
- Slice branch: `codex/pfc-branch-dead-explaino-controls`.
- Current schema precondition: the Step 6 branch-dead rows are hidden by `visible_if` filtering on `epsilon`, `explaino_warp_strength`, `explaino_root_spread`, and `explaino_damping`.
- Current runtime precondition: the `explaino_nova` branch shares the Nova update path and reads `nova_alpha` plus `epsilon`, but not `explaino_warp_strength` or `explaino_damping`.
- Current host-probe precondition: the `explaino_nova` probe branch mirrors the same missing warp/damping authority.
- Classification artifact: `artifacts/audit/pfc_branch_dead_explaino_controls/branch_dead_control_classification.json`.
- RED schema/binding: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_schema_binding_red --log artifacts/logs/pfc_branch_dead_schema_binding_red.log --out-json artifacts/validation/pfc_branch_dead_schema_binding_red.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_explaino_counterfactual_repair` failed because `explaino_warp_strength` was hidden on `explaino_nova`.
- RED descriptor: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_descriptor_red --log artifacts/logs/pfc_branch_dead_descriptor_red.log --out-json artifacts/validation/pfc_branch_dead_descriptor_red.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` failed because the descriptor omitted `explaino_warp_strength` on `explaino_nova`.
- RED sample: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_sample_red --log artifacts/logs/pfc_branch_dead_sample_red.log --out-json artifacts/validation/pfc_branch_dead_sample_red.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` failed because both Explaino Nova warp and damping samples were unchanged.
- GREEN schema/binding: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_schema_binding --log artifacts/logs/pfc_branch_dead_schema_binding.log --out-json artifacts/validation/pfc_branch_dead_schema_binding.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_explaino_counterfactual_repair` passed.
- GREEN descriptor native: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_descriptor --log artifacts/logs/pfc_branch_dead_descriptor.log --out-json artifacts/validation/pfc_branch_dead_descriptor.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` passed.
- GREEN sample: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_sample --log artifacts/logs/pfc_branch_dead_sample.log --out-json artifacts/validation/pfc_branch_dead_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` passed with `1016 failed=0`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_runtime_publish --log artifacts/logs/pfc_branch_dead_runtime_publish.log --out-json artifacts/validation/pfc_branch_dead_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse runtime: `py -3.14 -m pytest tests/test_fractal_runtime_branch_dead_explaino_controls.py -q --junitxml artifacts/pytest/pfc_branch_dead_runtime.junit.xml` passed, proving `explaino_nova` selector identity, visibility for both repaired controls, consumed set-value writes, one viewer process, preview scale fixed at 1.0, and frame-hash changes.
- Published descriptor CLI: `py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py -q --junitxml artifacts/pytest/pfc_branch_dead_descriptor_cli.junit.xml` passed.
- Full native helper suite: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_native_full --log artifacts/logs/pfc_branch_dead_native_full.log --out-json artifacts/validation/pfc_branch_dead_native_full.json --heartbeat-seconds 30 --timeout-seconds 3600 -- ui_app/build_tests_vsdevcmd.cmd` passed.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/pfc_branch_dead_explaino_controls.contract.json --out-json artifacts/validation/pfc_branch_dead_explaino_controls_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_branch_dead_explaino_controls_code_quality.json` passed with score 96/100 and no baseline regression.
- Whitespace: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_branch_dead_diff_check --log artifacts/logs/pfc_branch_dead_diff_check.log --out-json artifacts/validation/pfc_branch_dead_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually classify each hidden branch-dead Explaino row instead of hiding the unexamined rows behind a broad claim?
- Did I repair `explaino_nova` controls only where runtime and host probe paths both consume them?
- Did defaults preserve current Explaino Nova behavior?
- Did the schema, descriptor, native sample, and published no-mouse runtime paths all prove the repaired controls?
- Did selector identity remain `explaino_nova` after automation writes?
- Did I avoid changing Explaino-all registry/common-axis behavior, Color Pipeline, capture finding, FPS pacing, equation-pack integration, perturbation zoom, generated editors, or broad renderer architecture?
- Did I close with receipts, push, sprint-holder integration, clean tree, and no stale plan text?

## Audit Passes

- [done] Pass 1: classified hidden branch-dead rows and verified only the high-confidence Explaino Nova warp/damping rows were re-exposed.
- [done] Pass 2: reviewed runtime and host-probe diffs as if the two paths diverged; both now apply Explaino Nova warp to the Nova `c` coordinate and damping to the Newton step scale.
- [done] Pass 3: reviewed published-runtime proof as if the frame change came from selector churn, preview resolution churn, or a stale executable; the test asserts `current_fractal_type == "explaino_nova"`, `preview_min_scale == 1.0`, `render_pacing_preview_active is False`, consumed set-value writes, and runs after the runtime publish.
- [done] Pass 4: final clean re-read after full validation and holder integration found no further implementation defect or stale closeout text.

## Audit Findings

- [x] Finding 1: the initial contract listed unsupported focused target `test_schema_binding`. Repair: switch to the repo-supported `test_explaino_counterfactual_repair` rail, which runs `test_schema_binding` plus nearby schema validation.
- [x] Finding 2: the first schema fix exposed the controls but left the `param_anim_target` option mirror hidden for `explaino_nova`. Repair: add `explaino_nova` to both `damping` and `warp_strength` animation target visibility and keep the schema mirror test green.
- [x] Finding 3: the descriptor animatable check treated option IDs as identical to control IDs, so common alias controls such as `explaino_warp_strength` and `explaino_damping` were reported with incomplete authority. Repair: map common Explaino control IDs to the actual animation option IDs and prove native plus published descriptor CLI output.
- [x] Finding 4: the first sample default-parity witness compared against the test helper's non-product `explaino_warp_strength = 0.1` setup instead of the schema/product neutral default `0.0`. Repair: pin the default-parity witness to neutral `warp=0.0`, `damping=1.0`.

## Out Of Scope

- Explaino Collatz direct variants.
- Explaino Julia custom authority modes.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, FPS pacing, and renderer architecture changes.
