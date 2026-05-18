# Merged-Head Explaino Selector / Control / Smooth-Escape Regression Repair

## Current Phase

Closed on the committed head - the merged-head Explaino selector/control/smooth-escape repair is checkpointed, hostile-audited, validator-clean, preserves `SampleFractalPoints(...)` as the shipped legacy projection surface, and holds the merged-head public selector/control/runtime/perf contract without reopening Meta-Basin, Operator-Itinerary, DSL/program-space, or generic engine rewrite work.

## Phase Checklist

- [x] Phase 1 - replace the closed Meta-Basin lock with this checked-in merged-head repair contract and land RED proofs for selector collapse, false `explaino_all` ownership, regressed tests, bounded smooth-escape perf/usability failure, and preserved legacy projection semantics.
- [x] Phase 2 - land one truthful selector-identity repair and one truthful `explaino_all` control-ownership repair across schema/public-state/runtime-routing/catalog surfaces without widening into Meta-Basin, Operator-Itinerary, DSL/program-space, or generic engine redesign.
- [x] Phase 3 - land one bounded smooth-escape Explaino usability/perf repair, prove one-at-a-time control authority on the owning lanes, and keep `SampleFractalPoints(...)` / legacy callers unchanged as the shipped projection surface.
- [x] Phase 4 - hostile-audit the repaired merged head, restore the temporary Meta-Basin bridge scope, run focused native/runtime publish/published-runtime/perf proof plus contract validation and plan sync, checkpoint, write receipts, clear stale closeout text, and leave the repo clean.

## Explicit User Asks

- [done] Repair the merged-head Explaino selector regression so explicit family selectors like ripple/splice/vortex/tension stay selected in public state instead of collapsing to `explaino_all`.
- [done] Repair the merged-head `explaino_all` control-surface lie so only truthfully owned controls remain visible or the non-owner controls are hidden/blocked with proof.
- [done] Repair the merged-head Explaino smooth-escape usability/perf regression with one bounded witness on the same public path instead of a vague "it still renders" claim.
- [done] Update tests so they enforce the truthful merged-head product contract instead of encoding selector collapse or false `explaino_all` ownership as correct.
- [done] Preserve `SampleFractalPoints(...)` as the shipped legacy projection surface and keep Meta-Basin, Operator-Itinerary, DSL/program-space, generic color expansion, and broad engine rewrite out of scope unless one tiny blocker seam is proved exactly.
- [done] Close only with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, clean tree, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap on 2026-05-17 proved branch=`master`, `HEAD=62dcb01`, clean tree, fresh session, and an active locked contract still pointing at the already closed `explaino_native_meta_basin_viability_owner_seam_proof` slice.
- Minimal blocker-clearing preflight is required before mutation because the closed Meta-Basin contract does not authorize successor plan/contract creation; the bridge must stay limited to existing `docs/contracts` + `docs/notes` scope long enough to create and lock this slice, then be restored before closeout.
- Public selector collapse is live on the merged head: `ui_app/src/schema_binding.cpp` returns `ResolveExplainoPublicFractalType(view->fractal_type)` for `fractal.view.fractal_type`; `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/runtime_walk_headless.cpp`, `ui_app/src/explaino_sidecar_measurement.cpp`, `ui_app/src/runtime_reset.cpp`, and `ui_app/src/viewer_state_init.cpp` all canonicalize legacy Explaino projection selectors back to `explaino_all`.
- Current tests encode the collapsed model as correct: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_fractal_family_rules.cpp`, `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_runtime_reset.cpp`, `ui_app/tests/test_viewer_state_init.cpp`, `ui_app/tests/test_fractal_probe.cpp`, and `tests/test_fractal_runtime_explaino_escape_variants.py` currently expect projection-family selectors or control authority to round-trip through `explaino_all`.
- Current visible-control ownership is untruthful on `explaino_all`: `ui/fractal_binding_surface_v1.ui_schema.json` exposes family-axis sliders on `explaino_all`, while `ui_app/src/ui_schema.cpp` rewrites Explaino axis controls to canonical `explaino_all` visibility rather than their explicit owning lanes.
- Current published callable surfaces also inherit that lie: `ui_app/src/function_descriptor.cpp` publishes Explaino selector and per-control metadata from the canonicalized registry/schema surface, while `tests/test_function_descriptor_cli.py` and `ui_app/tests/test_function_descriptor.cpp` currently accept the resulting over-advertised catalog as valid.
- Current runtime route is coupled to the public-state lie: `ui_app/src/fractal_family_rules.h` collapses public identity with `ResolveExplainoPublicFractalType(...)`, while canonical `explaino_all` runtime routing falls back to `explaino`, `explaino_ripple`, or `explaino_balance_void` based on parameter heuristics instead of preserving the explicitly selected family lane.
- Current strongest source-level smooth-escape perf cliff candidate is `ui_app/src/fractal_sample_device.inl`: one combined explicit-variant branch for `explaino_ripple|splice|vortex|tension` executes inactive perturbation work before later dedicated per-variant branches that appear unreachable under normal control flow; this needs a bounded RED witness on the merged head before the repair is trusted.
- Existing merged-head validation artifacts already include staged Explaino benchmark outputs under `D:\salt-fractal\cuda_newton_fractal_clone\build_tests`, but this slice must refresh proof on the current repaired head rather than treating stale artifacts as authority.
- The first post-repair native rerun exposed one adjacent but in-scope blocker: `ui_app/tests/test_explaino_sidecar_measurement.cpp` still hand-builds projection/balance-void parity catalogs around `explaino_all`; that checked-in test must be updated to the repaired explicit-selector / hidden-umbrella ownership model before the merged-head native rail can go green truthfully.
- The next native rerun plus follow-up grep exposed three more stale proof surfaces that still encode the collapsed-selector / false-`explaino_all` model: `ui_app/tests/test_fractal_probe_coverage.cpp`, `ui_app/tests/test_explaino_zero_axis_equivalence.cu`, and `ui_app/tests/test_explaino_sidecar_window.cpp`; this slice may widen only enough to repair those witnesses to the explicit-owner runtime contract.
- `SampleFractalPoints(...)` and current legacy callers can remain the shipped projection surface if selector truth, control ownership, and bounded smooth-escape work stay inside binding/public-state/runtime-routing/catalog/device-color seams instead of widening the API or reopening Meta-Basin / DSL work.
- Published-runtime perf proof on the repaired head still caught a real merged-head bug after the selector/control fixes: `tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_smooth_escape_explicit_variants_stay_within_bounded_runtime_usability` failed with median wall times baseline=`0.2088s`, `explaino_ripple=0.5310s`, `explaino_splice=0.4198s`, `explaino_vortex=0.1992s`, `explaino_tension=0.5257s`, so packet-local native green was still lying about merged-head product usability.
- Direct published-runtime diagnostics isolated the concrete perf seam to render-time tier routing, not selector identity or sample-surface API drift: owner-active legacy Explaino projection selectors stayed on `resolved_backend=float64` under `render.sample_tier=tier_auto`, while forcing the same states to `fast` dropped ripple/splice/vortex/tension into the usable `0.216s` to `0.222s` range without changing `SampleFractalPoints(...)`.
- The bounded smooth-escape repair is now in `ui_app/src/sample_tier_resolver.cpp`: `ResolveSampleEvalModeForRender(...)` resolves the truthful Explaino runtime type first, keeps owner-active legacy projection selectors (`ripple`, `splice`, `vortex`, `tension`, `balance_void`) on `float32` for `tier_auto + smooth_escape`, and leaves neutral / umbrella Explaino lanes and other color signals on the prior path.
- Focused proof on the repaired head is green: repo-local targeted native unit `artifacts/build_targeted/test_sample_tier_resolver.exe` passed, runtime publish via `ui_app/build_vsdevcmd.cmd > artifacts/logs/explaino_selector_control_fix_runtime_build2.log 2>&1` succeeded, and focused published-runtime proof via `artifacts/logs/explaino_selector_control_fix_runtime_pytest2.log` finished `15 passed in 8.68s`.
- Post-fix merged-head smooth-escape witness medians are back inside the usability envelope on the same public path: baseline `explaino=0.1917s` (`float64`), `explaino_ripple=0.2093s` (`float32`), `explaino_splice=0.2005s` (`float32`), `explaino_vortex=0.1887s` (`float32`), and `explaino_tension=0.2139s` (`float32`).
- Contract validation, phased-plan sync, and hostile-audit validation are green on the repaired merged head via `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/merged_head_explaino_selector_control_smooth_escape_regression_repair.contract.json --out-json artifacts/validation/merged_head_explaino_selector_control_smooth_escape_regression_repair_contract.json`, `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`, and `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/merged_head_explaino_selector_control_smooth_escape_regression_repair_PHASED_PLAN.md --out-json artifacts/validation/merged_head_explaino_selector_control_smooth_escape_regression_repair_hostile_audit.json`.

## Hostile Audit

- Status: complete
- Required posture: assume selector identity, visible control ownership, runtime routing, smooth-escape usability, benchmark framing, tests, plan text, and my own summary are all wrong until merged-head machine proof says otherwise.
- Hostile review questions:
  Did I actually stop explicit Explaino selectors from collapsing to `explaino_all`?
  Did I actually repair `explaino_all` control ownership, or only move the confusion around?
  Did I actually make every visible Explaino control truthful on its owning lane?
  Did I actually repair smooth-escape usability, or only keep correctness rails green while it stays too slow?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into Meta-Basin, Operator-Itinerary, DSL/program-space, generic color work, or a broad engine rewrite?
  Did I stop with stale closeout text again?

## Audit Passes

- [x] Pass 1 - land the first REDs proving selector collapse, false `explaino_all` ownership, current regressed tests, bounded smooth-escape perf/usability failure, and preserved legacy projection semantics on the merged head.
- [x] Pass 2 - hostile-reread the repaired selector/control/perf seams for truthful ownership, one-at-a-time control authority, bounded scope, and preserved legacy sampling.
- [x] Pass 3 - rerun the repaired native/runtime/perf witnesses, restore the temporary Meta-Basin bridge scope, and reread the repaired head until no additional product lie, workflow lie, or stale closeout text remains.

## Audit Findings

- [x] Hostile audit found a real merged-head perf lie after the selector/control repair: `ResolveSampleEvalModeForRender(...)` still promoted owner-active Explaino legacy projection selectors onto the `float64` smooth-escape path, leaving the published runtime unusably slow even though native packet-local rails were green. Repaired the resolver seam only, added native resolver guards plus published-runtime backend guards, republished the runtime, and reran the perf witness until the merged-head medians returned to the bounded usability envelope.
- [x] Clean hostile reread after the resolver-only perf repair, targeted native resolver proof, runtime publish, focused published-runtime pytest, and post-fix timing witness did not expose another selector-identity lie, `explaino_all` ownership leak, hidden `SampleFractalPoints(...)` API drift, or unrelated engine-scope widening.

## Notes

- Required first REDs:
  1. prove explicit Explaino family selection currently collapses back to `explaino_all`
  2. prove the current schema/tests intentionally expose the disputed `explaino_all` sliders
  3. prove which of those visible controls are dead, weakly coupled, or misowned on `explaino_all`
  4. prove the current tests encode the regressed product model instead of catching it
  5. prove the smooth-escape Explaino path has a merged-head usability/perf regression
  6. prove `SampleFractalPoints(...)` and current legacy callers remain bound to legacy semantics on the RED head
  7. prove Meta-Basin, Operator-Itinerary, DSL/program-space, and broad engine rewrite remain out of scope for this slice

