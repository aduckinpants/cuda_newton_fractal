# Callable Transport And Session Cheatsheet

This is the practical transport guide for the current callable POC.

If you need the math surfaces themselves, read:

- [docs/callable_engine_fractal_sample_cheatsheet.md](docs/callable_engine_fractal_sample_cheatsheet.md)
- [docs/callable_engine_dynamic_function_cheatsheet.md](docs/callable_engine_dynamic_function_cheatsheet.md)

If you want copy-paste payloads, read [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md).

## Mental Model

- `fractal.sample` and `generic.sample` are the callable functions.
- `--sample-request-*` is the stateless transport.
- `--sample-session` is the stateful transport.
- `output_mode` changes how one request is serialized, not what math runs.

The transport question is separate from the math question.

## Stateless Transport

Use stateless mode when each request is self-contained.

CLI forms:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-stdin --sample-response-stdout
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-json request.json --sample-response-json response.json
```

Use this when:

- each request carries its full override set
- you want simple stdin/stdout scripting
- you want batch request arrays
- you do not need `state_token` diff carry-forward

### Single-object requests

Single JSON object in, single JSON object out.

This is still the normal path and keeps V1 compatibility.

### Batch request arrays

The stateless transport also accepts a JSON array of requests.

Behavior today:

- array in -> array out
- responses stay in request order
- per-request errors are isolated in the response array
- overall exit code is non-zero if any request in the batch fails
- empty array is valid and returns `[]`

Current limitation:

- `output_mode = ndjson` is not allowed inside batch arrays

## `output_mode`

Current output modes:

- `json` (default)
- `ndjson`

### `json`

Use this when you want one whole response object.

Good fit:

- normal scripting
- file output
- batch arrays
- one-shot probes where whole-response parsing is simplest

### `ndjson`

Use this when you want streamed line output for one request.

Behavior today:

- `point_set` -> one `sample_batch` line plus one `summary` line
- `grid` -> one `sample_batch` line per grid row plus one `summary` line
- `sequence_point_set` -> one `sample_batch` line per sequence step plus one `summary` line
- `sequence_grid` -> one `sample_batch` line per sequence step plus one `summary` line
- summary-only metric requests -> only one `summary` line
- every NDJSON line still carries `request_id`
- `function_id` is present on both sample batches and summary lines

Important limitation:

- NDJSON is for a single request, not request arrays

## Session Transport

Use session mode when requests should build on previous state.

CLI:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-session
```

The protocol is one JSON object per line.

Typical flow:

1. send `{"session":"open"}`
2. receive `{"session":"ready","state_token":"s0","engine_version":2}`
3. send one or more sample requests
4. receive one response per request
5. send `{"session":"close"}`
6. receive `{"session":"closed"}`

### `state_token` semantics

Session mode adds a `state_token` chain.

Current behavior:

- ready handshake starts at `s0`
- successful requests mint fresh tokens like `s1`, `s2`, ...
- bad requests do not mint a new token
- later requests can reference a prior token to inherit accumulated overrides
- requests without `state_token` still work as ordinary session requests
- `s0` is a valid empty baseline state

This is the right tool when you want diff-style probing such as:

- first request sets fractal family and baseline params
- second request only changes zoom
- third request only changes max_iter

### Session + NDJSON

Session mode also supports `output_mode = ndjson`.

Important behavior:

- streamed `sample_batch` lines still appear first
- the final session `summary` line carries the minted `state_token`
- the next request can immediately use that token
- bad NDJSON requests do not mint a token

### Named-pipe alternate transport

Windows also has a named-pipe session transport:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-session --sample-session-pipe my_session_pipe
```

Current scope:

- Windows-only
- alternate transport for the same one-line session protocol
- intended as a bounded single-client session path, not a general multi-client transport layer

## What To Use When

Use stateless JSON when:

- one request fully describes the probe
- you want the simplest possible script surface

Use stateless NDJSON when:

- you want streamed batches for one request
- you care about sequence-step or row-wise progressive consumption

Use batch arrays when:

- you want multiple independent requests in one call
- you do not need NDJSON
- you do not need diff-state carry-forward

Use session mode when:

- requests should inherit previous overrides
- you want incremental diff-style probing with `state_token`
- you want the runtime to keep accumulated state between requests

## Current Hard Rules And Limits

- `--sample-session` is mutually exclusive with the stateless sample-request verbs
- batch arrays do not allow `output_mode = ndjson`
- unknown `state_token` values fail fast
- malformed JSON returns an error response instead of crashing the session
- session open/close is explicit; EOF without `close` is treated as an unclean exit

## Copy-Paste Examples

- [docs/examples/callable_engine/stateless_batch_two_requests.json](docs/examples/callable_engine/stateless_batch_two_requests.json)
- [docs/examples/callable_engine/stateless_ndjson_sequence_grid_request.json](docs/examples/callable_engine/stateless_ndjson_sequence_grid_request.json)
- [docs/examples/callable_engine/session_diff_flow.ndjson](docs/examples/callable_engine/session_diff_flow.ndjson)