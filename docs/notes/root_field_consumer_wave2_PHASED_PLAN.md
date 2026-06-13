# Root-Field Consumer Wave 2 Phased Plan

## Explicit User Asks

- Continue the intake-packet Wave 2 work after Wave 1 foundation and the SDF measurement branch.
- Merge the completed SDF measurement branch before starting Wave 2.
- Implement only the selected Wave 2 low-hanging root-field consumers:
  1. `root_field_debug_views`
  2. `explaino_mandelbrot_root_trap`
  3. `explaino_magnet_root_well`
- Preserve legacy/default behavior and prove neutral collapse.
- Stop after these three are implemented and classified; do not start Wave 3 or Wave 4 work in this slice.

## Current Phase

Phase 4 - hardening, classification, and closure.

## Phase Checklist

- [x] Phase 0: fast-forward `master` to the completed SDF measurement branch and push it.
- [x] Phase 0: create `codex/root-field-consumer-wave2` from merged `master`.
- [x] Phase 0: validate this plan/contract and lock the active slice.
- [x] Phase 0: re-read the side-folder Wave 2 packet as `import_candidate`, compare against live repo state, and record drift/selection notes.
- [x] Phase 1: implement and prove `root_field_debug_views`.
- [x] Phase 2: implement and prove `explaino_mandelbrot_root_trap`.
- [x] Phase 3: implement and prove `explaino_magnet_root_well`, or classify it `HIDE_AND_DEFER` if exact neutral collapse is not cheap/provable.
- [ ] Phase 4: hostile audit, classify each idea, sync docs/status, validate, receipt, rearward-review, push, and stop for replan.

## Scope

In scope:

- Diagnostic root-field views that consume current root descriptor authority.
- One Mandelbrot root-trap consumer that leaves Mandelbrot recurrence exact.
- One Magnet root-well/trap consumer that has an exact zero-strength baseline.
- Minimal controls needed to prove the ideas.
- No-mouse runtime proof and capture/replay proof for promoted ideas.

Out of scope:

- Broad enum sprawl or additional Wave 2 ideas beyond the selected three.
- New SDF operators, recursive/apollonian packs, or SDF-native family expansion.
- Color Pipeline graph UI or Salticid adapter/removal work.
- Wave 3 medium enablers, Wave 4 second-pass ideas, root-history charts, seed hunting, GA/slime work, and perturbation zoom.
- Any recurrence modification without exact neutral/default collapse proof.

## Phase 0 - Live Revalidation And Planning Import

### Intent

Treat the side-folder bundle as planning input only, then verify the live repo supports the selected Wave 2 implementation.

### Required Checks

- Confirm Wave 1 foundation state: root descriptor, `preset_core`, AA, Root SDF, Root SDF control/seed repairs, generated N-root exposure.
- Confirm current SDF measurement branch is merged to `master`.
- Confirm selected Wave 2 candidates remain relevant after live repo drift.
- Write a short drift/selection note in this plan before product code mutation.

### Exit Criteria

- Plan and contract validate.
- Active slice is locked.
- Drift note names accepted/rejected bundle assumptions.

### Drift/Selection Result

- The side-folder Wave 2 packet remains `import_candidate`; current code and checked-in docs are authority.
- Current live repo includes the Wave 1 foundation pieces needed for Wave 2: root descriptor authority, `preset_core`, AA state, Root SDF generated N-root public exposure, Root SDF capture/report/state persistence, and the SDF measurement branch merged to `master`.
- The selected Wave 2 set remains valid after live drift:
  - `root_field_debug_views` first as a diagnostic/foundation consumer.
  - `explaino_mandelbrot_root_trap` second because Mandelbrot recurrence can remain exact.
  - `explaino_magnet_root_well` third, gated by exact zero-strength collapse.
- Rejected for this sprint: `explaino_mandelbrot_warp_c`, fold-wrapper, root-count gallery presets, root-metric Color Pipeline sources, h(t) Newton, SDF orbit traps, and all Wave 3/Wave 4 ideas.

## Phase 1 - Root-Field Debug Views

### Intent

Expose enough diagnostic root-field truth to make later root-field consumers inspectable and testable.

### Implementation Direction

- Use the existing root descriptor authority; do not invent another root source.
- Provide diagnostic views for root count/layout, nearest-root/proximity, root phase, and root gap where practical.
- Fail closed with a visible/reportable reason on lanes without usable root authority.
- Do not change recurrence math.

### Acceptance

- Selector identity and visible controls are proven.
- Debug view reports root layout/count matching descriptor authority.
- Unsupported lanes fail closed instead of substituting fake roots.

### Result

- Implemented as runtime automation/report debug fields instead of a separate visual selector: `root_field_consumer_active`, consumer kind, base fractal type, root layout/source/count, requested generated root count, trap strength/scale, base/effective root hashes, and fail-closed reason.
- Uses `ResolveExplainoRootFieldDescriptor(...)` as the descriptor authority.
- Classification: `KEEP_DEBUG_ONLY`.

## Phase 2 - ExplainO Mandelbrot Root Trap

### Intent

Add the safest first product consumer: exact Mandelbrot recurrence plus root-field trap/color influence.

### Implementation Direction

- Add `explaino_mandelbrot_root_trap` as an experimental/root-field consumer lane if the live seam supports a normal selector entry.
- Keep Mandelbrot recurrence exact.
- Apply root field only as a trap/color signal or bounded post-sample signal.
- Provide minimal controls for trap strength/scale and distance/phase/index mode.
- Default must be useful; zero strength or disabled trap must match baseline Mandelbrot.

### Acceptance

- Native test proves disabled/zero-strength output matches baseline Mandelbrot.
- Runtime no-mouse proof changes frame hash when trap is active.
- Capture finding/replay preserve the active root-trap state.

### Result

- Added selector `explaino_mandelbrot_root_trap` in `Explaino Experiments`.
- Recurrence routes through the baseline Mandelbrot direct formula; root field affects only the root-proximity trap/color signal.
- Zero-strength native proof matches baseline Mandelbrot.
- Classification: `PROMOTE_TO_EXPERIMENTAL`.

## Phase 3 - ExplainO Magnet Root Well

### Intent

Add one non-Mandelbrot root-field consumer if exact neutral collapse is provable.

### Implementation Direction

- Use Magnet Type I as the base.
- Prefer trap/well visualization first; add modulation only if it preserves exact zero-strength baseline.
- Provide minimal controls for well strength/scale/mode.
- If zero-strength parity or visibility authority becomes expensive, classify as `HIDE_AND_DEFER` and stop rather than forcing it.

### Acceptance

- Native test proves zero-strength matches baseline Magnet Type I if promoted.
- Runtime no-mouse proof changes frame hash when active.
- Capture finding/replay preserve the active root-well state if promoted.

### Result

- Added selector `explaino_magnet_root_well` in `Explaino Experiments`.
- Recurrence routes through the baseline Magnet Type I direct formula; root field affects only the root-proximity trap/color signal.
- Zero-strength native proof matches baseline Magnet Type I.
- Classification: `PROMOTE_TO_EXPERIMENTAL`.

## Phase 4 - Hardening And Classification

### Classifications

Each selected idea must close as exactly one:

- `PROMOTE_TO_EXPERIMENTAL`
- `KEEP_DEBUG_ONLY`
- `KEEP_PRESET_ONLY`
- `HIDE_AND_DEFER`
- `REVERT`

### Exit Criteria

- Hostile audit records a real finding or clean re-audit evidence after findings.
- Focused native rails pass.
- Runtime publish and published-runtime proof pass.
- Contract validation, plan sync, code quality, hostile-audit validation, and diff check pass.
- Validation and contract proof receipts are written.
- Rearward review is `ok`.
- Branch is pushed and tree is clean.
- Stop for replan before Wave 3.

## Hostile Audit

- Status: complete

Audit questions:

- Did I preserve `legacy_quartic_v1` and baseline Mandelbrot/Magnet behavior?
- Did every visible control affect the active lane or intentionally hide/fail closed?
- Did root-field consumers use descriptor authority rather than re-deriving or inventing roots?
- Did unsupported lanes fail closed with a useful reason?
- Did this stay limited to the three selected Wave 2 ideas?
- Did I stop before Wave 3/Wave 4 work?

## Audit Passes

- [x] Pass 1 - found replay-authority gap: new root-field consumer lanes rendered and captured, but saved `state.json` replay failed because `IsColorSignalAllowedForFractal(...)` rejected their default `root_proximity` pipeline.
- [x] Pass 2 - after repairing root-proximity allow-list and adding rule/state/runtime regressions, focused native rails and runtime root-field consumer proof passed.
- [x] Pass 3 - clean re-read after repair: focused native rail `focused_native_007.log`, runtime root-field consumer proof `4 passed`, Root SDF preservation proof `4 passed`, code-quality baseline `93/100`, and `git diff --check` all passed; no additional real defect found.

## Audit Findings

- [x] Real finding: root-field consumers defaulted to `root_proximity`, but state replay authorization still treated `root_proximity` as basin-only. Repaired by allowing `root_proximity` for `IsRootFieldConsumerFractal(...)` while still rejecting `root_index`/basin authority, plus native state replay and runtime Capture Finding/replay regressions.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Measurement branch merged | `git merge --ff-only codex/sdf-field-generation-downsample-second-pass` fast-forwarded `master` to `d8b7fe0`; `git push origin master` pushed it. |
| Merged-head rearward review | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `d8b7fe0`. |
| Wave 2 branch | `git switch -c codex/root-field-consumer-wave2` created the branch from merged `master`. |
| Contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/root_field_consumer_wave2.contract.json --out-json artifacts/validation/root_field_consumer_wave2_contract.json` returned `ok=true`. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` returned OK. |
| Active contract | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Root-field consumer Wave 2 mini-sprint" --profile runtime --plan docs/notes/root_field_consumer_wave2_PHASED_PLAN.md --contract docs/contracts/root_field_consumer_wave2.contract.json` locked checkpoint `ck:734ddb02`. |
| Focused native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_family_rules test_diagnostics_state_io test_fractal_runtime_validation test_escape_time_coloring test_fractal_renderer test_viewer_ui_automation_report > artifacts\root_field_consumer_wave2\focused_native_007.log 2>&1` passed. |
| Schema/UI native proof | `cmd /c ui_app\build_tests_vsdevcmd.cmd test_schema_binding test_ui_schema ... > artifacts\root_field_consumer_wave2\focused_native_006.log 2>&1` passed `test_schema_binding` and `test_ui_schema`; command then stopped only because `test_enum_id_utils` is not a supported focused target for that helper. |
| Enum native proof | Direct VS-shell `test_enum_id_utils` run passed; see `artifacts\root_field_consumer_wave2\test_enum_id_utils_001.log`. |
| Runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\root_field_consumer_wave2\runtime_publish_002.log 2>&1` passed and published the active runtime. |
| Runtime root-field consumer proof | `py -3.14 -m pytest tests/test_fractal_runtime_root_field_consumers.py -q` passed `4 passed`, including selector identity, control mutation, generated root count, Capture Finding sidecar, and replay. |
| Root SDF preservation proof | `py -3.14 -m pytest tests/test_fractal_runtime_explaino_root_sdf.py -q` passed `4 passed`. |
| Code quality | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/root_field_consumer_wave2_code_quality.json` passed baseline with score `93/100`, `CRITICAL=0`, `ERROR=0`. |
| Diff check | `git diff --check` passed with only CRLF normalization warnings. |
| Final contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/root_field_consumer_wave2.contract.json --out-json artifacts/validation/root_field_consumer_wave2_contract_final.json` returned `ok=true`. |
| Final plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` returned OK. |
| Hostile audit validation | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/root_field_consumer_wave2_PHASED_PLAN.md --out-json artifacts/validation/root_field_consumer_wave2_hostile_audit.json` returned `ok=true`. |
