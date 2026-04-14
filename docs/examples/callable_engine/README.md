# Callable Engine Examples

These are copy-paste request JSON files for the current callable POC.

Use them with either transport:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-json request.json --sample-response-json response.json
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-stdin --sample-response-stdout
```

## Example Files

- [docs/examples/callable_engine/generic_sample_newton_z3m1_point_set.json](docs/examples/callable_engine/generic_sample_newton_z3m1_point_set.json)
  - request-supplied Newton solve through `generic.sample`
- [docs/examples/callable_engine/fractal_sample_explaino_lambda_grid.json](docs/examples/callable_engine/fractal_sample_explaino_lambda_grid.json)
  - one real shipped fractal-family grid probe through `fractal.sample`
- [docs/examples/callable_engine/fractal_sample_sequence_grid_seed_drift_zip.json](docs/examples/callable_engine/fractal_sample_sequence_grid_seed_drift_zip.json)
  - zipped multi-axis runtime parameter sweep through `fractal.sample`
- [docs/examples/callable_engine/fractal_sample_variant_crossfade_grid.json](docs/examples/callable_engine/fractal_sample_variant_crossfade_grid.json)
  - dedicated Explaino `variant_crossfade` sequence example through `fractal.sample`

## Practical Usage Notes

- Keep `function_id` explicit.
- Keep `request_id` meaningful.
- Start with small grids first, then scale up.
- Ask only for the metrics you need.
- For discovery of current parameter names and applicability rules, use `--describe-functions` first.