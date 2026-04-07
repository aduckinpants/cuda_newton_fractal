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

### 3.3 Specialized escape-time helper extraction

Landed:
- `ui_app/src/escape_time_specialized_formulas.h`

Purpose:
- unify the specialized McMullen and Collatz escape-time loops used by the CUDA renderer and CPU probe path

Coverage now includes:
- McMullen
- Collatz

Important contract preserved:
- McMullen pole handling stays explicit in the probe path
- both paths share the `10000` specialized escape-radius contract

### 3.4 Perturbation reference-orbit extraction

Landed:
- `ui_app/src/perturbation_reference_orbit.h`

Purpose:
- move host-side perturbation enablement, cache-key matching, and reference-orbit generation out of `RenderFractalCUDA(...)`

Important contract preserved:
- perturbation stays limited to deep-zoom Mandelbrot and Julia
- the reference orbit still keys off `center_hp_x/y` and `max_iter`
- Julia keeps the existing reference-`z0` plus fixed-constant orbit contract

## 4) Next Cleanup Slices

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

1. isolate escape-time coloring helpers from `kernel_render`
2. leave perturbation cache plumbing and specialized-family logic alone for that slice
3. validate with helper + viewer builds
4. checkpoint immediately