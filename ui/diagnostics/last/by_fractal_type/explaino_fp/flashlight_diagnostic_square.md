# Flashlight Diagnostic Square (text)

## Pane A — Input (prompt surface)

- Seed: C:/code/setup-test/docs/seeds/explaino_bulb_change_60pt.md
- Fractal type: explaino_fp
- Ticks (requested): 2
- Ticks (recorded): 2
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

- Trace ticks recorded: 2
- Bands: {0: 1, 1: 1}
- min_abs_signed_px (min / p50 / p95): 16.89949226 / 16.89949226 / 24.89949226

## Pane C — Invariants clipped (constraints applied)

These are the guardrails that every ‘line path’ must pass through.

- Seed path is treated as the single source of prompt truth.
- Warp is clamped to [0,1].
- applied_schema_defaults: True
- camera_behavior: manual
- auto_dive: False
- dive_speed: 0

## Pane D — Invariants of invariants (meta-correctness)

This is the ‘Explain-o → invariants → invariants’ layer: rules about rules.

- Outputs are file-based and auditable (inputs/outputs exist as artifacts).
- Probe output is structured JSON with stable keys (machine-checkable).
- Exports are derived from probe JSON (no hidden state).

## Fixed Invariants Certificate

- certificate: PASS
- required: ['seed', 'probe_json', 'state_json', 'frame_bmp', 'lens_sdf_bmp', 'super_frame_bmp', 'trace_lines_obj']

Artifacts:
- diag_square_bmp: MISSING (flashlight_diagnostic_square.bmp)
- frame_bmp: ok bytes=3145782 sha256_12=c5a5765bc501
- lens_sdf_bmp: ok bytes=3145782 sha256_12=56b5f769287b
- probe_json: ok bytes=4458 sha256_12=9c06633b3e9a
- sdf_anchor_stl: ok bytes=468884 sha256_12=cf3bdb36c9f6
- seed: ok bytes=3050 sha256_12=3893f27cfef9
- state_json: ok bytes=1750 sha256_12=35cb03f69e5c
- super_frame_bmp: ok bytes=3145782 sha256_12=85a3e9f25309
- trace_gif: MISSING (flashlight_trace.gif)
- trace_lines_obj: ok bytes=1821 sha256_12=0a63646fd17e
- trace_overlay_bmp: ok bytes=3145782 sha256_12=d20e17d29334
- trace_stl: ok bytes=28884 sha256_12=a6f4457550ce
- vectors_overlay_bmp: ok bytes=3145782 sha256_12=4a0e602457bb

