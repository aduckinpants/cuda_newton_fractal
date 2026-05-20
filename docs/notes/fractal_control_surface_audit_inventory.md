# Fractal Control Surface Audit Inventory

Date: 2026-05-19
Slice: documentation-only research inventory before RED UI/UX harness work
Start branch/head: `codex/fractal-control-surface-audit-docs` / `c774128`

This document is an inventory, not a repair. It records the current control surface for every defined fractal, the static missing-control suspects, and the UI/UX proof gaps that should become RED tests before product work starts.

## Scope Boundary

In scope here:
- enumerate every current `FractalType` and every current schema fractal option
- identify visible numeric controls, combo controls, and known non-slider controls by family
- classify parameters that appear runtime-backed but are not user-exposed
- classify visible controls that lack a no-mouse live UI/UX proof that the rendered frame changes
- define the next RED-test matrix for the UI/UX harness

Out of scope here:
- changing schema, binding, renderer, Color Pipeline, runtime math, or tests
- claiming a visible control is currently broken without a direct proof
- claiming a control is fixed because it is present in schema or state only
- expanding this into a generic renderer rewrite

## Audit Taxonomy

- `UI-proven`: a current no-mouse runtime/UI harness test sets the visible control and proves the rendered frame changes.
- `Schema/binding-proven only`: current native or in-process tests prove lookup, binding, or value write, but not rendered-frame sensitivity.
- `Runtime/headless-proven only`: current CLI/headless tests prove the parameter affects published output, but not through the visible UI control path.
- `Exposed, UI proof missing`: visible user control exists, but this audit did not find direct no-mouse rendered-frame proof for that control on its lane.
- `Static missing-control suspect`: runtime formula or selectable mode appears to have a meaningful parameter not exposed as a user slider/control.
- `Preset-only by design or pending decision`: only a combo/preset is exposed; deeper numeric parameters exist conceptually but may intentionally stay grouped.
- `Generated/internal`: runtime data is generated from exposed controls or persisted for diagnostics and is not automatically a missing slider.

## Current Source Inventory

- `FractalType` enum count: 44.
- `fractal_type` schema option count: 44.
- Static schema source: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Runtime formula sources inspected: `ui_app/src/fractal_sample_device.inl`, `ui_app/src/escape_time_direct_formulas.h`, `ui_app/src/escape_time_specialized_formulas.h`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/fractal_family_rules.h`, and current runtime/native tests.
- Numeric controls below exclude global view/render/color controls such as size, camera, max iter, block size, device, sample tier, exposure, tint, and benchmark.

## Existing UI/UX No-Mouse Proof Coverage

Current direct no-mouse rendered-frame proof is narrow compared with the schema surface:

- `tests/test_fractal_runtime_magnet.py` proves all four Magnet controls: `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, `magnet_bailout`.
- `tests/test_fractal_runtime_runtime_walk_viewer.py::test_explaino_registry_controls_no_mouse_set_value_change_live_viewport` exports the C++ Explaino axis registry and proves the seven registry-axis controls through no-mouse set-value on `explaino_all` and each carrier lane: `ripple_amplitude`, `splice_offset`, `vortex_strength`, `tension_strength`, `balance_void`, `symmetry_tension`, `field_curvature`.
- The same runtime-walk file proves a generic schema integer set-value path with `width` and Color Pipeline no-mouse numeric set-value for `color_pipeline.source.smooth_escape_ramp.signal.scale.primary`.
- `ui_app/tests/test_schema_binding.cpp` proves in-process schema binding/set-value paths for `ripple_amplitude`, `width`, `explaino_seed_b`, and `explaino_seed`, but that is not the same as rendered-frame proof.

Everything else listed as visible below needs generated RED coverage before any repair claim. Some controls have headless or unit coverage, but this slice is about the UI/UX harness proving visible controls actually drive the product surface.

## Static Missing-Control Suspects

These are not all proven bugs. They are the first places where current code suggests meaningful runtime parameters are not fully user-exposed.

1. `julia`: standalone Julia hard-codes `c = -0.7 + 0.27015i` in `InitEscapeTimeDirectState(...)`. There is no visible `julia_c_real` / `julia_c_imag` control.
2. `nova`: `poly_kind` exposes `z4_minus_1` and `custom`, and the Nova runtime evaluates all five polynomial coefficients, but the schema exposes only `poly_c0` through `poly_c3` on `nova`; `poly_c4` is visible on `newton,halley` only. If Nova is meant to support quartic/custom polynomials, this is a real missing-control suspect.
3. `mcmullen`: only `mcmullen_preset` is exposed. The formula has preset-backed `(m, n, lambda)` values; direct `m`, `n`, and `lambda` controls are not exposed.
4. `collatz`: standalone Collatz uses fixed constants in `StepCollatzEscapeState(...)`. No Collatz coefficient controls are exposed.
5. Fixed escape-time families `mandelbrot`, `burning_ship`, `spider`, `celtic_mandelbrot`, and `perpendicular_burning_ship` currently have no family-specific controls beyond global view/render/color settings. That may be correct for canonical formulas, but it is a low-hanging exploration expansion decision.
6. `explaino_julia`: its Julia constant is derived from seeded Explaino roots, not exposed as direct `c_real/c_imag`. This is probably intentional, but should be classified when standalone Julia gets sliders.
7. `explaino_rational_escape`: no dedicated rational-escape numeric control is visible beyond the shared Explaino controls. This needs formula-level classification before deciding whether a slider is missing.
8. Generated/internal surfaces should not be treated as missing sliders by default: `explaino_root_count`, `explaino_roots[4]`, `poly_coeffs_b[5]`, and Color Pipeline stack internals are generated or editor-specific authority surfaces unless a later design intentionally exposes direct editors.

## Every Defined Fractal

| Fractal | Visible numeric controls | Visible non-numeric family controls | Missing-control/static suspect | Current proof classification | First RED UI/UX harness target |
|---|---|---|---|---|---|
| `newton` | `epsilon`, `poly_c0`, `poly_c1`, `poly_c2`, `poly_c3`, `poly_c4` | `poly_kind` | None obvious. | Exposed, UI proof missing for polynomial controls. | Set each coeff and `epsilon`; prove value consumed, dirty/write path, and rendered-frame delta for non-neutral values. |
| `nova` | `epsilon`, `nova_alpha`, `poly_c0`, `poly_c1`, `poly_c2`, `poly_c3` | `poly_kind` | `poly_c4` hidden while `poly_kind` offers quartic/custom and runtime reads five coeffs. | Exposed, UI proof missing; static missing-control suspect for `poly_c4`. | RED for `poly_c4` visibility or policy; set `nova_alpha` and each visible coeff and prove frame delta. |
| `mandelbrot` | None | None | No family-specific sliders; fixed canonical formula unless exploration knobs are desired. | No visible family controls to prove. | Harness should classify as fixed-formula lane and assert no expected family slider matrix row. |
| `julia` | None | None | Hard-coded Julia constant, no `julia_c_real` / `julia_c_imag`. | Static missing-control suspect. | RED that Julia exposes and applies real/imag constant controls, or explicit test documenting fixed constant policy. |
| `burning_ship` | None | None | No family-specific sliders; fixed canonical formula unless exploration knobs are desired. | No visible family controls to prove. | Classify fixed-formula lane. |
| `phoenix` | `phoenix_p_real`, `phoenix_p_imag` | None | None obvious. | Exposed, UI proof missing. | No-mouse set each Phoenix parameter and prove rendered-frame delta. |
| `halley` | `epsilon`, `poly_c0`, `poly_c1`, `poly_c2`, `poly_c3`, `poly_c4` | `poly_kind` | None obvious. | Exposed, UI proof missing. | Same polynomial/epsilon UI proof as Newton. |
| `projection_and_flow` | `epsilon`, `projection_and_flow_target_radius`, `projection_and_flow_pressure_threshold` | `projection_and_flow_root_family` | None obvious. | Headless/runtime coverage exists in places; direct UI proof missing. | Set radius and pressure via no-mouse UI; switch root-family combo or add enum harness support. |
| `counterfactual_pair` | `epsilon`, `counterfactual_pair_offset_x`, `counterfactual_pair_offset_y`, `counterfactual_pair_reconvergence_ratio` | `counterfactual_pair_root_family`, `counterfactual_pair_frame` | None obvious. | Headless/runtime coverage exists in places; direct UI proof missing. | Set offsets/ratio via no-mouse UI; add enum harness support for frame/root-family. |
| `multibrot` | `multibrot_power_float` | None | None obvious. | Exposed, UI proof missing. | Set power to a non-default and prove rendered-frame delta. |
| `spider` | None | None | No family-specific sliders; fixed formula. | No visible family controls to prove. | Classify fixed-formula lane. |
| `celtic_mandelbrot` | None | None | No family-specific sliders; fixed formula. | No visible family controls to prove. | Classify fixed-formula lane. |
| `perpendicular_burning_ship` | None | None | No family-specific sliders; fixed formula. | No visible family controls to prove. | Classify fixed-formula lane. |
| `multicorn` | `multicorn_power` | None | None obvious. | Exposed, UI proof missing. | Set integer power and prove frame delta. |
| `collatz` | None | None | Fixed Collatz constants are not exposed. | Static missing-control suspect only if exploration knobs are wanted. | RED should either require coefficient controls or explicitly mark standalone Collatz fixed. |
| `mcmullen` | None | `mcmullen_preset` | Direct `m`, `n`, and `lambda` not exposed; preset-only today. | Preset-only; combo UI proof missing. | Add enum/combo UI harness proof for `mcmullen_preset`; later decide direct sliders. |
| `lambda` | `lambda_real`, `lambda_imag` | None | None obvious. | Exposed, UI proof missing. | Set real/imag via no-mouse UI and prove frame delta. |
| `magnet` | `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, `magnet_bailout` | None | None obvious from current code. | UI-proven by `test_fractal_runtime_magnet.py`. | Keep in generated matrix as a green guard. |
| `explaino` | `epsilon`, `explaino_seed`, `explaino_warp_strength`, `explaino_root_spread`, `explaino_phase_strength`, `explaino_damping`, `explaino_phase`, `explaino_seed_drift` | `auto_increment_seed`, `prev_seed`, `next_seed`, `explaino_seed_tween` | None obvious. | Binding/headless coverage exists for some; full UI proof missing. | Set every numeric common Explaino control through no-mouse UI and prove frame delta or classified neutral behavior. |
| `explaino_all` | common Explaino controls plus `ripple_amplitude`, `splice_offset`, `vortex_strength`, `tension_strength`, `balance_void`, `symmetry_tension`, `field_curvature` | common Explaino non-numeric controls | Registry axes intentionally visible here and on owner lanes. | Seven registry axes are UI-proven; common controls still need matrix proof. | Keep registry-axis green guard and add common-control matrix. |
| `explaino_y` | common Explaino numeric controls | common Explaino non-numeric controls | None obvious. | Exposed, UI proof missing for this lane. | Generate common-control UI proof. |
| `explaino_fp` | common Explaino numeric controls | common Explaino non-numeric controls | None obvious. | Exposed, UI proof missing for this lane. | Generate common-control UI proof. |
| `explaino_nova` | common Explaino numeric controls plus `nova_alpha` | common Explaino non-numeric controls | Uses Explaino-derived polynomial authority; standalone Nova `poly_c4` suspicion does not automatically apply. | Exposed, UI proof missing for `nova_alpha` on this lane. | Set `nova_alpha` and common controls. |
| `explaino_halley` | common Explaino numeric controls | common Explaino non-numeric controls | None obvious. | Exposed, UI proof missing. | Generate common-control UI proof. |
| `explaino_dual` | common Explaino numeric controls plus `explaino_seed_b`, `explaino_mix` | common Explaino non-numeric controls | None obvious. | Schema/binding coverage exists for `explaino_seed_b`; full UI proof missing. | Set `explaino_seed_b`, `explaino_mix`, and common controls. |
| `explaino_mult` | common Explaino numeric controls plus `explaino_cluster_radius` | common Explaino non-numeric controls | Cluster radius has split ownership/follow-up context in family rules. | Runtime/headless classification exists; direct UI proof missing. | Set `explaino_cluster_radius` and common controls; preserve split-selector classification. |
| `explaino_phoenix` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag` | common Explaino non-numeric controls | None obvious. | Exposed, UI proof missing. | Set Phoenix parameters and common controls. |
| `explaino_transcendental` | common Explaino numeric controls | `transcendental_func`, common Explaino non-numeric controls | No numeric function parameter; function is enum-selected. | Combo UI proof missing. | Add enum/combo UI harness support for `transcendental_func`; prove common controls. |
| `explaino_inertial` | common Explaino numeric controls plus `momentum_beta` | common Explaino non-numeric controls | Coupling is legacy-only in family rules, not an `explaino_all` registry axis. | Runtime/headless legacy-coupling coverage exists; direct UI proof missing. | Set `momentum_beta` and prove frame delta or neutral collapse. |
| `explaino_julia` | common Explaino numeric controls | common Explaino non-numeric controls | Julia constant is derived from seeded root authority, not direct c sliders. | Exposed, UI proof missing; classify alongside standalone Julia. | Generate common-control proof; decide direct c controls only after Julia policy. |
| `explaino_rational` | common Explaino numeric controls plus `explaino_cluster_radius` | common Explaino non-numeric controls | Cluster radius shared with `explaino_mult` but different runtime role. | Runtime/headless classification exists; direct UI proof missing. | Set `explaino_cluster_radius` and common controls on this lane. |
| `explaino_collatz` | common Explaino numeric controls | common Explaino non-numeric controls | Collatz Newton path has no Collatz-specific visible numeric knob. | Exposed, UI proof missing for common controls. | Generate common-control proof; classify whether Collatz-specific knobs should exist. |
| `explaino_lambda` | common Explaino numeric controls plus `lambda_real`, `lambda_imag` | common Explaino non-numeric controls | None obvious. | Headless CLI override coverage exists; direct UI proof missing. | Set lambda real/imag and common controls. |
| `explaino_rational_escape` | common Explaino numeric controls | common Explaino non-numeric controls | No dedicated rational-escape numeric control visible. | Static classification needed; common-control UI proof missing. | Formula audit first, then RED if a rational parameter should be exposed. |
| `explaino_joy` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `joy_coupling` | common Explaino non-numeric controls | Coupling is legacy-only/non-registry in family rules. | Runtime/headless legacy-coupling coverage exists; direct UI proof missing. | Set `joy_coupling`, Phoenix params, and common controls. |
| `explaino_fold` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `fold_coupling` | common Explaino non-numeric controls | Coupling is legacy-only/non-registry in family rules. | Runtime/headless legacy-coupling coverage exists; direct UI proof missing. | Set `fold_coupling`, Phoenix params, and common controls. |
| `explaino_bell` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `bell_coupling` | common Explaino non-numeric controls | Coupling is legacy-only/non-registry in family rules. | Runtime/headless legacy-coupling coverage exists; direct UI proof missing. | Set `bell_coupling`, Phoenix params, and common controls. |
| `explaino_ripple` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `ripple_amplitude` | common Explaino non-numeric controls | None obvious. | `ripple_amplitude` UI-proven through registry test; common/Phoenix controls still need matrix proof. | Keep registry guard and add common/Phoenix proof. |
| `explaino_splice` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `splice_offset` | common Explaino non-numeric controls | `poly_coeffs_b` is generated from `splice_offset`, not a direct missing slider by default. | `splice_offset` UI-proven through registry test; common/Phoenix controls still need matrix proof. | Keep registry guard and add common/Phoenix proof. |
| `explaino_vortex` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `vortex_strength` | common Explaino non-numeric controls | None obvious. | `vortex_strength` UI-proven through registry test; common/Phoenix controls still need matrix proof. | Keep registry guard and add common/Phoenix proof. |
| `explaino_tension` | common Explaino numeric controls plus `phoenix_p_real`, `phoenix_p_imag`, `tension_strength` | common Explaino non-numeric controls | None obvious. | `tension_strength` UI-proven through registry test; common/Phoenix controls still need matrix proof. | Keep registry guard and add common/Phoenix proof. |
| `explaino_balance_void` | common Explaino numeric controls plus `balance_void`, `symmetry_tension`, `field_curvature` | common Explaino non-numeric controls | None obvious after registry repair; these are the three historically fragile axes. | The three registry axes are UI-proven through registry test and published-runtime tests; common controls still need matrix proof. | Keep registry guard and add common-control proof. |
| `explaino_counterfactual_pair` | common Explaino numeric controls plus `counterfactual_pair_offset_x`, `counterfactual_pair_offset_y`, `counterfactual_pair_reconvergence_ratio` | `counterfactual_pair_frame`, common Explaino non-numeric controls | Root-family combo is not visible here; current Explaino carrier uses custom Explaino polynomial authority. | Headless/runtime coverage exists; direct UI proof missing. | Set offsets/ratio and common controls; add enum proof for frame. |
| `explaino_projection_and_flow` | common Explaino numeric controls plus `projection_and_flow_target_radius`, `projection_and_flow_pressure_threshold` | `projection_and_flow_root_family`, common Explaino non-numeric controls | None obvious. | Headless/runtime coverage exists for several controls; direct full UI matrix missing. | Set radius/pressure and common controls; add enum proof for root-family. |

## Phase 8 Generated Re-Audit - 2026-05-20

This re-audit exists because the prior parameter-surface closeout treated a selected matrix as if it were the full all-44 ask. That was false. Current generated evidence from local head `696dabf` plus the Phase 8 working tree:

- Schema/enum parity: 44 `FractalType` enum ids, 44 public enum ids, and 44 `fractal_type` schema selector options.
- Generated visible family-control inventory: 224 visible family-control cells across the 44 lanes.
- Existing `KernelParams` exposure audit: after excluding generated/internal Color Pipeline stacks, `poly_coeffs`, `poly_coeffs_b`, `explaino_roots`, and generated `explaino_root_count`, no existing runtime parameter root is silently hidden from the schema.
- Static missing-control suspects that would require new runtime authority, not merely surfacing an existing hidden field: McMullen direct `(m,n,lambda)` and standalone Collatz coefficient controls. Those remain product decisions and must not be called fixed by the current cleanup.
- Headless runtime sensitivity sweep artifact: `artifacts/analysis/phase8_headless_surface_sensitivity.json`. It used `--capture-diagnostic` and temporary `state.json` files; it did not use the OS mouse or launch the interactive viewer.
- First-pass headless result: 205 / 224 visible family-control cells changed the rendered frame under the generated witness; 19 cells were unchanged and need either a stronger UI-path witness, a runtime repair, or an explicit non-rendering classification.

Unchanged first-pass headless rows:

| Fractal | Control | Binding |
|---|---|---|
| `nova` | `epsilon` | `fractal.params.epsilon` |
| `nova` | `poly_kind` | `fractal.params.poly_kind` |
| `explaino_y` | `epsilon` | `fractal.params.epsilon` |
| `explaino_nova` | `epsilon` | `fractal.params.epsilon` |
| `explaino_nova` | `explaino_warp_strength` | `fractal.params.explaino_warp_strength` |
| `explaino_nova` | `explaino_damping` | `fractal.params.explaino_damping` |
| `explaino_transcendental` | `explaino_seed` | `fractal.params.explaino_seed` |
| `explaino_transcendental` | `explaino_root_spread` | `fractal.params.explaino_root_spread` |
| `explaino_julia` | `epsilon` | `fractal.params.epsilon` |
| `explaino_julia` | `explaino_damping` | `fractal.params.explaino_damping` |
| `explaino_lambda` | `epsilon` | `fractal.params.epsilon` |
| `explaino_lambda` | `explaino_seed` | `fractal.params.explaino_seed` |
| `explaino_lambda` | `explaino_root_spread` | `fractal.params.explaino_root_spread` |
| `explaino_lambda` | `explaino_damping` | `fractal.params.explaino_damping` |
| `explaino_rational_escape` | `epsilon` | `fractal.params.epsilon` |
| `explaino_rational_escape` | `explaino_damping` | `fractal.params.explaino_damping` |
| `explaino_joy` | `epsilon` | `fractal.params.epsilon` |
| `explaino_tension` | `epsilon` | `fractal.params.epsilon` |
| `explaino_projection_and_flow` | `epsilon` | `fractal.params.epsilon` |

Interpretation discipline:

- These 19 rows are not automatically product-dead from this artifact alone, because the sweep mutates `state.json` directly and may bypass UI-only coherence work such as polynomial preset application.
- These 19 rows are also not allowed to be called fixed. Each row now needs a stronger no-mouse UI-path witness, a runtime repair, or an explicit classification.
- The old runtime-walk viewer E2E module is still not an acceptable default proof rail because it relaunches the interactive viewer repeatedly.


## Phase 8 Repair Reconciliation - 2026-05-20

Current after-repair evidence:

- Current schema/enum parity artifact: `artifacts/analysis/phase8_all44_schema_inventory_after_repair.json`; it reports 44 enum ids, 44 public ids, 44 schema selector options, and 214 visible family-control cells after hiding branch-dead controls.
- Original 19-row follow-up artifact: `artifacts/analysis/phase8_persistent_followup_19_after_repair.json`; it uses the persistent no-relaunch viewer harness and reports 9 visible controls with rendered-frame deltas, 10 branch-dead controls intentionally hidden, and 0 unresolved rows from the original first-pass set.
- Runtime proof rail: `py -3.14 -m pytest tests\test_fractal_runtime_persistent_viewer_harness.py -q --junitxml artifacts\pytest\fractal_runtime_persistent_viewer_harness_phase8_dead_control_repair_after_schema.junit.xml` passed with 4 tests.
- Native schema rail: `py -3.14 tools\viewer_host_run_logged_command.py --label phase8_native_dead_control_repair_focus2 --log artifacts\logs\phase8_native_dead_control_repair_focus2.log -- ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed, including Color Pipeline core/window, schema binding, escape-time coloring, diagnostics, finding archive, and runtime reset.

Original 19-row disposition:

| Fractal | Control | Disposition |
|---|---|---|
| `nova` | `epsilon` | UI-proven visible control |
| `nova` | `poly_kind` | UI-proven enum control |
| `explaino_y` | `epsilon` | UI-proven visible control |
| `explaino_nova` | `epsilon` | UI-proven visible control |
| `explaino_nova` | `explaino_warp_strength` | Hidden: branch does not read this control |
| `explaino_nova` | `explaino_damping` | Hidden: branch does not read this control |
| `explaino_transcendental` | `explaino_seed` | UI-proven visible control; default seed-driven warp is now active |
| `explaino_transcendental` | `explaino_root_spread` | Hidden: branch does not read root-spread/polynomial roots |
| `explaino_julia` | `epsilon` | Hidden: escape-time branch does not read convergence epsilon |
| `explaino_julia` | `explaino_damping` | Hidden: branch does not read Newton damping |
| `explaino_lambda` | `epsilon` | Hidden: escape-time branch does not read convergence epsilon |
| `explaino_lambda` | `explaino_seed` | UI-proven visible control; default seed-driven warp is now active |
| `explaino_lambda` | `explaino_root_spread` | Hidden: branch does not read root-spread/polynomial roots |
| `explaino_lambda` | `explaino_damping` | Hidden: branch does not read Newton damping |
| `explaino_rational_escape` | `epsilon` | Hidden: escape-time branch does not read convergence epsilon |
| `explaino_rational_escape` | `explaino_damping` | Hidden: branch does not read Newton damping |
| `explaino_joy` | `epsilon` | UI-proven visible control |
| `explaino_tension` | `epsilon` | UI-proven visible control |
| `explaino_projection_and_flow` | `epsilon` | UI-proven visible control |

Still not claimed until closure validation completes: full native green, hostile audit validation, plan sync, checkpoint commit, receipts, push, and clean tree.

## Next RED UI/UX Harness Series

The next work should be test-first and registry-driven. Do not hard-code only the controls that have recently broken.

1. Add a deterministic descriptor/export for the fractal exploration surface.
   - Inputs: `FractalType`, visible schema controls, binding paths, value types, neutral values, active test values, owner lanes, and expected render sensitivity classification.
   - Output should be consumable by native tests and Python runtime tests.
2. Add UI/UX harness REDs for visibility and set-value plumbing.
   - For each visible numeric control: verify automation rect exists on the owning lane, set-value is consumed, state/binding is changed through the same edit path as the UI, and unsupported/invisible controls fail closed.
   - For combos: either add in-process enum set support or explicitly classify combo proof as deferred.
3. Add rendered-frame REDs for render-sensitive numeric controls.
   - Baseline frame hash and edited frame hash must differ, or the descriptor must classify the control as intentionally non-rendering/indirect.
   - Use no OS cursor movement.
4. Add fixed-formula classifications.
   - `mandelbrot`, `burning_ship`, `spider`, `celtic_mandelbrot`, `perpendicular_burning_ship`, and maybe `collatz` must not look accidentally skipped. They should be explicit fixed-formula rows or have new controls proposed.
5. Split repair slices by family after REDs exist.
   - Start with high-confidence missing controls: Julia c sliders, Nova `poly_c4` policy, then preset-only McMullen and Collatz classification.
   - Keep Magnet as a green guard and keep Explaino-all registry axes as green guards.
6. Preserve performance and Color Pipeline guardrails.
   - Any new slider must have a bounded frame-delta witness and must not silently move the lane onto an expensive path unless that is explicitly expected.
   - Existing Color Pipeline no-mouse set-value and row click tests remain guardrails, not part of this fractal-control repair except to prevent harness regression.

## Priority Cut

Recommended first RED/fix order:

1. Julia c controls: obvious user value, obvious missing sliders, small surface.
2. Nova `poly_c4`: static schema/runtime contradiction when quartic/custom polynomial paths are available.
3. Generated UI/UX matrix for all existing exposed numeric controls, starting with standalone families before Explaino family lanes.
4. McMullen preset/combo proof and decision on direct `(m, n, lambda)` controls.
5. Collatz/fixed-formula classification and optional exploration knobs.
6. Explaino common-control matrix expansion across all lanes, while preserving the existing registry-axis proof.

## Closeout Position For This Slice

Phase 8 no longer stops at research only. The original all-44 audit and first-pass 19-row dead-control set have been repaired or classified in the working tree, but the slice is not closed until full validation, hostile audit, checkpoint, receipts, push, and clean tree complete.
