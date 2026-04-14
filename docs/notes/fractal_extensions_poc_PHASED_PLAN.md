# Fractal Extensions PoC

## Current Phase

Phase 3 - gallery scene sheet and sidecar proof

- [x] Phase 1 - composite contract and dry-run scaffold
- [x] Phase 2 - live capture/archive/analyzer integration
- [ ] Phase 3 - gallery scene sheet and sidecar proof
- [ ] Phase 4 - hostile review and continuity closure

## Notes

- Why this plan exists:
  - the repo already has three useful but separate seams: headless capture, archived findings with analysis, and callable math via `generic.sample`
  - the user wants a small, real implementation that can justify a future reality-toolkit fractal extensions tool instead of a one-off art experiment
  - the first useful product is a composite runner that proves these seams can work together on the same scene without inventing a second render stack
- Product boundary:
  - first shipped job: load one or more prepared scene states, capture deterministic findings, attach math sidecars, and optionally run analysis
  - first shipped surface: Python module plus thin CLI runner under `tools/reality_toolkit`
  - keep the first version manifest-driven and state-bundle-driven; do not broaden into a generalized plugin system yet
- Phase 1 exit criteria:
  - a new core module defines the scene and sidecar contract, builds `generic.sample` sidecar requests, and supports a dry-run path that writes planned artifacts without launching the runtime
  - focused tests prove request construction, output layout, and manifest writing using stubbed runners
- Current stop point:
  - landed: `fractal_extensions.py` core module, `run_fractal_extensions_composite.py` CLI entrypoint, package exports, and focused tests for manifest loading, dry-run planning, and live capture/archive/sidecar/analyzer orchestration with stubbed runtime seams
  - validated live: a pilot manifest using the archived `explaino_dual` state now captures to `D:/salt-fractal/cuda_newton_fractal_clone/findings/fractal_extensions_gallery_2026-04-13/explaino_dual_demo/`, writes a `newton_z3m1` sidecar request/response pair, and generates local analysis artifacts under `<finding>/analysis`
  - audit repair: `--dry-run` no longer resolves the published runtime path eagerly, so planning-only runs now work even when no runtime is installed for the repo name under `D:/salt-fractal`
  - next step: build the actual 3-5 scene sheet, decide which Collatz-flavored frame survives the scout pass, and strengthen the sidecar stories so the gallery reads as a tool concept instead of a single demo
- Phase 2 exit criteria:
  - the runner can drive headless capture through the published runtime, archive the diagnostics bundle to an explicit finding directory, and optionally run the finding analyzer on the archived result
  - live execution uses deterministic output paths and explicit finding ids instead of scraping ad hoc runtime output
- Phase 3 exit criteria:
  - at least one concrete scene set proves the tool concept with archived findings plus companion math sidecars
  - the tool writes enough metadata that a reviewer can understand the extension idea from the output directory alone
- Phase 4 exit criteria:
  - hostile review closes any reproducibility gaps, stale output naming, or unclear sidecar metadata issues
  - continuity surfaces match the landed scope and stop point

## Validation

- `py -3.14 -m pytest tests/test_fractal_extensions.py -q`
- `py -3.14 tools/reality_toolkit/scripts/run_fractal_extensions_composite.py --help`
- `py -3.14 tools/reality_toolkit/scripts/run_fractal_extensions_composite.py --manifest-json <temp-manifest> --out-dir artifacts/fractal_extensions_dry_run_smoke --dry-run --overwrite`
- `py -3.14 tools/reality_toolkit/scripts/run_fractal_extensions_composite.py --manifest-json <temp-manifest> --out-dir D:/salt-fractal/cuda_newton_fractal_clone/findings/fractal_extensions_gallery_2026-04-13 --overwrite`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`