# Precision Authority Inventory Phase 1 Report

## Disposition

`REVIEW_REQUIRED_BEFORE_PRODUCT_REPAIR`

The emergency audit found a broad authoring-identity problem, not a reason to
convert the engine wholesale to `double`. It also found runtime-tier and state
loading seams that need focused witnesses before they can be called defects.

This report reviews the deterministic source inventory at:

- machine-readable evidence: `artifacts/precision_authority/phase1/matrix.json`;
- readable generated projection: `artifacts/precision_authority/phase1/matrix.md`;
- generator: `tools/precision_authority_inventory.py`;
- source checkpoint: the exact commit recorded inside the generated matrix.

The matrix is source evidence. It is not proof that the packet-bound executable
exercises every source route it names. Published-runtime identity and behavioral
witnesses remain separate evidence.

## Reviewed Results

### General schema authoring

The current schema contains 107 numeric controls:

| Classification | Count | Reviewed meaning |
|---|---:|---|
| `AUTHORING_IDENTITY_LOSS` | 90 | The editable text route does not expose enough digits to re-enter every value owned by the underlying storage or authority route. |
| `TRUTHFUL_FLOAT32` | 4 | A float-backed logarithmic input uses nine significant digits, sufficient to spell a binary32 value round-trip. |
| `INTENTIONAL_MIXED_PRECISION` | 13 | Integer authoring is exact for this audit, including the deliberate long-edge-to-`int2` resolution projection. |

The 90 identity-risk controls split into these source-proven groups:

- 83 ordinary float-backed controls use `%.5f`; five fixed decimals are not a
  round-trip representation for arbitrary binary32 values.
- four double-backed controls use `%.6f`: `explaino_seed`, `explaino_seed_b`,
  `dynamics_root_field_seed`, and `color_root_field_seed`.
- `center_x` and `center_y` read the authoritative double
  `center_hp_x` / `center_hp_y`, narrow through a float editor, display five
  fixed decimals, and write that result back to the high-precision authority.
- `zoom` uses a double editor but presents linear zoom with `%.9g` while the
  authoritative member is double `log2_zoom`. Nine significant digits cannot
  represent arbitrary binary64 camera authority exactly.

`AUTHORING_IDENTITY_LOSS` has a deliberately narrow meaning here: typing the
shown editable spelling back can change the stored state even when the user
intended no semantic change. It does not assert that every slider step is bad,
that every rounded label is bad, or that the runtime recurrence uses the same
width as the UI member.

The inventory originally flattened camera controls into ordinary `BindFloat`
storage and reported the composite resolution control as unresolved. Hostile
review rejected both claims. The repaired inventory now records separately:

```text
schema binding storage
editor carrier
authoritative storage
special edit route
input spelling
```

It also identifies the combined ExplainO seed setter as a double authority with
derived float fields rather than a simple direct binding.

### State JSON

The current diagnostics serializer uses
`std::numeric_limits<double>::max_digits10`. That is a source-proven
round-trip-capable output policy for the numeric values it receives and confirms
that the Rational Escape trigger is not explained by generic save-side decimal
truncation.

The loader contains 149 explicit `static_cast<float>` sites. Their existence is
not itself a bug: many destination members are intentionally float-backed. The
current classification is therefore `NEEDS_RUNTIME_WITNESS`, not
`STATE_IO_NARROWING`. Phase 3 must first join each serialized key to its actual
owner and test whether the engine-emitted state truthfully reports any
normalization. A bulk cast removal is not authorized.

### Color Pipeline UI-Salt route

The compiled UI-Salt function-library contract contains 120 parameters. Draft
numeric values use a double carrier. That proves neither binary64 execution nor
loss: each selected function can still consume or deliberately narrow its value
in its runtime implementation.

The correct classification is `NEEDS_RUNTIME_WITNESS`. The next Color Pipeline
audit must derive its parameter list from the compiled contract and join each
parameter to its current function consumer. No C++ or Python precision table may
be introduced alongside that contract.

### Runtime tiers

The tier resolver currently:

- advertises `standard` for all 51 shipped selectors; and
- resolves `standard` to `float64` plus direct iteration.

The original Phase 1 projection counted 25 selectors without a selector-named
top-level `useFP64` token. Phase 2A proved that this was routing-accounting debt,
not 25 independent repair items. The canonical device currently exposes 31
top-level dispatch owners for 51 selectors; shared predicates and the generic
escape-time fallback own the apparent gaps.

The updated inventory derives those owners from current source and looks for the
canonical executed-arithmetic evidence assignment. It reports no dispatch owner
without that static marker after the bounded Phase 2A repairs. This remains
source-routing evidence rather than behavioral proof: the focused CUDA witness
is authoritative for the five repaired owners covering six selectors, and
published-runtime replay supplies the viewer-path check.

Presence of a marker still does not prove that every participating parameter is
binary64. Parameter storage and helper-consumer width remain separate audit
questions for later phases.

## Priority Repair Candidates

These are proposed successor slices for review, not authorization to mutate the
product during Phase 1.

### P0 - runtime-tier truth witnesses

Build a deterministic requested-tier / resolved-backend / executed-route witness
at each distinct sampler owner. Start with representative generic escape-time,
shared-predicate, delegated ExplainO, and selector-specific branches. A false
claim should be repaired by the smallest truthful choice: implement the missing
binary64 path, narrow advertised support, or emit explicit fallback/unsupported
evidence.

### P0 - exact editable identity for high-value double authority

Start Phase 3 with the observed Rational Escape combined seed, the other three
double controls, and the specialized camera routes. Keep display readability
separate from exact editable text. Prove unchanged readback, edited value
survival, saved state, reload, and engine-emitted state. Do not infer a generic
camera fix from ordinary float controls.

### P1 - mechanically identified float peers

After the double/camera witness is green, address the 83 ordinary float inputs as
one mechanically derived peer class if one shared edit-format repair is truthful.
Protect intentional widget step behavior and binary32 runtime continuity.

### P1 - state-load owner classification

Map the 149 cast sites to actual destination ownership and add sentinel
round-trips for high-impact state fields. Preserve casts that truthfully load
float members. Promote storage only under the stricter Phase 4 evidence gate.

### P1 - Color Pipeline consumer witnesses

Select representative int, float, and enum contract parameters plus the
precision-sensitive sources already used in findings. Prove the contract value,
draft carrier, serialized state, runtime consumer width, emitted state, and frame
effect without creating parallel metadata.

## Unknowns That Block Broader Repairs

- Which participating helper parameters remain intentionally float-backed inside
  otherwise truthful binary64 branches?
- Which helper calls inside nominally binary64 branches still intentionally or
  accidentally consume float-backed values?
- Which of the 149 state-load casts are truthful destination conversions, which
  are compatibility mirrors, and which lose an authoritative value?
- Which Color Pipeline functions consume their double draft carrier as double,
  float, integer, or enum?
- Which visible float controls intentionally expose coarse authoring versus
  merely inheriting a shared format that predates exact state workflows?
- What exact public UI automation and state replay witnesses should enforce
  camera no-op identity without recreating camera mathematics outside the
  engine?

## Boundaries Preserved

- No product C++, CUDA, schema, runtime, or state behavior changed in Phase 1.
- No mass float-to-double conversion is proposed.
- Nonzero ExplainO warp remains simply outside the trusted fixture surface.
- The state-tool repository remains untouched.
- The provisional fractal `ui.salt` examples remain design evidence, not engine
  authority or an implementation dependency.
- Engine merge remains unauthorized without separate user approval.

## Review Gate

The Phase 1 evidence supports continuing to focused runtime witnesses and
authoring repair, but the exact repair order and scope require user review. No
Phase 2 product mutation begins from this checkpoint automatically.
