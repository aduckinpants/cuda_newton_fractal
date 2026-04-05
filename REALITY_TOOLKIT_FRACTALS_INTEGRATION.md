# Reality-Toolkit-Fractals Integration

This repo should be treated as a fractal-focused extension of the broader reality-toolkit work, similar in spirit to the newer musical toolkit extensions.

Working name:
- `reality-toolkit-fractals`

## Role

Two layers matter here:
- `nine` remains the larger upstream research workspace with the mature broker/viewer pattern and the broader image-analysis toolkit.
- `cuda_newton_fractal_clone` is where fractal-specific generation, sweeps, diagnostics, and future fractal-analysis extensions live.

The local `tools/reality_toolkit` tree in this repo is the start of that fractal extension surface. It should grow by borrowing proven analysis ideas from `nine`, not by pretending the two repos are isolated worlds.

## Preferred live-view model

Preferred default:
- use `nine`'s broker/viewer model, including the flex-grid direction, **if** this repo can load sessions into it and unload them again cleanly.

That means the real question is not "shared broker or dedicated broker?" in the abstract.
The real question is whether this repo can prove a smooth session lifecycle inside the shared viewer:
- session create registers cleanly
- session destroy removes cleanly
- no stale-frame leakage between runs
- no stomping on in-flight testing
- no ambiguous ownership of the displayed session

Reasoning:
- the flex-grid design is explicitly meant to absorb multiple active sessions into one viewer surface
- one shared viewer is simpler than multiplying windows if the lifecycle contract is solid
- `nine` already has the richer broker/viewer QoL surface and broader toolkit context
- `nine`'s tests and prior bug history also show that session isolation is real engineering work, not something to assume blindly

## Required shape for future broker adoption here

When broker/live-view work lands in this repo, the sequence should be:
1. first try to integrate with `nine`'s shared broker/viewer path
2. prove smooth load/unload and non-stomping behavior with bounded tests/probes
3. only fall back to a repo-specific broker instance if that proof fails or if ownership/isolation remains muddy

If fallback is needed, the repo-specific stack should have:
- dedicated launcher scripts in this repo
- dedicated process/window identity so it is visually obvious which repo owns the live view
- dedicated default port or explicit per-repo port configuration
- dedicated publish/runtime roots under `D:\salt-fractal\cuda_newton_fractal_clone\...`

In short: prefer the shared `nine` viewer when it behaves like a clean multi-session surface; otherwise isolate.

## Analysis guidance

Generated image data from this repo should not be judged only by ad hoc visual inspection.

Use both:
- this repo's local `tools/reality_toolkit` fractal-specific helpers
- `nine`'s larger reality-toolkit image-analysis library when doing parity, drift, clustering, or dataset-comparison work

That keeps this repo aligned with the larger toolkit ecosystem while still letting the fractal layer evolve as its own extension.

## Planning note

A future full planning/spec turn should be devoted to:
- a much broader fractal catalog expansion
- new Explaino family concepts
- how to use existing salticid operators and CLI tooling to probe those ideas in real time
- how this repo should prove smooth shared-broker integration with `nine`, and what fallback broker shape is acceptable if that proof fails