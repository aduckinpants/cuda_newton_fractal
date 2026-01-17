# Spec Intake Packet

**Spec Title:** General Directive — No Implicit Fallback

**Date:** 2026-01-15

## 0) Proposed Directive Text (as provided)
> never introduce implicit fallback behavior; absence of a valid target must be treated as an error, not a candidate for inference, substitution, or repair

## 1) Intake Summary
This is a strong, system-wide hygiene rule. It reduces “mysterious success,” prevents silent misconfiguration, and makes rapid iteration faster (because failures are crisp and local).

It is also easy to misapply if we don’t define boundaries: some safety clamps (e.g., numeric stability guards) can be *valid engineering* rather than “fallback inference.” The directive should make this distinction explicit.

## 2) Clarifications Needed (to avoid ambiguity)
### 2.1 What counts as a “valid target”?
Recommend defining “target” as:
- a binding destination (e.g., schema binding path)
- a resource identifier (e.g., device id, file path)
- a requested mode (enum selection)
- a required dependency (driver/toolkit mismatch, missing shader)

### 2.2 What is “implicit fallback” vs “explicit defaulting”?
Recommend drawing a bright line:
- **Allowed:** explicit defaults applied at a well-defined boundary, with clear ownership (e.g., schema defaults at schema-load time; preset defaults when the user explicitly selects a fractal type).
- **Forbidden:** silently picking an alternate target when the requested one is missing/invalid (e.g., “unknown binding path -> ignore”, “invalid enum -> treat as first option”, “missing file -> use built-in stub”).

### 2.3 Error handling expectations
- “Treated as an error” should imply: surfaced to UI/logs, blocks the operation, and is actionable.
- Prefer deterministic failure points: validate inputs before executing expensive work.

## 3) Recommendations (tighten the directive)
Suggested refined phrasing:

"Never introduce implicit fallback behavior. If a referenced target is missing or invalid, raise an explicit error at the earliest validation boundary. Defaults are permitted only when they are explicit, documented, and applied at a single declared boundary (e.g., schema load, preset application triggered by explicit user choice)."

## 4) Example Applications (concrete)
- **Schema binding:** unknown `binding.path` must fail schema load (not be ignored).
- **Enum selection:** unknown enum id must error (not map to nearest/first).
- **Parameter validity:** out-of-range parameters must error (not be clamped silently).
- **Resource selection:** invalid device id must error (not silently choose device 0) *unless* the UI explicitly says “Auto-select device” and the user chose that mode.

## 5) Implementation Pattern (system-wide)
- Add explicit validation passes:
  - validate schema controls/bindings/types
  - validate runtime parameters before launching kernels / dispatch
  - validate resource handles before use
- Prefer returning structured error strings (or error codes) rather than implicit behavior.

## 6) Compatibility With Current Work
We already applied this pattern in the fractal demo by:
- validating schema bindings up front (unknown binding => schema error)
- treating invalid Newton-only coloring modes as explicit error color/warning rather than falling back silently
- moving away from silent parameter clamping (validate on host and fail fast)

## 7) Acceptance Criteria
- No “silent success” for missing/invalid targets.
- Errors are early, deterministic, and actionable.
- Defaults exist only at explicitly declared boundaries.
