# Reality-Toolkit-Fractals Integration

This repo should be treated as a fractal-focused extension of the broader reality-toolkit work, similar in spirit to the newer musical toolkit extensions.

Working name:
- `reality-toolkit-fractals`

## Role

Two layers matter here:
- `nine` remains the larger upstream research workspace with the mature broker/viewer pattern and the broader image-analysis toolkit.
- `cuda_newton_fractal_clone` is where fractal-specific generation, sweeps, diagnostics, and future fractal-analysis extensions live.

The local `tools/reality_toolkit` tree in this repo is the start of that fractal extension surface. It should grow by borrowing proven analysis ideas from `nine`, not by pretending the two repos are isolated worlds.

## Hard Constraint: dedicated live-view instance

Do **not** reuse `nine`'s active broker/viewer instance when this repo starts doing live-view or broker-fed work.

Reasoning:
- `nine`'s broker stack is a repo-owned runtime surface with always-on assumptions and a default port (`7745`).
- `nine` has duplicate-instance behavior meant to manage its own broker/viewer lifecycle.
- `nine`'s tests explicitly start brokers on free ports, which is a strong signal that broker isolation is part of the contract.
- Reusing the active `nine` broker from this repo risks stomping in-flight testing, stale-frame confusion, and accidental cross-session interference.

## Required shape for future broker adoption here

When broker/live-view work lands in this repo, it should use a **repo-specific** stack:
- dedicated launcher scripts in this repo
- dedicated process/window identity so it is visually obvious which repo owns the live view
- dedicated default port or explicit per-repo port configuration
- dedicated publish/runtime roots under `D:\salt-fractal\cuda_newton_fractal_clone\...`
- no silent attachment to a broker that was started by `nine`

In short: borrow the **broker model** from `nine`, but do not borrow the **running broker instance**.

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
- how a dedicated repo-local live-view/broker stack should expose those runs without interfering with `nine`