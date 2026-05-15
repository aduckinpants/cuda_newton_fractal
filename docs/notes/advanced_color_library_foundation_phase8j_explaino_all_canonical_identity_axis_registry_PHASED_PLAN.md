# Advanced Color Library Foundation Phase 8J - Explaino-all Canonical Identity + Axis Registry

## Current Phase

Complete - explaino_all is now the canonical public Explaino identity, the shared seven-axis registry owns the Explaino-all control surface, committed-head proofs plus machine receipts are written, and the stale-plan reread no longer leaves fake open closure chores behind.

## Phase Checklist

- [x] Phase 1 - open and lock the slice, inspect the owner seams, apply the first checked-in RED witnesses, and prove explaino_all is still missing as a real selector/runtime/default identity and that the seven family axes still rely on per-variant one-off authority instead of one canonical registry
- [x] Phase 2 - land the smallest truthful explaino_all canonical identity plus shared-axis registry path, make explaino_all first/default, and drive the Explaino-all control surface from the registry without widening into alias migration, enforcement, cleanup, or historical archive work
- [x] Phase 3 - validate through the narrowest truthful selector/schema/state/runtime rails, hostile-audit the repaired state, checkpoint the slice, write the machine receipts, clear stale closeout text, and keep umbrella authority truthful on the committed head

## Explicit User Asks

- [done] Open and execute Explaino-all slice 1 only: canonical public identity plus axis registry.
- [done] Introduce explaino_all as the canonical public Explaino identity.
- [done] Make explaino_all the first Explaino selector entry and the default selection on load.
- [done] Define one shared Explaino axis registry for ripple_amplitude, splice_offset, vortex_strength, tension_strength, balance_void, symmetry_tension, and field_curvature.
- [done] Drive the Explaino-all control surface from that registry instead of per-variant one-off control authority.
- [done] Keep legacy alias migration, enforcement guardrails, cleanup-only work, generic advanced-color core work, and historical archive compatibility out of this slice except for any minimal viability shim strictly required to make explaino_all runtime-real.
- [done] Close only with commit, validation receipt, contract proof receipt, clean tree, and a post-closeout stale-plan reread on the committed head.

## Proof Ledger

- Bootstrap on 2026-05-14 proved branch=feature/advanced-color-pipeline-draft-editor-reframe, HEAD=ccc5769, clean tree, and active locked contract advanced_color_library_foundation_phase8h_explaino_balance_void_owner_proof.
- py -3.14 tools/viewer_host_repo_status.py proved staged=none | unstaged=none | untracked=none, so this session starts fresh rather than as carryover.
- Current repo authority still treats explaino_all as planning-only: ui/fractal_binding_surface_v1.ui_schema.json defaults fractal_type to explaino, not explaino_all, and does not expose any explaino_all selector id.
- Current selector ordering authority is ui/fractal_binding_surface_v1.ui_schema.json, which still lists the peer Explaino variants separately under the Explaino group.
- Current Explaino family axes are already runtime/state bound in ui_app/src/fractal_types.h, ui_app/src/schema_binding.cpp, ui_app/src/runtime_reset.cpp, ui_app/src/diagnostics_capture.cpp, and ui_app/src/diagnostics_state_io.cpp, but the newer family axes still surface through per-variant visible_if gates and hand-coded defaults.
- Current per-variant one-off authority still exists in ui/fractal_binding_surface_v1.ui_schema.json visible_if entries and ui_app/src/fractal_derived_fields.cpp variant-specific default branches.
- Minimal continuity preflight re-opened the broader docs continuity scope only long enough to create and lock this successor slice truthfully.
- Action-level hostile preflight found one real blocker before GREEN: the phase8j contract omitted ui_app/src/ui_schema.cpp and ui_app/tests/test_explaino_zero_axis_equivalence.cu even though the slice needs them. The contract scope was updated and re-locked before repo mutation.
- Checked-in REDs are now machine-proven:
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd > artifacts\validation\phase8j_red_native_build.log 2>&1` failed at ui_app/tests/test_viewer_state_init.cpp because FractalType::explaino_all did not exist on RED HEAD.
  - `cmd /c "call tools\call_vsdevcmd.cmd && cd /d C:\code\cuda_newton_fractal_clone\ui_app && cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src .\tests\test_fractal_family_rules.cpp /Fe:C:\code\cuda_newton_fractal_clone\artifacts\validation\phase8j_red_test_fractal_family_rules.exe" > artifacts\validation\phase8j_red_test_fractal_family_rules.log 2>&1` failed because ExplainoCanonicalFractalType and kExplainoAxisRegistry were missing.
  - `cmd /c "call tools\call_vsdevcmd.cmd && cd /d C:\code\cuda_newton_fractal_clone\ui_app && cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_ui_schema.cpp /Fe:C:\code\cuda_newton_fractal_clone\artifacts\validation\phase8j_red_test_ui_schema.exe && C:\code\cuda_newton_fractal_clone\artifacts\validation\phase8j_red_test_ui_schema.exe" > artifacts\validation\phase8j_red_test_ui_schema.log 2>&1` failed because the schema still lacked the canonical Explaino-all selector and control surface.
- GREEN landed through the approved patch wrapper after regenerating the temp diff against current repo bytes and stripping EOF-only patch noise that initially blocked git apply.
- Focused native/runtime GREEN witnesses now pass from artifacts/validation/phase8j_focus/:
  - test_enum_id_utils.log
  - test_viewer_state_init.log
  - test_ui_schema.log
  - test_explaino_zero_axis_equivalence.log
- The CUDA runtime witness found one real code defect after GREEN: ExplainoCanonicalFractalType was host-only, so the canonical device/runtime routing in fractal_sample_device.inl failed to compile. That seam was repaired in ui_app/src/fractal_family_rules.h and the CUDA witness was rerun green.
- Published-runtime proof is now green on the repaired state:
  - `cmd /c "ui_app\build_vsdevcmd.cmd > artifacts\validation\phase8j_focus\build_vsdevcmd.log 2>&1"`
  - `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k explaino_all > artifacts\validation\phase8j_focus\runtime_pytest_explaino_all.log 2>&1`
- Hostile audit found one post-GREEN UI truth defect: family-wide Explaino controls such as auto_increment_seed, explaino_seed, and epsilon still omitted explaino_all from checked-in visible_if lists. The schema and test witness were repaired and the focused ui-schema plus published-runtime proofs were rerun green.
- `py -3.14 tools/viewer_host_checkpoint_slice.py commit --session-id global_active_contract --cwd . --checkpoint-id ck:c4b13525 --score 97 --handoff-message "Explaino-all slice 1 landed: explaino_all is now a real canonical Explaino identity, first/default selector wiring and the seven-axis registry are live, focused native/CUDA/runtime proofs are green, and hostile audit repaired the missing explaino_all family-control visibility defect without widening into alias migration or enforcement work." --commit-message "Canonicalize Explaino-all identity and axis registry"` checkpointed the bounded product slice at `e2009ff`.
- Committed-head narrow witnesses stayed green:
  - `artifacts/validation/phase8j_receipt_test_enum_id_utils.log`
  - `artifacts/validation/phase8j_receipt_test_viewer_state_init.log`
  - `artifacts/validation/phase8j_receipt_test_fractal_family_rules.log`
  - `artifacts/validation/phase8j_receipt_test_ui_schema.log`
  - `artifacts/validation/phase8j_receipt_test_fractal_derived_fields.log`
  - `artifacts/validation/phase8j_receipt_test_explaino_zero_axis_equivalence.log`
  - `artifacts/validation/phase8j_receipt_build_vsdevcmd.log`
  - `artifacts/validation/phase8j_receipt_runtime_pytest_explaino_all.log`
- `py -3.14 tools/viewer_host_checkpoint_slice.py write-receipts --session-id global_active_contract --cwd . --validation-summary "<phase8j committed-head proof summary>" ...` wrote both machine receipts for `e2009ff`.
- The first receipted head still carried stale pre-closeout continuity text in `## Current Phase`, `## Phase Checklist`, `## Explicit User Asks`, and the old next-step section; this bounded follow-up updates only the active phase8j plan so the checked-in authority matches the already-proven product head.

## Hostile Audit

- Status: complete
- Required posture: assume explaino_all is still only a label, the shared-axis registry is still fake or partial, selector/default behavior still lies, legacy variant behavior regressed, or the slice silently widened beyond canonical identity plus axis registry until hostile proof disproves each failure mode.

## Audit Passes

- [done] Pass 1 - added and ran REDs that proved explaino_all was still missing as a real enum/schema/default/runtime identity and that the seven family axes still did not live behind one canonical registry on RED HEAD.
- [done] Pass 2 - the first green read found two real defects: the canonical device/runtime helper was host-only, and several checked-in Explaino family controls still hid from explaino_all because their visible_if lists excluded the canonical identity.
- [done] Pass 3 - clean re-read the repaired selector/schema/default/runtime seams, reran the narrow native/runtime witnesses plus the published-runtime proof after the repairs, and found no additional real product defect before the plan-closeout continuity cleanup.

## Audit Findings

- [done] Hostile audit found a real UI truth defect after GREEN: checked-in family-wide Explaino controls still omitted explaino_all from visible_if lists, which would have hidden the canonical identity from existing Explaino controls. Repaired in ui/fractal_binding_surface_v1.ui_schema.json, extended ui_app/tests/test_ui_schema.cpp to witness epsilon and explaino_seed visibility on explaino_all, and reran the focused ui-schema and published-runtime proof green.
- [done] Committed-head stale-plan reread found a real continuity defect: the first receipted phase8j head still advertised open closure chores through pre-closeout `Current Phase`, unchecked `Phase 3`, an `[open]` closeout ask, and a stale next-step section. This follow-up plan cleanup removes that residue without changing the shipped Explaino-all product diff.

## Notes

- Required owner seams before mutation:
  - ui/fractal_binding_surface_v1.ui_schema.json
  - ui_app/src/fractal_types.h
  - ui_app/src/fractal_derived_fields.cpp
  - ui_app/src/schema_binding.cpp
  - ui_app/src/enum_id_utils.h
  - ui_app/src/runtime_reset.cpp
  - ui_app/src/diagnostics_capture.cpp
  - ui_app/src/diagnostics_state_io.cpp
  - ui_app/src/fractal_family_rules.h
  - ui_app/src/viewer_state_init.cpp
  - ui_app/src/safe_mode_schema.cpp
  - ui_app/src/fractal_sample_device.inl
  - ui_app/src/fractal_probe_runner.cpp
  - ui_app/src/runtime_walk_headless.cpp
- Canonical registry authority target for this slice: keep the registry definition in code, use it for explaino_all selector/default/control ownership, and stop letting per-variant one-off UI/default branches define the Explaino-all public surface.
- Bounded slice judgment at kickoff: slice 1 can stay bounded without opening broad alias migration if explaino_all uses only the minimal runtime projection required to behave as the canonical baseline identity while legacy variant ids keep their current compatibility semantics.
- Non-goals: no full alias migration, no preset projection truth rewrite, no enforcement guardrails, no cleanup-only pass, no generic advanced-color core reopen, no historical 234919_563__explaino_inertial archaeology.
- Narrow proof ladder target: start with focused helper rails over selector/default/schema/state seams, then add the smallest published-runtime witness needed to prove explaino_all is runtime-real enough for slice 1.

## Re-entry Note

Closed on this slice contract. If a future session continues Explaino-family canonicalization work, start from the checkpoint/handoff entry for `ck:c4b13525`, the receipts written for the committed head, the launch anchor in `docs/notes/advanced_color_library_foundation_explaino_all_launch_anchor.md`, and the current foundation authority surfaces instead of reopening slice-1 closure chores.

## Action Hostile Review

- Action ID: action-20260514-explaino-all-slice1-red-witnesses
- Suspected Failure Mode: the slice lands only a new selector label or default flip while the real enum/runtime/state authority still belongs to plain explaino plus per-variant one-off gates and default branches.
- Correct Owner/Action: create the real explaino_all identity, wire it through enum/schema/state/runtime seams, centralize the seven axes in one canonical registry, and let the Explaino-all control surface read from that registry without widening into alias migration or enforcement work.
- Proof Surface: checked-in selector/default/helper REDs, enum-id and state-init proof, diagnostics/reset round-trip proof, focused Explaino runtime proof, phased-plan sync, hostile-audit validation, checkpoint commit, validation receipt, contract proof receipt, and the post-closeout stale-plan reread.
- Blocked Action: no full alias migration, no enforcement guardrails, no cleanup-only sweep, no historical archive compatibility reopening, and no unrelated UI/tooling work in this slice.

