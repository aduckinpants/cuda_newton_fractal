# UI-Salt Typed Edge Resolution Campaign

## Current Phase

Phase 14 complete - Slice H shipped the minimal runtime-backed Shape-row Batch 1 (`log_compress`, `smoothstep_range`), classified the tempting alias candidates out of scope, and proved metadata/native/runtime no-mouse sensitivity on the published viewer.

## Phase Checklist

- [x] Phase 0 - merge the SDF field-generation optimization to `master`, push, verify clean state, and branch `codex/ui-salt-typed-edge-preplanning`.
- [x] Phase 1 - create and lock this planning contract.
- [x] Phase 2 - document the bounded implementation slices for typed signals, adapters, edge resolution, and function-library expansion.
- [x] Phase 3 - validate contract, plan sync, hostile audit, code-quality baseline, diff check, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 4 - incorporate typed-edge review refinement, validate, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 5 - implement Slice A shadow signal type registry, focused tests, validation, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 6 - fast-forward Slice A to `master`, push `master`, and branch `codex/ui-salt-port-signatures` for Slice B.
- [x] Phase 7 - implement Slice B pilot function port signatures, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 8 - fast-forward Slice B to `master`, push `master`, branch `codex/ui-salt-adapter-library-shadow`, implement Slice C adapter-library shadow metadata, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 9 - fast-forward Slice C to `master`, push `master`, branch `codex/ui-salt-edge-resolver-shadow`, implement Slice D edge resolver shadow audit, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 10 - fast-forward Slice D to `master`, push `master`, branch `codex/ui-salt-compat-override-demotion`, implement Slice E legacy compat override demotion shadow metadata, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 11 - fast-forward Slice E to `master`, push `master`, branch `codex/ui-salt-sdf-applicator-capability`, implement Slice F SDF applicator capability shadow metadata, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 12 - fast-forward Slice F to `master`, push `master`, branch `codex/ui-salt-compat-diagnostics-g1`, implement Slice G1 diagnostic/explanation lookup, focused tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 13 - fast-forward Slice G1 to `master`, push `master`, branch `codex/ui-salt-compat-switch-g2`, implement one pilot typed-resolver compatibility switch with fallback/kill switch, focused native/runtime tests, validation, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.
- [x] Phase 14 - branch `codex/ui-salt-function-library-batch1`, classify Slice H candidates, implement the minimal runtime-backed Batch 1 function set, prove typed ports/materialized metadata/native rails/published runtime sensitivity, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Plan bounded slices for applying the external UI-Salt architecture review.
- [closed] Resolve the worst architectural shortcomings before expanding function libraries.
- [closed] Keep graph UI replacement as a later/final step.
- [closed] Preserve the current visible Color Pipeline workflow until metadata proof is strong enough to switch seams.
- [closed] Incorporate the second review pass before Slice A implementation starts.
- [closed] Prevent the typed-edge system from recreating coarse `signal_kind` ambiguity under refined names.
- [closed] Keep this as a planning/doc refinement only; no runtime behavior, UI workflow, or materializer implementation changes belong in this slice.
- [closed] Implement Slice A only: materialized shadow signal type registry and coarse-to-typed mapping audit.
- [closed] Preserve all current visible Color Pipeline behavior and live compatibility behavior.
- [closed] Do not implement adapters, resolver routing, graph UI, or function-library expansion under Slice A.
- [closed] Implement Slice B only: shadow `inputs`/`outputs` for the bounded pilot function subset.
- [closed] Preserve current visible Color Pipeline row/function order, labels, controls, and runtime compatibility behavior.
- [closed] Prove `identity` is generic or explicitly overloaded without introducing unsafe `any`.
- [closed] Do not implement adapters, resolver routing, graph UI, runtime behavior switches, or new function-library entries under Slice B.
- [closed] Implement Slice C only: shadow adapter-library metadata, policy validation, materializer/native-reader proof, and checked-in generated JSON.
- [closed] Preserve current visible Color Pipeline behavior and live compatibility behavior; adapters must not be inserted into runtime routes in this slice.
- [closed] Prove lossy adapters cannot be marked `safe`, non-safe adapters carry fail-closed reasons, and explicit-only adapters remain metadata until a later explicit resolver/UI consent slice.
- [closed] Implement Slice D only: shadow edge-resolution policy, current linear route audit, materializer/native-reader proof, and checked-in generated JSON.
- [closed] Preserve current visible Color Pipeline behavior and live compatibility behavior; the resolver must not own live compatibility in this slice.
- [closed] Prove known-good pilot routes resolve deterministically and known-bad typed routes fail closed with specific reasons.
- [closed] Implement Slice E only: classify current compatibility rows as typed-resolved, true runtime legacy override, or unsupported/deferred.
- [closed] Add `compat_override` shadow metadata only for true runtime seams, with stable ids, explicit reasons, owner seams, and proof rails.
- [closed] Preserve current visible Color Pipeline behavior and live compatibility behavior; do not delete or switch the existing compatibility runtime path.
- [closed] Implement Slice F only: lift current SDF gate/downsample/applicator capability into shadow metadata.
- [closed] Preserve existing Source-row storage authority: `signal.sdf_gate`, `signal.sdf_gate_width_px`, `signal.sdf_sample_step`, and `signal.sdf_field_downsample`.
- [closed] Do not add new SDF source functions, field producers, visible controls, graph UI, or live compatibility switching under this slice.
- [closed] Implement Slice G1 only: expose typed-route/compat-override explanation lookup from installed materialized metadata.
- [closed] Preserve legacy compatibility behavior as authoritative; do not switch `TryBuildColorPipelineSelectionFromLaneIds(...)` to typed resolver behavior under G1.
- [closed] Do not add visible UI, graph UI, function-library entries, or runtime Salticid dependency under this slice.
- [closed] Implement Slice G2 only: switch exactly one pilot compatibility route, `smooth_escape_ramp / identity / heatmap / contrast_lift`, to typed resolver authority.
- [closed] Preserve visible Color Pipeline behavior and runtime tuple output; this is an authority switch, not a UI or visual change.
- [closed] Add and prove a temporary fallback/kill switch that restores legacy materialized compatibility behavior.
- [closed] Do not expand the function library, add graph UI, remove hardcoded fallback, or import Salticid runtime code.
- [closed] Implement Slice H only: function-library Batch 1.
- [closed] Classify `sdf_curvature_signed` before implementation as new function, alias, signed-preserving variant, or diagnostic-only.
- [closed] Classify `normal_angle_palette` before implementation as distinct function or alias.
- [closed] Ship only a small function set with real runtime backing, typed ports, parameter metadata, compatibility route proof, no-mouse runtime sensitivity proof, and measured/no-regression evidence.
- [closed] Preserve the current Color Pipeline layout and row editor workflow; graph UI, new fractal lanes, broad function dump, and Salticid runtime dependency remain out of scope.

## Current Repo Truth

- `docs/ui_salt/color_pipeline_function_library.ui.salt` is the authoring surface for current Color Pipeline function metadata.
- `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json` is the materialized viewer authority.
- `ui_app/src/color_pipeline_metadata_contract.*` parses the current materialized contract.
- The current materialized contract has lane functions, coarse `signal_kind`, recipes, row applicators, and legacy compatibility/recipe seams.
- Current `signal_kind` only distinguishes `scalar`, `phase`, and `categorical`; it does not encode domain, topology, arity, units, field/color/mask surfaces, or adapter policy.
- Current `row_applicator` metadata is useful but still tied to Source-row SDF gate storage and repeated SDF row parameters.

## Campaign Goal

Build the next metadata layer that lets the current linear Color Pipeline remain stable while the underlying contract becomes graph-capable:

- typed signal/surface registry;
- explicit function input/output ports;
- first-class adapter library;
- edge-resolution policy and route/audit artifacts;
- legacy compatibility overrides only where runtime seams truly require them;
- function-library expansion gated by typed route proof.

The goal is not to replace the UI yet. The goal is to stop growing a brittle `compat(source, shape, palette, grading)` matrix and make future graph UI consume an already-proven typed route contract.

## Type Semantics Lock

These terms are binding for Slice A and all later typed-edge work:

- `field.*` means a raw field or capability surface. It is not palette-ready row signal data and cannot flow through Shape or Palette lanes without an explicit adapter.
- `scalar.*` means sampled single-channel values that can flow through Shape and Palette lanes when their domain matches the receiving port.
- `category.*` means discrete semantic classes. Categories cannot be normalized into scalar domains unless an adapter with explicit policy allows that diagnostic or user-consented conversion.
- `palette.*` means palette-facing discrete/index domains. It is not a synonym for general category values.
- `identity` is generic or overloaded as `identity<T>: T -> T`. It is never `any -> any`, and it never erases type information.
- Shape functions are scalar-domain transforms unless a function explicitly declares an overload for another type.
- A signed scalar remains signed until an explicit signed-to-unit adapter normalizes it. Unit-domain scalar flow must not silently absorb signed values.


## Slice A - Signal Type Registry Shadow Contract

Scope:

- Extend UI-Salt materialization with `viewer.signal_type_registry_contract.v1`.
- Add initial signal/surface types only:
  - `scalar.unit`
  - `scalar.signed`
  - `scalar.sdf_signed_distance`
  - `phase.radians`
  - `category.root_index`
  - `palette.discrete_index`
  - `mask.alpha`
  - `color.linear_rgb`
  - `field.sdf_signed_distance`
- Treat `field.sdf_signed_distance` as raw SDF field/capability/applicator authority only.
- Treat `scalar.sdf_signed_distance` as the sampled Source-row output that can enter scalar Shape/Palette lanes through declared adapters.
- Track `kind`, `domain`, `topology`, `arity`, optional units/period/color-space/coordinate-space, and `default_adapter_policy`.
- Preserve existing `signal_kind` in generated function descriptors as compatibility output.
- Add a coarse-to-typed mapping audit for every legacy `scalar | phase | categorical` pilot function before any runtime switch.

Tests:

- Materializer accepts valid type declarations.
- Materializer rejects duplicate ids, unknown kinds, invalid topology/domain combinations, invalid periods, ambiguous palette/category declarations, and missing default adapter policy.
- Metadata parser validates the generated registry without changing runtime behavior.
- Coarse-to-typed mapping audit proves every current pilot function maps from legacy `scalar | phase | categorical` into precise type ids.
- Audit must distinguish raw SDF field authority from sampled SDF signed-distance Source output.

Out of scope:

- Runtime routing changes.
- New visible controls.
- Full graph recipes.

## Slice B - Pilot Function Port Signatures

Scope:

- Add explicit `inputs` and `outputs` to a small pilot function subset:
  - sources: `smooth_escape_ramp`, `phase_orbit`, `root_index`, `sdf_signed_distance`, `sdf_normal_angle`, `sdf_inside_outside`;
  - shapes: `identity`, `repeat`, `bias_gain_curve`;
  - palettes: `heatmap`, `phase_wheel_palette`, `root_classic_palette`;
  - grading: `contrast_lift`, `phase_finish`, `basin_default`, `neutral_finish`.
- Preserve the current lane/function descriptor shape for C++ compatibility.
- Define `identity<T>: T -> T` or explicit identity overloads for each supported type; no `any` ports are allowed.
- Require every pilot function to declare one canonical output for the current linear UI projection.
- Declare `repeat` and `bias_gain_curve` as `scalar.unit -> scalar.unit` transforms.
- Add a shadow port-signature audit artifact.

Tests:

- Every declared port references a known signal type.
- Source functions have no input ports and one canonical output port for the current linear UI projection.
- Shape functions declare signal-to-signal transforms and only accept non-scalar inputs when explicitly overloaded.
- `identity` is proven generic/overloaded without erasing the concrete type.
- Palettes declare signal-to-color materialization.
- Grading declares color-to-color transforms.
- Pilot metadata does not reorder, remove, or relabel current visible rows.

Out of scope:

- Making every function port-typed in one slice.
- Runtime route resolution.

## Slice C - Adapter Library Contract

Scope:

- Add `viewer.adapter_library_contract.v1`.
- Add first adapters only:
  - identity for `scalar.unit`, `scalar.sdf_signed_distance`, `phase.radians`, `color.linear_rgb`, and `mask.alpha`;
  - `scalar.signed -> scalar.unit` rescale/bias/clamp;
  - `scalar.sdf_signed_distance -> scalar.unit` normalization;
  - `phase.radians -> scalar.unit` wrap-normalize;
  - `scalar.unit -> phase.radians`;
  - `category.root_index -> palette.discrete_index`;
  - `field.sdf_signed_distance -> mask.alpha` boundary mask adapter.
- Record exactly one adapter policy enum value: `safe | visible_default | explicit_only | diagnostic_only | forbidden`.
- Record lossiness, reversibility, cost, and fail-closed reason.
- Enforce that `lossy=true` cannot be `safe`.
- Enforce that `explicit_only` adapters cannot be inserted by the live resolver unless explicit UI/state consent exists.

Tests:

- Reject adapters with unknown source/target types, missing policy, invalid cost, missing fail-closed reason for non-safe adapters, or unsafe lossy defaults.
- Prove risky conversions are not silently safe: categorical-to-scalar, color-to-scalar, signed-to-unit without normalization policy.
- Materialized JSON is consumed by parser/audit only, not runtime execution.

Out of scope:

- Visible adapter UI.
- Automatic runtime adapter insertion.

## Slice D - Edge Resolver Shadow Audit

Scope:

- Add `viewer.edge_resolution_contract.v1` and `viewer.color_pipeline_resolution_audit.v1`.
- Define connect rules for current linear projection:
  - source output to shape input;
  - shape output to palette input;
  - palette output to grading input.
- Add policy knobs: max adapter hops, allow/disallow lossy adapters, allow/disallow diagnostic adapters, preferred safe low-cost routes, fail-closed default.
- Add deterministic tie-breaks in this order: exact identity, safe non-lossy, lower cost, fewer hops, stable id/declaration order.
- The resolver must not revisit a type within one route.
- Generate a shadow route/audit artifact for pilot combinations without changing runtime behavior.
- Emit route inputs, candidate adapters, chosen route, rejected routes, policy blockers, costs, hop counts, and stable tie-break evidence so Slice E can mechanically classify current compatibility rows.

Tests:

- Known-good pilot routes resolve:
  - `smooth_escape_ramp -> identity -> heatmap -> contrast_lift`;
  - `phase_orbit -> identity -> phase_wheel_palette -> phase_finish`;
  - `root_index -> identity/root-safe path -> root_classic_palette -> basin_default`;
  - `sdf_normal_angle -> identity -> phase_wheel_palette -> phase_finish`;
  - `sdf_signed_distance -> bias_gain_curve -> heatmap -> contrast_lift`.
- Known-bad pilot routes fail closed with specific reasons:
  - `root_index -> repeat -> heatmap`;
  - `root_index -> heatmap` without `category.root_index -> palette.discrete_index` and a compatible root palette path;
  - `phase.radians -> root_classic_palette`;
  - `field.sdf_signed_distance -> phase_wheel_palette` without explicit sampled-signal adapter policy;
  - `color.linear_rgb -> scalar` without explicit adapter;
  - `field.sdf_signed_distance -> palette` without an explicit field-to-signal or mask adapter.
- Audit reports route counts, fail-closed counts, implicit adapters, explicit-only blockers, and missing runtime owners.

Out of scope:

- Switching live compatibility UI to the resolver.
- Graph UI.

## Slice E - Legacy Compat Override Demotion

Scope:

- Classify existing compatibility rows as either:
  - expressible by type/adapter routing;
  - true runtime legacy override;
  - obsolete or duplicate.
- Add `compat_override` metadata only for true runtime seams, especially root-basin dedicated paths.
- Require every `compat_override` entry to carry a stable id, explicit reason, owner seam, and proof rail.
- Keep existing runtime behavior unchanged while producing a shadow report that explains which current compat rows can be replaced later.

Tests:

- Every current product-supported route is either resolved by the typed edge resolver or explicitly covered by a compat override.
- Every compat override has a reason and proof rail.
- No unsupported route becomes silently accepted.

Out of scope:

- Deleting legacy compatibility code.

## Slice F - SDF Applicator Capability Cleanup

Scope:

- Lift SDF gate/downsample concepts into reusable capability/applicator metadata.
- Preserve current Source-row storage authority: existing `signal.sdf_gate`, `signal.sdf_gate_width_px`, `signal.sdf_sample_step`, and `signal.sdf_field_downsample`.
- Add metadata that says which Source functions support SDF applicators and which require SDF field capability.

Tests:

- SDF applicators require known SDF field capability.
- Non-SDF sources can be rejected or require explicit masks based on metadata.
- Existing SDF Normal Angle Diagnostic/Beauty behavior and row-local downsample remain unchanged.
- Existing SDF gate/downsample state keys round-trip through capture/replay and `fractal-state.json`.
- Capture/replay and `fractal-state.json` sidecar remain truthful.

Out of scope:

- New SDF functions or field producers.
- Per-edge graph masks in UI.

## Slice G - Metadata-Backed Compatibility Switch

Scope:

- After Slices A-F shadow audits are green, split the live switch into two bounded steps:
  - G1: diagnostic/explanation lookup uses the typed resolver where available, while legacy behavior remains authoritative;
  - G2: one pilot behavioral compatibility seam switches to typed resolver authority after G1 proof.
- Add a temporary typed-resolver fallback/kill switch before any behavioral switch.
- Legacy overrides remain hardcoded/metadata-backed for special runtime paths.
- Hardcoded fallback remains available if materialized metadata is missing, invalid, or the fallback/kill switch is active.

Tests:

- Visible function lists and controls remain unchanged.
- G1 exposes route/audit explanations without changing behavior.
- G2 proves one pilot supported combination behaves unchanged under typed resolver authority.
- Unsupported pilot combinations fail closed with route/audit reason.
- Fallback/kill switch restores legacy compatibility behavior.
- Published no-mouse runtime proof covers scalar, phase, categorical/root, SDF, unsupported route cases, and fallback behavior.

Out of scope:

- New UI layout.
- Full function-library expansion.

## Slice H - Function Library Expansion Batch 1

Prerequisite:

- Typed edge resolver has shadow and at least one live seam proof.

Scope:

- Add a small, typed, proof-covered first batch of functions rather than a broad dump.
- Candidate source additions:
  - `orbit_min_radius`
  - `orbit_average_radius`
  - `derivative_magnitude` if runtime-backed data is already available
  - `sdf_gradient_magnitude`
  - `sdf_curvature_signed`
- Before implementation, classify `sdf_curvature_signed` as exactly one of: new function, alias, signed-preserving variant, or diagnostic-only.
- Candidate shape additions:
  - `log_compress`
  - `signed_log_compress`
  - `smoothstep_range`
  - `triangle_wave`
  - `fold_centered`
- Candidate palette additions:
  - `diverging_signed_palette`
  - `two_tone_mask_palette`
  - `normal_angle_palette`
- Before implementation, classify `normal_angle_palette` as semantically distinct from `phase_wheel_palette` or mark it as an alias.

Selection rule:

- Ship only functions with real runtime backing, typed ports, parameter metadata, compatibility route proof, no-mouse runtime sensitivity proof, and no FPS regression beyond measured tolerance.

Out of scope:

- Graph editor.
- Large function-library dump.
- New fractal families as part of Color Pipeline metadata work.

## Slice I - Recipe V2 Graph-Ready Projection

Scope:

- Add `recipe_v2` metadata with nodes/edges plus `ui_projection="linear_color_stack"`.
- Materialize current preset recipes into both current recipe shape and graph-ready recipe shape.
- Keep `recipe_v2` as shadow metadata only.
- Existing recipe expansion remains live authority until a later explicit switch.
- Keep UI applying the existing row editor path.

Tests:

- Current presets expand identically.
- `recipe_v2` resolves through the edge resolver.
- Unsupported graph routes fail closed during materialization.

Out of scope:

- Rendering arbitrary graph recipes.
- Graph UI replacement.

## Slice J - Graph UI Replan Gate

Stop and replan after A-I.

Decide whether next work should be:

- visible adapter UI;
- function picker/layout improvements;
- limited branch/mask graph UI;
- authored SDF pack catalog/authoring UX;
- more SDF ops;
- Salticid adapter/removal path.

No graph UI work should begin until typed routes, adapters, audit receipts, compatibility switch, and one library expansion batch are proven.

## Proof Ledger

- Start state: `master` clean at `7f68f0d`, aligned with `origin/master`, rearward review `ok`.
- External review intake: attached review recommended signal/surface registry, port signatures, first-class adapters, edge resolver, compat override demotion, route audit output, and function-family expansion behind type proof.
- Repo scan: current UI-Salt surfaces are `docs/ui_salt/color_pipeline_function_library.ui.salt`, `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`, `tools/viewer_host_materialize_ui_salt.py`, and `ui_app/src/color_pipeline_metadata_contract.*`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "UI-Salt typed edge resolution preplanning" --profile catalog --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --contract docs/contracts/ui_salt_typed_edge_resolution_preplanning.contract.json` locked checkpoint token `ck:63046b2a`.
- Refinement slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "UI-Salt typed edge plan refinement" --profile catalog --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --contract docs/contracts/ui_salt_typed_edge_resolution_plan_refinement.contract.json` locked checkpoint token `ck:b8dc025d`.
- External refinement review intake: attached feedback required separating raw fields from sampled signals, categories from palette indices, generic identity from unsafe `any`, and signed scalar normalization from unit-domain scalar flow.
- Refinement contract validation: `artifacts/validation/ui_salt_typed_edge_plan_refinement_contract.json` passed.
- Refinement plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Refinement hostile audit validation: `artifacts/validation/ui_salt_typed_edge_plan_refinement_hostile_audit.json` passed with three refinement findings and clean re-read evidence.
- Refinement code quality: `artifacts/validation/ui_salt_typed_edge_plan_refinement_code_quality.json` passed baseline with score 93/100.
- Refinement diff check: `artifacts/validation/ui_salt_typed_edge_plan_refinement_diff_check.json` passed `git diff --check`.
- Slice A branch: `codex/ui-salt-signal-type-registry` from `22966f2`.
- Slice A contract: `docs/contracts/ui_salt_signal_type_registry_shadow.contract.json`.
- Slice A implementation: materializer accepts `signal_type_registry`, emits typed Source-row signals, regenerates `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`, and C++ parses/validates the shadow registry without changing live runtime behavior.
- Slice A RED: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` failed before implementation with `contract has invalid kind 'signal_type_registry'` and generated-contract freshness missing `signal_type_registry`.
- Slice A focused Python proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed `17 passed`.
- Slice A focused native proof: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core` passed `test_color_pipeline_core: passed=2747 failed=0`.
- Slice A contract validation: `artifacts/validation/ui_salt_signal_type_registry_shadow_contract.json` passed.
- Slice A plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Slice A hostile audit validation: `artifacts/validation/ui_salt_signal_type_registry_shadow_hostile_audit.json` passed with real findings recorded.
- Slice A code quality: `artifacts/validation/ui_salt_signal_type_registry_shadow_code_quality.json` passed baseline with score 93/100.
- Slice A logged Python proof: `artifacts/validation/ui_salt_signal_type_registry_shadow_pytest.json` passed.
- Slice A logged materialization proof: `artifacts/validation/ui_salt_signal_type_registry_shadow_materialize.json` passed.
- Slice A logged native proof: `artifacts/validation/ui_salt_signal_type_registry_shadow_native.json` passed.
- Slice A diff check: `artifacts/validation/ui_salt_signal_type_registry_shadow_diff_check.json` passed `git diff --check`.
- Slice B preflight: fast-forwarded `master` from `7f68f0d` to `86aa4a6`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-port-signatures`.
- Slice B contract: `docs/contracts/ui_salt_pilot_port_signatures_shadow.contract.json`.
- Slice B RED proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` initially failed because `port(...)` was unsupported and generated metadata lacked ports.
- Slice B focused Python proof after implementation/audit repair: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed 24 tests.
- Slice B focused native proof after implementation/audit repair: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_native_audit_fix.json` passed `test_color_pipeline_core: passed=2961 failed=0`.
- Slice B clean re-read: generated contract has zero `any`, `generic.any`, or `generic_group=any` ports; no live Color Pipeline runtime/catalog source files changed.
- Slice B final contract validation: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_contract.json` passed.
- Slice B final plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Slice B final hostile-audit validation: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_hostile_audit.json` passed.
- Slice B final code quality: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_code_quality.json` passed baseline with score 93/100.
- Slice B final logged materialization proof: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_materialize.json` passed.
- Slice B final logged Python proof: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_pytest.json` passed 24 tests.
- Slice B final logged native proof: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_native.json` passed `test_color_pipeline_core: passed=2961 failed=0`.
- Slice B final diff check: `artifacts/validation/ui_salt_pilot_port_signatures_shadow_diff_check.json` passed `git diff --check`.
- Slice C preflight: fast-forwarded `master` from `86aa4a6` to `9edd2d4`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-adapter-library-shadow`.
- Slice C contract: `docs/contracts/ui_salt_adapter_library_shadow.contract.json`.
- Slice C RED proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` initially failed because `adapter_library` contracts and `adapter(...)` statements were unsupported.
- Slice C native RED proof: `artifacts/validation/ui_salt_adapter_library_shadow_native_red.json` failed because the C++ materialized contract parser had no adapter struct or adapter vector.
- Slice C implementation: UI-Salt now materializes `viewer.adapter_library_contract.v1`, the generated JSON carries eleven shadow adapters, and C++ parses/validates adapter metadata without any live runtime insertion path.
- Slice C audit repair: hostile review found the native parser accepted empty adapter ids; `ui_app/tests/test_color_pipeline_core.cpp` now covers that case and C++ validation rejects it.
- Slice C focused Python proof: `artifacts/validation/ui_salt_adapter_library_shadow_pytest.json` passed `29 passed`.
- Slice C focused materialization proof: `artifacts/validation/ui_salt_adapter_library_shadow_materialize.json` passed.
- Slice C focused native proof: `artifacts/validation/ui_salt_adapter_library_shadow_native.json` passed `test_color_pipeline_core: passed=2976 failed=0`.
- Slice C contract validation: `artifacts/validation/ui_salt_adapter_library_shadow_contract.json` passed.
- Slice C plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md` passed.
- Slice C code quality: `artifacts/validation/ui_salt_adapter_library_shadow_code_quality.json` passed baseline with score 93/100.
- Slice C diff check: `artifacts/validation/ui_salt_adapter_library_shadow_diff_check.json` passed `git diff --check`.
- Slice D preflight: fast-forwarded `master` from `9edd2d4` to `cecbb83`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-edge-resolver-shadow`.
- Slice D contract: `docs/contracts/ui_salt_edge_resolver_shadow.contract.json`.
- Slice D RED proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` failed before implementation with `contract has invalid kind 'edge_resolution'`.
- Slice D implementation: UI-Salt now materializes `viewer.edge_resolution_contract.v1` and `viewer.color_pipeline_resolution_audit.v1`, including the current linear Source -> Shape -> Palette -> Grading links, deterministic route cases, adapter consent gates, route edge details, hop/cost totals, tie-break rule, and policy blockers. The generated JSON is checked in and C++ parses/validates the shadow resolver audit without changing live compatibility behavior.
- Slice D audit repair: hostile review found the first implementation missed the planned raw SDF field -> phase palette bad route and underreported route audit detail. The materializer fixture now proves raw `field.sdf_signed_distance` fails closed before phase palette routing, and generated/native audit metadata now carries adapter hops, adapter costs, tie-break rule, and policy blockers.
- Slice D focused Python proof: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed `32 passed`.
- Slice D generated-contract freshness proof: `fc.exe /b docs\ui_salt\generated\color_pipeline_function_library.contract.v1.json artifacts\validation\ui_salt_edge_resolver_shadow_generated_check.json` reported no differences.
- Slice D focused native proof after audit repair: `artifacts/validation/ui_salt_edge_resolver_shadow_build_tests_rerun.log` passed `test_color_pipeline_core: passed=2986 failed=0` and ended with `All helper tests passed`.
- Slice D final contract validation: `artifacts/validation/ui_salt_edge_resolver_shadow_contract.json` passed.
- Slice D final plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md` passed.
- Slice D final hostile-audit validation: `artifacts/validation/ui_salt_edge_resolver_shadow_hostile_audit.json` passed.
- Slice D final code quality: `artifacts/validation/ui_salt_edge_resolver_shadow_code_quality.json` passed baseline with score 93/100.
- Slice D final logged Python proof: `artifacts/validation/ui_salt_edge_resolver_shadow_pytest.json` passed `32 passed`.
- Slice D final logged materialization proof: `artifacts/validation/ui_salt_edge_resolver_shadow_materialize.json` passed.
- Slice D final logged native proof: `artifacts/validation/ui_salt_edge_resolver_shadow_native.json` passed `test_color_pipeline_core: passed=2986 failed=0` using the focused native target.
- Slice D final diff check: `artifacts/validation/ui_salt_edge_resolver_shadow_diff_check.json` passed `git diff --check`.
- Slice E preflight: fast-forwarded `master` from `cecbb83` to `856ebe9`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-compat-override-demotion`.
- Slice E contract: `docs/contracts/ui_salt_compat_override_demotion.contract.json`.
- Slice E contract validation: `artifacts/validation/ui_salt_compat_override_demotion_contract.json` passed.
- Slice E code-quality baseline: `artifacts/validation/ui_salt_compat_override_demotion_code_quality.json` passed with score `93/100`.
- Slice E logged materializer proof: `artifacts/validation/ui_salt_compat_override_demotion_pytest.json` passed `35 passed`.
- Slice E logged contract regeneration proof: `artifacts/validation/ui_salt_compat_override_demotion_materialize.json` passed.
- Slice E logged native proof: `artifacts/validation/ui_salt_compat_override_demotion_native.json` passed `test_color_pipeline_core: passed=2995 failed=0`.
- Slice E logged diff check: `artifacts/validation/ui_salt_compat_override_demotion_diff_check.json` passed.
- Slice F preflight: fast-forwarded `master` to `9fafabb`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-sdf-applicator-capability`.
- Slice F contract: `docs/contracts/ui_salt_sdf_applicator_capability.contract.json`.
- Slice F contract validation: `artifacts/validation/ui_salt_sdf_applicator_capability_contract.json` passed.
- Slice F code-quality baseline: `artifacts/validation/ui_salt_sdf_applicator_capability_code_quality.json` passed with score `93/100`.
- Slice F logged materializer proof: `artifacts/validation/ui_salt_sdf_applicator_capability_pytest.json` passed `38 passed`.
- Slice F logged contract regeneration proof: `artifacts/validation/ui_salt_sdf_applicator_capability_materialize.json` passed.
- Slice F logged native proof: `artifacts/validation/ui_salt_sdf_applicator_capability_native.json` passed `test_color_pipeline_core: passed=3000 failed=0`.
- Slice F logged diff check: `artifacts/validation/ui_salt_sdf_applicator_capability_diff_check.json` passed.
- Slice G1 preflight: fast-forwarded `master` to `937e388`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-compat-diagnostics-g1`.
- Slice G1 contract: `docs/contracts/ui_salt_compat_diagnostics_g1.contract.json`.
- Slice G1 contract validation: `artifacts/validation/ui_salt_compat_diagnostics_g1_contract.json` passed.
- Slice G1 code-quality baseline: `artifacts/validation/ui_salt_compat_diagnostics_g1_code_quality.json` passed with score `93/100`.
- Slice G1 logged native proof: `artifacts/validation/ui_salt_compat_diagnostics_g1_native.json` passed `test_color_pipeline_core: passed=3008 failed=0`.
- Slice G1 logged diff check: `artifacts/validation/ui_salt_compat_diagnostics_g1_diff_check.json` passed.
- Slice G2 preflight: fast-forwarded `master` to `1dc8fdb`, pushed `origin/master`, reran rearward review `ok`, and branched `codex/ui-salt-compat-switch-g2`.
- Slice G2 contract: `docs/contracts/ui_salt_compat_switch_g2.contract.json`.
- Slice G2 contract validation: `artifacts/validation/ui_salt_compat_switch_g2_contract.json` passed.
- Slice G2 code-quality baseline: `artifacts/validation/ui_salt_compat_switch_g2_code_quality.json` passed with score `93/100`.
- Slice G2 logged native proof: `artifacts/validation/ui_salt_compat_switch_g2_native.json` passed `test_color_pipeline_core: passed=3013 failed=0`.
- Slice G2 logged runtime publish: `artifacts/validation/ui_salt_compat_switch_g2_runtime_publish.json` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Slice G2 published-runtime proof: `artifacts/validation/ui_salt_compat_switch_g2_runtime_proof.json` passed `2 passed`.
- Slice G2 logged diff check: `artifacts/validation/ui_salt_compat_switch_g2_diff_check.json` passed.
- Slice G2 receipt preflight found a contract-proof assertion mismatch: the published-runtime proof evidence was the pytest JUnit artifact, not the logged-command JSON. The contract now asserts the concrete runtime JUnit testcase.
- Slice H branch: `codex/ui-salt-function-library-batch1` from clean `master` at `2281377`; rearward review for `2281377` reported `ok`.
- Slice H contract: `docs/contracts/ui_salt_function_library_batch1.contract.json`.
- Slice H classification gate: `sdf_curvature_signed` and `normal_angle_palette` are not allowed to ship as new entries until current runtime semantics prove they are not just aliases or diagnostic-only labels.
- Slice H initial selection bias: prefer a small Shape-row batch first if repo inspection confirms those functions can use existing row stack/runtime/state paths without new field producers or source-signal sidecars.
- Slice H classification result: current `sdf_curvature` already computes signed curvature through the SDF postprocess path, so `sdf_curvature_signed` is not a new runtime-backed function in this batch; it remains a future label/alias/diagnostic naming decision.
- Slice H classification result: current `phase_wheel_palette` already owns the available phase-wheel palette behavior, so `normal_angle_palette` would be an alias unless a later slice adds distinct phase-safe behavior; it is not shipped in Batch 1.
- Slice H shipped functions: `log_compress` and `smoothstep_range` are Shape-row functions with C++ enum/id mapping, runtime math, row-state import/build support, diagnostics/state mirror coverage, UI-Salt typed ports, generated metadata, native catalog/schema/state tests, and published no-mouse pixel sensitivity proof.
- Slice H validation receipts before checkpoint: contract validation passed at `artifacts/validation/ui_salt_function_library_batch1_contract.json`; materializer pytest passed at `artifacts/validation/ui_salt_function_library_batch1_pytest.json`; materialization passed at `artifacts/validation/ui_salt_function_library_batch1_materialize.json`; code quality baseline passed at `artifacts/validation/ui_salt_function_library_batch1_code_quality.json`; native rail passed at `artifacts/validation/ui_salt_function_library_batch1_native.json`; runtime publish passed at `artifacts/validation/ui_salt_function_library_batch1_runtime_publish.json`; published runtime proof passed at `artifacts/validation/ui_salt_function_library_batch1_runtime_proof.json`; diff check passed at `artifacts/validation/ui_salt_function_library_batch1_diff_check.json`.
- Contract validation: `artifacts/validation/ui_salt_typed_edge_preplanning_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `artifacts/validation/ui_salt_typed_edge_preplanning_hostile_audit.json` passed with two real planning findings and clean re-read evidence.
- Code quality: `artifacts/validation/ui_salt_typed_edge_preplanning_code_quality.json` passed baseline with score 93/100.
- Diff check: `artifacts/validation/ui_salt_typed_edge_preplanning_diff_check.json` passed `git diff --check`.

## Hostile Audit

- Status: complete
- Required posture: assume this plan accidentally overreaches into UI replacement, ships dead function rows, skips proof before function expansion, creates unsafe aliases, lets port metadata alter live compatibility behavior, or lets Batch 1 bypass no-mouse runtime sensitivity proof.

Required questions:

- Does the campaign preserve the current UI until typed metadata is proven?
- Does it avoid adding more raw `compat(...)` rows as the primary strategy?
- Does it require adapter policy and fail-closed reasons before compatibility expands?
- Does it put function-library expansion behind typed ports, runtime backing, and runtime sensitivity proof?
- Does it defer graph UI replacement until the route contract is proven?
- Does it avoid a Salticid runtime dependency?
- Does the type system separate raw fields, sampled scalars, categories, and palette domains instead of renaming coarse `signal_kind`?
- Does identity preserve concrete type information instead of becoming `any`?
- Does the first live switch start with diagnostics and keep a fallback/kill switch before behavior changes?
- Does Batch 1 prove every shipped new function is runtime-backed and changes pixels through the published viewer?
- Did `sdf_curvature_signed` and `normal_angle_palette` get classified before any implementation tried to expose them?
- Did the slice avoid graph UI, new fractal lanes, and broad function-library dumping?

## Audit Passes

- [x] Pass 1 - reviewed the external feedback against current repo metadata surfaces.
- [x] Pass 2 - split the work into shadow metadata slices before any live runtime switch.
- [x] Pass 3 - clean re-read found the first library expansion is gated behind typed route proof rather than bundled into the metadata infrastructure.
- [x] Pass 4 - reviewed the refinement feedback against Slices A-D and found raw SDF fields, sampled SDF signals, palette indices, and categories were still too easy to blur.
- [x] Pass 5 - reviewed the live-switch plan and found Slice G needed an explanation-only step before any behavioral compatibility switch.
- [x] Pass 6 - clean re-read found graph UI, runtime behavior changes, and materializer implementation remain deferred out of this refinement slice.
- [x] Pass 7 - reviewed Slice A implementation diff for schema drift, accidental runtime behavior changes, and missing typed mapping checks.
- [x] Pass 8 - repaired Slice A audit findings and re-ran focused Python and native validations.
- [x] Pass 9 - clean re-read after repair confirms no Slice A scope leak into adapters, resolver routing, graph UI, or visible workflow changes.
- [x] Pass 10 - reviewed Slice B implementation diff for unsafe `any`, missing canonical outputs, pilot subset drift, and accidental runtime behavior changes.
- [x] Pass 11 - repaired the real Slice B defect where `generic.any` and extra identity ports could pass validation, then revalidated Python and native rails.
- [x] Pass 12 - clean re-read confirmed zero generated `any`/`generic.any` ports, no live Color Pipeline behavior switch, and no graph UI, adapter, resolver, or function-library expansion changes.
- [x] Pass 13 - reviewed Slice C implementation diff for unsafe adapter policies, missing fail-closed reasons, source/target type drift, and accidental live runtime adapter insertion.
- [x] Pass 14 - repaired the real native validation gap where an empty adapter id could pass the C++ materialized-contract loader, then revalidated the focused native rail.
- [x] Pass 15 - clean re-read confirmed adapter metadata remains shadow-only: no Color Pipeline live compatibility switch, no resolver route insertion, no visible adapter UI, no graph UI, and no function-library expansion.
- [x] Pass 16 - reviewed Slice D implementation diff and found two real audit gaps: raw SDF field -> phase palette bad route coverage was missing, and route audit output lacked hop/cost/tie-break/blocker detail needed by later compat demotion.
- [x] Pass 17 - repaired the Slice D audit findings, reran focused Python proof, regenerated the checked-in JSON, and reran the native helper rail.
- [x] Pass 18 - clean re-read confirmed the repaired state has no live Color Pipeline compatibility switch, no visible UI change, no graph UI, no Salticid runtime dependency, and no function-library expansion beyond shadow resolver metadata.
- [x] Pass 19 - reviewed Slice E implementation diff and found a real audit gap: compat overrides could be added for rows already covered by direct typed routes, which would blur demotion authority. The materializer now rejects overrides that do not map to an untyped compatibility row, Python tests cover that rejection, and the native reader rejects duplicate override ids.
- [x] Pass 20 - clean re-read confirmed Slice E does not switch live compatibility authority, does not add visible UI, does not add graph UI, does not import Salticid runtime code, and does not expand the function library beyond shadow compat audit metadata.
- [x] Pass 21 - reviewed Slice F implementation diff and found a real capability metadata gap: applicator ids did not encode the persisted enum value, so `sdf_boundary_band` could have been miswritten as a state value instead of existing `boundary_band`. Row applicators now carry explicit `storage_value`, and Python/native tests assert the current values.
- [x] Pass 22 - clean re-read confirmed Slice F does not change visible Color Pipeline layout/control ids, does not change SDF state storage keys, does not add SDF functions or field producers, does not switch live compatibility authority, and does not import Salticid runtime code.
- [x] Pass 23 - reviewed Slice G1 implementation diff and found a real risk: explanation lookup could be mistaken for or accidentally coupled to live compatibility behavior. Added a regression proving an unsupported explained route still stays rejected by `TryBuildColorPipelineSelectionFromLaneIds(...)`.
- [x] Pass 24 - clean re-read confirmed Slice G1 does not switch live Color Pipeline compatibility, does not delete hardcoded fallback behavior, does not add visible UI, does not add graph UI, does not expand the function library, and does not import Salticid runtime code.
- [x] Pass 25 - reviewed Slice G2 implementation and found a real risk: a generic typed-resolved lookup could silently switch every typed route instead of the one approved pilot. The implementation now gates the switch through `IsColorPipelineTypedResolverPilotRoute(...)`, and tests prove `phase_orbit / phase_wheel_palette` stays on materialized compatibility.
- [x] Pass 26 - clean re-read confirmed Slice G2 does not change visible Color Pipeline controls, does not expand the function library, does not add graph UI, does not delete hardcoded/materialized fallback, does not import Salticid runtime code, and does not allow unsupported routes.
- [x] Pass 27 - receipt preflight exposed a real workflow defect: the contract assertion for published runtime proof pointed at logged-command JSON while validation evidence recorded the pytest JUnit testcase. The contract assertion now uses `runtime_junit_case` for the published runtime UI-Salt contract test.
- [x] Pass 28 - clean re-read confirmed the repaired contract remains scoped to G2, preserves the same required validation commands, and now matches the machine-written validation receipt evidence.
- [x] Pass 29 - review Slice H candidate classification for alias traps, diagnostic-only traps, and unsupported runtime seams before implementation.
- [x] Pass 30 - review Batch 1 implementation for dead rows, missing typed ports, stale generated metadata, state/capture omissions, and UI workflow drift.
- [x] Pass 31 - clean re-read after repairs confirms Batch 1 shipped only proven runtime-backed entries and left graph UI/new fractal lanes/Salticid runtime out of scope.

## Audit Findings

- [x] The initial tempting slice would have mixed typed metadata infrastructure with function-library expansion. This plan now explicitly gates function expansion behind typed port, adapter, resolver, and live compatibility proof.
- [x] The current SDF applicator metadata could become another duplicated Source-row parameter bundle. The plan includes a dedicated SDF applicator capability cleanup slice that preserves storage authority while extracting reusable metadata.
- [x] Clean re-read after splitting the slices found no additional scope leak into graph UI replacement, Salticid runtime dependency, or new visible rows.
- [x] The refined type system could have recreated coarse `signal_kind` with nicer names. The plan now locks `field.*`, `scalar.*`, `category.*`, and `palette.*` semantics and requires a coarse-to-typed mapping audit.
- [x] The prior adapter language could allow lossy routes to look safe. The plan now uses one policy enum, forbids `lossy=true` with `safe`, and blocks `explicit_only` insertion without explicit consent.
- [x] The prior Slice G switch was too broad. The plan now splits G1 diagnostics from G2 one-pilot behavior and requires a temporary fallback/kill switch.
- [x] Slice A first implementation missed an existing categorical SDF Source-row domain: `sdf_inside_outside` needed `category.inside_outside` in addition to the originally listed `category.root_index`. The registry now includes that current shipped category without adding new behavior.
- [x] Slice A native rail caught a malformed C++ error string and a tampered-fixture redeclaration after adding parser tests. Both were repaired before closeout, and the native rail now passes.
- [x] Clean Slice A re-read found no adapters, resolver routing, graph UI, visible workflow changes, or function-library expansion in this implementation.
- [x] Slice B hostile audit found the first port-validation implementation still allowed `generic.any` and allowed `identity` to carry extra ports beyond the required generic `T -> T` pair. Python and C++ validation now reject both, with focused regressions.
- [x] Clean Slice B re-read found no adapters, resolver routing, graph UI, visible workflow changes, live compatibility switch, or new function-library entries in this implementation.
- [x] Slice C hostile audit found that the Python materializer rejected empty adapter ids but the native JSON parser would accept `id=""` if unique. Native validation now rejects empty adapter ids and `test_color_pipeline_core` covers the regression.
- [x] Clean Slice C re-read found no live runtime adapter insertion, no visible Color Pipeline behavior change, no graph UI, no Salticid runtime dependency, and no function-library expansion beyond shadow adapter metadata.
- [x] Slice D hostile audit found the first implementation did not prove raw `field.sdf_signed_distance -> phase_wheel_palette` fail-closed behavior and did not emit enough audit detail for later compatibility demotion. The materializer fixture now covers the raw-field bad route, generated cases expose adapter hops/costs/tie-break policy/policy blockers, and the C++ reader validates those totals.
- [x] Clean Slice D re-read found no live runtime route insertion, no visible Color Pipeline behavior change, no graph UI, no Salticid runtime dependency, and no function-library expansion beyond shadow resolver metadata.
- [x] Slice E hostile audit found the first pass could have allowed redundant/incorrect compat overrides to sit beside direct typed-resolved routes. The materializer now classifies only direct identity/no-adapter route cases as `typed_resolved`, requires override ids/reasons/owner seams/proof for remaining runtime rows, rejects overrides on already typed rows, and the native contract reader rejects duplicate compat override ids.
- [x] Clean Slice E re-read found no live Color Pipeline compatibility switch, no visible UI change, no graph UI, no Salticid runtime dependency, and no function-library expansion beyond shadow compat override metadata.
- [x] Slice F hostile audit found the first pass did not preserve the distinction between applicator id and persisted enum value. The metadata now records `storage_value` for every row applicator (`none`, `boundary_band`, `sdf_inside`, `sdf_outside`) while preserving `signal.sdf_gate` as the storage key.
- [x] Clean Slice F re-read found no visible UI change, no SDF storage-key churn, no new SDF source functions, no field-producer changes, no graph UI, no Salticid runtime dependency, and no live compatibility switch.
- [x] Slice G1 hostile audit found the first pass lacked an explicit guard that diagnostics cannot switch live compatibility behavior. The test now explains unsupported `phase_orbit / heatmap / contrast_lift` metadata while separately proving the live builder still rejects it.
- [x] Clean Slice G1 re-read found no visible UI change, no graph UI, no function-library expansion, no hardcoded fallback deletion, no Salticid runtime dependency, and no live compatibility behavior switch.
- [x] Slice G2 hostile audit found the first pass could over-broaden typed resolver authority beyond the pilot route. The final code gates the switch to `smooth_escape_ramp / heatmap`, proves the runtime tuple matches hardcoded output, proves the kill switch falls back to `materialized_json`, and keeps unsupported routes rejected.
- [x] Clean Slice G2 re-read found no visible UI change, no graph UI, no function-library expansion, no fallback deletion, no Salticid runtime dependency, and no physical mouse automation.
- [x] Slice G2 receipt audit found the first contract assertion for published runtime proof used the wrong evidence artifact. The contract now binds the acceptance assertion to `artifacts/pytest/ui_salt_compat_switch_g2_runtime.junit.xml` and testcase `tests/test_fractal_runtime_ui_salt_contract.py::test_published_runtime_consumes_staged_ui_salt_contract`.
- [x] Clean Slice G2 contract re-read found no validation command removal, no weaker runtime proof, and no change to product scope.
- [x] Slice H classification found `sdf_curvature_signed` would be an alias/label decision because current `sdf_curvature` already computes a signed SDF curvature value in CPU/CUDA postprocess; Batch 1 did not ship it as a fake new row.
- [x] Slice H classification found `normal_angle_palette` would be an alias of `phase_wheel_palette` without new phase-safe behavior; Batch 1 did not ship it as a fake new row.
- [x] Slice H implementation proves new rows are not catalog-only: `log_compress` and `smoothstep_range` have C++ runtime backing, typed ports, regenerated metadata, native catalog/schema/state proof, and published no-mouse frame-hash sensitivity proof.
- [x] Hostile audit finding: first native rail exposed missing runtime-backed filtering for the new Shape rows; fixed by adding `log_compress` and `smoothstep_range` to `IsColorPipelineFunctionRuntimeBacked(...)` and extending the native parity/count assertions.
- [x] Hostile audit finding: first runtime proof exposed a nested automation lock bug in the new no-mouse test helper; fixed by removing the nested lock and relying on the existing autouse runtime automation lock.
- [x] Hostile audit finding: code-quality baseline exposed function growth in `BuildColorPipelineShapeFunctions()` and `MirrorLegacyColorShapeFromStackEntry(...)`; fixed by splitting the Shape catalog builder and sharing legacy window-shape mirror logic.

## Slice H Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_function_library_batch1.contract.json --out-json artifacts/validation/ui_salt_function_library_batch1_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_function_library_batch1_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_function_library_batch1_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_pytest --log artifacts/logs/ui_salt_function_library_batch1_pytest.log --out-json artifacts/validation/ui_salt_function_library_batch1_pytest.json --heartbeat-seconds 30 --timeout-seconds 180 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_materialize --log artifacts/logs/ui_salt_function_library_batch1_materialize.log --out-json artifacts/validation/ui_salt_function_library_batch1_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_native --log artifacts/logs/ui_salt_function_library_batch1_native.log --out-json artifacts/validation/ui_salt_function_library_batch1_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_schema_binding test_diagnostics_state_io test_fractal_types`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_runtime_publish --log artifacts/logs/ui_salt_function_library_batch1_runtime_publish.log --out-json artifacts/validation/ui_salt_function_library_batch1_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_runtime_proof --log artifacts/logs/ui_salt_function_library_batch1_runtime_proof.log --out-json artifacts/validation/ui_salt_function_library_batch1_runtime_proof.json --heartbeat-seconds 30 --timeout-seconds 360 -- py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_presets.py::test_color_pipeline_function_library_batch1_shapes_are_runtime_backed_no_mouse tests/test_fractal_runtime_ui_salt_contract.py::test_published_runtime_consumes_staged_ui_salt_contract -q --junitxml artifacts/pytest/ui_salt_function_library_batch1_runtime.junit.xml`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_function_library_batch1_diff_check --log artifacts/logs/ui_salt_function_library_batch1_diff_check.log --out-json artifacts/validation/ui_salt_function_library_batch1_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice G2 Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_compat_switch_g2.contract.json --out-json artifacts/validation/ui_salt_compat_switch_g2_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_compat_switch_g2_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_compat_switch_g2_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_switch_g2_native --log artifacts/logs/ui_salt_compat_switch_g2_native.log --out-json artifacts/validation/ui_salt_compat_switch_g2_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_switch_g2_runtime_publish --log artifacts/logs/ui_salt_compat_switch_g2_runtime_publish.log --out-json artifacts/validation/ui_salt_compat_switch_g2_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_switch_g2_runtime_proof --log artifacts/logs/ui_salt_compat_switch_g2_runtime_proof.log --out-json artifacts/validation/ui_salt_compat_switch_g2_runtime_proof.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_fractal_runtime_ui_salt_contract.py -q --junitxml artifacts/pytest/ui_salt_compat_switch_g2_runtime.junit.xml`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_switch_g2_diff_check --log artifacts/logs/ui_salt_compat_switch_g2_diff_check.log --out-json artifacts/validation/ui_salt_compat_switch_g2_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice G1 Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_compat_diagnostics_g1.contract.json --out-json artifacts/validation/ui_salt_compat_diagnostics_g1_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_compat_diagnostics_g1_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_compat_diagnostics_g1_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_diagnostics_g1_native --log artifacts/logs/ui_salt_compat_diagnostics_g1_native.log --out-json artifacts/validation/ui_salt_compat_diagnostics_g1_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_diagnostics_g1_diff_check --log artifacts/logs/ui_salt_compat_diagnostics_g1_diff_check.log --out-json artifacts/validation/ui_salt_compat_diagnostics_g1_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice F Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_sdf_applicator_capability.contract.json --out-json artifacts/validation/ui_salt_sdf_applicator_capability_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_sdf_applicator_capability_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_sdf_applicator_capability_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_sdf_applicator_capability_pytest --log artifacts/logs/ui_salt_sdf_applicator_capability_pytest.log --out-json artifacts/validation/ui_salt_sdf_applicator_capability_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_sdf_applicator_capability_materialize --log artifacts/logs/ui_salt_sdf_applicator_capability_materialize.log --out-json artifacts/validation/ui_salt_sdf_applicator_capability_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_sdf_applicator_capability_native --log artifacts/logs/ui_salt_sdf_applicator_capability_native.log --out-json artifacts/validation/ui_salt_sdf_applicator_capability_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_sdf_applicator_capability_diff_check --log artifacts/logs/ui_salt_sdf_applicator_capability_diff_check.log --out-json artifacts/validation/ui_salt_sdf_applicator_capability_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice E Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_compat_override_demotion.contract.json --out-json artifacts/validation/ui_salt_compat_override_demotion_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_compat_override_demotion_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_compat_override_demotion_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_override_demotion_pytest --log artifacts/logs/ui_salt_compat_override_demotion_pytest.log --out-json artifacts/validation/ui_salt_compat_override_demotion_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_override_demotion_materialize --log artifacts/logs/ui_salt_compat_override_demotion_materialize.log --out-json artifacts/validation/ui_salt_compat_override_demotion_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_override_demotion_native --log artifacts/logs/ui_salt_compat_override_demotion_native.log --out-json artifacts/validation/ui_salt_compat_override_demotion_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compat_override_demotion_diff_check --log artifacts/logs/ui_salt_compat_override_demotion_diff_check.log --out-json artifacts/validation/ui_salt_compat_override_demotion_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice D Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_edge_resolver_shadow.contract.json --out-json artifacts/validation/ui_salt_edge_resolver_shadow_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_edge_resolver_shadow_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_edge_resolver_shadow_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_edge_resolver_shadow_pytest --log artifacts/logs/ui_salt_edge_resolver_shadow_pytest.log --out-json artifacts/validation/ui_salt_edge_resolver_shadow_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_edge_resolver_shadow_materialize --log artifacts/logs/ui_salt_edge_resolver_shadow_materialize.log --out-json artifacts/validation/ui_salt_edge_resolver_shadow_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_edge_resolver_shadow_native --log artifacts/logs/ui_salt_edge_resolver_shadow_native.log --out-json artifacts/validation/ui_salt_edge_resolver_shadow_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_edge_resolver_shadow_diff_check --log artifacts/logs/ui_salt_edge_resolver_shadow_diff_check.log --out-json artifacts/validation/ui_salt_edge_resolver_shadow_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice C Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_adapter_library_shadow.contract.json --out-json artifacts/validation/ui_salt_adapter_library_shadow_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_adapter_library_shadow_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_adapter_library_shadow_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_adapter_library_shadow_pytest --log artifacts/logs/ui_salt_adapter_library_shadow_pytest.log --out-json artifacts/validation/ui_salt_adapter_library_shadow_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_adapter_library_shadow_materialize --log artifacts/logs/ui_salt_adapter_library_shadow_materialize.log --out-json artifacts/validation/ui_salt_adapter_library_shadow_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_adapter_library_shadow_native --log artifacts/logs/ui_salt_adapter_library_shadow_native.log --out-json artifacts/validation/ui_salt_adapter_library_shadow_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_adapter_library_shadow_diff_check --log artifacts/logs/ui_salt_adapter_library_shadow_diff_check.log --out-json artifacts/validation/ui_salt_adapter_library_shadow_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice B Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_pilot_port_signatures_shadow.contract.json --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_pilot_port_signatures_shadow_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_pilot_port_signatures_shadow_pytest --log artifacts/logs/ui_salt_pilot_port_signatures_shadow_pytest.log --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_pilot_port_signatures_shadow_materialize --log artifacts/logs/ui_salt_pilot_port_signatures_shadow_materialize.log --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_pilot_port_signatures_shadow_native --log artifacts/logs/ui_salt_pilot_port_signatures_shadow_native.log --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_pilot_port_signatures_shadow_diff_check --log artifacts/logs/ui_salt_pilot_port_signatures_shadow_diff_check.log --out-json artifacts/validation/ui_salt_pilot_port_signatures_shadow_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Slice A Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_signal_type_registry_shadow.contract.json --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_signal_type_registry_shadow_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_signal_type_registry_shadow_pytest --log artifacts/logs/ui_salt_signal_type_registry_shadow_pytest.log --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_signal_type_registry_shadow_materialize --log artifacts/logs/ui_salt_signal_type_registry_shadow_materialize.log --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_materialize.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_materialize_ui_salt.py --ui-salt docs/ui_salt/color_pipeline_function_library.ui.salt --out docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_signal_type_registry_shadow_native --log artifacts/logs/ui_salt_signal_type_registry_shadow_native.log --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_signal_type_registry_shadow_diff_check --log artifacts/logs/ui_salt_signal_type_registry_shadow_diff_check.log --out-json artifacts/validation/ui_salt_signal_type_registry_shadow_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Refinement Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_typed_edge_resolution_plan_refinement.contract.json --out-json artifacts/validation/ui_salt_typed_edge_plan_refinement_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_typed_edge_plan_refinement_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_typed_edge_plan_refinement_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_typed_edge_plan_refinement_diff_check --log artifacts/logs/ui_salt_typed_edge_plan_refinement_diff_check.log --out-json artifacts/validation/ui_salt_typed_edge_plan_refinement_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_typed_edge_resolution_preplanning.contract.json --out-json artifacts/validation/ui_salt_typed_edge_preplanning_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_typed_edge_preplanning_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_typed_edge_preplanning_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_typed_edge_preplanning_diff_check --log artifacts/logs/ui_salt_typed_edge_preplanning_diff_check.log --out-json artifacts/validation/ui_salt_typed_edge_preplanning_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
