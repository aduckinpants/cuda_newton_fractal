# Flashlight Diagnostic Square (text)

## Pane A — Input (prompt surface)

- Seed: C:/code/setup-test/docs/seeds/explaino_bulb_change_60pt.md
- Fractal type: (unknown)
- Ticks: 8
- Warp: 0

Excerpt:
```text
# explaino seed — bulb change (60 narrators)

Explain the cosmic, technical, emotional, and bureaucratic implications of the moment someone says:

> “Are you sure, sir? It will mean changing the bulb!!”

Produce a 60‑point Red‑Dwarf‑style explaino sequence that collectively explains ONE coherent phenomenon triggered by that line.

Phenomenon (must remain the same across all points):
The act of approving the bulb change initiates a cascading shipwide procedure that briefly destabilizes the vessel’s grav‑lens manifold and causes a periodic reality phase‑stutter.

State variables (must be referenced repeatedly and consistently):
- `Δt_desync_s` (time desync seconds; starts near 0, spikes, then stabilizes)
- `pump_hz` (coolant pump frequency; may drift)
- `lens_Q` (manifold resonance sharpness
…
```

## Pane B — Observations (measured surface)

- Trace ticks recorded: 8
- Bands: {0: 2, 1: 2, 2: 2, 3: 2}
- min_abs_signed_px (min / p50 / p95): 2.0 / 11.31370735 / 39.18376541

## Pane C — Invariants clipped (constraints applied)

These are the guardrails that every ‘line path’ must pass through.

- Seed path is treated as the single source of prompt truth.
- Warp is clamped to [0,1].
- applied_schema_defaults: False
- camera_behavior: manual
- auto_dive: False
- dive_speed: 0

## Pane D — Invariants of invariants (meta-correctness)

This is the ‘Explain-o → invariants → invariants’ layer: rules about rules.

- Outputs are file-based and auditable (inputs/outputs exist as artifacts).
- Probe output is structured JSON with stable keys (machine-checkable).
- Exports are derived from probe JSON (no hidden state).

