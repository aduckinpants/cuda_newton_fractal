# TDD Slice Protocol — 2026-04-05

This repo is the proving ground for a later mainline implementation pattern.

The goal is not "more tests" in the abstract. The goal is a bounded slice protocol that keeps fractal work out of the weeds:
- write a focused headless test first
- isolate the rule or seam that is actually broken
- implement the smallest fix that makes the test pass
- add regression coverage around the extracted seam
- validate the app-level surface
- checkpoint the slice with a commit

## Working Rule
Every low-risk improvement slice should follow this order:
1. **Red:** add or extend one focused helper test that demonstrates the bug or missing contract.
2. **Isolate:** extract the smallest reusable helper needed to remove branching drift from `main.cpp`, presets, or renderer code.
3. **Green:** change the implementation only enough to satisfy the failing test.
4. **Regression proof:** add adjacent assertions that lock the repaired rule in place for sibling fractal families.
5. **App validation:** run the helper-test lane plus `fractal_ui.exe --validate-ui` when schema/runtime surfaces changed.
6. **Checkpoint:** commit and push the bounded slice.

## Why This Exists
The current repo has been operating too close to a zero-test workflow for behavior changes. This protocol is the guardrail for moving toward the mainline repo's intended discipline without letting each change balloon into a broad refactor.

The pattern is intended to be copied forward into later mainline implementation work once it proves itself here.

## First Worked Pattern
### Slice: Nova contract repair
Phase:
- extract fractal-family and coloring rules from scattered conditionals

Exit criteria:
- Nova is treated as an escape-time family for coloring and default preset selection
- Nova no longer appears in basin-coloring UI messaging
- a focused helper test proves the Nova rule and locks sibling families in place
- helper-test lane passes
- `fractal_ui.exe --validate-ui` passes

## Easy-Win Queue After Nova
These are the next bounded slices that fit the same pattern:
- fractal-family coloring rule coverage for all currently shipped families
- default-coloring preset coverage for all currently shipped families
- any additional startup-default seams that can be proven headlessly
- future catalog additions only after their rule helper and defaults are tested first

## Non-Goals For This Rail
- no giant refactor before the first failing test exists
- no GUI-only debugging as the primary proof path
- no speculative catalog expansion implementation without a small headless seam first