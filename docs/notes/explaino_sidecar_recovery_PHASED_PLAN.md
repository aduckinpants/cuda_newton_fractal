# Explaino Sidecar Recovery

## Current Phase

Phase 3 - runtime motion proof surface

## Phase Checklist

- [x] Phase 1 - real-schema proof surface
- [x] Phase 2 - contract repair for baseline Explaino auto-demo
- [ ] Phase 3 - runtime motion proof surface
- [ ] Phase 4 - closure audit and continuity cleanup

## Notes

- Spec source: `spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md`
- Recovery posture for this plan:
  - treat the shipped sidecar behavior as untrusted until it is proven against the real schema and runtime, not against synthetic test fixtures
  - do not add or trust sidecar tests that bypass the checked-in schema or the real `BuildEngineCatalog(...)` path when making claims about shipped behavior
  - keep the recovery fail-closed: if baseline `explaino` cannot auto-demonstrate under the real schema, the sidecar must either be repaired to do so or state that limitation explicitly
- Current known mismatch to close:
  - the real-schema contract is now repaired at the native helper level: baseline `explaino` and `explaino_joy` both expose actionable controller decisions under the shipped schema/catalog path
  - the remaining gap is runtime-visible proof that explicit apply and paced-loop motion behave as real product motion, not just headless contract mutation

- Phase 1 exit criteria:
  - a native helper test loads the checked-in schema, builds the real engine catalog, and proves the current baseline `explaino` sidecar state under that real surface
  - the same proof surface also demonstrates that at least one variant path with real cost-hint metadata can produce an action recommendation under the same test host
  - the phased plan clearly records that this slice is characterization only and does not yet claim the spec is satisfied
  - current receipt: `ui_app/tests/test_explaino_sidecar_schema_contract.cpp` under `ui_app/build_tests_vsdevcmd.cmd`

- Phase 2 exit criteria:
  - the real-schema native contract proves that baseline `explaino` and `explaino_joy` expose actionable recommendations under the shipped schema/catalog path
  - applying the armed controller decision in that contract changes both the bound parameter value and sampled output residual
  - the shipped describe/catalog surface attaches generic cost metadata to the common Explaino-family controls needed for that contract while preserving measured sensitivity reports only on the staged ripple/splice/vortex/tension rows

- Phase 3 exit criteria:
  - add runtime proof that the user-visible motion contract is real, not inferred
  - required proofs: default-startup path behavior, explicit Apply Armed Step behavior, and paced-loop behavior
  - these proofs must measure both parameter delta and rendered-frame delta

- Phase 4 exit criteria:
  - hostile re-audit the repaired seams against the spec rows and the runtime proofs
  - update the older sidecar continuity docs so they no longer contradict the direct-mutation follow-on work
  - validate the current real stop point with `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`