# Callable Engine API Compatibility Follow-Ups

Status: deferred. This note records low-hanging callable/sample API cleanup items found during a read-only review on 2026-06-11. None of these should interrupt the current SDF work.

Current repo context:

- Viewer repo branch at review time: `codex/sdf-measurement-replan-20260611`
- Viewer `HEAD` at review time: `322ff14`
- Salticid mainline was reviewed read-only for operator-surface context only.
- No product code or API behavior was changed by this note.

## Summary

The viewer-side callable API is not the likely root cause of the active Salticid `sample_fn` crash. The shipped `fractal.sample` and `generic.sample` paths are real, fail closed on bad IDs, and have runtime coverage. The low-hanging work is compatibility hardening: make defaults explicit, expose request-block requirements more mechanically, add exact cross-repo fixtures, and update Salticid's operator contract so it no longer relies on old POC assumptions.

## 1. Descriptor/Runtime Default Contract Mismatch

Finding:

- `fractal_probe_contract.cpp` still defaults absent `function_id` to `fractal.sample`.
- `fractal_probe_runner.cpp` builds a no-override `fractal.sample` base state as `newton`.
- `ui/fractal_binding_surface_v1.ui_schema.json` advertises the normal UI default as `explaino_all`.
- The public cheatsheet correctly tells callers to set `fractal.view.fractal_type` explicitly, which keeps this from being urgent.

Risk:

Descriptor-driven tooling can infer a default from UI/schema metadata and then get a different no-override runtime result.

Deferred recommendation:

- Add one focused test proving the policy for absent `fractal.view.fractal_type`.
- Choose one explicit contract:
  - make no-override runtime match the descriptor/UI default; or
  - make `fractal.view.fractal_type` mechanically required for external callers and reject or warn on omitted type; or
  - keep `newton` as the callable default but advertise that default directly in the callable descriptor instead of inheriting the UI default.

Preferred future direction:

Require or strongly surface explicit `fractal.view.fractal_type` for external-provider requests. The callable API is a sampling contract, not the live UI default contract, so implicit family choice should be minimized.

## 2. Generic Sample Request-Block Discovery

Finding:

- `generic.sample` correctly requires a `function` block with `expression` or `ast`.
- The runtime error is clear: `generic.sample requires a 'function' block with 'expression' or 'ast'`.
- `--describe-functions` exposes `generic.sample` outputs, but it does not provide a machine-readable request schema for the required `function` block.

Risk:

External lowerers can discover `generic.sample` as a callable ID without discovering how to construct its required request body.

Deferred recommendation:

- Extend callable descriptors with a small request-shape capability section, for example:
  - required blocks: `function.expression` or `function.ast`
  - optional blocks: `function.params`, `function.epsilon`, `function.escape_radius`, `function.iterate.count_param`
  - supported sequence paths: `function.params.*`
  - supported backend preferences: `default`, `cpu`, `cuda`
- Keep expression/AST semantics in the request body; do not pretend dynamic formulas are ordinary parameter paths.

## 3. Salticid-Shaped Compatibility Fixture In Viewer Runtime Tests

Finding:

- Viewer runtime tests cover explicit Salticid-style `operator_context`.
- They cover explicit fractal type overrides, NDJSON, generic sample, backend pins, and bad IDs.
- They do not carry one exact current Salticid `sample_fn` request fixture as a compatibility sentinel.

Risk:

Small wire-shape drift could break Salticid resync without a viewer-side test showing the external-provider request shape that matters.

Deferred recommendation:

- Add one compact published-runtime fixture that mirrors current Salticid `lower_sample_fn(...)` output:
  - `function_id = "fractal.sample"`
  - `mode = "sequence_grid"` if keeping the legacy fractal wrapper behavior
  - explicit `overrides` including `fractal.view.fractal_type`
  - `operator_context.source = "salticid"`
  - `operator_context.operator = "sample_fn"`
  - `operator_context.why` present
- Add one negative fixture for `generic.sample` without `function`, proving the viewer fails closed with the useful error.

## 4. Operator Context Strictness

Finding:

- Viewer parsing requires `operator_context.source`, `operator_context.operator`, and `operator_context.why`.
- Current Salticid `lower_sample_fn(...)` supplies all three.

Risk:

Older POC clients with partial `operator_context` will fail before sampling. This is acceptable if intentional, but it should be an explicit all-or-nothing contract.

Deferred recommendation:

- Either document `operator_context` as all-or-nothing and keep strict parsing, or make the parser tolerate missing optional fields by defaulting them to empty strings.

Preferred future direction:

Keep strict parsing for now. The context is useful provenance, and current Salticid already sends the full shape. If compatibility pressure appears, loosen only with a regression test proving old clients are intentionally supported.

## 5. Spec/Docs Stale Edge Around Defaults

Finding:

- The newer callable cheatsheet tells callers to set `function_id` and `fractal.view.fractal_type` explicitly.
- Older spec-intake text still emphasizes `function_id` defaulting to `fractal.sample`.
- That default is still true, but it is less important than the fractal-type default policy.

Risk:

Future agents can read the older spec and assume absent `function_id`/absent fractal type is a good integration shape.

Deferred recommendation:

- Update older spec-intake/default docs when the callable API gets its next real slice.
- Add a short "external provider requests should be explicit" rule:
  - set `function_id`
  - set `fractal.view.fractal_type` for `fractal.sample`
  - set `function.expression` or `function.ast` for `generic.sample`
  - include full `operator_context`

## 6. Salticid Operator-Surface Resync Recommendations

Finding:

- Salticid's `lower_sample_fn(...)` is still partly shaped by the older E1/E2 POC.
- It forwards `function_id`, `overrides`, `backend`, and `operator_context`, which is good.
- Some checked-in `demos/cuda_engine/sample_fn_cuda_mandelbrot*.salt` examples do not explicitly set `fractal.view.fractal_type`.
- That means they currently rely on this viewer's no-override callable default, which is `newton`, not Mandelbrot.
- Salticid tests also explicitly preserve a legacy `fractal.sample` `grid -> sequence_grid` wrapper while `generic.sample` uses native `grid`.
- Recent Salticid product-gate work already classifies external-provider `sample_fn` as contract debt rather than a fully green product lane.

Deferred Salticid-side recommendations:

1. Make `sample_fn(function_id="fractal.sample")` require or auto-inject an explicit `fractal.view.fractal_type`.
   - For Mandelbrot demos, inject `{"fractal.view.fractal_type": "mandelbrot"}`.
   - For user-facing docs, show the fractal type every time.

2. Split `sample_fn` modes by provider contract instead of treating every function ID as one loose operator:
   - `fractal.sample`: shipped fractal family sampler using binding-path overrides.
   - `generic.sample`: request-supplied formula sampler requiring `function.expression` or `function.ast`.
   - synthetic/no-exe fallback: local smoke-test fallback only, not evidence of external provider compatibility.

3. Add first-class `generic.sample` authoring kwargs on the Salticid side.
   - Examples: `expression=...`, `ast=...`, `params={...}`, `epsilon=...`, `escape_radius=...`, `iterate_count_param=...`.
   - Lower those into the viewer's `function` block instead of trying to represent formula authority as normal overrides.

4. Keep `backend` valid only for `generic.sample`.
   - Salticid already enforces this. Preserve it.
   - Add live fixture coverage that `backend="cuda"` produces `runtime.backend_used == "cuda"` through the real published viewer when available.

5. Stop using old no-exe synthetic fallback tests as product proof for external-provider `sample_fn`.
   - They are useful smoke tests for the Salticid operator surface.
   - They do not prove the viewer protocol, function block, runtime backend, or descriptor compatibility.

6. Add a current exact-request golden fixture generated by Salticid and replayed by the viewer.
   - This should be a file or JSON artifact both repos can point at.
   - It should include one `fractal.sample` request and one `generic.sample` request.
   - It should prove request construction, response parsing, metric extraction, backend reporting, and fail-closed behavior.

7. Treat `emit="discover"` as a discovery/probing tool, not a sufficient schema for generic function authoring.
   - It can count functions today.
   - It should eventually surface enough descriptor/request-shape metadata to guide Salticid lowering decisions.

Preferred future Salticid direction:

Build `sample_fn` as a thin external-provider adapter over the viewer's callable contract, not as a generic "sample anything" operator. The adapter should select one of a few typed provider paths and fail closed when required request authority is missing.

## Suggested Future Slice

Suggested slug: `callable_engine_api_compatibility_followups`.

Recommended order:

1. Add viewer-side tests for default policy and Salticid-shaped fixtures.
2. Add or update docs to make explicit external requests the pit of success.
3. Add descriptor/request-shape metadata for `generic.sample`.
4. Update Salticid `sample_fn` docs/tests/lowering to require explicit fractal type and support generic function blocks.
5. Add cross-repo golden request/response fixtures.

This should stay behind the SDF engine work unless the active Salticid crash is proven to require a viewer-side change.
