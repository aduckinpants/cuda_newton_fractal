# Viewer-Host Backport Feature Matrix

**Date:** 2026-04-05
**Status:** Implementation-ready bounded work plan
**Current branch:** `working/salt-fractal-sweep-viewer` (HEAD decf7e7)
**Donor branch:** `origin/feature/seed-refactor` (commits 3180247, cbfe6a2)
**Divergence point:** `d997f3f` (initial commit from `origin/master`)

## 0) Summary

The donor branch carried real, source-backed features that were never merged
into the current branch. Only the Explaino seed seam was surgically backported.
This doc is the bounded work plan for recovering those features under forward
TDD, without reimporting the donor bugs.

Total work: 7 phases, each independently shippable.

---

## 1) Source Inventory

### 1.1 Current Branch Binding Surface (3 panels, 41 controls)

| Panel     | Controls | Key paths |
|-----------|----------|-----------|
| view      | 15       | center, zoom, rotation, camera, dive, auto_refresh, render_once, reset_view, reset_all, load_state, capture_finding, fractal_type, multibrot_power |
| fractal   | 21       | max_iter, epsilon, nova_alpha, phoenix, poly_kind, poly_c0..c4, coloring_mode_newton, coloring_mode_escape, exposure, explaino_warp/phase/seed/drift/tween/auto_increment/rate |
| render    | 5        | resolution.x/y, block_size, device_id, benchmark |

### 1.2 Reference (Donor) Schema Binding Surface (5 panels, 41 controls)

| Panel     | Controls | Key paths |
|-----------|----------|-----------|
| panel_0 (View) | 10 | fractal_type, center, zoom, rotation, camera, dive, auto_increment_seed, explaino_seed_rate |
| lens      | 2        | lens_enabled, lens_downsample |
| fractal   | 16       | max_iter, epsilon, nova_alpha, phoenix, multibrot_power, poly, explaino_seed, prev/next_seed, explaino_warp |
| color     | 7        | coloring_mode, exposure, color_saturation, color_contrast, tint_r/g/b |
| render    | 6        | resolution, block_size (int combo), device_id, benchmark, capture_diag |

### 1.3 Current C++ Structs

```
ViewState:      center, zoom, rotation, auto_refresh, hp_x/y, log2_zoom,
                fractal_type, explaino_alive, explaino_seed_tween,
                explaino_phase, explaino_seed_drift, auto_increment_seed,
                explaino_seed_rate, camera_behavior, auto_dive, dive_speed

KernelParams:   max_iter, epsilon, nova_alpha, phoenix_p_real/imag,
                poly_kind, poly_coeffs[5], multibrot_power, coloring_mode,
                exposure, explaino_seed, explaino_warp_strength,
                explaino_root_count, explaino_roots[4]

RenderSettings: resolution, block_size, device_id, benchmark

(LensSettings:  DOES NOT EXIST on current branch)
```

### 1.4 Donor C++ Structs (delta from current)

```
KernelParams ADDS: color_saturation{1.15f}, color_contrast{1.10f},
                   color_tint_r{1.0f}, color_tint_g{1.0f}, color_tint_b{1.0f}

LensSettings NEW:  enabled{false}, downsample{2}
```

### 1.5 Donor Renderer Seams (NOT on current branch)

```
__device__ saturate_rgb(uchar4, float)
__device__ contrast_rgb(uchar4, float)
__device__ tint_rgb(uchar4, float, float, float)
outMask parameter on kernel + host RenderFractalCUDA
Per-pixel mask write (inside ? 255 : 0)
Post-iteration grading: tint -> saturate -> contrast pipeline
```

### 1.6 Current Visibility Predicate Model

Current branch uses `fractal.view.fractal_type` with `in` operator:
```
visible_if: { path: "fractal.view.fractal_type", op: "in", value: "newton,nova" }
```

Reference used capability predicates routed through code:
```
visible_if: { path: "fractal.cap.supports_poly_kind", op: "eq", value: "True" }
```

Current branch `EvalVisibleIf` resolves enum, bool, and numeric paths.
It has **no** `fractal.cap.*` provider. The `in` approach is functionally
equivalent and avoids a C++ code seam per predicate. It is the better model.

### 1.7 Current Action Allowlist

| Action path | Current | Donor |
|-------------|---------|-------|
| `fractal.actions.render_once` | YES | YES |
| `fractal.actions.reset_view` | YES | YES |
| `fractal.actions.reset_all` | YES | YES |
| `fractal.actions.load_state` | YES | NO (had export_state/import_state instead) |
| `fractal.actions.capture_finding` | YES | NO |
| `fractal.actions.capture_diagnostic` | NO (backend exists, UI action seam missing) | YES |
| `fractal.actions.next_seed` | NO | YES |
| `fractal.actions.prev_seed` | NO | YES |
| `fractal.actions.export_state` | NO (replaced by load_state) | YES |
| `fractal.actions.import_state` | NO (replaced by load_state) | YES |

### 1.8 Existing Test Guardrails (10 files)

| Test file | Covers |
|-----------|--------|
| test_diagnostics_state_io | State round-trip, optional field defaults, finding JSON resolution |
| test_finding_state_actions | LoadFindingSelectionIntoRuntime, bad metadata |
| test_finding_archive_actions | BuildUniqueFindingIdentity, collision suffixes |
| test_explaino_seed_dynamics | Auto-increment, non-Explaino guard, non-finite guard |
| test_explaino_seed | Seed math |
| test_schema_startup_policy | Startup behavior on binding failure |
| test_fractal_family_rules | IsExplainoFamily, IsEscapeTimeFamily, IsColoringModeAllowed |
| test_fractal_derived_fields | Derived field computation |
| test_view_hp_sync | HP view sync |
| test_sweep_player | Sweep player logic |

---

## 2) Feature Matrix: What to Restore, Keep, or Drop

### RESTORE from donor (not on current branch)

| ID | Feature | Donor source | Touches | Phase |
|----|---------|-------------|---------|-------|
| R1 | **Lens pipeline** — LensSettings struct, lens_enabled/downsample bindings, mask output in renderer, SDF+Mask D3D11 windows | fractal_types.h, main.cpp, fractal_renderer.cu | struct, bind, renderer, schema, D3D11 | 4 |
| R2 | **Color grading** — saturation, contrast, tint_r/g/b in KernelParams + CUDA grading math | fractal_types.h, fractal_renderer.cu, main.cpp | struct, renderer, bind, schema | 3+4 |
| R3 | **Next/prev seed buttons** — action dispatch routing seed +-1 | main.cpp, schema | action allowlist, dispatch, schema | 2 |
| R4 | **capture_diagnostic action** — button -> CaptureDiagnosticsLastBundle (backend already exists) | main.cpp, schema | action allowlist, dispatch, schema | 2 |
| R5 | **Int-valued combo** — combo renderer supports int value_type with stoi on option IDs | main.cpp | combo renderer | 1 |
| R6 | **Color panel** — dedicated panel for coloring_mode, exposure, grading | schema | schema JSON | 5 |
| R7 | **Lens panel** — dedicated panel for lens_enabled, lens_downsample | schema | schema JSON | 5 |

### KEEP from current branch (not on donor)

| ID | Feature | Why keep |
|----|---------|----------|
| K1 | `load_state` action | Replaces old export/import pair; cleaner |
| K2 | `capture_finding` action | New finding workflow; not on donor |
| K3 | `auto_refresh` control | New on current branch; useful |
| K4 | Split `coloring_mode_newton` / `coloring_mode_escape` | Family-safe; prevents invalid mode for family |
| K5 | `explaino_phase`, `explaino_seed_drift`, `explaino_seed_tween` controls | Current-only Explaino controls with `in`-style visibility |
| K6 | `in` operator for `visible_if` predicates | Schema-side family filtering; no C++ cap provider needed |
| K7 | Hardened state IO (GetOptionalBool/Number) | diagnostics_state_io.cpp is current-only; backward compat |
| K8 | HP view state (center_hp_x/y, log2_zoom) | Precision path; donor lacked this |
| K9 | fractal_family_rules.h headless predicates | Shared by renderer + UI + tests |

### DROP from donor (do NOT reimport)

| ID | Feature | Why drop |
|----|---------|----------|
| D1 | `export_state` / `import_state` actions | Replaced by `load_state` + `capture_finding` |
| D2 | Single `coloring_mode` combo without visibility guard | Allows invalid mode/family combos |
| D3 | `fractal.cap.*` C++ predicate provider | `in` operator on fractal_type achieves same result with zero C++ |
| D4 | `explaino_alive` animation tick (dormant) | Was always disabled; removing does not lose functionality |
| D5 | Old monolithic state import/export code | Current diagnostics_state_io.cpp is hardened and versioned |
| D6 | Hardcoded saturate values in per-branch coloring | Donor had `saturate_rgb(..., 1.40f)` in basins; should use only the grading pipeline |

---

## 3) Phase Plan

### Phase 0 — Make Validation Honest

**Goal:** `--validate-ui` must return exit 1 when schema bindings fail.

**What:**
- `schema_startup_policy.cpp`: change `enter_safe_mode = true` to `enter_safe_mode = false`
  when the app was launched with `--validate-ui`
- Or: add a validate-ui-aware policy that crashes instead of entering Safe Mode

**Test:** `test_schema_startup_policy.cpp` — add case: validate-ui + bad binding => exit 1

**Files touched:**
- `ui_app/src/schema_startup_policy.cpp` (8 lines)
- `ui_app/src/schema_startup_policy.h` (add validate-ui flag if needed)
- `ui_app/tests/test_schema_startup_policy.cpp`
- `ui_app/src/main.cpp` (pass validate-ui flag to policy)

**Exit criteria:** `--validate-ui` with a schema containing an unknown binding path
returns nonzero. Existing tests green.

**Risk:** Low. Strictly a policy change. Safe Mode fallback still works for
normal (non-validate) launches.

---

### Phase 1 — Recover Schema-Binding Substrate

**Goal:** Widen the binding infrastructure so the merged schema can pass validation.

**What:**
1. **Int-valued combo support** in `RenderControlFromSchema`:
   - When `c.type == "combo"` and `c.value_type == "int"`, use `BindInt` + `stoi(option.id)` instead of `GetEnumId/SetEnumId`
   - This enables `block_size` as an int combo (reference had it as combo with int options: 64, 128, 256, 512)
2. **New action paths** in `ValidateSchemaBindings` allowlist + dispatch:
   - `fractal.actions.capture_diagnostic`
   - `fractal.actions.next_seed`
   - `fractal.actions.prev_seed`
3. **New float bindings** (grading fields — struct added in Phase 3):
   - `fractal.params.color_saturation`
   - `fractal.params.color_contrast`
   - `fractal.params.color_tint_r`
   - `fractal.params.color_tint_g`
   - `fractal.params.color_tint_b`
4. **New bool/int bindings** (lens fields — struct added in Phase 4):
   - `fractal.lens.enabled`
   - `fractal.lens.downsample`

NOTE: Items 3 and 4 can be deferred until the struct fields exist (Phase 3/4).
If done early, the bindings can point at temporary stub fields or be gated.

**Test:** Validate-ui against a schema with int combo + new actions passes.

**Files touched:**
- `ui_app/src/main.cpp` (combo renderer, validation, dispatch, bind tables)

**Exit criteria:** `--validate-ui` passes with schema containing int combos and
new action paths. Existing tests green.

---

### Phase 2 — Restore Safe Host-Only Controls

**Goal:** Next/prev seed buttons and capture_diagnostic work in the UI.

**What:**
1. **next_seed / prev_seed dispatch:**
   - In action handler: `explaino_seed += 1.0` / `explaino_seed -= 1.0`
   - Guard: only fire when `IsExplainoFamily(view.fractal_type)`
2. **capture_diagnostic dispatch:**
   - Wire `captureDiagnosticAction` flag in the panel render loop
   - Call `CaptureDiagnosticsLastBundle(...)` (already exists in diagnostics_capture.cpp)

**Test:**
- `test_explaino_seed.cpp` or new test: next_seed increments, prev_seed decrements, guarded by family
- Manual: button visible, triggers capture

**Files touched:**
- `ui_app/src/main.cpp` (action dispatch block, ~20 lines)
- Schema JSON (controls added in Phase 5, but dispatch must exist first)

**Exit criteria:** Action handlers exist and pass headless test. Wiring to UI
buttons happens when schema is updated in Phase 5.

---

### Phase 3 — Restore State and Serialization Breadth

**Goal:** Color grading fields in KernelParams, backward-compatible state IO.

**What:**
1. **Add to KernelParams:**
   ```cpp
   float color_saturation{1.15f};
   float color_contrast{1.10f};
   float color_tint_r{1.0f};
   float color_tint_g{1.0f};
   float color_tint_b{1.0f};
   ```
2. **Add BindFloat entries:**
   - `fractal.params.color_saturation` -> `&params->color_saturation`
   - `fractal.params.color_contrast` -> `&params->color_contrast`
   - `fractal.params.color_tint_r` -> `&params->color_tint_r`
   - `fractal.params.color_tint_g` -> `&params->color_tint_g`
   - `fractal.params.color_tint_b` -> `&params->color_tint_b`
3. **State IO:** Add `GetOptionalNumber` for grading fields in state loader.
   Missing fields use defaults. Bump `state_version` to 3.

**Test:**
- `test_diagnostics_state_io.cpp`: round-trip with grading fields, backward compat (v2 JSON loads with defaults)
- Validate-ui passes with color bindings

**Files touched:**
- `ui_app/src/fractal_types.h` (5 lines in KernelParams)
- `ui_app/src/main.cpp` (5 BindFloat + validation entries)
- `ui_app/src/diagnostics_state_io.cpp` (save + load for 5 new fields)
- `ui_app/tests/test_diagnostics_state_io.cpp`

**Exit criteria:** Grading fields exist, bind, serialize, round-trip. Existing tests green.

---

### Phase 4 — Restore Renderer-Backed Seams

**Goal:** CUDA grading math, mask output, LensSettings struct.

#### 4a — CUDA Grading Pipeline

**What:**
1. Add `__device__` helpers: `tint_rgb`, `saturate_rgb`, `contrast_rgb`
2. Apply grading in pixel output path:
   ```
   color = tint_rgb(color, params.color_tint_r, ...)
   color = saturate_rgb(color, params.color_saturation)
   color = contrast_rgb(color, params.color_contrast)
   ```
3. Remove any hardcoded `saturate_rgb(..., 1.40f)` calls from the per-branch coloring paths

**Files touched:**
- `ui_app/src/fractal_renderer.cu` (~50 lines device helpers + grading callsite)

**Exit criteria:** Renders with grading defaults look identical to current output.
Custom grading values produce visually correct results. Existing tests green.

#### 4b — Lens Pipeline (Mask Output)

**What:**
1. **Add LensSettings struct** to fractal_types.h:
   ```cpp
   struct LensSettings {
       bool enabled{false};
       int downsample{2};
   };
   ```
2. **Add `outMask` parameter** to `RenderFractalCUDA`:
   ```cpp
   bool RenderFractalCUDA(..., uint8_t* outMask, ...);
   ```
   When `outMask` is non-null, write per-pixel mask (inside ? 255 : 0).
3. **BindBool/BindInt for lens:**
   - `fractal.lens.enabled` -> `&lens->enabled`
   - `fractal.lens.downsample` -> `&lens->downsample`
4. **D3D11 SDF window + Mask window** — host-side texture plumbing for lens
   preview. This is the most complex piece and may warrant its own sub-phase.

**Files touched:**
- `ui_app/src/fractal_types.h` (LensSettings struct, signature change)
- `ui_app/src/fractal_renderer.cu` (mask write in kernel, host alloc)
- `ui_app/src/main.cpp` (lens bind, D3D11 textures, ImGui windows)

**Exit criteria:** Lens checkbox toggles mask output. SDF window shows the field.
Mask window shows inside/outside. All existing tests green.

**Risk:** Highest phase. D3D11 texture plumbing is nontrivial. Can defer 4b
to a follow-up if 4a lands cleanly.

---

### Phase 5 — Build Final Merged Binding JSON

**Goal:** Single schema with 5 panels, all bindings backed, no regressions.

**Target panel layout:**

| Panel | Controls |
|-------|----------|
| **View** (16) | fractal_type, center_x/y, zoom, rotation, camera_behavior, auto_dive, dive_speed, auto_refresh, render_once, reset_view, reset_all, load_state, capture_finding, auto_increment_seed, explaino_seed_rate |
| **Lens** (2) | lens_enabled, lens_downsample |
| **Fractal** (19) | max_iter, epsilon, nova_alpha, phoenix_p_real/imag, multibrot_power, poly_kind, poly_c0..c4, explaino_seed, explaino_prev_seed, explaino_next_seed, explaino_warp_strength, explaino_phase, explaino_seed_drift, explaino_seed_tween |
| **Color** (8) | coloring_mode_newton, coloring_mode_escape, exposure, color_saturation, color_contrast, tint_r, tint_g, tint_b |
| **Render** (6) | width, height, block_size (int combo), device_id, benchmark, capture_diag |

**Visibility predicate model:** Keep current `in` operator on `fractal.view.fractal_type`.
Do NOT introduce `fractal.cap.*` provider.

**Files touched:**
- `ui/fractal_binding_surface_v1.ui_schema.json` (rewrite)

**Exit criteria:** `--validate-ui` passes. All panels render. Controls visible for
correct families. Existing tests green.

---

### Phase 6 — Validate and Checkpoint

**Goal:** Full regression pass + commit.

**What:**
1. All 10+ test files pass
2. `--validate-ui` passes with merged schema
3. Manual smoke: launch viewer, switch families, check visibility,
   test grading sliders, test next/prev seed, test capture_diagnostic
4. Git commit on `working/salt-fractal-sweep-viewer`

**Exit criteria:** Clean commit with all phases landed.

---

## 4) Dependency Graph

```
Phase 0 (honest validation)
   |
   v
Phase 1 (binding substrate)  <-- enables schema to use new paths
   |
   +---> Phase 2 (seed buttons, capture_diag)  [host-only, no struct changes]
   |
   +---> Phase 3 (grading state + IO)  [struct + serialization]
              |
              v
         Phase 4a (CUDA grading)
              |
              v
         Phase 4b (lens pipeline)  [can be deferred]
   |
   v
Phase 5 (merged schema JSON)  <-- needs all bindings to exist
   |
   v
Phase 6 (validate + checkpoint)
```

Phases 2 and 3 are independent of each other and can run in parallel.
Phase 4a depends on Phase 3 (struct fields must exist).
Phase 4b depends on Phase 4a (renderer signature changes).
Phase 5 depends on all binding-adding phases.

---

## 5) Bugs to Leave Behind (Donor Anti-Patterns)

| # | What | Why it was bad |
|---|------|----------------|
| 1 | `export_state` / `import_state` pair | Replaced by `load_state` + `capture_finding`. Better workflow. |
| 2 | Single `coloring_mode` combo | No family guard. User can select root_basin for mandelbrot. |
| 3 | `fractal.cap.*` C++ predicate provider | 8 predicates requiring C++ code. `in` on fractal_type is zero-code. |
| 4 | Hardcoded `saturate_rgb(..., 1.40f)` in basin coloring | Should go through the uniform grading pipeline only. |
| 5 | Missing optional field handling in state loader | Old branch had no GetOptionalBool/Number. Fatal on missing fields. |
| 6 | `explaino_alive` animation tick (dormant code) | Was always false. Dead code path in the main loop. |
| 7 | No HP view state | Old branch only had float center. Current has double HP path. |
| 8 | No schema version validation | Current branch validates state_version; keep that. |

---

## 6) Slice Size Guidance

| Phase | Estimated scope | Confidence |
|-------|----------------|------------|
| 0     | ~30 lines code + 1 test case | High — policy-only |
| 1     | ~80 lines in main.cpp | High — mechanical |
| 2     | ~30 lines dispatch + test | High — host-only |
| 3     | ~40 lines struct/bind + ~40 lines IO + test | High — no renderer |
| 4a    | ~60 lines CUDA | Medium — need visual verification |
| 4b    | ~200+ lines D3D11 | Lower — complex host plumbing |
| 5     | Schema JSON rewrite | High — bounded by inventory above |
| 6     | Validation pass | High — scripted |

Phases 0-3 + 5 are straightforward bounded work.
Phase 4a is moderate.
Phase 4b is the only genuinely complex piece (D3D11 textures).
