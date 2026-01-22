## Explain-o Probe — Post-Work Code Review & TODO

Date: 2026-01-22

Summary
- Session completed: UI `explaino_seed` -> double slider, smooth fractional auto-increment, `explaino_seed_rate` slider, CLI `--explaino-seed` handling, and UI display of combined seed (integer + fractional drift).
- Verified: headless capture run with `--explaino-seed=2.718281828459045` wrote diagnostics showing the expected seed.
- Status: goals met for today. This doc lists remaining work, priorities, and short estimates for follow-up.

Completed (today)
- `project/cuda_newton_fractal/ui/fractal_binding_surface_v1.ui_schema.json`: added `explaino_seed_rate` slider and lowered min/step to 1e-5.
- `project/cuda_newton_fractal/ui_app/src/fractal_types.h`: added `view.explaino_seed_rate`.
- `project/cuda_newton_fractal/ui_app/src/main.cpp`:
  - bound `fractal.view.explaino_seed_rate` and used it for auto-increment rate.
  - replaced hardcoded rate with `view.explaino_seed_rate`.
  - special-cased `fractal.params.explaino_seed` control to show `params + drift` and write edits back.
  - implemented fractional-drift carry and UpdateExplainoPolynomial() triggering for smooth transitions.
  - added CLI parsing for `--explaino-seed` and made override resilient to schema/preset defaults.

Remaining / Recommended Follow-ups (prioritized)

High priority
- Remove temporary debug artifact: stop writing `cli_debug.json` from the startup path. (Files: `project/cuda_newton_fractal/ui_app/src/main.cpp`) — ETA 15–30m.
- Add CLI `--help` or document `--explaino-seed` in startup text so users discover it. (main.cpp) — ETA 15–30m.
- Add unit tests / small harness that verifies the canonical seed mapping (hash/logistic → double) and roundtrip behavior for seeds near boundaries (ensure deterministic outputs). (tests/ or project/scripts) — ETA 1–2h.

Medium priority
- Persist `explaino_seed_rate` into workspace presets/prefill handling so presets can include a slower rate. (UI schema presets handling in `main.cpp` and any preset apply code) — ETA 30–60m.
- Update canonical schema docs and binding map to reference `fractal.view.explaino_seed_rate` and the changed seed control (update `project/.../ui/*.binding_map.md` or the binding docs). — ETA 15–30m.
- Add a small UI indicator (icon/text) that `Auto-Increment Seed` is active to make it obvious when the seed is drifting. (main UI panel) — ETA 15–30m.

Low / Nice-to-have
- Add an interactive unit (or integration) test that runs a short headless probe at several `explaino_seed_rate` values and records `state.json` snapshots for regression checks. (project/scripts) — ETA 1–2h.
- Consider exposing a per-preset 'seed increment rate' control in saved presets and providing a preset that demonstrates very slow drift (1e-5) so it's easy to reproduce. — ETA 30–60m.
- Small UX: show the fractional drift digit count or small fractional readout next to the slider for precision tuning. — ETA 15–30m.

Notes / Observations
- The CLI override was being clobbered by `ApplySchemaDefaults(...)` and preset default application; this was fixed by reapplying CLI overrides in the init paths. Keep this pattern in mind for other CLI flags.
- The fractional-drift approach yields smooth visual interpolation via the existing `UpdateExplainoPolynomial()` path but requires careful persistence semantics if you save/load workspace state (decide whether to save fractional drift). If you want deterministic replays, persist both integer seed and fractional drift in `state.json` snapshots.

Suggested next session (short plan)
1. Remove `cli_debug.json` writes and add a small CLI help string. (30–60m)
2. Add persistence for `explaino_seed_rate` in presets and update docs. (45–90m)
3. Add unit tests for canonical seed mapping and a headless regression harness. (1.5–3h)

Acknowledgement
- We met today's goals; the work took ~6 hours longer than expected but the core features are implemented and verified.
