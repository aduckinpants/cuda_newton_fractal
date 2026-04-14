# Transcendental Lambda Probe

## Current Phase

Phase 3 - hostile audit and continuity closure (complete)

## Phase Checklist

- [x] Phase 1 - manifest and runtime validation
- [x] Phase 2 - composite execution and output review
- [x] Phase 3 - hostile audit and continuity closure

## Notes

- Why this plan exists:
  - the shipped `generic.sample` surface already supports `sin`, `cos`, `exp`, `log`, `compose`, and `iterate`, but the current gallery only lightly exercises transcendental chains and barely exercises `compose(...)` inside the loop body
  - the goal is to validate the landed lambda/composition preview surface with a new visual probe, not to add a new intrinsic or reopen the transpiler thread
- Product boundary:
  - one known-good archived Explaino state acts as the hero frame
  - three new `generic.sample` sidecars provide the actual lambda/composition stress cases over a shared complex-plane region
  - this slice should stay inside the current runtime + composite tooling surface
  - audit correction: this slice validated the callable/JSON surface and caught a real serialization bug, but the archived hero frame means it does not count as a novel mathematical exploration result
- Phase 1 exit criteria:
  - a checked-in manifest exists for the transcendental probe
  - the published runtime passes the existing `generic.sample` CLI regression surface before the new manifest is run
- Phase 2 exit criteria:
  - the composite run captures the base finding, writes all three sidecar request/response pairs, and writes the local analyzer outputs
  - the summary JSON shows non-null sidecar results with non-zero sample counts for all three expressions
- Phase 3 exit criteria:
  - hostile review confirms the slice validated transcendental Newton ladders plus `compose(...)` inside `iterate(...)` as callable-surface stress cases without overstating the result as novel exploration
  - continuity surfaces record the final stop point, diagnostics, and validated command set

## Validation

- `py -3.14 -m pytest tests/test_generic_probe_cli.py -q`
- `py -3.14 tools/reality_toolkit/scripts/run_fractal_extensions_composite.py --manifest-json docs/manifests/transcendental_lambda_probe_manifest.json --out-dir D:/salt-fractal/cuda_newton_fractal_clone/findings/transcendental_lambda_probe --finding-group transcendental_lambda_probe --overwrite`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
