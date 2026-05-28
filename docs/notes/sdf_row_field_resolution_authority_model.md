# SDF Row Field Resolution Authority Model

Status: Step 3A design/RED surface. This is not a shipped product contract yet.

## Current Shipped Authority

The current viewer has one shared SDF field-resolution authority:

- `LensSettings::downsample` is the saved/runtime field-resolution value.
- The Color Pipeline Source section exposes an alias control,
  `color_pipeline.source.sdf_field.downsample.primary`, bound to that shared
  Lens setting.
- Runtime reports expose `lens_sdf_requested_downsample`,
  `lens_sdf_effective_downsample`, and `lens_sdf_quality_mode` for the shared
  field.
- `ColorPipelineSourceRuntimeParams::sdf_sample_step` is row-local, but it is
  a postprocess/source sampling step. It does not request a distinct SDF field
  producer resolution.

## Candidate Model

- Add a source-row-local field-resolution policy to
  `ColorPipelineSourceRuntimeParams`.
- The default policy is `inherit_shared`, which preserves current behavior:
  existing rows and old state JSON continue to use `LensSettings::downsample`.
- Explicit row policy selects a field downsample from `1`, `2`, `4`, `8`, or
  `16`.
- Enabled SDF-backed Source rows are grouped by effective field downsample.
- Disabled rows and non-SDF rows request no SDF field.
- Mixed enabled SDF/non-SDF Source stacks keep the existing fail-closed behavior.
- Distinct live SDF field groups are capped at `4`. Exceeding the cap must
  fail closed and report a clear reason.

## Step 3A RED Evidence

`tools/sdf_row_field_resolution_red_matrix.py` records the current expected-red
matrix:

- Source-row runtime params have `sdf_sample_step` but no row-local field policy.
- State IO parses/writes source rows but cannot load a row-local field
  downsample.
- Capture state writes source-row params but cannot persist row-local field
  authority.
- Effective-source summaries report source-stack rows but no field groups.
- UI exposes the shared field-downsample alias but no row-local field policy.
- Automation reports shared requested/effective downsample fields but no
  per-group field count or timing.
- The live render path resolves one effective Lens SDF downsample and computes
  or reuses one field.

This proves Step 3B must add a multi-field planner/producer/cache before Step 3C
can productize row-local controls.
