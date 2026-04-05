# Spec Intake Packet

**Spec Title:** Slime C++ Structural Analysis — Mainline Pre-Work for Viewer-Host Extraction

**Date:** 2026-04-04

**Target Repo:** salticid-cuda (mainline)

**Downstream Consumer:** cuda_newton_fractal_clone -> salticid-viewer bootstrap

## 0) Intake Summary

The slime relaxation engine (`slime_relax.py`, `slime_oracle.py`, `slime_rules_v1.py`) is a deterministic energy-minimization rewriting framework. It has two working rule families (mean3x3 smoother, logic tape simplifiers) and minimal test coverage. This spec defines the mainline work to:

1. Harden and probe the existing slime framework (the prototype that motivated the project)
2. Add a C++ structural analysis domain that can encode function-level properties of a C++ monolith
3. Run the structural analyzer on `ide_ui_dx11/ui_app/src/main.cpp` to produce deterministic extraction-boundary proposals
4. Generate artifacts that feed directly into the viewer-host extraction plan

Five outcomes from one effort: (a) slime gets tested/probed properly, (b) C++ structural rules get built, (c) main.cpp extraction gets data-driven, (d) viewer repo bootstrap gets concrete boundaries, (e) the agent onboarding for the new repo starts with proven structural context instead of chat archaeology.

## 1) Current State Assessment

### 1.1 What Exists

| Module | Lines | Tests | Coverage Gaps |
|--------|-------|-------|---------------|
| `slime_relax.py` | ~310 | `test_slime_relax_v1.py` (1 test: clamp-to-zero) | No adversarial inputs, no convergence proof, no multi-rule interaction, no large-field perf |
| `slime_oracle.py` | ~75 | `test_slime_oracle_v1.py` (1 test: fixed-point) | No multi-step trajectory, no budget-exhaustion path, no projection edge cases |
| `slime_rules_v1.py` | ~310 | `test_slime_rules_v1.py` (2 tests: stabilizer + logic) | Individual rule correctness untested, registry snapshot ordering untested, rule conflict untested |
| `slime_demo_rules.py` | ~100 | `test_slime_demo_ops_smoke.py` (3 tests: shape/range) | Salt operator round-trip only, no semantic validation |
| `busy_beaver.py` | ~60 | indirect via oracle test | No direct unit tests |
| `projection_v1.py` | ~80 | indirect via oracle test | No direct tests for downsample, channel selection, quantization |
| native parity | ctypes bridge | `test_native_parity_slime_seed_ops.py` (5 tests) | Parity only for seed ops, not for relax or logic |

### 1.2 Demos (3 checked-in .salt files)

- `01_slime_smooth_relax_rgb.salt` — visual noise smoothing
- `02_slime_smooth_relax_rgba_edges.salt` — edge-emphasis pipeline
- `03_slime_logic_simplify_tape.salt` — logic tape rewrite demo

All three are visual-only. No assertion harness, no golden-output comparison.

## 2) Work Phases

### Phase A: Harden the Existing Framework

**Goal:** The slime engine is trustworthy before building new domains on it.

**Exit criteria:** All new tests green; existing tests still green; no regressions.

#### A.1 — Direct unit tests for foundation modules

- `test_busy_beaver_v1.py`: cycle detection, budget exhaustion, degenerate tick (identity), large cycle length
- `test_projection_v1.py`: downsample stride correctness, channel selection, quantization round-trip, edge shapes (1x1, prime dims)

#### A.2 — Slime relax edge cases

- Empty registry => immediate convergence (already handled, needs explicit test)
- Single-site field (1x1xC) with out-of-bounds values
- All-zero energy field => zero updates
- `nonoverlap_radius` blocking correctness (verify spatial exclusion geometry)
- Multi-rule interaction: two rules applicable at same site, verify lexicographic winner
- Convergence proof: known initial state + rules must reach known fixed point within bounded iters

#### A.3 — Oracle trajectory probing

- Multi-step trajectory where score > 2 (distinct intermediate states before cycle)
- Budget-exhaustion path (max_steps reached before cycle)
- Determinism proof: same inputs => identical `BusyBeaverResultV1` across runs

#### A.4 — Logic rule correctness

Individual rule tests with minimal tape:
- `DoubleNegationRule`: VAR NOT NOT -> VAR (center becomes VAR, inner cleared)
- `NotConstRule`: CONST0 NOT -> CONST1, CONST1 NOT -> CONST0
- `AndOrConstRule`: AND(CONST0, x) -> CONST0, AND(CONST1, VAR) -> VAR, OR(CONST1, x) -> CONST1, OR(CONST0, VAR) -> VAR
- `XorSelfRule`: XOR(VAR, VAR) -> CONST0
- Rule inapplicability: verify rules decline when pattern doesn't match
- Tape boundary: rules near x=0 or x=W-1 must not crash

### Phase B: C++ Structural Encoding

**Goal:** Represent `main.cpp` function-level structure as a slime-compatible (H,W,C) field.

**Exit criteria:** Encoder produces a field from real main.cpp; field round-trips to human-readable summary.

#### B.1 — C++ function-level parser (Python, lightweight)

Not a full C++ parser. Needs to extract from a single translation unit:
- Function boundaries (name, start line, end line, body line count)
- Call edges (which functions call which — grep-level, not AST-level)
- State touches (which global/member variables are read/written — heuristic grep)
- Include dependencies (which headers the function's body references)

Recommended approach: extend or wrap the existing `code_quality_audit.py` function-size scanner (it already parses function boundaries). Add call-edge and state-touch heuristics.

Output: `CppStructuralGraph` dataclass with functions, edges, state-touches.

#### B.2 — Field encoding

Map `CppStructuralGraph` to `(N_functions, N_functions, C)` field:
- Channel 0: adjacency weight (call count between functions i and j)
- Channel 1: shared-state coupling (count of shared global/member touches)
- Channel 2: module assignment (integer label, initially all 0 = "monolith")
- Channel 3: function size (normalized body line count, replicated on diagonal)
- Channel 4: extraction cost estimate (interface width if this function were extracted alone)

Diagonal entries `[i,i,:]` hold per-function properties. Off-diagonal `[i,j,:]` hold pairwise coupling.

#### B.3 — Field decoder

Reverse map: read channel 2 (module assignments) back to named groups of functions. Produce a Markdown report: proposed modules, their functions, interface width, cohesion score.

### Phase C: Structural Energy Terms

**Goal:** Define what "good extraction" means as minimizable energy.

**Exit criteria:** Energy terms produce non-trivial gradients on the encoded main.cpp field.

#### C.1 — `energy_module_cohesion`

Penalize modules whose internal call-edge density is low relative to their cross-module edges. A module with many functions that don't call each other is a bad grouping.

#### C.2 — `energy_interface_width`

Penalize extraction boundaries that cross many call edges or shared-state dependencies. A module with 50 external callers is expensive to extract.

#### C.3 — `energy_size_balance`

Penalize modules that are too large (> threshold lines) or too small (< threshold). The goal is to produce modules in the 200-800 line range.

#### C.4 — `energy_naming_affinity` (optional, stretch)

Use function name prefixes/tokens as a soft clustering signal. Functions named `viewport_*` should prefer the same module.

### Phase D: Structural Rewrite Rules

**Goal:** Local moves that the relaxer can apply to improve extraction proposals.

**Exit criteria:** Rules fire on encoded main.cpp field; energy decreases; proposals are non-trivial.

#### D.1 — `ReassignFunctionRule`

Move a function from one module label to another. Predict delta using cohesion + interface energy change. Applicable when the function has stronger coupling to the target module than its current one.

#### D.2 — `SplitModuleRule`

Split a module at a natural boundary (largest gap in call-edge density within the module's function sequence). Applicable when a module exceeds the size threshold.

#### D.3 — `MergeFragmentRule`

Merge a small module (< threshold) into the neighbor it's most coupled with. Applicable when a module is below the size floor.

### Phase E: Probe Run on main.cpp

**Goal:** Produce concrete extraction proposals for viewer-host work.

**Exit criteria:** Deterministic report artifact checked into mainline; proposals reviewed by human.

#### E.1 — Encode main.cpp

Run the B.1 parser on `ide_ui_dx11/ui_app/src/main.cpp`. Produce `CppStructuralGraph` JSON artifact.

#### E.2 — Initial field + relaxation

Encode to field. Run slime relaxation (Phase D rules, Phase C energy terms). Produce trajectory (oracle busy-beaver score for the structural domain).

#### E.3 — Decode and report

Decode final module assignments. Produce Markdown report:
- Proposed modules with function lists
- Per-module metrics (size, cohesion, interface width)
- Top extraction candidates (lowest interface width, highest cohesion)
- Comparison against known viewer-host extraction targets (viewport, colormap, schema binding, fractal renderer, input handling, HUD)

#### E.4 — Iterate

If proposals don't match intuition, adjust energy weights and re-run. The relaxer is deterministic — same weights + initial state => same result. Document the weight tuning in the artifact.

## 3) Acceptance Criteria

### Framework (Phase A)
- [ ] All foundation modules have direct unit tests
- [ ] Slime relax passes edge-case and convergence-proof tests
- [ ] Oracle trajectory test with score > 2
- [ ] Each logic rule has an individual correctness test
- [ ] All existing tests still green (no regressions)

### Structural Domain (Phases B-D)
- [ ] C++ function parser extracts boundaries + call edges from main.cpp
- [ ] Field encoding round-trips (encode -> decode -> human-readable -> matches parser output)
- [ ] Energy terms produce non-zero gradients on the main.cpp field
- [ ] At least 2 rewrite rules fire and reduce energy
- [ ] Relaxation converges (oracle detects cycle or budget)

### Probe (Phase E)
- [ ] Deterministic structural report artifact checked in
- [ ] Report includes proposed modules with function lists and metrics
- [ ] Report identifies top extraction candidates for viewer-host
- [ ] Weight tuning documented if adjustments were needed

## 4) Fail-Fast Rules (No Implicit Fallback)

- If the C++ parser can't find function boundaries: error, don't guess
- If a function has zero call edges: represent as isolate, don't infer
- If field encoding produces NaN/Inf: error at encode time
- If energy terms return negative values: clamp to 0 with warning (existing behavior)
- If no rules fire on a probe run: report "no proposals" explicitly, don't fabricate

## 5) Files To Create/Modify in Mainline

### New files
- `salticid_runner/cpp_structural_graph.py` — B.1 parser + B.2 encoder + B.3 decoder
- `salticid_runner/slime_rules_structural_v1.py` — D.1/D.2/D.3 rules + C.1-C.3 energy terms
- `scripts/slime_structural_probe.py` — E.1-E.4 probe script (CLI, deterministic, artifact output)
- `tests/test_busy_beaver_v1.py` — A.1
- `tests/test_projection_v1.py` — A.1
- `tests/test_slime_relax_edge_cases.py` — A.2
- `tests/test_slime_oracle_trajectory.py` — A.3
- `tests/test_slime_logic_rules_individual.py` — A.4
- `tests/test_cpp_structural_graph.py` — B tests
- `tests/test_slime_structural_rules.py` — D tests

### Modified files
- `salticid_runner/__init__.py` — register new modules if needed
- `spec/operators.d/slime.json` — add structural operators if exposed to Salt

### Artifacts (gitignored or artifacts/)
- `artifacts/structural_probe/main_cpp_graph.json` — E.1 output
- `artifacts/structural_probe/main_cpp_extraction_report.md` — E.3 output

## 6) Dependency Chain

```
Phase A (harden framework)
  |
  v
Phase B (C++ encoding)  -- can overlap with A if A.1-A.2 are done
  |
  v
Phase C (energy terms)  -- depends on B.2 for field shape contract
  |
  v
Phase D (rewrite rules) -- depends on C for energy prediction
  |
  v
Phase E (probe run)     -- depends on all above
```

Phase A is independently valuable and should land first. Phases B+C can be developed together. Phase D depends on the energy term contracts from C. Phase E is the integration test.

## 7) What Comes Back Here

When Phase E lands and the extraction report exists, return to this clone repo with:

1. The structural report (`main_cpp_extraction_report.md`) — concrete extraction boundaries
2. The hardened slime test surface — confidence the framework works
3. Knowledge of which main.cpp functions belong to which proposed module

Then here we:
- Use the extraction boundaries to plan the `salticid-viewer` repo structure
- Write agent customization files (.instructions.md, .prompt.md) pre-loaded with the structural context
- Create a session startup script/prompt that gives the next agent the full picture without chat archaeology
- Start the actual viewer-host extraction work with data-driven module boundaries

## 8) Relation to Existing Tooling

| Tool | Domain | Slime Complement |
|------|--------|-----------------|
| `salt_ndepend` | Salt AST lint (pure functions) | Slime handles imperative/stateful C++ that ndepend can't parse |
| `transpiler` | Salt -> CoreIR -> backends | Transpiler converts pure math; slime analyzes the structural container |
| `code_quality_audit.py` | Function-size ratchet | Extend its parser for call-edge extraction (Phase B.1) |
| `slime_demo_rules.py` | Salt operator wrappers | Structural rules stay Python-only (no Salt operator surface needed in v1) |
