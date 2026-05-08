# Advanced Color Explaino Escape-Magnitude Semantics

## Current Phase

Phase 3 complete - Explaino programmable `escape_magnitude` is back on honest orbit-magnitude semantics, the focused native plus published-runtime proofs are green, and the paced-loop sidecar failure is explicitly deferred into its own follow-up slice

## Phase Checklist

- [x] Phase 1 - add focused native and published-runtime regressions proving Explaino `escape_magnitude` semantics drifted from orbit magnitude to basin residual
- [x] Phase 2 - repair the runtime signal owner so Explaino programmable `escape_magnitude` uses orbit magnitude again without disturbing explicit basin-only sources
- [x] Phase 3 - rerun the bounded validation rails, hostile-audit the repaired seam, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Correct the new Explaino advanced-color bug before moving on.
- [done] Stop letting advanced-color recipes silently change meaning under old labels.
- [done] Prove the real runtime behavior instead of handing back another helper-only guess.
- [done] Keep the fix bounded to the actual signal semantics bug instead of reopening unrelated UI work.

## Presumption Loop

The local hypothesis is that `ui_app/src/escape_time_coloring.h` now aliases `ColorSignal::escape_magnitude` to `ResolveBasinResidualMetric(residual)` inside `ResolveProgrammableBasinSignal(...)` for basin-capable families such as Explaino. That semantic drift makes the shipped `escape_magnitude` label dishonest and turns high-frequency Shape/Palette recipes into float32-sensitive residual visualizations. The cheapest disconfirming checks are a focused native semantic regression in the runtime color owner seam and a published-runtime Explaino capture/hash regression that changes only the residual-sensitive exit threshold while keeping the labeled source set to `escape_magnitude`.

## Presumption Evidence

- `ui_app/src/escape_time_coloring.h` routes Explaino programmable non-basin tuples through `MakeProgrammableBasinColor(...)`.
- Inside that basin path, `ColorSignal::escape_magnitude` currently resolves through `ResolveEscapeMagnitudeSignal(ResolveBasinResidualMetric(residual), params)` rather than orbit `|z|` magnitude.
- The user observed that Explaino `escape_magnitude / mirror_repeat / explaino_cmap` now needs float64 immediately to avoid a wall of fuzz, which matches residual-driven high-frequency boundary structure rather than orbit-magnitude behavior.
- Existing published-runtime tests in `tests/test_fractal_runtime_explaino_escape_variants.py` already provide a headless capture/hash harness for Explaino color-pipeline behavior.

## Proof Ledger

- Read-only finding: the current Explaino programmable renderer path is alive, but the `escape_magnitude` source label no longer means what it says.
- Read-only finding: the narrowest honest fix is in the runtime signal owner, not the color window or family tuple bridge.
- Native proof: `ui_app/tests/test_escape_time_coloring.cpp` now fails if Explaino programmable `escape_magnitude` changes when only basin residual changes, and the native helper rail is green after routing that source back to orbit `|z|`.
- Published-runtime proof: `tests/test_fractal_runtime_explaino_escape_variants.py -k escape_magnitude` now compares epsilon-driven image deltas for `escape_magnitude` versus intentionally residual-sensitive `smooth_escape`, and the published runtime keeps `escape_magnitude` materially more stable after the fix.
- Audit finding: a full-file Explaino runtime pytest pass still reports an unrelated failure in `test_explaino_sidecar_headless_paced_loop_respects_stop_threshold` (`params.multibrot_power` mutates under zero-coverage stop threshold). That failure is outside this signal-semantics slice and remains unresolved here.

## Notes

- Expected owner files for this pass:
  - `docs/contracts/advanced_color_explaino_escape_magnitude_semantics.contract.json`
  - `docs/notes/advanced_color_explaino_escape_magnitude_semantics_PHASED_PLAN.md`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this pass:
  - do not redesign the advanced color UI
  - do not reopen the separate viewport zoom bug
  - do not change `root_proximity`, `root_basin`, or `joy_basins` semantics
  - do not attempt the future far-field witness/color-source work in this slice

## Resume Point

If continuing this slice, checkpoint the validated Explaino `escape_magnitude` repair and receipts cleanly. Treat the paced-loop zero-coverage stop-threshold failure as a separate follow-up, not as a reason to reopen the repaired signal semantics owner seam.