# Explaino Sidecar Recovery

## Current Phase

Phase 2 - runtime motion proof surface

## Phase Checklist

- [x] Phase 1 - real-schema proof surface
- [ ] Phase 2 - runtime motion proof surface
- [ ] Phase 3 - contract repair for baseline Explaino auto-demo
- [ ] Phase 4 - closure audit and continuity cleanup

## Notes

- Spec source: `spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md`
- Recovery posture for this plan:
  - treat the shipped sidecar behavior as untrusted until it is proven against the real schema and runtime, not against synthetic test fixtures
  - do not add or trust sidecar tests that bypass the checked-in schema or the real `BuildEngineCatalog(...)` path when making claims about shipped behavior
  - keep the recovery fail-closed: if baseline `explaino` cannot auto-demonstrate under the real schema, the sidecar must either be repaired to do so or state that limitation explicitly
- Current known mismatch to close:
  - the existing sidecar window tests use a synthetic catalog where `fractal.params.explaino_mix` is cost-annotated and applicable on baseline `explaino`
  - the shipped schema only exposes `explaino_mix` on `explaino_dual`, while the measured cost hints currently exist only for variant-only params such as ripple/splice/vortex/tension
  - this lets the tests prove a recommendation path that the default shipped startup surface cannot actually take

- Phase 1 exit criteria:
  - a native helper test loads the checked-in schema, builds the real engine catalog, and proves the current baseline `explaino` sidecar state under that real surface
  - the same proof surface also demonstrates that at least one variant path with real cost-hint metadata can produce an action recommendation under the same test host
  - the phased plan clearly records that this slice is characterization only and does not yet claim the spec is satisfied
  - current receipt: `ui_app/tests/test_explaino_sidecar_schema_contract.cpp` under `ui_app/build_tests_vsdevcmd.cmd`

- Phase 2 exit criteria:
  - add runtime proof that the user-visible motion contract is real, not inferred
  - required proofs: default-startup path behavior, explicit Apply Armed Step behavior, and paced-loop behavior
  - these proofs must measure both parameter delta and rendered-frame delta

- Phase 3 exit criteria:
  - choose and implement one explicit contract for baseline `explaino`
  - preferred contract: baseline `explaino` can produce sidecar recommendations on the real schema
  - acceptable fallback only if made explicit in product behavior: baseline `explaino` is blocked and the UI states why instead of implying autoplay readiness

- Phase 4 exit criteria:
  - hostile re-audit the repaired seams against the spec rows and the runtime proofs
  - update the older sidecar continuity docs so they no longer contradict the direct-mutation follow-on work
  - validate the current real stop point with `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`