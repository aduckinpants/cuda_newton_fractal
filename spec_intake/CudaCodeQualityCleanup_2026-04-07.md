# Research Packet

**Title:** CUDA Code Quality Cleanup Plan

**Date:** 2026-04-07

**Status:** active cleanup plan after the bounded common-fractal wave

## 0) Objective

Reduce structural risk in `ui_app/src/fractal_renderer.cu` and closely coupled runtime seams without changing the shipped feature set.

The goal is not a one-shot rewrite. The goal is a ratcheting cleanup sequence where each slice:
- extracts one real seam
- adds focused tests first
- preserves current rendering behavior
- leaves the repo in a checkpointed, build-green state

## 1) Constraints

- Keep the viewer-host contract intact: no implicit fallback, schema remains layout authority, runtime structs remain state authority.
- Do not mix new fractal families into cleanup slices.
- Prefer helpers shared between CUDA render and CPU probe paths when the math or validation contract is the same.
- Keep perturbation, McMullen, Collatz, and other specialized paths isolated until they get their own bounded cleanup slice.
- End each cleanup slice with:
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - a checkpoint commit

## 2) Tooling Note

The standing ratchet note references `tools/code_quality_audit.py` and `tools/code_quality_baseline.json`, but those files are not present in this workspace snapshot.

Until that tooling exists here or is explicitly bridged in, cleanup must be driven by:
- focused duplication hunts
- seam extraction tests
- helper/viewer build validation

That is weaker than a full audit ratchet, so every slice should stay small and explicit.

## 3) Landed Cleanup Slices

### 3.1 Shared direct escape-time formulas

Landed:
- `ui_app/src/escape_time_direct_formulas.h`

Purpose:
- unify the direct escape-time state machine used by the CUDA renderer and CPU probe path for the common direct families

Coverage now includes:
- Mandelbrot
- Julia
- Burning Ship
- Multibrot
- Phoenix
- Multicorn
- Lambda
- Spider
- Celtic Mandelbrot
- Perpendicular Burning Ship

Intentionally not included yet:
- perturbation orbit path
- McMullen
- Collatz

### 3.2 Shared runtime validation helper

Landed:
- `ui_app/src/fractal_runtime_validation.h`

Purpose:
- unify the fail-fast fractal parameter validation rules between the probe path and `RenderFractalCUDA(...)`

Important drift fixed:
- `explaino_nova` now shares the same `nova_alpha` validation contract in both paths

## 4) Next Cleanup Slices

### Slice A — Specialized escape-time helper extraction

Target:
- extract McMullen and Collatz specialized loops into dedicated helpers instead of leaving them inline in the renderer monolith and probe sampler

Exit criteria:
- focused helper test(s)
- renderer/probe call the helper(s)
- no behavior changes for specialized families

### Slice B — Perturbation/reference-orbit extraction

Target:
- move host reference-orbit generation and cache key logic out of `RenderFractalCUDA(...)`

Exit criteria:
- render entrypoint gets smaller
- perturbation enablement stays explicit and unchanged
- cache/state handling is easier to reason about in isolation

### Slice C — Escape-time coloring extraction

Target:
- isolate smooth-escape color smoothing and palette logic from the kernel monolith into named helpers with narrower responsibilities

Exit criteria:
- no palette change
- reduced branching density inside `kernel_render`

### Slice D — Common complex helper convergence pass

Target:
- review remaining duplicated complex math helpers across CUDA and host code once the main render/probe seams are extracted

Exit criteria:
- only extract helpers that are materially reused
- avoid abstracting math that is still genuinely one-off

## 5) Stop Rules

- Do not start feature growth again until at least Slice A and Slice B land cleanly.
- Do not bundle specialized-family cleanup, perturbation extraction, and coloring extraction into one commit.
- If a cleanup slice needs more than one new helper plus one caller rewrite, it is probably too large.

## 6) Immediate Next Step

The best next bounded cleanup slice is:

1. extract specialized McMullen/Collatz helpers
2. leave perturbation and coloring alone for that slice
3. validate with helper + viewer builds
4. checkpoint immediately