# Explaino Experimental Build Plan — Ripple / Splice / Vortex / Tension

Date: 2026-04-09
Branch: `feature/explaino-joy`
Reference: `docs/EXPLAINO_EXPERIMENTAL_FAMILY_REFERENCE.md`

## Protocol

Each turn follows the same forward-TDD slice:

1. **Test first** — write `test_explaino_<name>_continuity.cpp` with 3-4 tests
2. **Wire enum + param** — add to fractal_types.h, all 6 string/enum files,
   family rules, derived fields
3. **GPU kernel** — write f32+f64 paths in fractal_renderer.cu
4. **Host probe runner** — add path in fractal_probe_runner.cpp + supported list
5. **UI schema** — dropdown entry, slider, visible_if lists, param_anim_target
6. **Update existing tests** — probe coverage, sample pipeline, CLI args
7. **Build tests** — `ui_app/build_tests_vsdevcmd.cmd` (exit 0, 0 failures)
8. **Build runtime** — `ui_app/build_vsdevcmd.cmd` (exit 0)
9. **Commit** — descriptive message + checkpoint

Each turn produces one working, tested, committed fractal.

---

## Turn 1: explaino_ripple

### Continuity Tests (test_explaino_ripple_continuity.cpp)
- [ ] amplitude=0 matches pure Newton (max delta < 1e-4)
- [ ] amplitude continuity (0.0 vs 0.01 delta < threshold)
- [ ] amplitude=0.5 stability (all z remain finite after 200 iterations)
- [ ] perpendicularity verification (kick direction dot step direction ~ 0)

### Wiring Checklist
- [ ] `explaino_ripple = 33` in FractalType enum
- [ ] `float ripple_amplitude{0.0f}` in KernelParams
- [ ] fractal_family_rules.h: IsExplainoFamily, SupportsBasinColoring
- [ ] fractal_derived_fields.cpp: root count, IsExplainoPreset, DefaultMaxIter(500),
      ApplyExplainoPresetDefaults(ripple_amplitude = 0.15f)
- [ ] schema_binding.cpp: FractalTypeToString, StringToFractalType, float binding
- [ ] cli_args.cpp, diagnostics_capture.cpp, diagnostics_state_io.cpp,
      finding_archive_actions.cpp, safe_mode_schema.cpp: string/enum mapping
- [ ] fractal_renderer.cu: GPU kernel (f32 + f64)
- [ ] fractal_probe_runner.cpp: host path + supported list
- [ ] UI schema JSON: dropdown, slider, visible_if, param_anim_target
- [ ] test_cli_args.cpp, test_fractal_probe_coverage.cpp,
      test_fractal_sample_pipeline.cpp, test_fractal_probe.cpp: update

### Exit Criteria
- `build_tests_vsdevcmd.cmd` exit 0, 0 test failures
- `build_vsdevcmd.cmd` exit 0
- Git commit with descriptive message

---

## Turn 2: explaino_splice

### Continuity Tests (test_explaino_splice_continuity.cpp)
- [ ] offset=0 matches pure Newton (same polynomial, same result)
- [ ] offset continuity (0.0 vs 0.01 delta < threshold)
- [ ] offset=1.0 stability
- [ ] alternation verification (even vs odd iterations use different polynomials)

### Wiring Checklist
- [ ] `explaino_splice = 34` in FractalType enum
- [ ] `float splice_offset{0.0f}` in KernelParams
- [ ] All 6 string/enum files, family rules, derived fields
- [ ] GPU kernel: needs two polynomial evaluations per iteration (P_A and P_B)
- [ ] Host probe runner + supported list
- [ ] UI schema: dropdown, slider, visible_if, param_anim_target
- [ ] Update test files

### Implementation Note
Splice needs a second seeded polynomial per iteration. The approach:
- Compute roots_A from `explaino_seed` (existing)
- Compute roots_B from `explaino_seed + splice_offset` (using same seed curve)
- Build both coefficient sets at warp-start time
- Alternate which one drives each iteration step

### Exit Criteria
Same as Turn 1.

---

## Turn 3: explaino_vortex

### Continuity Tests (test_explaino_vortex_continuity.cpp)
- [ ] strength=0 matches pure Newton
- [ ] strength continuity (0.0 vs 0.01 delta < threshold)
- [ ] strength=1.0 stability
- [ ] rotation verification (step angle change proportional to strength * arg(step))

### Wiring Checklist
- [ ] `explaino_vortex = 35` in FractalType enum
- [ ] `float vortex_strength{0.0f}` in KernelParams
- [ ] All 6 string/enum files, family rules, derived fields
- [ ] GPU kernel: atan2 for arg(step), cos/sin for rotation
- [ ] Host probe runner + supported list
- [ ] UI schema: dropdown, slider, visible_if, param_anim_target
- [ ] Update test files

### Implementation Note
The rotation `exp(i * V * theta)` where `theta = atan2(step.y, step.x)`:
```
cos_r = cos(V * theta)
sin_r = sin(V * theta)
rotated.x = step.x * cos_r - step.y * sin_r
rotated.y = step.x * sin_r + step.y * cos_r
```
Uses only P and P'. Cheapest kernel of the four.

### Exit Criteria
Same as Turn 1.

---

## Turn 4: explaino_tension

### Continuity Tests (test_explaino_tension_continuity.cpp)
- [ ] strength=0 matches pure Newton
- [ ] strength continuity (0.0 vs 0.001 delta < threshold)
- [ ] strength=0.1 stability
- [ ] pull direction verification (pull vector points toward second-closest root)

### Wiring Checklist
- [ ] `explaino_tension = 36` in FractalType enum
- [ ] `float tension_strength{0.0f}` in KernelParams
- [ ] All 6 string/enum files, family rules, derived fields
- [ ] GPU kernel: needs nearest + second-nearest root lookup per iteration
- [ ] Host probe runner + supported list
- [ ] UI schema: dropdown, slider, visible_if, param_anim_target
- [ ] Update test files

### Implementation Note
Tension needs per-iteration root distance computation. The roots are already
in `params.explaino_roots[]` / `params.explaino_root_count`. The kernel needs:
1. Find closest root (already done for basin coloring — reuse pattern)
2. Find second-closest root
3. Compute inverse-distance gravity: `T * (r_far - z) / |r_far - z|^2`

This is the most expensive kernel of the four (extra distance loop per iter)
but the root count is small (typically 3-4).

### Exit Criteria
Same as Turn 1.

---

## After All 4 Turns

- [ ] Full `build_tests_vsdevcmd.cmd` green
- [ ] Full `build_vsdevcmd.cmd` green
- [ ] All 7 experimental explaino variants (joy, fold, bell, ripple, splice,
      vortex, tension) selectable in dropdown
- [ ] Update `EXPLAINO_EXPERIMENTAL_FAMILY_REFERENCE.md` — move proposed
      variants to shipped section
- [ ] Final commit with summary
