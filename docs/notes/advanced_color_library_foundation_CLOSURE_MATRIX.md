# Advanced Color Library Foundation Closure Matrix

This matrix is the checked-in closure boundary for the advanced-color foundation after `ck:44faf037`. It is not a marketing status page. A row is shipped only when it has runtime ownership, visible controls that execute, reset/default behavior, diagnostics/capture persistence where applicable, and proof in the checked-in rails.

## Status Legend

- Shipped: runtime-backed and covered by the existing helper/runtime proof ledgers.
- Bounded: shipped only inside the named boundary; behavior outside that boundary is not claimed.
- Deferred: deliberately not shipped and must be hidden, unselectable, or blocked by a later contract.
- Blocker: must be fixed before claiming foundation closure. The manual `234919_563__explaino_inertial` capture reproduction failure is now accepted as a blocker until a repair slice proves the archived frame re-renders from saved state.

## Lane Matrix

| Lane | Row or Surface | Status | Closure Boundary | Primary Evidence |
| --- | --- | --- | --- | --- |
| Source | `smooth_escape_ramp` | Shipped, single-row Source | Runtime-backed row with `signal.scale` and `signal.bias`; generic multi-Source mixing is deferred. | `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp` |
| Source | `phase_orbit` | Shipped, single-row Source | Runtime-backed row with phase offset/wrap controls; no generic Source blend semantics claimed. | `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_schema_binding.cpp` |
| Source | `banded_signal` | Shipped, single-row Source | Runtime-backed iteration-band source; composition with other Source rows is deferred. | `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_schema_binding.cpp` |
| Source | `escape_magnitude` | Shipped, single-row Source | Runtime-backed escape magnitude source; descriptor-owned precision follow-up remains a separate Source-composition concern. | `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `HANDOFF_LOG.md` |
| Source | `orbit_stripe` | Shipped, single-row Source | Runtime-backed orbit stripe source; no arbitrary signal mixer claimed. | `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_escape_time_coloring.cpp` |
| Source | `root_proximity` | Bounded Source | Runtime-backed only for basin-capable family behavior; generic Source stacking is not shipped. | `ui_app/src/fractal_family_rules.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h` |
| Source | `root_index` | Bounded pair Source | Shipped as the Source side of bounded row-indexed root-basin pairs with `root_classic_palette` or `joy_root_palette`. | `ui_app/src/color_pipeline_window.h`, `ui_app/tests/test_schema_binding.cpp`, `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md` |
| Shape | `identity`, `offset_scale`, `repeat`, `posterize`, `mirror_repeat`, `bias_gain_curve`, `smooth_window` | Shipped ordered stack | Bounded `color_shape_stack` executes rows in order, persists, resets, captures, and mirrors final row for compatibility. | `ui_app/src/fractal_types.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_runtime_reset.cpp` |
| Palette | `heatmap`, `phase_wheel_palette`, `banded_heatmap`, `explaino_cmap` | Shipped Palette RGB stack | Bounded `color_palette_stack` samples each row to RGB and blends with persisted `palette.blend_weight` and `palette.blend_mode`; phase-wheel saturation is runtime-backed. | `ui_app/src/fractal_types.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_escape_time_coloring.cpp`, `docs/notes/advanced_color_library_foundation_phase6_palette_blend_stack_PHASED_PLAN.md` |
| Palette | `root_classic_palette`, `joy_root_palette` | Bounded pair Palette | Shipped as root-basin palette rows for row-indexed `root_index` pairs, not as arbitrary Palette RGB stack blend rows. | `ui_app/src/color_pipeline_window.h`, `ui_app/tests/test_schema_binding.cpp`, `tests/test_fractal_runtime_explaino_escape_variants.py` |
| Grading | `contrast_lift`, `phase_finish`, `band_finish` | Shipped ordered stack | Bounded `color_grading_stack` executes shipped Grading rows in order, persists, resets, captures, and mirrors the final row for compatibility. | `ui_app/src/fractal_types.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_runtime_reset.cpp`, `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md` |
| Grading | `neutral_finish`, `tone_map_finish` | Deferred | Named inventory only; no shipped runtime owner/control proof in this closure boundary. Must remain unselectable until a later owner-proof contract ships them. | `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, `ui_app/src/color_pipeline_core.h` |
| Grading | `grade.glow` | Deferred | Explicitly not shipped; `band_finish` closed without exposing `grade.glow` because no reusable owner exists. | `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md`, `ui_app/src/color_pipeline_core.h` |
| Grading | `balance_void_grade` / Balance-Void family | Deferred | Product inventory remains valid, but no foundation closure claim includes this operator until a separate owner-proof slice exists. | `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` |
| Grading | basin-default lane retention | Deferred | UI cleanup/fidelity fork, not part of the shipped stack claim. Must be a separate owner-proof slice if Adam makes it a blocker. | `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md` |

## Cross-Cutting Proof Matrix

| Surface | Status | Boundary | Evidence |
| --- | --- | --- | --- |
| Descriptor/catalog authority | Shipped | Runtime-backed rows are filtered through `IsColorPipelineFunctionRuntimeBacked`; unknown ids fail closed. | `ui_app/src/color_pipeline_core.h`, `ui_app/tests/test_color_pipeline_core.cpp`, `ui_app/tests/test_schema_binding.cpp` |
| Live bridge pit-of-success | Shipped with bounded Source caveat | Shape, Palette, and Grading stacks apply/import through bounded runtime owners; Source remains single-row except root-basin pairs. | `ui_app/src/color_pipeline_window.h`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_color_pipeline_window.cpp` |
| Diagnostics serialization | Blocker for manual `explaino_inertial` reproduction | Shape, Palette, and Grading stacks save/load, and the manual `explaino_joy` fixture catches high-precision save-path truncation. Foundation closure is blocked because `234919_563__explaino_inertial` exits 0 through `--capture-diagnostic` but re-renders as a solid orange frame instead of the archived fractal. | `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ck:2f9efd8f` |
| Capture/archive persistence | Blocked by manual `explaino_inertial` reproduction | Capture Finding carries advanced-color draft/stack state covered by helper tests, but closure cannot cite archive persistence as complete while a real manual finding stores a frame that does not reproduce from its archived state. | `ui_app/tests/test_finding_archive_actions.cpp`, `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md`, `docs/notes/advanced_color_library_foundation_phase6_palette_blend_stack_PHASED_PLAN.md` |
| Reset/defaults | Shipped for current closure boundary | Runtime reset/default paths clear Shape, Palette, and Grading stacks. | `ui_app/src/runtime_reset.cpp`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/tests/test_runtime_reset.cpp` |
| Published runtime | Shipped for latest implementation slices | Grading stack and Palette blend stack both published to the active `D:` runtime and passed probe/session pytest rails. | `artifacts/grading_stack_runtime_publish.log`, `artifacts/grading_stack_runtime_probe_session_pytest.log`, `artifacts/palette_blend_stack_runtime_publish_final.log`, `artifacts/palette_blend_stack_runtime_probe_session_pytest_final.log` |
| Generic Source composition | Deferred | No scalar mixer semantics are chosen. Implement only after a semantics contract names weighted blend/max/min/modulation or equivalent and test expectations. | This matrix, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` |
| Remaining Grading inventory | Deferred | Not a hidden blocker unless Adam reclassifies it; each row needs a separate owner-proof contract before becoming visible runtime work. | This matrix, `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md` |

## Known Closure Blockers

| Blocker | Evidence | Required Next Proof |
| --- | --- | --- |
| Manual `234919_563__explaino_inertial` capture does not reproduce from `state.json` | The archived repro command exits 0 through `--capture-diagnostic`, but the fresh frame is a solid orange render instead of the archived detailed fractal. The saved state mostly survives load; the only observed emitted-state drift so far is `params.multibrot_power` changing from `3` to `2`. | A dedicated runtime repair slice must add a focused capture-backed regression, fix the root cause, publish the active `D:` runtime, and prove the failing capture re-renders from saved state. |

## Closure Decision Forks

1. Repair the manual `explaino_inertial` reproduction blocker before any foundation closure claim.
2. Accept this matrix as the foundation boundary only after that blocker is repaired: proceed to final closure proof with Source composition and remaining Grading inventory explicitly deferred.
3. Reclassify Source composition as a blocker: write a semantics-only Source mixer contract first, then implement no code until the mixer behavior is locked.
4. Reclassify remaining Grading inventory as blockers: split into basin lane-retention, neutral/tone-map, `grade.glow`, and Balance/Void owner-proof slices.

No closure claim may cite this matrix while also showing addable visible rows that are marked Deferred here, or while the manual `explaino_inertial` capture reproduction blocker remains open.
