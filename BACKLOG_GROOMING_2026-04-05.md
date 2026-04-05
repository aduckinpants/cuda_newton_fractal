# Backlog Grooming — 2026-04-05

## Current Facts
- The shipped catalog currently includes `newton`, `nova`, `mandelbrot`, `julia`, `burning_ship`, `multibrot`, `phoenix`, `explaino`, `explaino_y`, and `explaino_fp`.
- Nova is the current top technical blocker because its escape-time iteration contract is mismatched with its preset defaults, schema messaging, and renderer coloring dispatch.
- Startup defaults were also wrong for the current UX goal. The active worktree now changes startup and Reset All behavior so `auto_dive` starts off and Explaino warp starts at `0.0` instead of `0.35`.
- The helper-test lane is in place and currently covers explaino seed math, derived-field defaults, sweep playback, and high-precision view sync.
- `TDD_SLICE_PROTOCOL_2026-04-05.md` is the working rail for bounded test-first slices and is intended to be portable to later mainline implementation work.

## Priority Order
1. Nova repair and rule extraction
2. Checkpoint the startup default-off fix
3. Explaino family expansion groundwork
4. Explaino-Nova
5. Explaino-Halley
6. Transcendental Newton presets
7. Non-integer Multibrot
8. Tricorn / Multicorn
9. General Halley fractal
10. McMullen rational presets
11. View preset dropdown and preset catalog
12. Lambda research spike

## Detailed Execution Plan

### Slice 1 — Checkpoint startup defaults
**Goal:** make startup less aggressive and more legible.

Deliverables:
- `auto_dive` starts disabled
- Explaino warp starts at `0.0`
- Reset All re-applies the same defaults
- schema defaults match runtime defaults
- focused helper assertions protect the behavior

Validation:
- `ui_app/build_tests_vsdevcmd.cmd`
- `fractal_ui.exe --validate-ui`

Stop point:
- commit the default-off fix before moving on to larger catalog work

### Slice 2 — Nova repair
**Goal:** make Nova obey one consistent family contract everywhere.

Required work:
- extract a small headless rules surface for fractal-family classification and allowed coloring modes
- move Nova onto the escape-time side of that rules surface
- update preset defaults so Nova starts with escape-time coloring, not basin coloring
- update schema help and visibility so Nova is not presented as a basin-colored mode
- capture a fresh diagnostic image and retune its default view only after the rules are fixed

Recommended seam:
- add a small helper such as `fractal_family_rules.{h,cpp}` instead of scattering family checks across renderer, defaults, and UI text

Tests to add first:
- Nova allowed-coloring rules
- Nova default coloring preset
- any extracted family-classification helper

Exit criteria:
- Nova is no longer routed through the root-finding coloring branch
- invalid Nova coloring states are unselectable or explicit-error states
- one known-good diagnostic capture is checked manually

### Slice 3 — Explaino family groundwork
**Goal:** prepare for high-priority Explaino growth without exploding the schema or the kernel switch.

Required work:
- adopt the new Explaino expansion packet as the planning source
- decide the next 2 enum ids to reserve now (`explaino_nova`, `explaino_halley` recommended)
- identify the shared helper seams needed for future variants:
  - seeded polynomial generation
  - optional second-derivative evaluation
  - shared Explaino diagnostics labels

Do not do yet:
- no palette v2 work
- no generic `explaino_mode` surface yet

Exit criteria:
- the next two Explaino families are named, specced, and ordered
- the shared math helpers required for them are identified before implementation starts

### Slice 4 — Explaino-Nova
**Goal:** make the seeded procedural polynomial surface participate in an escape-time contract.

TDD plan:
- add rule/default tests first
- extract shared Nova update math if needed
- add CLI and schema visibility only after defaults and validation exist

Exit criteria:
- deterministic startup defaults
- explicit allowed coloring modes
- one diagnostic capture that looks intentionally different from both Nova and baseline Explaino

### Slice 5 — Explaino-Halley
**Goal:** compare higher-order root-finding against the same seeded Explaino roots.

TDD plan:
- add second-derivative helper tests first
- reuse the seeded polynomial generator
- keep warp off by default

Exit criteria:
- stable convergence behavior on at least one known-good seed
- no fallback to baseline Explaino on denominator problems

### Slice 6 — Catalog expansion after Explaino
Recommended order after the Explaino work starts landing:
- Transcendental Newton presets
- Non-integer Multibrot
- Tricorn / Multicorn
- General Halley fractal
- McMullen rational presets
- Lambda research spike

Reasoning:
- the first three are closest to the current substrate
- Halley benefits from second-derivative seams built for Explaino-Halley
- McMullen and Lambda add more ontology risk and should wait until the catalog rules surface is cleaner

## Backlog Notes
- The view preset dropdown remains worthwhile, but it should follow Nova repair and the next catalog wave rather than interrupt them.
- Dive-depth work is still important, but it should not take priority over Nova correctness and the near-term catalog roadmap.
- Any future catalog slice should keep the current no-warp / no-auto-dive startup posture unless a spec explicitly reopens that decision.