# Digit Scrub POC Reference Snapshot

Status: reference-only evidence for the Parameter Motion planning campaign. Nothing in this directory is compiled, linked, imported, or executed by the viewer.

## Provenance

- Source checkout: `C:\Users\Adam\Documents\New project\digit-scrub-poc`
- Source base commit: `cb5710f9a4aaa3f02b0149d2cbde235906fbbc2d`
- Snapshot date: 2026-08-25
- Source working tree: dirty; this snapshot records the current working files, not a clean commit.
- Authorization: operator-owned local POC copied into this repo for internal design and test reference.
- License: no license file was present in the source checkout. Treat this snapshot as internal reference material; do not redistribute or make it a runtime/build dependency without an explicit licensing decision.

## Copied Files

| Snapshot path | Source SHA-256 | Purpose |
|---|---|---|
| `README.source.md` | `B8735BAEA639D620FF155FCF4AED9F1F817ECAF4245965785B27BCDD7EFD427D` | Original metadata and interaction goals. |
| `src/model/digitValue.ts` | `88CD7F11170BA0D82D4BEF81A070C20B33AB208C57B7F3775DB7617517BAE682` | Deterministic place decomposition and delta application. |
| `src/model/digitValue.test.ts` | `0A20ADA054A4037858C988B902968B34BEF416FDDF81E1C1CEC7A05F5B0C5074` | Model edge cases and exact behavior. |
| `src/model/scrubProfiles.ts` | `01289E8E54949D9AA6BC835D5E27642583BBC230D06066524DE5334CAD818BC5` | Deterministic modifier/profile policy. |
| `src/model/scrubProfiles.test.ts` | `0B60F8D138F2DA143D736DD4D292370C6B88205190EBC3B86330BAB8FF68F45E` | Modifier/profile tests. |
| `src/runtime/controlMode.ts` | `D1100582433AAF5114C4F1509241FB8FB571B731B3DD093F1E7D4CFE13769EC1` | Separation of display/edit authority from runtime-bound live authority. |
| `src/runtime/controlMode.test.ts` | `AD3DA7509367E42F5408AB9F8B2809F221CA44D962C4B10EA5D37D66A4488295` | Ownership-mode tests. |
| `src/main_focus_drag_excerpt.ts` | source `src/main.ts` lines 2962-3140; full source hash `7BFBD1CD173E1124C889D6016885CFFE20DBAB4EC7C0DC400C6C71DC78921BD5` | Focus-owned keyboard and pointer gesture reference. |
| `docs/ast-runtime-repair-postmortem-2026-04-25.md` | `01262C7245D64B5F64EC34413C6F4B7B0F733C773AE001137DAB1CC0798848AE` | Evidence that scrub-safety metadata must not double as runtime binding/commit authority. |

Line endings were normalized by the repo patch path. Hashes above identify the source files, not normalized snapshot bytes.

## Decision Map

### Keep As Behavior

- Keyboard mutation belongs to the focused scrub control, never to a global key poll.
- Pointer drag has explicit begin/update/end ownership and gesture-local residual state.
- Scrub quantum/modifier calculation is pure and deterministic.
- Display/edit capability and runtime mutation authority are separate facts.
- Mutation operations are tested independently from UI rendering.

### Adapt For The Viewer

- Use schema-owned canonical numeric step times a decimal exponent instead of rendering one DOM cell per digit.
- Use ImGui focus/active-item identity and in-process key events instead of browser DOM events.
- Use typed authoritative setter/applicator routes instead of text/source patch callbacks.
- Reuse viewer interaction pacing and settled-render behavior.
- Keep a compact Parameter Motion panel rather than the POC's full digit-strip UI.

### Defer

- Arbitrary numeric bases and exact rational editing.
- Nonlinear scrub profiles beyond the fixed v1 modifier rule.
- Source/AST patch generation.
- Runtime broker/websocket integration.
- Color Pipeline or graph animation.
- Multiple simultaneous tracks, timelines, and keyframes.

### Reject For Parameter Motion V1

- Treating `safe_to_scrub` as proof that a binding resolves or may commit live state.
- Retaining raw pointers or stale UI item identity across frames.
- Global arrow-key mutation.
- Selecting a target as an activation command.
- Importing TypeScript/browser code into the C++ viewer runtime.

## Reference Rule

Future implementation may reproduce these behaviors in native viewer code and tests. It must not cite the POC snapshot as product proof. Product proof comes from native unit tests and the published no-mouse viewer path.
