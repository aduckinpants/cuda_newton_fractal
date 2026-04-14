# Generic Sampler General Gallery

## Current Phase

Phase 4 - hostile review and continuity closure (complete)

## Phase Checklist

- [x] Phase 1 - broaden generic coverage and define a non-family render seam
- [x] Phase 2 - add focused generic tests and render helper coverage
- [x] Phase 3 - run non-family generic galleries and inspect outputs
- [x] Phase 4 - hostile review and continuity closure

## Notes

- Why this plan exists:
  - the repo already proves that `generic.sample` works for a few core expressions, but the current validations still under-represent the broader unary/transcendental surface and the current visual probes are mostly attached to base fractal-family scene captures
  - the user wants another verification pass that stays on the generic sampler itself: more general functions, more tests, and more interesting sample renders that are not one of the shipped fractal families
- Product boundary:
  - keep this slice on `generic.sample`, its direct request/response contract, and a thin generic-only visualization seam for grid outputs
  - do not reopen dynamic kernel registration or add a new fractal family
- Current stop point:
  - landed broader `generic.sample` regressions on both the native helper rail and the published-runtime CLI rail for `log(z)`, `abs(z_conj)`, `sequence_grid` parameter overrides on `function.params.scale_real`, and the current parser boundary that still rejects `iterate(z, steps)` when the count is not an integer literal
  - landed a thin generic-only gallery seam via `tools/reality_toolkit/fractal_explorer/generic_sampler_gallery.py` plus `tools/reality_toolkit/scripts/run_generic_sampler_gallery.py`, with checked-in request examples for `sine_newton`, `exp_log_sequence`, and `conjugate_wave`
  - archived non-family generic outputs under `D:/salt-fractal/cuda_newton_fractal_clone/findings/generic_sampler_general_gallery_2026-04-14/`, including `conjugate_wave`, `exp_log_sequence`, the original `sine_newton`, and the hostile-review rerender `sine_newton_iteration_aware_v2`
  - audit repair: the gallery helper no longer colors converged iterate maps from terminal value alone; it now folds in `iterations` so Newton-style basins expose contour structure instead of collapsing into flat root-color stripes, with focused regression coverage proving that differing iteration counts now produce differing PNG bytes
 - Current shortfalls to plan around:
  - the public `generic.sample` probe path currently evaluates expressions through the CPU recursion path in `ui_app/src/fractal_probe_runner.cpp`, even though a standalone CUDA generic kernel surface already exists in `ui_app/src/generic_sample_core.cu`; correctness work can continue now, but higher-throughput or broader kernel-surface work should likely route through that core or a later mainline-kernel extraction phase
  - parser surface is still intentionally small: user expressions currently expose `z`, `z_conj`, `+ - * / ^`, `sin`, `cos`, `exp`, `log`, `abs`, `conj`, `compose`, and `iterate`; internal ops such as `gf_pack2` exist below the parser, but they are not yet operator-facing
  - `iterate()` still requires an integer literal count at parse time, so sequence-driven loop-depth sweeps such as `iterate(body, steps)` with `function.params.steps` do not ship yet
  - there is no built-in `generic.render` family or viewer surface yet; any interesting non-family images in this slice need to be derived from `generic.sample` grid or `sequence_grid` response JSON through a thin post-processing helper
 - Mainline extraction note:
  - if we outgrow the current parser/op/kernel surface, this repo can port bounded kernel/evaluator slices from the mainline Salticid codebase into the DX11 core here; that should be treated as an explicit later phase, not as an ad hoc mid-slice copy, because the merge target is the mainline DX11 core itself
- Phase 1 exit criteria:
  - the missing generic-function coverage gaps are explicit
  - the intended non-family render path is chosen and documented in this plan
- Phase 2 exit criteria:
  - focused tests cover additional generic built-ins or compositions beyond the current z^3-1 / z^2+c / exp spot checks
  - any new helper used to turn generic grid responses into images has focused regression coverage
- Phase 3 exit criteria:
  - at least a small set of non-family generic expressions produce archived output images and response JSON through the chosen helper path
  - the resulting outputs are clearly expression-driven rather than fractal-family captures
- Phase 4 exit criteria:
  - hostile review checks that the new outputs truly come from `generic.sample`, not from a base fractal scene
  - continuity surfaces record the validation commands, output locations, and future-tool boundary

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- `py -3.14 -m pytest tests/test_generic_probe_cli.py tests/test_generic_sampler_gallery.py -q`
- `py -3.14 tools/reality_toolkit/scripts/run_generic_sampler_gallery.py --request-json docs/examples/callable_engine/generic_sample_sine_newton_grid.json --out-dir D:/salt-fractal/cuda_newton_fractal_clone/findings/generic_sampler_general_gallery_2026-04-14/sine_newton_iteration_aware_v2`