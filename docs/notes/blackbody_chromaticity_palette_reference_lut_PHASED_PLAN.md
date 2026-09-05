# Black Body Reference And LUT Slice

Status: ready to lock as Slice A. This slice is headless/reference-only and must not change viewer behavior.

## Explicit User Asks

- [open] Vendor the official CIE 1931 2-degree observer CSV and metadata with checksum and license provenance.
- [open] Implement the deterministic float64 Planck/CIE/XYZ/internal-linear-RGB reference generator.
- [open] Generate a checked-in 512-entry reciprocal-temperature float32 LUT initializer and audit metadata.
- [open] Prove coordinate identities, independent chromaticity witnesses, linear-not-sRGB values, deterministic freshness, and bounded interpolation error.
- [open] Checkpoint, receipt, rearward-review, push, and stop before runtime integration.

## Current Phase

Slice A RED and implementation. Umbrella planning is closed at 9ac0529. No viewer/runtime file may change in this slice.

## Phase Checklist

- [ ] Lock Slice A plan/contract.
- [ ] Add RED checksum/domain, coordinate, independent witness, determinism, freshness, and interpolation tests.
- [ ] Vendor official CIE CSV/metadata and attribution.
- [ ] Implement the exact reference generator and generated artifacts.
- [ ] Run the complete focused Python court.
- [ ] Complete three-pass hostile audit and repair findings.
- [ ] Validate, checkpoint, receipt, rearward-review, push, and stop before Slice B.

## Exact File Surface

Expected files are this plan/contract, the umbrella plan, docs/reference/blackbody_chromaticity contents, tools/blackbody_palette_generator.py, tests/test_blackbody_palette_generator.py, HANDOFF_LOG.md, and campaign artifacts. Directory scope is used only because contract validation rejects future file paths. This list is the semantic allowlist; unrelated edits are forbidden.

## Locked Reference Semantics

Use the umbrella plan exact CIE source/checksum, Planck c2 constant, trapezoidal integration, Y normalization, rational XYZ-to-linear-sRGB matrix, neutral-axis lift, peak normalization, clamp, reciprocal coordinates, and 512 entries over 800 to 40000 K.

The generator must reject source mismatch, use float64 reference math, write deterministic float32 literals, record source hashes/constants/matrix/gamut statistics/six witnesses, and support explicit output paths.

## Independent Court

Tests must independently parse the CSV and encode at least three broad xy witnesses rather than obtaining every expectation from generator helpers. Required anchors include approximately 2856 K and 6500 K.

Required proof:

- exact CSV SHA-256 and 360..830 nm one-nm domain;
- q(800)=0, q(40000)=1, monotonicity, and all 512 inverse-coordinate identities;
- two temporary generations are byte-identical and match checked-in artifacts;
- finite bounded LUT channels;
- 4096-temperature full-pipeline interpolation maximum channel error at most 0.002;
- six metadata witnesses include XYZ, xy, pre-gamut RGB, lift, final RGB, and LUT RGB;
- neutral-lift count, maximum, and maximum temperature are recorded;
- nontrivial values match internal linear RGB and differ from display-encoded sRGB.

## Action Hostile Review

- Action ID: action-20260905-blackbody-slice-a-red-and-generator-1
- Suspected Failure Mode: deterministic output can still be wrong through bad source bytes, wavelength order, reciprocal orientation, matrix rounding, gamut normalization, float formatting, or shared implementation assumptions.
- Correct Owner/Action: freeze source checksum first, encode independent broad xy witnesses outside generator helpers, then implement and require byte freshness plus dense reference comparison.
- Proof Surface: provenance, RED tests, independent witnesses, 4096-point accuracy, metadata, code-quality baseline, hostile audit, receipts, rearward review, and clean remote state.
- Blocked Action: any ColorPalette enum, viewer runtime, CUDA sampler, UI-Salt, state/capture, recipe, runtime publish, screenshot, or performance mutation.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Umbrella planning | complete | 9ac0529, receipts and rearward review ok |
| Slice A lock | pending | plan/contract not checkpointed |
| RED court | pending | no Slice A tests |
| Official data | pending | not vendored |
| Generator/LUT | pending | not implemented |
| Closure | pending | validation/audit/receipts/push |

## Hostile Audit

- Status: pending
- Required posture: assume deterministic output can still be scientifically or numerically wrong.

## Audit Passes

- [open] Pass 1 - inspect source provenance, checksum, wavelength parsing, and independent witness separation.
- [open] Pass 2 - inspect integration, matrix, gamut, reciprocal coordinates, float32 lowering, and metadata.
- [open] Pass 3 - re-read repaired state, rerun freshness/accuracy, and inspect final closure truth.

## Audit Findings

- [ ] No findings recorded yet.

## Stop Point

Stop after the reference/LUT checkpoint. Slice B runtime integration is preplanned but requires a fresh viewer-first contract.
