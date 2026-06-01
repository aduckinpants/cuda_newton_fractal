# UI-Salt Typed Edge Resolution Campaign

## Current Phase

Phase 8 complete - Slice C adapter-library shadow metadata is implemented and validated with current UI and live compatibility behavior frozen. Next work is Slice D edge resolver shadow audit after this branch is merged and a fresh slice is opened.

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
- Contract validation: `artifacts/validation/ui_salt_typed_edge_preplanning_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `artifacts/validation/ui_salt_typed_edge_preplanning_hostile_audit.json` passed with two real planning findings and clean re-read evidence.
- Code quality: `artifacts/validation/ui_salt_typed_edge_preplanning_code_quality.json` passed baseline with score 93/100.
- Diff check: `artifacts/validation/ui_salt_typed_edge_preplanning_diff_check.json` passed `git diff --check`.

## Hostile Audit

- Status: complete
- Required posture: assume this plan accidentally overreaches into UI replacement, skips proof before function expansion, creates unsafe `any` ports, lets port metadata alter live compatibility behavior, or lets adapter metadata become live automatic routing before the resolver/audit slices exist.

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
