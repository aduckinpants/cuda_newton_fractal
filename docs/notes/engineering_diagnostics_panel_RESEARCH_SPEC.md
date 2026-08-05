# Engineering Diagnostics Panel Research Specification

## Status

Research/specification only. This document does not authorize implementation of a new panel, UI controls, runtime probes, production behavior changes, or new tests.

> **2026-08-05 general-coverage amendment:** This selected-target probe specification is a narrower predecessor, not the governing implementation plan for a public diagnostics panel. Before any panel, live overlay, or annotated capture work, the catalog-wide capability and coordinate-space gates in [model_diagnostics_overlay_capability_campaign_PHASED_PLAN.md](model_diagnostics_overlay_capability_campaign_PHASED_PLAN.md) must be satisfied. Its MD-0 through MD-6 foundation replaces the earlier recommendation to begin directly with a Newton/ExplainO orbit UI. The target/registry/result-envelope concepts here remain useful inputs to that campaign.

## Summary Recommendation

Proceed, but narrow the first implementation to a selected-target probe registry vertical rather than a broad diagnostics workspace.

The repo already has useful ingredients: `fractal.sample` / `generic.sample` callable probes, widened per-sample evidence, runtime automation reports, Capture Finding sidecars, Color Pipeline graph receipts, SDF capability reports, frame hashes, and headless runtime-walk artifacts. What it does not yet have is a single user-facing diagnostic target model, probe registry, result envelope, lifecycle/cost policy, or shared CLI/IDE command contract.

Recommended first implementation after review:

1. Build a read-only probe registry and target identity contract.
2. Add one selected-coordinate Newton/ExplainO orbit trace probe.
3. Surface it through the same typed request/result path for UI, CLI, tests, and future agent use.
4. Stop before Science Mode, arbitrary scripting, or a full graph/workspace UI.

## Product Boundary

Engineering Diagnostics is for implementation/runtime inspection:

- convergence and terminal classification;
- precision/backend disagreement;
- resource and texture lifecycle;
- Color Pipeline provenance;
- runtime strategy and timing;
- state/frame identity;
- operator/channel provenance.

It is distinct from Science Mode. Science Mode may later compose probes into experiments, repeated trials, external analysis, or intervention packages, but the Engineering panel should first provide bounded, typed, read-only inspection.

It is also distinct from ordinary viewport controls. The panel should not become another color or camera editor. It should inspect what happened, why, and under what runtime identity.

Future agents should consume the same probe contracts. Agents must not get hidden diagnostics that users cannot run, inspect, or export themselves.

## Current Capability Inventory

### Callable Probe Surface

Existing source:

- `ui_app/src/fractal_probe_contract.h`
- `ui_app/src/fractal_probe_runner.cpp`
- `ui_app/tests/test_callable_engine_adversarial.cpp`
- `tests/test_callable_engine_adversarial_cli.py`

Current shape:

- `FractalProbeRequest` supports `function_id`, point-set mode, grid mode, sequence modes, backend preference, base-state loading, overrides, metrics, and generic function fields.
- Known callable IDs include `fractal.sample` and `generic.sample`.
- `FractalProbeResponse` returns `runtime`, `cost`, `summary`, `sequence_results`, and sampled records.
- Sample status already distinguishes `escaped`, `converged`, `bounded`, `pole`, `nonfinite`, and `invalid_param`.
- Tests assert unknown/empty functions fail instead of silently falling through.

Assessment: reusable as a backend execution substrate, but not yet a panel registry. It lacks display descriptors, target identity, cost classes, cancellation, visualizer hints, and saved result identity.

### Sample Evidence

Existing source:

- `ui_app/src/fractal_sample_result.h`
- `ui_app/src/fractal_sample_core.cu`
- `ui_app/src/fractal_sample_device.inl`

Current shape:

- `FractalSampleResult` is the legacy per-point result.
- `FractalSampleEvidence` adds sampled coordinate beside the legacy result.
- The code explicitly preserves legacy projection through `BuildLegacySampleResult`.

Assessment: good base for point probes and evidence widening. Orbit traces will need a richer per-iteration payload, not just terminal samples.

### Runtime Automation Report

Existing source:

- `ui_app/src/viewer_ui_automation_report.h`
- `ui_app/src/viewer_ui_automation_report.cpp`
- `ui_app/src/main.cpp`
- `tests/runtime_harness.py`

Current shape:

- Reports can include visible controls, frame hash/readiness, render pacing, SDF field and postprocess telemetry, field cache state, source-stack kind, root-field consumers, root patterns, root hashes, backend names, and Color Pipeline graph receipts.
- The Python harness already waits on report JSON and frame hashes rather than OS mouse movement.

Assessment: strong candidate for diagnostic readback and UI automation parity, but it is report-oriented rather than a general diagnostic result store.

### Capture Finding And Replay

Existing source:

- `ui_app/src/diagnostics_capture.cpp`
- `ui_app/src/diagnostics_state_io.cpp`
- `tests/test_fractal_runtime_capture_replay_authority.py`
- `ui_app/tests/test_diagnostics_capture.cpp`

Current shape:

- `state.json` remains replay authority.
- `fractal-state.json` is a review sidecar.
- Capture sidecars include active controls, Color Pipeline rows, graph receipts, SDF/lens values, root patterns, root-field consumer state, and hashes.
- Finding selection and `--load-state-json` paths are already differentiated in state IO.

Assessment: useful persistence precedent. Engineering results should probably produce separate result artifacts, not bloat ordinary viewer state.

### Color Pipeline Graph Receipt

Existing source:

- `ui_app/src/color_pipeline_graph_receipt.h`
- `ui_app/src/diagnostics_capture.cpp`
- `tests/test_fractal_runtime_color_pipeline_presets.py`

Current shape:

- Emits `viewer.color_pipeline_graph_receipt.v1`.
- Records nodes, edges, lane projection, unsupported routes, source-stack kind, row params, SDF gate/downsample, root pattern ref, and enabled state.

Assessment: useful receipt pattern. The current implementation also exposes a gap: `active_execution` is currently row-enabled based, not necessarily actual renderer contribution. Diagnostic contracts must distinguish requested/configured/effective execution.

### SDF Capability And Field Reports

Existing source:

- `ui_app/src/sdf_field_capability.h`
- `ui_app/src/color_pipeline_sdf_postprocess.cpp`
- `ui_app/src/color_pipeline_sdf_field_groups.h`
- `ui_app/src/main.cpp`
- `tests/test_fractal_runtime_color_pipeline_sdf_rows.py`

Current shape:

- Reports producer kind, supported signal IDs, backend, field dimensions/downsample, cache status, postprocess backend, sample counts, quality mode, and fail-closed reasons.
- Field-primary mixed routes fail closed when unsupported.

Assessment: good model for capability reporting and cost attribution.

### Runtime Walk / Slime / Trace Artifacts

Existing source:

- `ui_app/src/runtime_walk_headless.cpp`
- `tests/test_explaino_runtime_walk_tool.py`
- `tests/test_explaino_slime_trace_runner.py`
- `tests/test_explaino_trace_receipt_contract.py`

Current shape:

- Headless walks emit report artifacts.
- Slime trace work defines mutation traces, root samples, measurement samples, state hashes, and summary artifacts.

Assessment: conceptually useful for future Science Mode and trace workflows, but the Engineering panel should start with smaller read-only probes.

## Probe Registry Proposal

Add a compiled built-in registry first. Do not add arbitrary scripting or plugin DLL execution in the first vertical.

Descriptor fields:

```json
{
  "probe_id": "fractal.newton.orbit_trace",
  "display_name": "Newton Orbit Trace",
  "supported_targets": ["viewport_point", "complex_coordinate"],
  "supported_fractal_families": ["newton", "explaino"],
  "required_capabilities": ["polynomial_roots", "derivative"],
  "configuration_schema": {},
  "execution_class": "short",
  "result_schema": "viewer.diagnostic.newton_orbit_trace.v1",
  "visualizer_hints": ["summary", "iteration_table", "orbit_plot", "raw_json"],
  "read_only": true
}
```

Required descriptor concepts:

- stable ID and version;
- supported target types;
- family/backend capability requirements;
- config schema;
- estimated cost class;
- result schema;
- cancellation support;
- result visualizer hints;
- provenance requirements;
- whether the probe can mutate state. Default: no.

## Target Identity Model

Diagnostic targets need stable meaning. A pixel target is not enough.

Recommended target envelope:

```json
{
  "target_type": "viewport_point",
  "viewport_id": "main",
  "frame_generation": 1234,
  "render_dimensions": {"width": 2048, "height": 1280},
  "screen_pixel": {"x": 911, "y": 412},
  "normalized_viewport": {"x": 0.445, "y": 0.322},
  "world_coordinate": {"x": -0.124, "y": 0.778},
  "state_hash": "fnv1a64:...",
  "frame_hash": "fnv1a64:...",
  "fractal_type": "explaino",
  "precision_tier": "float64",
  "backend": "cuda"
}
```

Supported target classes should grow in this order:

1. viewport point / complex coordinate;
2. rectangular region;
3. current frame;
4. saved state comparison;
5. operator/channel/runtime generation targets.

Do not start with arbitrary runtime object selection.

## Initial Probe Catalog

### Newton/ExplainO Orbit Trace

Target: one coordinate.

Per-iteration fields:

- iteration index;
- `z`;
- polynomial value;
- derivative;
- step vector;
- step magnitude;
- residual;
- nearest root;
- nearest-root distance;
- orbit magnitude;
- terminal flags.

Terminal classes:

- converged root;
- escaped;
- iteration budget exhausted;
- derivative singularity;
- pole/prepole candidate;
- detected period candidate;
- nonfinite;
- unresolved.

Classification must separate proven from heuristic. For example, pole/prepole and period detection should report confidence and thresholds.

### Neighborhood Convergence Scan

Target: point plus radius or rectangular region.

Outputs:

- terminal class grid;
- winning root grid;
- iteration count grid;
- residual grid;
- histogram;
- disagreement map if multiple backends are requested.

This is likely the second or third probe, not first, because it adds grid visualization and cost controls.

### Precision / Backend Comparison

Target: point or small region.

Backends:

- f32;
- f64;
- future deep/perturbation when available.

Compare terminal class, root identity, residual, iteration count, and orbit divergence.

### Budget Escalation

Run bounded iteration ceilings such as 1k, 5k, 20k, 60k. Must report budgeted evidence, not mathematical certainty.

### Presentation Trace

Inspect effective Color Pipeline authority, graph receipts, renderer path, texture generation, and frame hash.

This is a strong follow-up because recent work exposed configured-versus-effective reporting gaps.

## Generic Result Envelope

Recommended generic wrapper:

```json
{
  "schema_id": "viewer.diagnostic_result.v1",
  "probe_id": "fractal.newton.orbit_trace",
  "probe_version": 1,
  "target": {},
  "request": {},
  "runtime_identity": {},
  "state_identity": {},
  "status": "complete",
  "summary": {},
  "data_schema_id": "viewer.diagnostic.newton_orbit_trace.v1",
  "data": {},
  "warnings": [],
  "artifacts": [],
  "timing": {},
  "provenance": {}
}
```

Status values:

- `complete`;
- `partial`;
- `failed`;
- `cancelled`;
- `stale_target`;
- `unsupported_target`;
- `unsupported_runtime`.

Hashing rules:

- Stable result identity excludes volatile timing.
- Runtime identity includes executable/build identity, backend, precision tier, relevant state hash, and probe version.
- Artifacts carry their own hashes.

## UI Model Recommendation

Start with Candidate C plus a small dockable result drawer:

```text
select viewport point
→ Inspect with...
→ run probe
→ open result drawer/table
```

Why not a full workspace first:

- the repo already has many feature surfaces competing for UI space;
- the architecture risk is in target/probe/result contracts, not panel layout;
- a drawer can later graduate to a workspace if result history and plotting justify it.

Minimum UI sections:

- frozen target summary;
- probe picker;
- config form;
- cost warning;
- run/cancel;
- summary;
- result viewer tabs: table, plot, raw JSON, artifacts.

## CLI And Automation Contract

Use the same schema from UI and CLI.

Candidate commands:

```text
viewer_diagnostic.list_probes
viewer_diagnostic.describe_probe
viewer_diagnostic.run
viewer_diagnostic.cancel
viewer_diagnostic.get_result
viewer_diagnostic.export
```

CLI equivalents can be added around existing `--sample-request-stdin` patterns after the internal schema stabilizes.

The built-product automation should not have separate diagnostic implementations. It should call the same command surface and read the same result envelope.

## Lifecycle And Safety Rules

Default policy:

> Diagnostics are read-only and execute against a frozen state snapshot unless the probe explicitly declares otherwise.

Execution classes:

- `interactive`: expected under 100 ms;
- `short`: expected under several seconds;
- `long`: explicit confirmation or background execution;
- `disruptive`: may pause rendering, rebuild, or consume major resources.

Rules:

- expensive probes must show cost before execution;
- cancellation must be supported for long probes;
- probes must detect stale targets;
- probes must not silently mutate live state;
- CUDA resource use must be isolated or serialized through declared runtime ownership;
- result replay must be possible from captured state and probe config when feasible.

## Persistence And Comparison

Do not store large diagnostic results in ordinary viewer state.

Store diagnostics as artifacts:

```text
diagnostics/<timestamp>_<probe_id>/
  request.json
  result.json
  target.json
  state.json
  artifacts/
```

Useful comparison dimensions:

- before/after code change;
- f32/f64;
- iteration budget;
- selected points;
- backend;
- state hash.

## Extension Model

Phase 1 should support only built-in compiled probes with metadata descriptors.

Later options:

- pack-declared descriptors backed by known implementations;
- Python host probes for Science Mode;
- plugin probes only after security and lifecycle rules are explicit.

Do not add arbitrary executable probe scripting in the first implementation.

## First Vertical Slice Recommendation

Implement `fractal.newton.orbit_trace` as the first vertical.

Acceptance for that future slice:

1. Select/freeze one viewport coordinate.
2. Capture target identity and state identity.
3. Run bounded orbit trace through real engine code.
4. Emit typed per-iteration data.
5. Classify terminal status with confidence/warnings.
6. Show summary and iteration table.
7. Export JSON.
8. Invoke through the same UI/automation/CLI contract.
9. Add deterministic CPU/reference tests and one built-product no-mouse scenario.

## Risk Register

- Target drift after rerender or resize.
- Probe results overstating mathematical certainty.
- Resource contention with live CUDA rendering.
- Hidden agent-only diagnostics creating divergent authority.
- Result artifacts becoming another bloated state format.
- Presentation trace repeating the current configured-versus-effective confusion unless authority is explicit.
- Scope creep into Science Mode before probe contracts are stable.

## Effort Estimate

Research/spec only: complete in this pass.

First vertical slice estimate:

- target identity plus probe registry: medium;
- Newton orbit trace CPU/reference path: medium;
- UI drawer/table and automation command: medium;
- persistence/export and tests: medium.

Expected implementation shape: 2-4 bounded slices, depending on whether CLI and UI land together or sequentially.

## Final Recommendation

Do not begin with a selected-pixel Newton/ExplainO UI. Resume through MD-0 of the catalog-wide [Model Diagnostics Overlay Capability Campaign](model_diagnostics_overlay_capability_campaign_PHASED_PLAN.md), then complete its capability-foundation gates before any public panel, live overlay, or annotated-capture product work. The selected-target registry, identity, result-envelope, lifecycle, and automation concepts in this document remain inputs to that broader foundation.
