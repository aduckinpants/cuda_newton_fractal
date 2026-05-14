# Advanced Color Feature Restart Inventory

This note is the restart surface for the current advanced-color and manual-capture feature state on `feature/advanced-color-pipeline-draft-editor-reframe`.

## Launch Authority Rule

This note is not the default launch authority once a newer slice-specific handoff exists.

Use this note only to recover broad product context or to find the first bounded lane after major continuity loss.

When the repo already has a newer real handoff for the same topic, that newer handoff plus the active slice plan become the launch authority instead.

Current live examples:

- the generic first-lane startup prompt and this restart inventory are no longer the right launch surface for manual ExplainO work
- the latest real launch authority is the current manual slice plan plus the latest relevant `HANDOFF_LOG.md` closeout for that slice
- today that means `docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md` plus `ck:20260512-serializer-rail`, not a reset back to the original `Priority 0` archaeology prompt
- the next core-feature launch authority is no longer the historical archive lane by default; weighted blend, basin-default, `neutral_finish`, the bounded `tone_map_finish` owner-proof row, `grade.glow`, and the bounded `balance_void_grade` owner-proof row now define the shipped generic grading boundary, and the later enabled-toggle preserve-disabled-rows regression closeout keeps that boundary truthful without opening a new feature row, so the next core decision is explicit foundation closure or a separate later family-track lane

It is intentionally not a workflow status page. It answers four questions only:

1. What product code is actually landed on this branch right now?
2. What is blocked versus merely deferred?
3. What feature work was only workflow churn and should be ignored for product-state reconstruction?
4. What are the next bounded topic lanes, in priority order, if work resumes one topic area at a time?

---

## 1. Current Branch Truth

- Branch: `feature/advanced-color-pipeline-draft-editor-reframe`
- Last reread checkpoint for this note: active `balance_void_grade` owner-proof slice worktree on 2026-05-14
- Live session state must still be rechecked with bootstrap commands before using this note as launch authority

Important continuity facts:

- `93ceeb6` is not a product-feature checkpoint.
- `4ec49c8` is no longer the last product-code checkpoint on this branch.
- The later product checkpoints that now matter are `5e93e48` (`ck:5972173a` weighted blend), `e2e14df` (`ck:47bd4450` basin-default lane retention), `7d7f779` (`ck:a0ce2d03` neutral_finish owner proof), `a59e657` (`ck:ae3b50a8` tone_map_finish owner proof), `5273e7b` (`ck:2f8a4f58` grade.glow owner proof), and `0e35f7a` (`ck:a72faa5a` enabled-toggle preserve-disabled-rows regression).
- The continuity/tooling-only follow-up heads around those product checkpoints include `ab83e0a` and `a087faf`; `5273e7b` and `0e35f7a` are later product-facing closeouts, not more workflow churn.

Practical meaning:

- The app state now includes the closed `grade.glow` owner-proof checkpoint at `5273e7b` (`ck:2f8a4f58`), the later enabled-toggle preserve-disabled-rows regression fix at `0e35f7a` (`ck:a72faa5a`), and the active `balance_void_grade` owner-proof slice that widens the shipped generic Grading boundary to include the reusable Balance/Void operator.
- The neutral-finish continuity closeout and phase8c proof-ladder work still matter for restart/proof routing, but `grade.glow`, the enabled-toggle preservation fix, and `balance_void_grade` are product-state surfaces rather than active continuity chores.

---

## 2. Product Work That Is Actually Landed

This section is the real shipped state, not the aspirational inventory.

### 2.1 Shipped advanced-color runtime spine

The checked-in foundation authority says the currently shipped spine is:

- bounded Shape stacks
- bounded root-basin Source/Palette pairs
- ordered Grading stacks
- explicit Palette RGB blend stacks

That statement is still current in:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md:5)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md:3)

The live code anchors are:

- stack counts and persistent runtime storage in [fractal_types.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/fractal_types.h:328)
- grading stack execution in [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h:146)
- shape stack execution in [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h:473)
- palette stack execution in [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h:844)
- editor import/apply synchronization in [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:1741), [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:1995), and [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:2069)

### 2.2 Shipped catalog rows by lane

#### Source

Shipped single-row Source:

- `smooth_escape_ramp`
- `phase_orbit`
- `banded_signal`
- `escape_magnitude`
- `orbit_stripe`

Bounded Source:

- `root_proximity`

Bounded pair-only Source:

- `root_index`

Catalog and bridge anchors:

- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:101)
- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:118)

#### Shape

Shipped ordered Shape stack:

- `identity`
- `offset_scale`
- `repeat`
- `posterize`
- `mirror_repeat`
- `bias_gain_curve`
- `smooth_window`

Catalog anchors:

- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:108)

#### Palette

Shipped Palette RGB stack:

- `heatmap`
- `phase_wheel_palette`
- `banded_heatmap`
- `explaino_cmap`

Bounded pair-only Palette:

- `root_classic_palette`
- `joy_root_palette`

Catalog anchors:

- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:111)
- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:122)

#### Grading

Shipped ordered Grading stack:

- `contrast_lift`
- `phase_finish`
- `band_finish`
- `neutral_finish`
- `tone_map_finish`
- `grade.glow`
- `balance_void_grade`

Bounded pair-only Grading:

- `basin_default`

Catalog anchors:

- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:115)
- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:124)

### 2.3 Most recent real feature checkpoints that matter

These are the product checkpoints that still define the branch:

- `5273e7b` / `ck:2f8a4f58` closed `grade.glow` as the next real runtime-backed Grading row.
- `0e35f7a` / `ck:a72faa5a` later closed the preserve-disabled-rows regression without opening a new feature row and is the latest advanced-color product-facing checkpoint on this branch.

1. `7d7f779` / `ck:a0ce2d03`
   - `neutral_finish` shipped as a real runtime-backed Grading row.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

2. `e2e14df` / `ck:47bd4450`
   - basin-default grading lane retention closed as a shipped bounded Grading owner path for root-basin tuples.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

3. `5e93e48` / `ck:5972173a`
   - generic Source composition via weighted blend landed as a real bounded Source-stack feature.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

4. `80cc000` / `ck:9a48c0a8`
   - `band_finish` shipped as a real grading row.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

5. `3c1e411` / `ck:679b2bc3`
   - grading stack composition landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

6. `9a0009b` / `ck:2f9efd8f`
   - diagnostics save precision and capture-backed serialization repair landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

7. `f9d26f3` / `ck:44faf037`
   - palette blend stack landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

8. `6170dc4` / `ck:85fa0a84`
   - `root_proximity` backend precision policy landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

9. `3b08179` / `ck:49038a61`
   - forward manual ExplainO capture authority landed for current saves/replays.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

10. `564f724` / `ck:colorpreset1`
   - root-basin preset checkbox pair behavior landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

11. `4ec49c8` / `ck:presetpreserve1`
   - row function switching now preserves same-path/type authored values.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md).

### 2.4 Feature behaviors that have explicit proof anchors

These are not inferred; they are locked by tests or code:

- root-basin pair bridging and tuple co-switch behavior
  - [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h:984)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:1550)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:1667)

- palette blend parameters and stack persistence
  - [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h:172)
  - [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:1360)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:2177)

- grading tuple import/apply through `phase_finish` and `band_finish`
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:1795)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:1823)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:2296)

- advanced-color window recovery from invalid live state without silent mutation on open
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:2468)
  - [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp:2498)

- preset checkbox pair safety and bounded root-basin pair enablement
  - [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:1551)
  - [test_color_pipeline_window.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_window.cpp:291)

- row-function authored value preservation across compatible switches
  - [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h:696)
  - [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h:321)

---

## 3. Historical Compatibility Issue

The old manual archive mismatch is still real, but it is no longer the active sprint-core blocker.

### Historical issue - manual archive `234919_563__explaino_inertial`

Current truthful status:

- current capture-diagnostic and capture-finding replays are self-consistent
- the historical archived frame is still not reproducible from the saved artifacts
- this is separate historical compatibility work, not the default next sprint lane in a moving beta surface
- the latest bounded follow-up on this same topic is not “restart archaeology from zero”; it is the serializer-owner fast-rail follow-up checkpointed at `ck:20260512-serializer-rail`

Authority surfaces:

- [manual_explaino_inertial_reload_repair_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [diagnostics_state_io.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/src/diagnostics_state_io.cpp)
- [diagnostics_capture.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/src/diagnostics_capture.cpp)
- [tests/test_fractal_runtime_manual_capture_repro.py](/C:/code/cuda_newton_fractal_clone/tests/test_fractal_runtime_manual_capture_repro.py)

What is still not true:

- the old archive has not been visually recovered
- the old archive is not a trustworthy replay witness
- this is not a “maybe fixed” state

---

## 4. What Is Deferred, Not Shipped Yet

These are real planned/spec'd surfaces, but they are explicitly not shipped today.

### Generic Source composition

Current meaning:

- weighted blend already shipped at `5e93e48` / `ck:5972173a`
- bounded generic multi-Source runtime/bridge/persistence/reset truth now exists for the shipped non-pair Source set
- this is no longer the next unshipped feature-row resume surface

Authority:

- [advanced_color_library_foundation_phase8_source_weighted_blend_stack_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_phase8_source_weighted_blend_stack_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md)

### Remaining later family-track or closure work

- explicit foundation closure
- `ExplainO-BalanceVoid`
- `Explaino-all`

Current meaning:

- `balance_void_grade` now belongs to the shipped generic grading boundary alongside `contrast_lift`, `phase_finish`, `band_finish`, `basin_default`, `neutral_finish`, `tone_map_finish`, and `grade.glow`
- no later session should relist `balance_void_grade` as deferred inventory or treat it as a hidden blocker for historical archive compatibility
- the closed phase8c command ladder is still the cheapest truthful starting proof surface for any later family-track or regression lane

Authority:

- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [advanced_color_library_foundation_phase8c_test_lane_acceleration_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_phase8c_test_lane_acceleration_PHASED_PLAN.md)

---

## 5. Workflow Churn That Should Not Be Counted As Feature Progress

Ignore these when reconstructing product status:

- anti-lie guard work
- crash-recovery hook work
- clean-baseline and prompt-hook fixes
- capstone/task-complete workflow enforcement

Why:

- none of that changes product state by itself
- some later commits did land real product behavior again, so workflow-only heads must be separated from feature checkpoints instead of collapsed into one story
- phase8c specifically changes proof cost, not the shipped advanced-color row set

The cleanest seam is:

- product feature checkpoints after preset preservation include `5e93e48`, `e2e14df`, `7d7f779`, `a59e657`, `5273e7b`, and `0e35f7a`
- continuity/tooling-only follow-ups around them include `ab83e0a` and `a087faf`

If a future session asks “what changed in the app after preset preservation?”, the answer is: weighted blend Source composition, basin-default lane retention, neutral_finish, tone_map_finish, `grade.glow`, `balance_void_grade`, and the later enabled-toggle preserve-disabled-rows regression fix all landed; later hook/tooling churn still should not be mistaken for additional feature rows.

---

## 6. Status Of The Bigger Product Asks

These were broader asks in the foundation planning thread. This is their current truth status.

### Full initial reusable library per category

Status:

- partially satisfied and still being completed inside the current bounded foundation

What is true:

- the shipped Source, Shape, Palette, and Grading rows listed in the closure matrix are real
- they are descriptor-backed and runtime-backed within the currently proven boundaries

What is not true:

- the broader inventory is not fully shipped just because it appears in planning docs
- later family-track work such as `ExplainO-BalanceVoid` or `Explaino-all` is not shipped just because it remains in planning inventory

### Stronger module boundaries and reusable core

Status:

- materially advanced, not “done forever”

What is true:

- `color_pipeline_window.h` is no longer the only authority
- catalog, ids, row construction, tuple bridging, and parameter preservation have real core ownership in [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h)

What is not true:

- this has not been turned into a separate DLL/static library product
- “reusable later” is true as an architecture direction, not as a completed packaging milestone

### Runtime-real controls instead of fake dropdowns

Status:

- true within the shipped boundary, false outside it

What is true:

- shipped rows have runtime-backed controls, import/apply behavior, persistence, and reset/default behavior

What is not true:

- deferred rows are not runtime-real just because they appear in the catalog family planning

### Foundation closure

Status:

- not currently claimed

Why:

- the generic core grading boundary is now shipped through `balance_void_grade`, but foundation closure itself is still an explicit decision
- later family-track work remains outside the current closure claim until a later slice resolves it
- historical archive compatibility is tracked separately rather than as the default next lane

---

## 7. Prioritized Restart TODOs

This is the one-topic-at-a-time work list. Each lane is intentionally bounded so a single session can focus on one topic area without dragging three others behind it.

### Priority 0 - Post-`balance_void_grade` closure or family-track decision

Goal:

- choose the next bounded post-`balance_void_grade` lane truthfully instead of reopening already-shipped core grading rows by inertia

Why this is first now:

- `balance_void_grade` now closes the remaining generic Grading/operator row in this restart surface
- foundation closure is still a separate explicit decision, not an implied side effect of landing `balance_void_grade`
- later family-track work like `ExplainO-BalanceVoid` / `Explaino-all` must stay separate from the shipped generic core

Owner surfaces:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md)
- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)

Bounded TODOs:

1. Choose exactly one of explicit foundation closure or a separate later family-track lane.
2. Keep the chosen lane single-topic and bounded.
3. Preserve shipped behavior for `contrast_lift`, `phase_finish`, `band_finish`, `basin_default`, `neutral_finish`, `tone_map_finish`, `grade.glow`, and `balance_void_grade`.
4. Do not reopen weighted blend, `grade.glow`, `balance_void_grade`, manual archive, or workflow/tooling churn by inertia.
5. Require runtime-backed proof, not editor-only behavior, before promoting any later family-track work or closure claim.

Proof to require before calling it done:

- one closure or family-track lane per slice
- runtime-visible proof for any newly promoted family track
- explicit proof that historical archive compatibility stays out of scope unless reclassified
- explicit proof that shipped generic core rows remain shipped and regression-free

Stop point:

- closure becomes explicit, or one separate later family-track lane opens truthfully without pretending more generic core grading debt exists

Start from:

- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md)
- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)

### Priority 1 - Later defined family tracks

Goal:

- open later post-foundation family work only if it is explicitly chosen and kept separate from the shipped generic core

Candidate order:

1. `ExplainO-BalanceVoid`
2. `Explaino-all`

Owner surfaces:

- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)

Proof to require before calling it done:

- one family track per slice
- runtime-visible proof for any newly promoted family behavior
- explicit proof that the shipped generic core rows stay stable
- explicit proof that historical archive compatibility remains separate unless reclassified

Start from:

- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)

### Priority 2 - Shipped `balance_void_grade` regression boundary

Goal:

- keep `balance_void_grade` treated as a shipped generic grading operator, not as deferred inventory or stealth family-track work

Owner surfaces:

- [advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)

Bounded TODOs:

1. Reopen only exact regression seams if machine proof says the shipped row broke.
2. Keep historical archive compatibility separate.
3. Do not fuse regression repair with ExplainO-BalanceVoid / ExplainO-all expansion.

Stop point:

- the shipped row stays in the generic grading boundary, or a bounded regression slice names the exact broken seam
### Priority 3 - ExplainO-BalanceVoid / ExplainO-all follow-on

Goal:

- move from the reusable pipeline core into the later ExplainO family expansion only after the shipped generic core boundary and any explicit foundation-closure decision are truthful

Current meaning:

- this is a later family track, not the next core pipeline slice
- it should not hitch a ride on Source composition or `balance_void_grade` regression repair

Owner surfaces:

- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)

Stop point:

- a separate family-track slice opens truthfully with neutral-default axis rules, or it stays deferred without code churn

### Priority 4 - Foundation closure decision after core lanes

Goal:

- close or explicitly defer the advanced-color foundation truthfully after the chosen core lanes are actually shipped

Owner surfaces:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_closure_control_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_closure_control_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)

Decision fork:

1. Accept the current bounded beta foundation after the new core lanes.
2. Promote one additional deferred lane if the user explicitly wants it before closure.
3. Keep historical archive compatibility separate unless the user later reclassifies it again.

Stop point:

- closure is either explicit and receipted or still deferred, not ambiguous

### Priority 5 - Historical archive compatibility / archive UX

Goal:

- preserve an honest path for the unreplayable historical archive without treating it as the active sprint-core lane

This lane should only open after the active core pipeline lanes above are no longer the priority.

Possible bounded outcomes:

1. viewer/archive UI explicitly marks “image preserved, runtime replay unavailable”
2. archive surfaces show preserved PNG and notes without implying deterministic replay
3. capture docs stop treating that archive as a silent replay candidate

Owner surfaces:

- archive/viewer UI code
- capture/archive docs
- manual inertial repair plan

Stop point:

- the product has an honest user-facing story for unreplayable historical captures

### Priority 6 - Product polish on already-shipped advanced-color behavior

Goal:

- tighten UX around shipped behavior without broadening scope

Current known shipped polish surfaces:

- preset pair enable/disable behavior
- same-path/type authored-value preservation
- invalid live-state recovery and open-window non-mutation behavior
- row-id stability and tuple co-switch affordances

Use this lane only if the user wants polish on already-shipped behavior, not new capability.

Owner surfaces:

- [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h)
- [test_color_pipeline_window.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_window.cpp)
- [test_schema_binding.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_schema_binding.cpp)

Stop point:

- concrete UX bug fixed with no scope broadening into new row/category work

---

## 8. Recommended Single-Session Operating Rule

If you resume from this note, do exactly one of these at a time:

1. foundation closure decision
2. one separate later family-track lane
3. `balance_void_grade` regression repair if a real seam reopens
4. historical archive compatibility / archive UX
5. polish on already-shipped behavior

Do not mix:

- Source composition plus unrelated Grading or family-track work
- active core feature work plus workflow/hook hardening
- historical archive compatibility plus new pipeline/operator implementation

If a session starts drifting, stop and restate which one topic lane is active.

---

## 9. What To Ignore On Re-entry

On future re-entry, do not use these as the primary feature resume surfaces:

- anti-lie plans
- crash-recovery plans
- hook validation receipts
- workflow guard handoff chains

Use them only if the question is specifically about workflow behavior.

For product-state re-entry, start with:

1. [advanced_color_feature_restart_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_feature_restart_inventory.md)
2. [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
3. [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)

---

## 10. Immediate Next Recommendation

If you want the fastest path back to truthful feature velocity:

1. Use the latest advanced-color handoff chain through `ck:2f8a4f58`, `ck:a72faa5a`, and the `balance_void_grade` owner-proof slice instead of restarting from older pre-`grade.glow` prompts.
2. Reuse the closed phase8c command ladder for whichever next family-track, regression, or closure slice is chosen.
3. Choose explicitly between foundation closure or a separate later family-track lane from the matrix.
4. Keep historical archive compatibility separate unless it is explicitly reprioritized.

That is the shortest path back to truthful core feature velocity instead of reopening already closed weighted-blend/basin-default/neutral-finish/tone-map/`grade.glow` work or historical compatibility by inertia.
