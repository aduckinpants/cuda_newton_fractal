# Magnet State Pack

Small loadable Magnet Type I states for quick manual exploration and regression capture.

Each example is a folder containing a literal `state.json`, because the runtime loader accepts direct state paths with that filename. Load one with:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json <example-folder>\state.json --capture-diagnostic
```

| Example | Purpose |
| --- | --- |
| `default_smooth/state.json` | Default smooth Magnet Type I baseline. |
| `high_relaxation/state.json` | High relaxation response with faster pull toward the Magnet fixed point. |
| `low_relaxation/state.json` | Low relaxation response with slower Magnet settling. |
| `seed_offset/state.json` | Nonzero Magnet seed offset for asymmetric basin/escape structure. |
| `low_bailout_edge/state.json` | Lower bailout edge case that changes escape classification without changing the family lane. |
