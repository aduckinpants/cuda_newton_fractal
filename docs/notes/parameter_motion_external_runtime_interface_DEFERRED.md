# Deferred: Versioned External Runtime-State Interface

Status: documented and deferred. This note protects the design boundary while Parameter Motion work proceeds; it does not authorize IPC implementation.

## Motivation

A future companion program should be able to inspect and eventually control the current viewer state without embedding prototype UX into the viewer. This could support programmable Color Pipeline animation, external experiment controllers, scientific diagnostics, agent verification, and state-tool integration.

The interface should resemble the existing `sample_fn` local protocol in operational shape, but its semantic authority is different: it exposes viewer state and capabilities rather than evaluating an isolated sample request.

## Existing Reusable Seams

- The headless `sample_fn` named-pipe/NDJSON protocol demonstrates versioned local request/response framing.
- The persistent viewer JSON command loop demonstrates process-owned no-mouse commands and report generation.
- The fractal parameter surface descriptor demonstrates discoverable public parameter metadata.
- Diagnostic state serialization demonstrates replay-authoritative input capture.
- The UI automation report and Color Pipeline graph receipt demonstrate derived runtime/report surfaces.

These are evidence and reusable patterns. None alone is the future API authority.

## Boundary Locks

1. Do not expose raw `KernelParams` memory or struct layout.
2. Do not treat diagnostic/report values as writable inputs.
3. Use stable binding paths and semantic IDs, not C++ member offsets or UI labels.
4. Separate authoritative inputs, derived runtime state, capabilities, and diagnostics.
5. Every snapshot carries schema version, process/session identity, state generation, and frame generation.
6. Read-only inspection ships before writes.
7. Writes, when added, are generation-checked transactions with prepare/commit semantics.
8. No external client becomes renderer authority.
9. Color Pipeline state uses graph/row semantic IDs rather than widget positions.
10. The viewer remains usable with no external process connected.

## Proposed Protocol Family

Transport candidate: local named pipe with newline-delimited JSON, following the operational lessons of `sample_fn` while using a separate protocol and endpoint.

Suggested schema ids:

- `viewer.runtime_state_request.v1`
- `viewer.runtime_state_response.v1`
- `viewer.runtime_state_snapshot.v1`
- `viewer.runtime_state_patch.v1` (later, not v1 launch)

## Read-Only V1 Commands

### `describe`

Returns protocol version, supported commands, parameter descriptor schema, capability groups, and limits.

### `get_snapshot`

Returns a coherent snapshot from one state generation:

- selector/fractal identity;
- authoritative camera/render/fractal inputs;
- Parameter Motion configuration and execution status;
- active Color Pipeline graph/row receipt;
- SDF/root-field producer capabilities where active;
- derived values and timing/report diagnostics;
- state generation and frame generation;
- latest settled/full-quality generation.

The server must not assemble a snapshot from unrelated mutable frames. It either captures one coherent generation or reports that a coherent snapshot is unavailable.

### `await_frame`

Deferred from the minimum read-only launch unless needed for deterministic clients. When added, waits for a requested state generation to produce a matching settled or any-quality frame under a bounded timeout.

## Snapshot Categories

### `authoritative_inputs`

Values that state load or a future patch may own: selector, camera, normal parameter bindings, render settings, root-pattern inputs, Lens/SDF inputs, and current Color Pipeline rows.

### `derived_runtime`

Resolved roots, field dimensions, effective preview quality, selected backend, generated hashes, active producer identity, and motion execution state. These are read-only.

### `capabilities`

Writable/readable/scrubbable/motion-animatable parameter descriptors, producer support, Color Pipeline route support, and structured exclusions.

### `diagnostics`

Frame timing, validation/fail-closed reasons, cache state, generation counters, and bounded receipts.

## Future Transactional Writes

A later `apply_patch` request should contain:

- expected state generation;
- ordered operations using stable paths;
- requested interaction mode;
- optional await-frame policy;
- client request id.

Server flow:

1. resolve all paths;
2. validate types, bounds, capabilities, and current-lane activity;
3. prepare every mutation and required allocation;
4. reject the entire patch on any failure;
5. commit non-failing authoritative replacements;
6. increment generation once;
7. emit one interaction/settle lifecycle;
8. return committed generation and per-operation receipts.

No partial writes and no silent adapter/substitution behavior are allowed.

## Parameter Motion Relationship

The Parameter Motion capability provider should be reusable by this interface later:

- stable binding paths;
- typed read/prepare/commit operations;
- independent capability flags;
- active-lane validation;
- structured exclusion reasons.

The Parameter Motion campaign must not add an IPC endpoint, background listener, or protocol placeholder. Its only obligation is to avoid a UI-only target registry that would need to be replaced later.

## Color Pipeline Relationship

Color Pipeline animation is intentionally excluded from Parameter Motion v1. A future external controller should address graph/row semantic IDs and parameter IDs from the materialized composition contracts. It must not scrape ImGui labels or row positions.

A Salticid-driven programmable control surface can later compile into transactional runtime patches or a dedicated animation graph. That is a separate campaign after the state interface and graph authority are stable.

## State And Security

- Local transport only by default.
- Explicit endpoint naming and process identity.
- Bounded message size, operation count, and wait timeout.
- No arbitrary file access or code execution.
- No implicit network listener.
- Snapshot reads do not mutate dirty state or render generations.
- Write capability, if added, is opt-in and visibly reportable.

## Deferred Qualification Plan

1. Capability and schema court over all public writable inputs.
2. Read-only in-process snapshot builder with deterministic JSON tests.
3. Local named-pipe `describe` and `get_snapshot` transport.
4. External state-tool consumer proof.
5. Generation-consistent `await_frame`.
6. Transactional `apply_patch` only after prepare/commit authority is proven.
7. Programmable Color Pipeline/animation clients only after graph parameter addressing is stable.

## Known Risks

- Snapshot bloat if broad diagnostic supersets replace scoped categories.
- False writability if schema visibility is confused with runtime binding authority.
- Stale derived values if state/frame generations are omitted.
- Hidden dual authority if external writes bypass UI/runtime mutation services.
- Permanent protocol debt if display labels or struct layouts become wire IDs.
- UI stalls if synchronous clients block the render loop.

## Stop Rule

Do not begin this interface as incidental work inside Parameter Motion, Color Pipeline, SDF, diagnostics, or state-tool slices. It requires its own plan/contract, baseline, protocol limits, threat review, and published-runtime proof.
