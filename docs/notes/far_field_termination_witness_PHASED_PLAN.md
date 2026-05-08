# Far-Field Termination Witness

## Current Phase

Phase 1 in progress - termination vocabulary and inert transport scaffolding are landed and native-validated; contract proof, plan sync, and checkpoint closure remain

## Phase Checklist

- [ ] Phase 1 - add compatibility-first tests and transport scaffolding for `TerminationKind`, witness config, and emitted sample/probe fields without changing iteration behavior
- [ ] Phase 2 - implement the optional far-field-settled witness behind family-gated policy, still disabled by default, with real behavior-owner and integration proofs
- [ ] Phase 3 - expose the witness to tooling and the advanced color pipeline as a family-gated Source row, then plan the follow-on parameterized basin successors

## Explicit User Asks

- [open] Do not supersede the earlier advanced-color foundation and grading planning; this witness work is additive layering.
- [open] Treat the far-field-settled logic as a new optional termination witness, not a global bailout replacement.
- [open] Keep the authority in the fractal iteration / sample result layer first, then let color consume the result later.
- [open] Stop using useless surrogate tests; the repo must prove the behavior so the user is not the guinea pig.
- [open] The current parameterless basin rows are not acceptable as a long-term endpoint; plan genuinely parameterized basin-facing successors later.

## Presumption Loop

The first bounded implementation slice should not attempt the witness math itself. The falsifiable local hypothesis for Phase 1 is that the repo can add explicit termination vocabulary plus emitted transport fields for the witness without changing current iteration behavior, probe status semantics, or generic-sample semantics. The cheapest disconfirming checks are compatibility-focused: existing sample/probe behavior should stay green while new defaulted fields round-trip cleanly through the owning transport types.

## Presumption Evidence

- `ui_app/src/fractal_sample_result.h` is the canonical per-sample result surface, but it currently carries only `iterations`, `final_z`, `residual`, `converged`, and `escaped`.
- `ui_app/src/fractal_probe_contract.h` still uses a narrow `FractalProbeSampleStatus` enum and sample transport struct, so a non-breaking Phase 1 should add parallel fields rather than replacing status semantics immediately.
- `ui_app/src/generic_function_types.h` uses a parallel `GenericSampleResult` transport struct that will need the same additive treatment if `generic.sample` is expected to consume witness output later.
- The current advanced-color foundation in `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` remains active, so this witness track must live in its own plan/contract surface and reconnect later at defined seams.

## Proof Ledger

- Read-only finding: the witness belongs in sample/termination ownership first and should only become a color Source row later.
- Read-only finding: the first honest implementation slice is transport and vocabulary only; the witness math and family enablement should remain for the next bounded slice.
- Implemented: `TerminationKind` and inert far-field delta transport now exist on the sample/probe surfaces, serialize through probe JSON, and default to compatibility-safe `none`/`null` values.
- Implemented: `generic.sample` now carries the same inert termination transport shape through `GenericSampleResult` and probe marshalling without changing current status behavior.
- Audit fix: the transport slice intentionally stopped short of wiring `OrbitTerminationConfig` into `KernelParams`; the config type exists as vocabulary only until the real behavior-owner slice lands.

## Notes

- Expected owner files for Phase 1:
  - `docs/notes/far_field_termination_witness_PHASED_PLAN.md`
  - `docs/contracts/far_field_termination_witness_phase1_transport.contract.json`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/fractal_sample_result.h`
  - `ui_app/src/generic_function_types.h`
  - `ui_app/src/fractal_probe_contract.h`
  - `ui_app/src/fractal_probe_contract.cpp`
  - `ui_app/src/fractal_probe_runner.cpp`
  - focused tests covering transport compatibility
- Non-goals for Phase 1:
  - do not implement the far-field witness math yet
  - do not enable the witness on any family yet
  - do not add a new `ColorSignal` yet
  - do not add fake parameters to `root_index`, `root_classic_palette`, or `joy_root_palette`

## Resume Point

Finish Phase 1 closure from the landed transport scaffolding: validate the checked-in contract and phased-plan sync, then checkpoint. After that, move to the next slice: witness math and family-gated default-off behavior.