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
- [docs/examples/callable_engine/stateless_batch_two_requests.json](docs/examples/callable_engine/stateless_batch_two_requests.json)
  - stateless batch array example
- [docs/examples/callable_engine/stateless_ndjson_sequence_grid_request.json](docs/examples/callable_engine/stateless_ndjson_sequence_grid_request.json)
  - one stateless NDJSON sequence-grid example
- [docs/examples/callable_engine/session_diff_flow.ndjson](docs/examples/callable_engine/session_diff_flow.ndjson)
  - one open -> request -> diff request -> close session transcript example

## Practical Usage Notes

- Keep `function_id` explicit.
- Keep `request_id` meaningful.
- Start with small grids first, then scale up.
- Ask only for the metrics you need.
- Use `execution.backend_preference` only when you need to pin `generic.sample` to `default`, `cpu`, or `cuda`; successful responses report the actual executor in `runtime.backend_used`.
- For discovery of current parameter names and applicability rules, use `--describe-functions` first.

## Session Example Usage

`session_diff_flow.ndjson` is a line protocol transcript, not a single JSON object.

Use it like this:

```powershell
Get-Content docs/examples/callable_engine/session_diff_flow.ndjson | D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-session
```