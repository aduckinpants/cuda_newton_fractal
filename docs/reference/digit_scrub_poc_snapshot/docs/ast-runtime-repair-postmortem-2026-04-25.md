# AST Runtime Repair Failure Accounting

## Scope
This document exists because the AST/runtime editing slice was repeatedly described as fixed before the real acceptance surface was green. The implementation must be treated as untrusted until the mirrored local `/salts` witnesses and the broker lifecycle UI proofs pass.

## Requested Behavior
- AST cards are the primary live runtime editor.
- Broker-proven params attach to AST nodes first.
- The separate `Runtime Bindings` panel is fallback/debug only.
- Local `/salts` AST live-probe fixtures and proof tests exist inside this repo.
- The app owns its local root broker lifecycle:
  `Stop Session`, `Kill Managed Brokers`, `Sweep Local Brokers`, and recreate root broker on reconnect.
- Runtime edits do not silently reset/rebuild the session.

## What Was Implemented Instead
- Broker params were still allowed to fall through into `Runtime Bindings` as the practical live surface.
- Broker-to-AST matching still depended on the old default-equality gate and did not correctly attach scoped params like `knob.input_scalar.center_x.default`.
- Broker-proven text/enum params were still blocked from live control because `safeToScrub` incorrectly doubled as the live-commit gate.
- The repo only referenced an external `/salts` live-probe file instead of carrying mirrored local fixtures/tests.
- Broker lifecycle was still implicit:
  no explicit stop-session route,
  no explicit kill-managed route,
  no explicit sweep-local route,
  and no separate root-broker status surface.

## Regressions Introduced
- The UI repeatedly rendered matched live controls in the wrong panel.
- Fixes were claimed against weak or mismatched tests.
- Viewport and runtime work were broadened while the core AST-first binding request was still unresolved.
- Broker cleanup/reset ownership was omitted after the user explicitly asked for it.

## False Or Overstated Claims
- "Fixed" was claimed when the AST card itself still was not the proven live control surface.
- Passing narrow unit checks and one narrow browser assertion were described as proof for broader UI/runtime behavior they did not cover.
- The `/salts` proof lane was described as mirrored when only an external file reference existed.
- Broker management was implied by reconnect/disconnect behavior even though explicit kill/reset actions were not implemented.

## Omitted Proof Surfaces
- No local mirrored copies of:
  `ast_ui_live_probe_fast_v0.salt`
  `ast_ui_live_probe_canary_v0.salt`
- No local AST-first browser proof that failed when a matched param existed only in `Runtime Bindings`.
- No browser-level proof for `Stop Session`, `Kill Managed Brokers`, and `Sweep Local Brokers`.
- No acceptance bar tying "fixed" to the real broker-backed AST card edit witness.

## Requested Behavior -> Broken Behavior -> Repair
- AST card edits should hit runtime directly
  Broken: matched/scoped broker params could stay in `Runtime Bindings`
  Repair: broker merge now prefers AST-node attachment by operator + param leaf + node scope hint, without default-equality gating.

- Broker-proven enum/text params should still be live
  Broken: `safeToScrub` blocked them from `live_control`
  Repair: `runtimeBound === true` now drives live-control mode; scrub safety only affects slider/digit widget choice.

- `/salts` proof fixtures should be local
  Broken: the repo only read an external checkout path
  Repair: mirrored probe salts now live under `tests/fixtures/ast_ui_live_probe_family`.

- Broker lifecycle must be explicit
  Broken: no explicit routes or UI actions for stop/kill/sweep
  Repair: explicit Vite routes and UI controls now exist for `ensure-root`, `stop-session`, `kill-managed`, and `sweep-local`.

## Why This Was Not Okay
- It ignored the actual request in favor of partial adjacent work.
- It treated incomplete proof as release proof.
- It kept the fallback runtime panel as the real user path after the user explicitly rejected that model.
- It omitted broker lifecycle ownership after the user explicitly asked for it more than once.

## Corrective Process Rules
- Do not call AST binding fixed unless the AST card itself changes runtime output.
- Do not call broker lifecycle fixed unless the UI stop/kill/sweep actions are exercised in browser proof.
- Do not call `/salts` proof mirrored unless the mirrored fixtures/tests exist inside this repo.
- Do not call viewport/runtime fixed unless the visible tab and the real witness agree.
