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
- the next core-feature launch authority is no longer the historical archive lane by default; it is the weighted-blend generic Source composition lane until a newer real handoff replaces that startup surface

It is intentionally not a workflow status page. It answers four questions only:

1. What product code is actually landed on this branch right now?
2. What is blocked versus merely deferred?
3. What feature work was only workflow churn and should be ignored for product-state reconstruction?
4. What are the next bounded topic lanes, in priority order, if work resumes one topic area at a time?

---

## 1. Current Branch Truth

- Branch: `feature/advanced-color-pipeline-draft-editor-reframe`
- Last reread checkpoint for this note: `82d666c`
- Live session state must still be rechecked with bootstrap commands before using this note as launch authority

Important continuity facts:

- `93ceeb6` is not a product-feature checkpoint.
- The last product-code checkpoint on this branch is `4ec49c8` (`ck: preserve color pipeline shared row params`).
- Every commit after `4ec49c8` is workflow-only: hooks, plans, contracts, workflow docs, or tests for workflow guards.
- That statement no longer means every newer commit is ignorable for launch flow. Newer commits also contain the current manual slice continuity surfaces and the latest bounded serializer follow-up handoff, which are now the correct launch authority for this topic.

Practical meaning:

- The app state is the product state at `4ec49c8`, plus all earlier advanced-color and manual-capture product commits beneath it.
- The hook and anti-lie work after `4ec49c8` changed the workflow, not the shipped feature behavior.

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

Catalog anchors:

- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:115)
- [test_color_pipeline_core.cpp](/C:/code/cuda_newton_fractal_clone/ui_app/tests/test_color_pipeline_core.cpp:124)

### 2.3 Most recent real feature checkpoints that matter

These are the product checkpoints that still define the branch:

1. `80cc000` / `ck:9a48c0a8`
   - `band_finish` shipped as a real grading row.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:159).

2. `3c1e411` / `ck:679b2bc3`
   - grading stack composition landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:170).

3. `9a0009b` / `ck:2f9efd8f`
   - diagnostics save precision and capture-backed serialization repair landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:172).

4. `f9d26f3` / `ck:44faf037`
   - palette blend stack landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:174).

5. `6170dc4` / `ck:85fa0a84`
   - `root_proximity` backend precision policy landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:156).

6. `3b08179` / `ck:49038a61`
   - forward manual ExplainO capture authority landed for current saves/replays.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:182).

7. `564f724` / `ck:colorpreset1`
   - root-basin preset checkbox pair behavior landed.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:186).

8. `4ec49c8` / `ck:presetpreserve1`
   - row function switching now preserves same-path/type authored values.
   - See [HANDOFF_LOG.md](/C:/code/cuda_newton_fractal_clone/HANDOFF_LOG.md:187).

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

- weighted blend is now the chosen first mixer contract for the next bounded implementation lane
- no generic multi-Source runtime claim exists yet
- work must not start as opportunistic code without the checked-in weighted-blend slice authority

Authority:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md:198)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md:42)

### Remaining defined pipeline/operator work

- `neutral_finish`
- `tone_map_finish`
- `grade.glow`
- basin lane-retention
- `balance_void_grade`

Current meaning:

- these are still inventory or follow-up rows
- they are not part of the shipped foundation closure claim
- each needs its own owner-proof slice before it becomes visible product work

Authority:

- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md:25)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md:5)

---

## 5. Workflow Churn That Should Not Be Counted As Feature Progress

Ignore these when reconstructing product status:

- anti-lie guard work
- crash-recovery hook work
- clean-baseline and prompt-hook fixes
- capstone/task-complete workflow enforcement

Why:

- none of that changed runtime advanced-color behavior
- none of that changed the shipped bounded advanced-color spine
- all of it happened after the last product-code checkpoint

The cleanest seam is:

- product code stops at `4ec49c8`
- workflow-only churn starts immediately after `4ec49c8`

If a future session asks “what changed in the app after preset preservation?”, the answer is: nothing product-facing landed after that point on this branch.

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
- deferred rows and generic Source composition are not shipped yet

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

- active sprint-core lanes are still unfinished
- deferred Source and Grading work remain explicitly outside the current closure claim
- historical archive compatibility is tracked separately rather than as the default next lane

---

## 7. Prioritized Restart TODOs

This is the one-topic-at-a-time work list. Each lane is intentionally bounded so a single session can focus on one topic area without dragging three others behind it.

### Priority 0 - Generic Source composition via weighted blend

Goal:

- ship the first real generic multi-Source composition model for the reusable pipeline core

Why this is first now:

- this is the next missing core composition seam
- the user explicitly reprioritized the sprint back to core feature correctness over historical back-compat
- this is a prerequisite for finishing the general composition pattern before later Balance/Void and ExplainO follow-ons

Owner surfaces:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h)
- [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h)
- [fractal_types.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/fractal_types.h)
- [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h)

Bounded TODOs:

1. Lock weighted blend as the first generic Source mixer semantics.
2. Add a real Source stack owner analogous to Shape / Palette / Grading stacks.
3. Keep `root_index` out of the generic Source stack and leave root-basin pairing separate.
4. Add live bridge, runtime math, diagnostics persistence, archive persistence, and reset/default proof for multi-Source rows.
5. Prove the lane through runtime-backed tests, not only draft/editor behavior.

Proof to require before calling it done:

- weighted multi-Source runtime witness
- live import/apply proof
- diagnostics and archive persistence proof
- reset/default proof
- explicit proof that `root_index` remains pair-only

Stop point:

- generic Source composition is runtime-real for the bounded shipped Source set, or the slice stops at a truthful blocked owner seam without claiming it shipped

Start from:

- [gpt54_vscode_source_composition_weighted_blend_startup_prompt.md](/C:/code/cuda_newton_fractal_clone/docs/notes/gpt54_vscode_source_composition_weighted_blend_startup_prompt.md)

### Priority 1 - Remaining defined pipeline/operator sets

Goal:

- finish the remaining bounded pipeline/operator work one owner-proof slice at a time

Candidate order:

1. basin lane-retention
2. `neutral_finish`
3. `tone_map_finish`
4. `grade.glow`

Owner surfaces:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [advanced_color_library_foundation_CLOSURE_MATRIX.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md)
- [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h)
- [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h)
- [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h)

Proof to require before calling it done:

- one operator/row per slice
- real runtime-backed controls and math
- persistence/reset/default proof
- runtime publish/probe proof

Start from:

- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)

### Priority 2 - Balance/Void grading operator

Goal:

- ship `balance_void_grade` as a real reusable grading operator after the generic composition and remaining defined pipeline/operator work are in place

Owner surfaces:

- [advanced_color_library_foundation_oracle_and_inventory.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_oracle_and_inventory.md)
- [advanced_color_library_foundation_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_PHASED_PLAN.md)
- [color_pipeline_core.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_core.h)
- [escape_time_coloring.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/escape_time_coloring.h)
- [color_pipeline_window.h](/C:/code/cuda_newton_fractal_clone/ui_app/src/color_pipeline_window.h)

Bounded TODOs:

1. Use the checked-in parameter pack direction:
   - `balance_void`
   - `chroma_tension`
   - `accent_bias`
2. Keep it generic and reusable across families.
3. Do not fuse it with ExplainO family geometry work.

Stop point:

- one owner-proof grading slice closes truthfully, or the lane stops blocked without pretending the operator shipped

### Priority 3 - ExplainO-BalanceVoid / ExplainO-all follow-on

Goal:

- move from the reusable pipeline core into the later ExplainO family expansion only after the generic composition and operator lanes above are real

Current meaning:

- this is a later family track, not the next core pipeline slice
- it should not hitch a ride on Source composition or Balance/Void grading implementation

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

1. generic Source composition via weighted blend
2. one specific remaining pipeline/operator lane
3. Balance/Void grading
4. ExplainO-BalanceVoid / ExplainO-all follow-on
5. foundation closure decision
6. historical archive compatibility / archive UX
7. polish on already-shipped behavior

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
3. [gpt54_vscode_source_composition_weighted_blend_startup_prompt.md](/C:/code/cuda_newton_fractal_clone/docs/notes/gpt54_vscode_source_composition_weighted_blend_startup_prompt.md)

---

## 10. Immediate Next Recommendation

If you want the fastest path back to truthful feature velocity:

1. Open one bounded slice on generic Source composition via weighted blend.
2. Then land the remaining defined pipeline/operator lanes one bounded owner-proof slice at a time.
3. Only after that move on to Balance/Void and the later ExplainO family track.

That is the shortest path back to truthful core feature velocity instead of reopening historical compatibility work by inertia.
