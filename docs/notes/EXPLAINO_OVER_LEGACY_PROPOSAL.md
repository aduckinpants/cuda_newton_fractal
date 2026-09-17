# ExplainO over ExplainO-Legacy

**Proposal: a legacy-preserving, step-aligned disagreement fractal**
**Date:** 2026-09-17
**Proposed mode identifier:** `explaino_over_legacy`
**Status:** Product and semantic proposal; source inspection and implementation qualification remain to be done. Names introduced below are proposed contracts, not claims about existing APIs.

## 1. Product thesis

Bring the recovered, pre-`h(t)` integer-seeded ExplainO into the modern engine as a preserved execution path. Place modern `explaino_all` over that path as a switchable, step-aligned second half, and make their parameterized disagreement available as renderable fields and inspectable execution evidence.

The legacy construction remains itself. The modern construction remains itself. Their difference becomes the subject of the new fractal rather than a compatibility error to eliminate.

The defining user experience is:

> Step the legacy integer seed. Step modern ExplainO with it. Explore where and how their executions disagree. Disable the modern half and recover the old engine’s behavior exactly.

This is not an RGB blend, a screenshot difference filter, or a modern preset called “legacy.” It is a first-class paired evaluator with an explicit primary path, comparison path, observation map, and orientation.

## 2. Starting evidence and boundaries

The recovered seed-78 specimen is the first qualification anchor. Its saved state records `fractal_type: explaino`, seed `78`, `max_iter: 500`, epsilon approximately `1e-6`, zero warp, camera center `(0,0)`, `log2_zoom: 0`, and a `1024×768` render. Its original Joy exposure, saturation, contrast, and tint settings must travel with that reference. [S1]

The user has established that this build predates ExplainO `h(t)`. Therefore, the modern construction is not expected to reproduce its geometry merely because both receive the number 78.

Both supplied modern states identify themselves as `explaino_all`; they are modern witnesses, not independently verified evidence of two distinct modern execution paths. Their recorded camera, sampling, and grading differ from the legacy reference. Those differences must be normalized for controlled measurement, not mistaken for construction semantics. [S2] [S3]

The captures and JSON establish useful fixtures. They do not establish the legacy generator’s exact algorithm, arithmetic path, seed domain, or which serialized values actually control execution. Recover those facts from the old source and executable before freezing the implementation contract.

## 3. Composition and stepping semantics

### Keep three indices separate

Use `n` for the legacy integer seed, `k` for an orbit iteration at a fixed specimen, and `j` for an explicitly requested seed-sweep position. A render frame is not automatically any of these, and none is automatically the modern `h(t)` input.

For an integer sweep:

\[
n_j=n_0+j\,\Delta n,
\]

where `Δn` is integral and all visited seeds must lie within the qualified legacy domain. “Previous seed” and “next seed” change this discrete cursor. They do not silently interpolate, tween, or run legacy through modern `h(t)`.

The baseline paired policy advances both specimens at each accepted seed change. Initially, modern ExplainO receives the same numeric seed coordinate. An explicit modern-only offset or scale may subsequently parameterize that correspondence:

\[
u_j=u_0+r(n_j-n_0).
\]

The default is `u0 = n0`, `r = 1`. The existing modern resolver interprets `u`; this proposal does not replace or rederive its `h(t)` construction. A matching numeric input names a comparison policy, not a claim of equal generated content.

No wall-clock motion or automatic parameter mutation is implicit in this policy.

### Preserve two native execution paths

For a shared world-space starting sample `x`, resolve a legacy specimen `L_n` and a modern specimen `M_(u,θ)`, where `θ` contains the explicitly selected modern controls. Advance their full native states:

\[
\ell_{k+1}=L_n(\ell_k),
\qquad
m_{k+1}=M_{u,\theta}(m_k).
\]

Each branch uses its own correct initialization and auxiliary state. Sharing a starting coordinate does not imply that their entire internal states have identical layouts or meanings.

At each aligned iteration, publish an observation record containing the two branch states’ supported observables, their execution statuses, the seed correspondence, and the comparison orientation. Evaluate disagreement from this record, before color mapping.

A branch that has terminated does not acquire invented extra iterations so the other can catch up. Continuing the surviving branch is permitted, but every observation must identify whether it compares two live states, two terminal states, or a held terminal endpoint against a live state.

### Meaning of “over” in the first version

The legacy trajectory is the primary execution being observed; modern ExplainO supplies the comparison trajectory and, through focused probes, an alternative transition at a selected legacy state.

The composed evaluator publishes legacy, modern, and disagreement outputs. It does not feed disagreement back into the legacy recurrence by default. Feedback would define additional dynamics and should be a separately named future mode, not an accidental consequence of enabling observation.

## 4. Exact legacy recovery is a hard contract

Disabling the modern half selects the preserved legacy execution and presentation directly:

\[
\operatorname{RenderOverLegacy}(n,\text{modern enabled}=\text{false})
=
\operatorname{RenderLegacyReference}(n)
\]

under matched, qualified hardware/runtime, arithmetic, camera, sampling, and display conditions.

“Disabled” is a structural bypass, not a blend coefficient of zero. It must not execute the modern evaluator, resolve modern roots, allocate modern orbit buffers, apply comparison grading, or allow dormant observer state to modify the legacy result. The selected output returns to legacy rather than displaying a misleading all-zero disagreement image.

Exact recovery includes the integer-seed conversion and generation path; operative coefficients or roots; update arithmetic and operation order; initialization, stopping rules, and guards; root classification and ordering; pixel-to-world mapping; and the original Joy color/grading path. Recover the historical main-render behavior, and identify any old Mask/Lens behavior separately rather than silently substituting a modern lens.

Do not “improve” the historical numerical path while claiming exact preservation. A higher-precision or otherwise revised legacy variant can be useful, but it needs a distinct profile and receipt.

Qualification should distinguish:

- **Execution parity:** resolved inputs, selected intermediate states, terminal classifications, and iteration results agree under the pinned reference conditions.
- **Render parity:** decoded image pixels agree under the matched historical presentation profile. Different BMP/PNG file containers need not have identical file bytes.

A visually close result is not exact parity. Bitwise recovery outside the qualified hardware/compiler/arithmetic envelope is not promised. Within that envelope, a parity failure remains a blocking discrepancy to resolve, not a reason to quietly relax the requirement.

Seed 78 is the first anchor, not the whole test corpus. Include other seeds and viewports before claiming recovery of the supported legacy family.

## 5. Parameterized disagreement

Keep construction controls, correspondence controls, and observation controls separate. Changing modern dynamics changes the comparison specimen; changing the modern seed relation changes which specimens are paired; changing a projection, gain, or display palette changes how disagreement is measured or shown. None may silently mutate legacy dynamics.

### Accumulated orbit separation

Let `zL` and `zM` be the complex-coordinate observables of the two native states. At an aligned iteration:

\[
\Delta z_k=z^M_k-z^L_k.
\]

This measures separation after each branch has followed its own history. Publish the vector, magnitude, validity, and iteration alignment, not only a scalar heatmap.

For a declared unit direction `v = (vx,vy)` and its positive perpendicular `v⊥ = (−vy,vx)`:

\[
D_\parallel=\Delta z\cdot v,
\qquad
D_\perp=\Delta z\cdot v_\perp.
\]

These signed channels expose along-direction and cross-direction disagreement. A magnitude-only image cannot preserve that distinction. Direction at zero separation, or a tangent derived from a zero-length step, must be marked undefined rather than assigned an invented orientation.

### Local alternative-transition disagreement

At a selected legacy focus, ask a different question:

> Starting from this same situated input, what would the modern transition do instead?

Evaluate both transitions under a declared shared-input adapter and subtract their next-coordinate observables. This separates local rule disagreement from accumulated trajectory separation.

For modern modes with auxiliary history, momentum, or other state, the adapter must explicitly supply that state and report its origin. A common coordinate alone is insufficient. Where no valid adapter exists, report that focused comparison as unsupported; do not reset hidden state and label the result equivalent. Independently initialized paired-orbit comparison can still be supported.

### Other disagreement channels

Expose iteration-count differences, terminal-status pairs, convergence timing, and compatible native residuals as separate typed quantities. Residuals from different definitions are not directly subtractable merely because both are floats. Make selected-iteration, peak, and terminal readouts explicit reductions rather than conflating them.

Native root indices are categorical labels local to their own branch. Subtracting indices is meaningless, and testing equality is not a cross-construction basin comparison until a correspondence is declared. Show native root identities and positions; enable matched-basin disagreement only with a recorded mapping that preserves unmatched and ambiguous cases.

Do not collapse all of these into one obligatory “disagreement score.” A weighted scalar projection may be useful, but it must retain access to its components, weights, units, and validity masks.

## 6. Oriented interrogation tools

Orientation is more than an arrow on the image. An interrogation declares the ordered branches, coordinate frame, selected quantity, sampling direction or path, and iteration interval. The default subtraction orientation is **modern minus legacy**.

**Point/orbit probe.** Click a world-space sample and inspect the aligned native traces, local alternative transitions where supported, termination events, disagreement channels, and selected-step readbacks. Stepping backward through a recorded trace is history navigation, not a claim that the recurrence has a unique inverse.

**Directed transect.** Draw a line or curve through a beaded interface or central junction. Sample it in a declared direction and show signed separation, status changes, and valid root correspondences along the path. Keep the sample coordinates and direction in the result so opposite traversals remain interpretable.

**Local frame probe.** Choose world axes, view axes, a user direction, or a supported orbit tangent/normal. Transform vectors into that declared frame before projection. Camera motion must not silently alter a world-oriented measurement; view-oriented measurements should transform as specified.

**Integer-seed strip and parameter intervention.** Hold the camera and query fixed while traversing an explicit integer-seed sequence. Separately vary a modern control or a declared direction through modern parameter space. Adjacent integer seeds produce a discrete response, not an assumed derivative or a smooth legacy path. Keep seed progression, modern construction input, and orbit iteration independently inspectable.

**Region summary.** Select a region, mask, or boundary neighborhood and request distributions, first-threshold events, peak-separation iterations, or a seed-by-iteration view. Preserve valid-sample counts and status populations so failed or terminated samples do not become false agreement.

These should be registered operations available through both the engine’s diagnostic UI and its agentic state tools. A query should travel from selected coordinates to native evaluation, typed result, view, and reproducible receipt—not require an agent to invent a one-off implementation each time.

## 7. Rendering and authoring surface

Add the new mode without replacing ordinary `explaino_all`. Its initial control groups should be:

| Group | Responsibility |
|---|---|
| Legacy anchor | Qualified legacy profile, integer seed, original settings, previous/next seed. |
| Modern half | Enable switch, explicit seed correspondence, existing modern construction controls. |
| Observation | Quantity, ordered comparison, frame, iteration selection, reductions, normalization. |
| Presentation | Legacy, modern, or disagreement source; applicable palette and grading. |
| Interrogation | Focus, directed path, region, seed range, parameter intervention, capture. |

Provide legacy-only and modern-only inspection views alongside disagreement views. Useful disagreement sources include separation magnitude, signed parallel/transverse displacement, displacement direction with validity, and status or matched-basin disagreement.

The sources belong upstream of the existing color pipeline. Changing a palette should not regenerate a specimen or change probe values. Color nodes must consume runtime observations, not contain second implementations of legacy or modern dynamics.

The exact-legacy presentation profile must remain selectable even while the modern half is collecting observations. This makes it possible to verify that turning observation on has not disturbed the primary path. Every exposed control needs a declared consumer and an end-to-end binding test; inactive or unsupported controls must be identified rather than silently ignored.

## 8. Runtime, capture, and authority

Reuse the modern engine’s actual `explaino_all` implementation. Recover and freeze one authoritative legacy implementation, rather than maintaining independent UI, CUDA, exporter, and Python reconstructions of its semantics. Share compatible plumbing where it preserves behavior; do not force both generators through a common resolver that changes either construction.

Normal rendering should use bounded per-sample state and selected accumulators. Full iteration histories belong to selected probes or explicitly budgeted captures, not an unconditional `width × height × max_iter` allocation. Report the actual extra work performed by paired rendering and focused probes; do not assume a fixed performance multiplier.

Python/RTK analysis may consume captured native results, but it must not become a separate product execution authority.

A saved state must record the legacy profile, modern enablement, seed correspondence, modern configuration, observation definition, orientation, and presentation selection. Record resolved specimen identities for evaluated branches. A disabled modern branch may retain dormant configuration and an explicitly historical receipt, but it must not claim a freshly resolved identity or execute merely to populate metadata. Receipts must also record build/backend identity, actual arithmetic, world sampling and AA policy, termination alignment, adapters or root mappings used, and the origin of authoritative versus derived values.

Capture raw typed signals before colorization with validity/status information. Use an existing native numeric export path or an explicitly versioned addition where needed. Do not reconstruct authoritative numeric disagreement from PNGs, and do not promise FITS support until the relevant path exists and is qualified.

For every seed-sweep capture, record the actual starting integer seed, step, sweep position, and modern resolved input. For an orbit excerpt, also record its starting iteration. A timestamp or filename alone is not sufficient.

## 9. Delivery ladder and acceptance gates

### Gate A — Recover the legacy reference

Pin the recovered source revision, executable, build configuration, and seed-78 fixture. Trace the operative seed-to-render path, including what serialized coefficients, tween/phase fields, and display settings actually do. Define the supported seed and arithmetic domain. Add a small deterministic reference corpus.

**Exit:** reproducible legacy fixtures and a source-grounded execution contract, without assuming modern `h(t)` semantics.

### Gate B — Exact legacy inside the modern engine

Implement the preserved path and historical presentation profile. Prove execution and decoded-pixel parity against the pinned reference. Verify save/load, repeated renders, and process restarts within the qualified envelope.

**Exit:** selecting the new mode with its modern half disabled reproduces the reference. Dormant modern and observation settings do not change that output or cause modern execution.

### Gate C — Step-aligned composition

Add modern specimen resolution, shared seed progression, shared world sampling, native branch stepping, explicit termination alignment, and raw disagreement output. Prove modern branch equivalence to standalone `explaino_all` at the same modern settings and inputs.

**Exit:** enabling the observer preserves the legacy result while producing a correctly identified modern witness and deterministic observations. Comparing genuinely identical branch executions gives the appropriate zero-disagreement null. Legacy versus modern is not presumed to be a null.

### Gate D — Oriented interrogation and presentation

Add typed color sources, point/orbit probes, directed transects, declared frames, and integer-seed strips. Support focused alternative-transition probes for an explicitly declared initial set of modern profiles; reject unsupported adapters truthfully.

**Exit:** reversing a declared projection direction reverses its signed components and preserves its magnitude; valid orthonormal components reconstruct the vector norm within the declared tolerance. Frame transformation, invalidity handling, and terminal alignment pass their own tests. Observation-only changes leave both native trajectories unchanged.

### Gate E — Reproducible exploration

Integrate saved queries, raw captures, replay, and agentic invocation. Run a bounded exploration of seed 78, adjacent integer seeds, selected additional seeds, and a few explicit modern interventions. Record the cost of each supported operation.

**Exit:** a saved finding reproduces not only an image, but the paired constructions, oriented query, raw observations, and presentation that produced it.

## 10. Definition of success

Open the new mode and turn the modern half off: the recovered integer-seed engine is there, exactly, under its qualified profile.

Turn it on: the legacy execution remains intact, modern ExplainO advances with it, and their disagreement becomes a field you can render, orient, probe, and replay.

Select one of the interleaved boundaries: the tool can distinguish accumulated separation from a local alternative transition, show which direction the difference points, and identify which changes came from the integer specimen, the modern construction, or the observation.

That supplies concrete evidence for later interleaving or zipper investigations without assuming that a visual resemblance already establishes a formal construction.

**Preserve the original dynamics; make the relationship to modern ExplainO a new, inspectable fractal.**

---

## Source fixtures

These are the uploaded artifacts inspected for this proposal. They are starting evidence, not a substitute for the source/build qualification in Gate A.

**[S1]** `state(20260917-161915).json` — recovered legacy saved state. SHA-256: `a48aada992e49fa5238671afb41d340208f1013e99ab7fe98ca909eef65b30fd`.

**[S2]** `state(20260917-161959).json` — modern saved state, recording `fractal_type: explaino_all`. SHA-256: `a81a58966bdc36ce9bb8ed85ca6382cfcaf0c016c48deaf2e1ef89f6007e4763`.

**[S3]** `state(20260917-162004).json` — second modern saved state, also recording `fractal_type: explaino_all`. SHA-256: `058660f01fd45ba60e7edf929c0f3f1d5197594cbf174973cf45828e2d682c54`.

Legacy image: `frame.bmp`, decoded as `1024×768 RGB`. File SHA-256: `792673bb4dd42f34f1cbedcaea2dd68fa0a378cbf7e1841b6544fab8abd3551a`.

Modern images: `frame(20260917-161957).png` and `frame(20260917-162004).png`, each decoded as `2048×1280 RGBA`. Both files have SHA-256 `dad89c6d87d912a74ee09d8efd6c8d10db27e1428cd024eacb0f61f09646cf7c`; their decoded pixel arrays are identical. This does not establish that two distinct modern evaluator paths ran.

---

## Repository integration note

This proposal was imported from the operator-supplied document on 2026-09-17 as design input. It is not an instruction surface, and its proposed names are not claims about current code. The source fixtures listed above were not present in the import bundle and are prerequisites for the first implementation campaign rather than checked-in evidence.

The repo-grounded review, authority decisions, bounded implementation ladder, and proof requirements live in `docs/notes/explaino_over_legacy_PHASED_PLAN.md`. That plan becomes implementation authority only after a future review and goal start; this proposal remains the product thesis and source-fixture inventory.
