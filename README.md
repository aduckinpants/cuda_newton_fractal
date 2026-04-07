# CUDA Newton Fractal — Interactive Viewer & Engine

Real-time CUDA fractal renderer with DX11/ImGui viewer, headless sampling API, and 27+ fractal types.

## Quick Start

### Interactive viewer (DX11 + ImGui)

```powershell
cd ui_app
cmd /s /c build_vsdevcmd.cmd          # build viewer + all tests
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe
```

See [ui_app/README.md](ui_app/README.md) for full build instructions, runtime launcher, and CLI flags.

### Run tests

```powershell
# C++ / CUDA helper tests (35+ binaries)
cd ui_app && cmd /s /c build_tests_vsdevcmd.cmd

# Python integration + regression tests
py -3.14 -m pytest tests/ -v

# Fractal catalog smoke audit (27/27 types)
py -3.14 tools/reality_toolkit/scripts/run_fractal_catalog_smoke.py --strict
```

### CUDA smoke-test (standalone PPM renderer)

```powershell
cmd /s /c build_vsdevcmd.cmd
D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton_fractal.exe --width 1024 --height 1024
```

## Repository Layout

| Path | Purpose |
|------|---------|
| `ui_app/` | Interactive viewer: DX11, ImGui, CUDA kernels, schema binding |
| `ui/` | UI schema JSON (single source of truth for layout + controls) |
| `spec_intake/` | Design specs and planning docs — see [_STATUS.md](spec_intake/_STATUS.md) for index |
| `docs/` | Research artifacts (FITS invariance studies) |
| `tools/` | Code quality audits, reality-toolkit probe scripts, FITS analysis |
| `tests/` | Python integration tests (probe, capture, sweep, finding analysis) |
| `artifacts/` | Test reports (catalog smoke coverage) |

## Key Documents

| Document | Role |
|----------|------|
| [DEFERRED_THREADS.md](DEFERRED_THREADS.md) | Active forward planning — paused work threads with resume criteria |
| [KNOWN_ISSUES.md](KNOWN_ISSUES.md) | Bug/limitation catalog by priority |
| [TDD_SLICE_PROTOCOL_2026-04-05.md](TDD_SLICE_PROTOCOL_2026-04-05.md) | Working discipline: Red → Isolate → Green → Regression → Checkpoint |
| [HANDOFF_LOG.md](HANDOFF_LOG.md) | Agent-continuity audit trail — append-only session log |
| [spec_intake/_STATUS.md](spec_intake/_STATUS.md) | Spec index: implemented / deferred / research |

## Fractal Types (27 advertised)

**Newton family:** newton (z^3-1, z^4-1, z^5-1, custom), transcendental presets (sin, exp-1, cosh)
**Explaino family:** explaino, explaino_y, explaino_fp, explaino_nova, explaino_dual, explaino_lambda, explaino_rational_escape
**Escape-time:** mandelbrot, julia, burning_ship, multibrot (non-integer power), phoenix, lambda_map, spider, celtic_mandelbrot, perpendicular_burning_ship
**Specialized:** nova, halley

## Prerequisites (Windows)

- Visual Studio 2026 (or Build Tools) with **Desktop development with C++**
- Windows 10/11 SDK
- CUDA Toolkit 13.1+
- Python 3.14 (for integration tests and tools)

## Diagnostics

```powershell
py .\doctor.py
```
