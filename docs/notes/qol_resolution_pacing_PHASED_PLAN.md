# Resolution UX And FPS Recovery QoL Plan

## Current Phase

Phase 5 - implemented, validated, audited, and awaiting checkpoint/receipt/merge closure.

## Phase Checklist

- [x] Phase 1 - opened and locked a bounded resolution/pacing slice on `codex/qol-resolution-pacing`.
- [x] Phase 2 - RED/proof: native schema and pacing tests covered the old width/height default UX and step-based pacing assumptions; the first native integration pass also exposed the sidecar computed-int write seam.
- [x] Phase 3 - Green slice 1: added aspect preset plus long-edge controls while keeping `RenderSettings.resolution.x/y` authoritative.
- [x] Phase 4 - Green slice 2: replaced stepped preview pacing with budget-based downscale and gradual recovery.
- [x] Phase 5 - validated native/runtime rails, hostile audit, and checkpoint preparation surfaces.

## Explicit User Asks

- [done] Branch 2 started only after Branch 1 was merged and pushed to `master` at `2092a68`.
- [done] `RenderSettings.resolution.x/y` remain runtime and saved-state authority.
- [done] Default render UX exposes an aspect preset combo plus one long-edge slider.
- [done] Default derived UI state is `4:3` with long edge `2048`, preserving `2048x1536`.
- [done] Exact width/height controls remain bound to `resolution.x/y` and are visible only when the computed aspect is `custom`.
- [done] CLI `--width/--height`, diagnostics state, capture, and saved-state compatibility remain on `RenderSettings.resolution.x/y`; no second persisted resolution source was added.
- [done] Interactive FPS recovery uses `sqrt(target_frame_ms / last_render_ms)`, clamps to `preview_min_scale..1`, drops immediately on slow frames, recovers gradually, and issues one settle render.
- [done] Capture-quality rendering and finding capture resolution authority were not changed.
- [done] Runtime proof uses in-process no-mouse automation only.

## Proof Ledger

- Branch 1 prerequisite: `master` was at pushed `2092a68` before `codex/qol-resolution-pacing` was created.
- Schema/binding proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, and `ui_app/tests/test_schema_binding.cpp` now cover aspect preset, long edge, custom-only exact dimensions, computed binding writes, and no-mouse set-value through the schema path.
- Pacing proof: `ui_app/tests/test_viewer_render_pacing.cpp` covers default config, budget scale, unknown large-render timing, quick downscale, gradual recovery, fast full-resolution interaction, and exactly one settle render.
- Sidecar computed-int proof: `ui_app/build_tests_vsdevcmd.cmd` initially failed on `fractal.render.resolution.long_edge`; the repaired `BindingContext::SetIntValue` path is now covered by the full native helper suite.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label qol_resolution_pacing_native_final --log artifacts/logs/qol_resolution_pacing_native_final.log -- ui_app/build_tests_vsdevcmd.cmd` ended with `All helper tests passed`.
- Runtime publish proof: `py -3.14 tools/viewer_host_run_logged_command.py --label qol_resolution_pacing_runtime_publish_final --log artifacts/logs/qol_resolution_pacing_runtime_publish_final.log -- ui_app/build_vsdevcmd.cmd` succeeded and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse proof: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py -q --junitxml artifacts/pytest/qol_resolution_pacing_runtime.junit.xml` passed `1 passed`.
- Code quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/logs/qol_resolution_pacing_code_quality.json` passed the baseline check.
- Contract proof input: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/qol_resolution_pacing.contract.json --out-json artifacts/validation/qol_resolution_pacing_contract.json` passed.

## Hostile Audit

- Status: done
- Did the normal Render panel expose aspect preset plus long-edge controls by default? Yes; schema and safe-mode schema tests enforce both controls and default `4:3`/`2048`.
- Are exact width/height controls visible only for `Custom`? Yes; main and safe-mode schema tests require `visible_if fractal.render.resolution.aspect_preset == custom` on both exact controls.
- Does every computed UI edit write back to `RenderSettings.resolution.x/y` without adding a second persisted source? Yes; `BindingContext::SetEnumId` and `BindingContext::SetIntValue` update `resolution.x/y`, and no new persisted aspect/long-edge field was added.
- Do CLI width/height, diagnostics state, saved-state load, and capture paths still use `RenderSettings.resolution.x/y`? Yes; this slice did not alter state IO, CLI parsing, diagnostics export, or capture authority.
- Does preview pacing scale from budget, drop quickly, recover gradually, and issue one settle render? Yes; `test_viewer_render_pacing` covers those cases directly.
- Did this avoid physical mouse automation and avoid touching Branch 1 Multibrot behavior? Yes; runtime proof uses `set_enum_id` and `set_control_value`, and no Multibrot files were changed in Branch 2.

## Audit Passes

- [done] Pass 1 - reviewed schema/binding/runtime resolution authority for duplicate persisted state; clean re-read found no persisted aspect or long-edge field.
- [done] Pass 2 - reviewed pacing math for immediate preview, gradual recovery, and exactly-one settle render; clean re-read found the behavior covered by focused native tests.
- [done] Pass 3 - re-read runtime tests, plan text, contract proof, and stale wording; clean re-read found no live viewer mouse automation and no stale pre-closeout wording.

## Audit Findings

- [done] Real finding: schema-derived sidecar measurement/controller code still used raw `BindInt` for integer mutations, so the new computed `fractal.render.resolution.long_edge` binding could be read but not swept/mutated. Fixed by adding `BindingContext::SetIntValue` and routing sidecar int writes through it.
- [done] Real finding: code quality rejected a first pacing helper shape because `viewer_render_pacing.cpp` increased max function length. Fixed by splitting timing and recovery helpers until the baseline check passed.
