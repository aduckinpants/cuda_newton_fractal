# UI-Salt Typed Edge Resolution Campaign

## Current Phase

Phase 3 - validation, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0 - merge the SDF field-generation optimization to `master`, push, verify clean state, and branch `codex/ui-salt-typed-edge-preplanning`.
- [x] Phase 1 - create and lock this planning contract.
- [x] Phase 2 - document the bounded implementation slices for typed signals, adapters, edge resolution, and function-library expansion.
- [x] Phase 3 - validate contract, plan sync, hostile audit, code-quality baseline, diff check, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Plan bounded slices for applying the external UI-Salt architecture review.
- [closed] Resolve the worst architectural shortcomings before expanding function libraries.
- [closed] Keep graph UI replacement as a later/final step.
- [closed] Preserve the current visible Color Pipeline workflow until metadata proof is strong enough to switch seams.

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

## Slice A - Signal Type Registry Shadow Contract

Scope:

- Extend UI-Salt materialization with `viewer.signal_type_registry_contract.v1`.
- Add initial signal/surface types only:
  - `scalar.unit`
  - `scalar.signed`
  - `phase.radians`
  - `category.root_index`
  - `mask.alpha`
  - `color.linear_rgb`
  - `field.sdf_signed_distance`
- Track `kind`, `domain`, `topology`, `arity`, optional units/period/color-space/coordinate-space, and `default_adapter_policy`.
- Preserve existing `signal_kind` in generated function descriptors as compatibility output.

Tests:

- Materializer accepts valid type declarations.
- Materializer rejects duplicate ids, unknown kinds, invalid topology/domain combinations, invalid periods, and missing default adapter policy.
- Metadata parser validates the generated registry without changing runtime behavior.
- Parity/audit proves every current coarse `signal_kind` can map to at least one declared type for the planned pilot functions.

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
- Add a shadow port-signature audit artifact.

Tests:

- Every declared port references a known signal type.
- Source functions have no input ports and one output port.
- Shape functions declare signal-to-signal transforms.
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
  - identity for `scalar.unit`, `phase.radians`, `color.linear_rgb`, and `mask.alpha`;
  - `scalar.signed -> scalar.unit` rescale/bias/clamp;
  - `phase.radians -> scalar.unit` wrap-normalize;
  - `scalar.unit -> phase.radians`;
  - `category.root_index -> palette.index` or equivalent discrete palette surface if introduced in Slice A/B;
  - `field.sdf_signed_distance -> mask.alpha` boundary mask adapter.
- Record policy fields: `safe`, `visible-default`, `explicit-only`, `diagnostic-only`, `forbidden`, lossiness, reversibility, cost, and fail-closed reason.

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
- Generate a shadow route/audit artifact for pilot combinations without changing runtime behavior.

Tests:

- Known-good pilot routes resolve:
  - `smooth_escape_ramp -> identity -> heatmap -> contrast_lift`;
  - `phase_orbit -> identity -> phase_wheel_palette -> phase_finish`;
  - `root_index -> identity/root-safe path -> root_classic_palette -> basin_default`;
  - `sdf_normal_angle -> identity -> phase_wheel_palette -> phase_finish`;
  - `sdf_signed_distance -> bias_gain_curve -> heatmap -> contrast_lift`.
- Known-bad pilot routes fail closed with specific reasons:
  - `root_index -> repeat -> heatmap`;
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
- Capture/replay and `fractal-state.json` sidecar remain truthful.

Out of scope:

- New SDF functions or field producers.
- Per-edge graph masks in UI.

## Slice G - Metadata-Backed Compatibility Switch

Scope:

- After Slices A-F shadow audits are green, switch one live seam:
  - compatibility/fail-closed explanation lookup uses the typed resolver where available;
  - legacy overrides remain hardcoded/metadata-backed for special runtime paths;
  - hardcoded fallback remains available if materialized metadata is missing or invalid.

Tests:

- Visible function lists and controls remain unchanged.
- Supported pilot combinations behave unchanged.
- Unsupported pilot combinations fail closed with route/audit reason.
- Published no-mouse runtime proof covers scalar, phase, categorical/root, SDF, and unsupported route cases.

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
- Contract validation: `artifacts/validation/ui_salt_typed_edge_preplanning_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `artifacts/validation/ui_salt_typed_edge_preplanning_hostile_audit.json` passed with two real planning findings and clean re-read evidence.
- Code quality: `artifacts/validation/ui_salt_typed_edge_preplanning_code_quality.json` passed baseline with score 93/100.
- Diff check: `artifacts/validation/ui_salt_typed_edge_preplanning_diff_check.json` passed `git diff --check`.

## Hostile Audit

- Status: complete
- Required posture: assume this plan accidentally overreaches into UI replacement, skips proof before function expansion, or creates another broad metadata abstraction with no route to runtime proof.

Required questions:

- Does the campaign preserve the current UI until typed metadata is proven?
- Does it avoid adding more raw `compat(...)` rows as the primary strategy?
- Does it require adapter policy and fail-closed reasons before compatibility expands?
- Does it put function-library expansion behind typed ports, runtime backing, and runtime sensitivity proof?
- Does it defer graph UI replacement until the route contract is proven?
- Does it avoid a Salticid runtime dependency?

## Audit Passes

- [x] Pass 1 - reviewed the external feedback against current repo metadata surfaces.
- [x] Pass 2 - split the work into shadow metadata slices before any live runtime switch.
- [x] Pass 3 - clean re-read found the first library expansion is gated behind typed route proof rather than bundled into the metadata infrastructure.

## Audit Findings

- [x] The initial tempting slice would have mixed typed metadata infrastructure with function-library expansion. This plan now explicitly gates function expansion behind typed port, adapter, resolver, and live compatibility proof.
- [x] The current SDF applicator metadata could become another duplicated Source-row parameter bundle. The plan includes a dedicated SDF applicator capability cleanup slice that preserves storage authority while extracting reusable metadata.
- [x] Clean re-read after splitting the slices found no additional scope leak into graph UI replacement, Salticid runtime dependency, or new visible rows.

## Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_typed_edge_resolution_preplanning.contract.json --out-json artifacts/validation/ui_salt_typed_edge_preplanning_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_typed_edge_resolution_campaign_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_typed_edge_preplanning_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_typed_edge_preplanning_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_typed_edge_preplanning_diff_check --log artifacts/logs/ui_salt_typed_edge_preplanning_diff_check.log --out-json artifacts/validation/ui_salt_typed_edge_preplanning_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
