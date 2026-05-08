# Advanced Color Library Foundation

## Current Phase

Phase 5 queued - repair the stack/backend requirement and grading authority boundary so the next executable slice can land truthful multi-row lane support instead of jumping straight to Grading

## Phase Checklist

- [x] Phase 0 - check in the oracle/terminology note, architecture contract, and initial library inventory for the four-category foundation
- [x] Phase 1 - extract category/function/catalog/live-bridge authority out of `ui_app/src/color_pipeline_window.h` so the window becomes a consumer for the shipped catalog truth before any library widening starts
- [x] Phase 2 - land the full initial Source library through the extracted descriptor/registry core and the broader runtime-source authority refactor the user explicitly chose
- [x] Phase 3 - land the full initial Shape library through the extracted descriptor/registry core
- [x] Phase 4 - land the full initial Palette library while preserving the ExplainO CMap versus basin/root palette split
- [ ] Phase 5 - repair the stack/backend requirement and grading authority boundary so the next executable slice lands truthful multi-row lane support instead of a false Grading continuation
- [ ] Phase 6 - promote Grading to a first-class category and land the initial Grade library, including the bounded generic Balance/Void operator, only after the backend recovery slice closes honestly
- [ ] Phase 7 - close the four-category foundation with proof matrix, D: gallery/runtime captures, and explicit extension rules
- [ ] Phase 8 - only after the foundation closes, widen into additional categories; recommended order remains Blend first, then Mask/Domain

## Explicit User Asks

- [open] Phase 5-7 - Do the next step as a real planned-out full initial library of reusable functions per category, not a vague "3-ish" placeholder.
- [open] Phase 5-7 - Treat this as a critical move that needs real effort, not a lazy option pass.
- [done] Phase 4 - Use ExplainO CMap as part of the palette planning, but do not conflate it with basin/root/joy palette work.
- [open] Phase 5-7 - Make the result simple to extend with nice module boundaries and clean coding.
- [open] Phase 5-7 - Strengthen the architecture beyond dropdowns: make this a reusable, descriptor-driven color-pipeline core that could plausibly become its own DLL/static library later.
- [open] Phase 5-6 - `color_pipeline_window.h` must stop being the authority for category/function identity, parameter meaning, runtime applicability, import/apply behavior, reset/default behavior, or serialization truth.
- [open] Phase 6 - Add a bounded generic filmic Balance/Void grading operator, but keep that separate from fractal-family geometry work.
- [done] Phase 0 - Plan one experimental ExplainO-BalanceVoid deformation-pack family with three neutral-default axes instead of a branch explosion of separate fractals.
- [open] Phase 5-7 - Keep the reusable color pipeline generic and separately owned even if fractal families later emit fields/signals that the pipeline can consume.

## Presumption Loop

The controlling product risk is no longer uncertainty about the advanced-color inventory. That mapping already exists in `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`, and the lane-stack requirement already exists in `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md`. The current blocker is planning drift plus a still-single-row live/backend model: this plan previously pointed straight to Grading even though `ui_app/src/color_pipeline_window.h` still hard-rejects more than one enabled row per lane and `ui_app/src/fractal_types.h` still stores Shape state as one enum plus one flat parameter set. The most falsifiable current hypothesis is that Phase 5 must first repair the stack/backend requirement and grading authority boundary so the next executable slice is a bounded backend recovery rather than a dishonest Grading continuation. The cheapest disconfirming checks are deterministic repo rails: the Phase 5 recovery contract must validate, the phased plans must stay synchronized, and the next implementation slice must start with focused reds around the one-enabled-row live bridge and per-row parameter scoping.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the four-category foundation and the initial Source / Shape / Palette / Grading inventory, including the bounded generic Balance/Void operator and the separate ExplainO-BalanceVoid family track.
- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already maps the lane-stack requirement: Source / Shape / Palette lane stacks, plus-button row insertion, ordered composition, and runtime-real-only shipping.
- `ui_app/src/color_pipeline_window.h` still initializes, imports, snapshots, and applies the live path through single-row helpers plus explicit one-enabled-row bridge checks, which proves backend recovery must come before any honest Grading follow-on.
- `ui_app/src/fractal_types.h` still stores Shape state as one active enum plus one flat parameter set, which means the first real multi-row runtime slice needs an explicit per-row parameter-scoping strategy instead of silent implementation drift.
- The verified references still split the ExplainO surfaces cleanly: the legacy LUT lineage lives in `c:\code\salticid-cuda\carl-ca-python\ca_viewer.py`, while the current basin/root palettes live in `c:\code\salticid-cuda\content\packs\explaino\cuda\explaino_cuda_helpers.inl` and `c:\code\salticid-cuda\cuda_core\src\ops_special.cu`, and the generic viewport colormap surface in `c:\code\salticid-cuda\ide_ui_dx11\ui_app\src\viewport_colormap.cpp` / `.h` does not contain an ExplainO colormap.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` explicitly distinguishes legacy ExplainO CMap, Root Classic Palette, Joy Root Palette, and the generic viewport colormap surface as separate authorities.
- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` records the stronger architecture rule that `color_pipeline_window.h` must become a consumer and not remain the authority for ids, semantics, adapters, or serialization truth.
- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` records the initial balanced library inventory for Source / Shape / Palette / Grading, including the bounded Balance/Void-inspired grading operator and a separate ExplainO-BalanceVoid family track.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase0_oracle_inventory_contract.json` shows the Phase 0 contract validated cleanly, and `artifacts/validation/viewer_host_assert_phased_plan_sync.json` shows the phased plan stayed synchronized.
- Checkpointed: Phase 0 closed at commit `6624781` with handoff and machine proof receipts.
- Landed: `ui_app/src/color_pipeline_core.h` now owns the shipped advanced color lane-catalog type, descriptor builders, function-id mappings, runtime-backed filtering, and schedule-bridge tuple mapping.
- Landed: `ui_app/src/color_pipeline_window.h` now delegates the shipped catalog and bridge helpers to `ui_app/src/color_pipeline_core.h` instead of defining that authority inline.
- Landed: `ui_app/tests/test_schema_binding.cpp` now exercises the extracted core directly, proving the shipped Shape catalog and the banded schedule-bridge tuple without routing through the window helper first.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase1_core_extraction_contract.json` shows the Phase 1 contract validated cleanly, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` shows the plan stayed synchronized, `artifacts/code_quality_report.json` stayed at baseline (`97/100`), `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` published the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Deferred explicitly: apply/reset/default/serialization ownership cleanup is still part of the broader foundation architecture, but Phase 1 did not claim to finish that seam; it only moved the shipped catalog and bridge truth into the dedicated core header.
- Resolved ambiguity: the user explicitly chose the broader long-term-correct Phase 2 path instead of narrowing to only already-runtime-real source rows. Phase 2 therefore owns the runtime-source expansion and mirror/bridge refactor needed to land `escape_magnitude`, `orbit_stripe`, and family-gated `root_proximity` honestly.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/fractal_family_rules.h`, and `ui_app/src/schema_binding.cpp` now treat `escape_magnitude`, `orbit_stripe`, and family-gated `root_proximity` as shipped Source rows with real runtime ids, selectable bridge tuples, and fail-closed family gating.
- Landed: `ui_app/src/escape_time_coloring.h`, `ui_app/src/basin_coloring.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/diagnostics_state_io.cpp` now own the widened Source runtime parameters, programmable signal sampling, draft/live bridge behavior, and persisted diagnostics state for the new rows.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_finding_archive_actions.cpp` now lock the widened Source catalog, runtime behavior, diagnostics load/save, capture-finding persistence, and the phase-wheel interior-color regression found during hostile audit.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase2_source_runtime_expansion_contract.json` remains green for the Phase 2 contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` is back on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green after the hostile-audit regression fix, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the republished runtime.
- Audit: hostile review found that programmable `orbit_stripe` samples mirrored through the phase wheel were forcing interior pixels to black instead of preserving the dim phase-wheel interior color; `ui_app/tests/test_escape_time_coloring.cpp` now locks that case and `ui_app/src/escape_time_coloring.h` allows programmable phase-wheel palettes to render their interior color.
- Checkpointed: Phase 2 closed at commit `f3c8bd0` with the linked `ck:021d50d4` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: Phase 3 starts with `posterize` as the first honest Shape widening step because the descriptor already exists while the runtime, live-bridge, and persistence seams still reject it as draft-only.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `posterize` as a real Shape row with runtime ids, defaults, live import/apply/reset support, shape math, and diagnostics persistence.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock posterize catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics round-trip, Capture Finding persistence, and reset defaults.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_posterize_contract.json` validates the Phase 3 posterize contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the posterize runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the local draft-apply choke-point repair and the strengthened posterize runtime regression sample.
- Checkpointed: the `posterize` Phase 3 sub-slice closed at commit `080701a` with the linked `ck:76888967` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `mirror_repeat` is the next honest Shape widening step because it can reuse the existing repeat frequency/phase owners while widening the runtime shape math and live bridge beyond the current sawtooth-only repeat behavior.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/diagnostics_capture.cpp` now treat `mirror_repeat` as a real Shape row with runtime ids, descriptor authority, mirrored-wave shape math, live import/apply support, and Capture Finding serialization while reusing the existing repeat frequency/phase owner fields.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_finding_archive_actions.cpp` now lock mirror_repeat catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics load, and Capture Finding persistence through the reused repeat owner fields.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_mirror_repeat_contract.json` validates the Phase 3 mirror_repeat contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the mirror_repeat runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the targeted hard-coded-shape-list audit passes.
- Checkpointed: the `mirror_repeat` Phase 3 sub-slice closed at commit `6147b08` with the linked `ck:f44e55b8` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `bias_gain_curve` is the next honest Shape widening step after `mirror_repeat` because it needs its own explicit shape.bias / shape.gain runtime owners rather than descriptor-only coverage or owner-field reuse, followed by `smooth_window` through the same extracted registry/core and live-bridge seams.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `bias_gain_curve` as a real Shape row with runtime ids, dedicated shape.bias / shape.gain owners, live import/apply support, runtime curve math, diagnostics load/save, and reset defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock bias_gain_curve catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics round-trip, Capture Finding persistence, and reset defaults through the dedicated owner pair.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_bias_gain_curve_contract.json` validates the Phase 3 bias_gain_curve contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the bias_gain_curve runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the targeted hard-coded-shape-list and owner-field audit passes.
- Checkpointed: the `bias_gain_curve` Phase 3 sub-slice closed at commit `75e63b3` with the linked `ck:bd23c697` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `smooth_window` is the final honest Shape widening step after `bias_gain_curve` because the existing `EscapeTimeColorSmoothstep(...)` helper already supports a bounded smooth window once the row gets explicit shape.center / shape.width / shape.softness runtime owners.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `smooth_window` as a real Shape row with runtime ids, dedicated shape.center / shape.width / shape.softness owners, live import/apply support, wrapped smooth-window math, diagnostics load/save, and reset defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock smooth_window catalog visibility, live bridge apply/sync behavior, runtime signal shaping, seam-wrap behavior, diagnostics round-trip, Capture Finding persistence, and reset defaults through the dedicated owner triple.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_smooth_window_contract.json` validates the Phase 3 smooth_window contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the published runtime.
- Audit: hostile review found that the first smooth_window implementation failed to wrap a window centered near the upper domain edge across the shape seam; `ui_app/tests/test_escape_time_coloring.cpp` now locks that case and `ui_app/src/escape_time_coloring.h` computes smooth_window via wrapped center distance instead of a non-wrapping lo/hi interval.
- Known issue tracked separately: `KNOWN_ISSUES.md` now records that Reset All, the legacy/advanced-color dropdown surfaces, and the smooth-escape source row can still desynchronize in the live UI; that product bug is documented but not claimed as part of the `explaino_cmap` slice unless the new palette row proves it depends on the same defect.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/fractal_family_rules.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `explaino_cmap` as a real Palette row with explicit palette.seed_scale / palette.seed_phase / palette.colorfulness owners, runtime tuple mirroring, live import/apply/reset support, Capture Finding persistence, diagnostics load support, and reset defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock explaino_cmap catalog visibility, bridge selection, live apply/sync behavior, runtime palette behavior, diagnostics load, Capture Finding persistence, and reset defaults.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase4_palette_explaino_cmap_contract.json` validates the Phase 4 explaino_cmap contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the published runtime.
- Audit: hostile review found that the first explaino_cmap runtime sampler duplicated the ExplainO wedge tween math inline instead of reusing the existing shared seed-curve seam; `ui_app/src/escape_time_coloring.h` now routes that palette through `ui_app/src/explaino_seed_curve.h` and the helper plus runtime rails stayed green on the repaired state.
- Audit: confidence rebuild after the user's runtime report found that opening the advanced color pipeline window on an unsupported or invalid live tuple was auto-applying the starter draft on first render; `ui_app/src/color_pipeline_window.h` now requires actual in-window interaction before the end-of-frame auto-apply helper can mutate the runtime, and `ui_app/tests/test_schema_binding.cpp` now locks both the unsupported-startup-open path and the live `explaino_cmap` render path.
- Next bounded slice: after checkpointing this explaino_cmap sub-slice, run the requested reset/dropdown desync issues pass before returning to the family-gated root palette rows.
- Closed detour: the requested reset/dropdown desync issues pass landed separately and no longer blocks Palette widening.
- Closed detour: the published-runtime ExplainO programmable basin bugfix landed separately, so Phase 4 can now resume without the dead-slider/runtime-truth blocker.
- Closed detour: the viewer-first runtime-proof guardrail follow-up landed separately, so the next Palette slice should close under the stronger runtime publish plus published-runtime proof enforcement.
- Next bounded slice: `root_classic_palette` is the first honest family-gated Palette widening step because the runtime tuple already exists as the basin default while the advanced palette catalog and bridge table still treat it as unsupported.
- Landed: `ui_app/src/color_pipeline_core.h` now treats `root_index` as a runtime-backed Source row and `root_classic_palette` as a runtime-backed Palette row, with advanced-id parsing and bridge ids for the default basin tuple.
- Landed: `ui_app/tests/test_schema_binding.cpp` now locks the extracted core catalogs, the shipped window catalogs, the default basin live-snapshot import, and the diverged-draft sync behavior for the `root_index` plus `root_classic_palette` bridge.
- Landed: `tests/test_fractal_runtime_explaino_escape_variants.py` now proves the published runtime still renders distinct Explaino basin frames for `root_classic` versus `joy` palettes after the `root_classic` advanced-row widening.
- Validated: `artifacts/root_classic_green_native_3.log` is green for `ui_app/build_tests_vsdevcmd.cmd`, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q -k root_classic_and_joy_palettes_render_distinct_published_runtime_frames` passed against the freshly published runtime.
- Follow-up landed: `ui_app/src/color_pipeline_window.h` now auto-completes shipped single-row Source/Palette selections into their supported companion pairs, recognizes `root_index + root_classic_palette` through a shared tuple builder, and explains that parameterless fixed Source/Palette rows change the live bridge directly rather than exposing tunable controls.
- Follow-up landed: `ui_app/tests/test_schema_binding.cpp` now locks the repaired root tuple-switch path from a non-basin live tuple into the basin pair and updates the shipped-pair classification coverage so selector choices no longer present as bogus preview-only dead ends.
- Follow-up validated: `artifacts/root_tuple_switch_native_v5.log` is green for the focused native helper rail, `artifacts/code_quality_report.json` stayed at the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the active runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the freshly published runtime.
- Landed: `ui_app/src/color_pipeline_core.h` and `ui_app/src/color_pipeline_window.h` now treat `joy_root_palette` as a runtime-real advanced Palette row, bridge the `joy_basins` tuple through `root_index`, and let the editor co-switch/apply joy-basin selections without falling back to a draft-only dead end.
- Landed: `ui_app/tests/test_schema_binding.cpp` now locks the joy palette row in the extracted core catalog, the shipped palette lane, the manual non-basin-to-joy apply path, and live joy-basins import as a supported `root_index + joy_root_palette` snapshot.
- Validated: `artifacts/joy_root_green_native.log` is green for the focused native helper rail, `artifacts/code_quality_report.json` stayed at the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the active runtime cleanly, and `artifacts/joy_root_runtime_smoke.log` reports the published-runtime `root_classic` versus `joy` Explaino basin smoke passing.
- Phase boundary advanced: the initial Palette library is now complete, but hostile review found the next truthful step is not direct Grading promotion. Phase 5 must first repair the stack/backend requirement and grading authority boundary so the mapped Grade library can land honestly.

## Hostile Audit

- Status: in progress
- Required posture: treat the mapped inventory as settled and keep auditing the backend/planning seams until the live multi-row stack requirement and the grading authority boundary are both proven instead of inferred.

## Audit Passes

- [done] Pass 1 - re-read the main foundation plan against the mapped inventory and record the concrete planning defects: phase drift, missing hostile-audit state, globally open user asks, and the vague direct-to-Grading resume point.
- [done] Pass 2 - re-read the live backend seams and record the concrete technical blockers: `ui_app/src/color_pipeline_window.h` still enforces one enabled row per lane and `ui_app/src/fractal_types.h` still stores Shape state as one flat parameter set.
- [open] Pass 3 - audit the repaired planning surfaces after the bounded Phase 5 recovery slice lands and confirm the next executable implementation slice is backend recovery rather than an underspecified Grading continuation.

## Audit Findings

- [done] Real defect found: this plan previously said Phase 5 was in progress and pointed straight to Grading even though the mapped lane-stack requirement remained open and the plan lacked the hostile-audit sections now required by the repo workflow.
- [done] Real defect found: the live advanced-color backend still hard-rejects more than one enabled row per lane and imports/snapshots through single-row builders, so the next truthful implementation slice must repair backend/state authority before any honest Grading continuation.

## Notes

- Expected owner files for the next bounded slice:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase5_recovery_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_recovery.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
- Non-goals for Phase 5:
  - do not redefine the mapped Source / Shape / Palette / Grading inventory
  - do not start Grading implementation before the backend recovery slice closes honestly
  - do not reopen the common-fractal wave or the CUDA catalog refactor as the active thread yet
  - do not treat editor-only stack behavior as satisfying the live/runtime requirement

## Resume Point

Checkpoint the bounded Phase 5 planning-repair slice, then start the backend-recovery implementation slice under `docs/notes/advanced_color_library_foundation_phase5_recovery_PHASED_PLAN.md` and `docs/contracts/advanced_color_library_foundation_phase5_recovery.contract.json`, targeting the one-enabled-row live bridge, single-row import/snapshot helpers, flat Shape parameter scoping, and the first truthful multi-row runtime REDs before any Grading implementation.









