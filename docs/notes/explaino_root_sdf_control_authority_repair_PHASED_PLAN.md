# ExplainO Root SDF Control Authority Repair

## Current Phase

Complete - root-SDF control authority repair is validated and closed on the committed head.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the repair slice.
- [x] Phase 2 - add RED/native tests for seed action authority and dead-control visibility.
- [x] Phase 3 - repair seed action predicates without adding `explaino_root_sdf` to `IsExplainoFamily`.
- [x] Phase 4 - hide inactive root-SDF controls and preserve generated/custom root authority visibility.
- [x] Phase 5 - add no-mouse runtime proof for Prev Seed, Next Seed, and hidden-control absence.
- [x] Phase 6 - hostile audit, focused validation, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [active] Prev Seed, Next Seed, and arrow-key seed scrub must work on `explaino_root_sdf`.
- [active] `explaino_root_sdf` must stay out of escape/iteration `IsExplainoFamily` semantics.
- [active] Visible root-SDF controls must be active, or intentionally hidden if the lane cannot consume them.
- [active] Hide `explaino_warp_strength` for `explaino_root_sdf`.
- [active] Hide `color_smooth_escape_interior_strength` when the active lane/color path cannot consume smooth-escape interior coloring.
- [active] Preserve generated/custom root authority visibility and never write derived roots back as authority.
- [active] Runtime tests must cover action controls, not only set-value sliders.
- [active] Defer lane-local/global SDF downsample UX cleanup, ExplainO Warp redesign, and broader visible-control/domain metadata refactor.

## Contract Lock

This is a repair slice on `codex/explaino-root-sdf-field-lane`, not a new feature lane.

`explaino_root_sdf` remains a field-primary SDF lane:

- It must not enter `RenderFractalCUDA`.
- It must not become `IsExplainoFamily`.
- It may use ExplainO root-layout authority for generated/custom roots and seed actions.
- Unsupported non-SDF Source rows remain fail-closed for this producer.

Seed action authority:

- Prev Seed, Next Seed, and arrow-key seed scrub must use a root-layout authority predicate such as `UsesExplainoRootLayoutAuthority(...)` or a narrower seed-action predicate.
- Existing escape-time ExplainO family behavior must remain unchanged.

Control visibility authority:

- `explaino_warp_strength` is hidden for `explaino_root_sdf` because the root-SDF field producer does not consume it.
- `color_smooth_escape_interior_strength` is hidden for field-primary SDF paths that do not consume smooth-escape interior coloring.
- Custom root coordinate fields remain visible only under custom root authority.
- Generated/root-h controls remain available for generated authority and h-mode behavior.

## Proof Ledger

- Starting branch expected: `codex/explaino-root-sdf-field-lane`.
- Starting head expected: `24d9d0e9211416cd0f6fa3b4b4629aa68b936120`.
- Rearward review before repair: expected `ok`.
- First confirmed defect: seed actions were gated by `IsExplainoFamily(...)`, excluding `explaino_root_sdf` despite its root-layout authority.
- First confirmed dead-control exposure: schema exposed `explaino_warp_strength` for `explaino_root_sdf`.
- RED/native proof: `cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_schema_binding > artifacts\explaino_root_sdf_control_authority_repair\red_schema_binding.log 2>&1` failed before the repair because root-SDF seed action and dead-control visibility expectations were unmet.
- Focused native proof after repair: `cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_schema_binding test_fractal_family_rules > artifacts\explaino_root_sdf_control_authority_repair\focused_native_after_fix.log 2>&1` passed.
- Runtime publish proof: `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\explaino_root_sdf_control_authority_repair\build_vsdevcmd_runtime.log 2>&1` passed and refreshed the published runtime.
- Published no-mouse action proof: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_root_sdf.py -q` passed with Prev Seed and Next Seed click automation plus hidden-control absence checks.
- Full native helper sweep: `cmd /c ui_app\build_tests_vsdevcmd.cmd > artifacts\explaino_root_sdf_control_authority_repair\build_tests.log 2>&1` reached `All helper tests passed.`
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_root_sdf_control_authority_repair.contract.json --out-json artifacts/validation/explaino_root_sdf_control_authority_contract.json` passed.
- Phased-plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Diff check: `git diff --check > artifacts\explaino_root_sdf_control_authority_repair\diff_check.log 2>&1` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_root_sdf_control_authority_code_quality.json` passed with existing warnings only.
- Logged runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_sdf_control_publish --log artifacts/logs/explaino_root_sdf_control_publish.log --out-json artifacts/validation/explaino_root_sdf_control_publish.json --heartbeat-seconds 30 --timeout-seconds 1800 -- cmd /c ui_app\build_vsdevcmd.cmd` passed.
- Logged post-publish runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_sdf_control_runtime_after_publish --log artifacts/logs/explaino_root_sdf_control_runtime_after_publish.log --out-json artifacts/validation/explaino_root_sdf_control_runtime_after_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_root_sdf.py` passed with `3 passed`.
- Rearward repair: review of `93a628f53b75828b43b7e2d3af6fb10b5bb8263d` found stale closeout wording in this plan; this docs-only repair removes that wording.

## Hostile Audit

- Status: complete

Required questions:

- Did seed buttons and arrow scrub use root-layout authority without making `explaino_root_sdf` an escape-time ExplainO family?
- Did the runtime proof invoke the same UI action path as the buttons instead of only setting sliders?
- Did hidden controls disappear from the visible-control report?
- Did custom/generated root authority still expose the correct fields in each mode?
- Did the repair avoid SDF downsample UX redesign and ExplainO Warp redesign?
- Did Capture Finding, Color Pipeline, and root-SDF producer behavior stay unchanged except for the repaired controls?

## Audit Passes

- [x] Pass 1 - hostile diff review after first implementation found the animation target mirror leak.
- [x] Pass 2 - re-read the repaired state after removing the root-SDF warp animation target and confirmed the owner/animation visibility mirror is covered.
- [x] Pass 3 - final clean re-read found no additional real defect, no additional real issue, and no additional workflow mistake before closure.

## Audit Findings

- [x] Hiding `explaino_warp_strength` on the owner control was not sufficient because the animation target option `warp_strength` still mirrored root-SDF visibility. The schema/binding rail caught this after the first implementation, root-SDF was removed from that option, and the focused schema/binding tests now enforce the mirrored owner/animation visibility contract.
- [x] Clean re-read note: Prev/Next Seed runtime coverage uses the first-party `click_control` action path and proves consumed clicks, selector identity, root-hash change, and frame-hash change. The repo does not currently expose a no-mouse keyboard event automation surface; arrow-key scrub uses the same `SupportsExplainoSeedControls(...)` predicate in code and is covered by predicate/native review, but not by a synthetic keypress runtime test in this slice.

## Deferred Notes

- Lane-local/global SDF field downsample UX cleanup remains deferred; root-SDF inherits the existing awkward shared SDF field resolution authority.
- ExplainO Warp redesign remains deferred; this slice hides dead exposure on root-SDF instead of changing warp math or bounds.
- Broader "visible control must be consumed by active domain" metadata remains deferred; this slice adds focused coverage for the root-SDF regression class.
