# Explaino Magnet Variant Research And Low-Hanging Fruit

Date: 2026-05-19
Repo/head at research time: `master` / `04359a9`
Status: deferred research note, not an implementation plan.

## Explaino Magnet Feasibility

`explaino_magnet` is feasible, but it is not the next low-effort slice.

The safe design is to treat Magnet as an escape-time family with Explaino-style axis modulation. It should not be pushed through the legacy `IsExplainoFamily(...)` path as if it were another basin/root Newton-style Explaino carrier.

Current relevant seams:

- `ui_app/src/escape_time_direct_formulas.h` owns the reusable direct escape-time formula seam used by Magnet.
- `ui_app/src/fractal_sample_device.inl` dispatches Magnet through the standard direct escape-time branch and applies Magnet-specific convergence/residual rules.
- `ui_app/src/fractal_family_rules.h` owns the canonical seven-axis `kExplainoAxisRegistry` and the legacy Explaino selector/runtime routing rules.
- `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, and `ui_app/src/safe_mode_schema.cpp` expose Magnet's current controls.
- `tests/test_fractal_runtime_magnet.py` is the current no-mouse published-runtime proof for Magnet control effect.

Why this is medium effort:

- The current Explaino family flag carries basin/root, seed, sidecar, runtime-walk, and selector semantics beyond simple axis modulation.
- Magnet is currently escape-time, not basin-coloring, and should remain on the direct escape-time path for FPS.
- A truthful `explaino_magnet` should use all seven registry axes: `ripple_amplitude`, `splice_offset`, `vortex_strength`, `tension_strength`, `balance_void`, `symmetry_tension`, and `field_curvature`.
- The implementation needs registry-generated sensitivity tests so every axis moves output and no slider becomes dead UI.
- The existing Magnet controls should remain authoritative: `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, and `magnet_bailout`.

Best future slice shape:

1. Add `FractalType::explaino_magnet` as a real escape-time selector.
2. Add schema/binding/safe-mode/diagnostics/catalog visibility for it.
3. Add a small `ApplyExplainoMagnetAxisModulation(...)` seam near the Magnet direct formula path.
4. Keep `magnet` unchanged and prove it.
5. Prove every registry axis affects `explaino_magnet` output.
6. Publish runtime and run no-mouse proofs for at least one Magnet control and one Explaino axis control.

Estimated implementation level: 1-2 focused days if bounded well; 3-5 days if it accidentally inherits the full legacy Explaino basin/sidecar machinery.

## Low-Hanging Fruit Ranking

These are cheaper and safer than `explaino_magnet` right now.

### 1. Magnet Example/State Pack

Effort: low, about half a day.

Create a small set of checked-in or generated Magnet example states/captures that exercise visually distinct parameter zones:

- default smooth Magnet
- high relaxation
- low relaxation
- nonzero seed real/imag
- high bailout edge case

Why it is useful: it improves manual exploration and gives future tests concrete known-good states without touching core math.

Proof: headless `--capture-finding` or `--capture-diagnostic` on each state, plus hashes/metadata saved as artifacts.

### 2. Magnet Control Proof Expansion

Effort: low, half day to one day.

Current proof strongly covers `magnet_relaxation`. Add focused native/runtime witnesses for the other Magnet controls:

- `magnet_seed_real`
- `magnet_seed_imag`
- `magnet_bailout`

Why it is useful: it closes the same failure class that hurt Explaino controls, but on a much smaller and currently healthy family lane.

Proof: no-mouse `--ui-automation-set-control-value` runtime tests and native renderer pixel-difference checks.

### 3. Capture Stats Follow-Up Polish

Effort: low, a few hours.

Now that capture stats export `last_iters_sum` and `last_pixel_count`, add a small reader/analyzer or UI wording pass so humans do not misread stale/zero timing again.

Candidate outcomes:

- state analyzer labels `last_iters_avg` as average iteration count and shows raw sum/pixel count when present
- UI status panel distinguishes benchmark timing from unmeasured live timing
- runtime capture proof asserts 4k finding captures carry nonzero `last_render_ms`

Why it is useful: this prevents another stats interpretation failure without touching fractal behavior.

### 4. Magnet Type I Formula Hygiene/Descriptor Note

Effort: low, half day.

Move a bit more Magnet formula metadata into a small descriptor/helper surface if it can be done without refactoring the renderer. This is not a broad engine rewrite; it is just making the existing Magnet direct-formula seam easier to clone for Type II/III later.

Proof: `test_escape_time_direct_formulas`, `test_fractal_renderer`, and runtime Magnet proof unchanged.

### 5. Magnet Type II Or Type III Spike

Effort: medium-low if formulas are fixed upfront; otherwise medium.

This is the next plausible new fractal family after Type I because it likely reuses the same direct escape-time infrastructure. It is only low-hanging if the exact recurrence and parameter defaults are known before coding.

Proof: enum/schema/binding/runtime/sample/render/diagnostics/probe coverage like Type I, plus one no-mouse slider proof.

### 6. Lyapunov Sequence Contract

Effort: medium, likely 1-2 days.

This remains the best next genuinely new family substrate from the long-term roadmap, but it needs a small sequence descriptor first. It is less risky than IFS or 3D work, but it is not a tiny polish task.

Proof: sequence parser/default tests, render sensitivity, runtime proof, and catalog visibility.

## Not Low-Hanging Right Now

- `explaino_magnet`: feasible but medium because of the legacy Explaino semantic split.
- perturbation deep zoom beyond Mandelbrot/Julia: important but recurrence-specific and proof-heavy.
- IFS/gasket: needs a different sampling/render substrate.
- attractor-density fields: needs accumulation/histogram infrastructure.
- Mandelbulb/3D DE: needs camera/raymarch/normals/lighting/perf gates.
- broad Color Pipeline work: explicitly separate from this Magnet/Explaino research path.

## Recommended Next Small Slice

If the goal is low risk and visible value, do Magnet control proof expansion first. It is the closest match to the recent pain: prove every visible slider actually changes the rendered output, with no OS mouse harness. It also gives a reusable proof pattern before attempting `explaino_magnet`.
