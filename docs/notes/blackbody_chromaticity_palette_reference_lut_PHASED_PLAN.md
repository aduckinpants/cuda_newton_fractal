# Black Body Reference And LUT Slice

Status: ready to lock as Slice A. This slice is headless/reference-only and must not change viewer behavior.

## Explicit User Asks

- [x] Vendor the official CIE 1931 2-degree observer CSV and metadata with checksum and license provenance.
- [x] Implement the deterministic float64 Planck/CIE/XYZ/internal-linear-RGB reference generator.
- [x] Generate a checked-in 1024-entry reciprocal-temperature float32 LUT initializer and audit metadata.
- [x] Prove coordinate identities, independent chromaticity witnesses, linear-not-sRGB values, deterministic freshness, and bounded interpolation error.
- [x] Checkpoint, receipt, rearward-review, push, and stop before runtime integration.

## Current Phase

Slice A closure is active after the reference generator, 1024-entry LUT, provenance controls, and independent numerical court passed. No viewer/runtime file changed. Slice B runtime Palette integration is the next preplanned slice and requires a fresh viewer-first plan/contract.
## Phase Checklist

- [x] Lock Slice A plan/contract.
- [x] Add RED checksum/domain, coordinate, independent witness, determinism, freshness, and interpolation tests.
- [x] Vendor official CIE CSV/metadata and attribution.
- [x] Implement the exact reference generator and generated artifacts.
- [x] Run the complete focused Python court.
- [x] Complete three-pass hostile audit and repair findings.
- [x] Validate, checkpoint, receipt, rearward-review, push, and stop before Slice B.

## Exact File Surface

Expected files are `.gitattributes`, this plan/contract, the umbrella plan, docs/reference/blackbody_chromaticity contents, tools/blackbody_palette_generator.py, tests/test_blackbody_palette_generator.py, HANDOFF_LOG.md, and campaign artifacts. Directory scope is used only because contract validation rejects future file paths. This list is the semantic allowlist; unrelated edits are forbidden.

## Locked Reference Semantics

Use the umbrella plan exact CIE source/checksum, Planck c2 constant, trapezoidal integration, Y normalization, rational XYZ-to-linear-sRGB matrix, neutral-axis lift, peak normalization, clamp, reciprocal coordinates, and 1024 entries over 800 to 40000 K. The RED dense court proved 512 entries insufficient at the peak-normalization crossover (`0.005422` maximum error versus the locked `0.002` bound); 1024 entries preserve the runtime lowering and measure `0.000837` maximum error.

The generator must reject source mismatch, use float64 reference math, write deterministic float32 literals, record source hashes/constants/matrix/gamut statistics/six witnesses, and support explicit output paths.

## Independent Court

Tests must independently parse the CSV and encode at least three broad xy witnesses rather than obtaining every expectation from generator helpers. Required anchors include approximately 2856 K and 6500 K.

Required proof:

- exact CSV SHA-256 and 360..830 nm one-nm domain;
- q(800)=0, q(40000)=1, monotonicity, and all 1024 inverse-coordinate identities;
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
| Slice A lock | complete | active contract `blackbody_chromaticity_palette_reference_lut`; contract revised to 1024 entries after RED evidence |
| RED court | complete | initial missing generator failure; 512-entry dense court failed at `0.005422` against `0.002` bound |
| Official data | complete | official CIE CSV SHA-256 `fa663e...13c1`; canonical metadata SHA-256 `88a821...9774` |
| Generator/LUT | complete | 1024 entries; measured dense maximum channel error `0.000837`; 3 focused tests green |
| Closure | active | validators, checkpoint, receipts, rearward review, and push are the remaining mechanical sequence |
## Hostile Audit

- Status: complete
- Scope: source provenance, checksum and checkout stability, wavelength parsing, independent reference witnesses, Planck integration, exact matrix, gamut policy, reciprocal coordinate lowering, generated C syntax, float32 storage, metadata, and deterministic freshness.
- Result: six real issues were repaired. The third clean re-read passed the focused test court, canonical generation check, source attribute check, code-quality baseline, and diff check with no additional real issue found.

## Audit Passes

- [x] Pass 1 - implementation review found malformed generator string escaping and invalid integer-form C float literals; parsing and literal regressions now cover both.
- [x] Pass 2 - numerical/provenance review found the 512-entry table missed the locked error bound, metadata bytes were checkout-dependent, the CIE CSV needed binary tracking, and the xy court reused generator integration; the contract, attributes, canonical hash, and independent integration witness were repaired.
- [x] Pass 3 - clean re-read regenerated the artifacts, reran all focused tests, checked source attributes and canonical metadata, and found no additional real issue.

## Audit Findings

- [x] The first large guarded import patch joined the metadata terminator to the next diff header and mangled Python escape sequences. The import was split/repaired and `py_compile` now gates the generator.
- [x] A uniformly reciprocal 512-entry LUT measured `0.005422` maximum channel error at the peak-normalization crossover. The locked contract now uses 1024 entries, measuring `0.000837` while preserving the affine-coordinate/two-load runtime design.
- [x] Generated literals such as `0f` and `1f` were not portable C++ floating literals. `_float_literal` now emits `0.0f` and `1.0f`, with regression assertions.
- [x] Raw CIE metadata hashes varied with checkout line endings, and `core.autocrlf` could corrupt the official CSV identity on a clean checkout. Path-specific `.gitattributes`, official source SHA, and canonical JSON SHA now separate source authority from checkout formatting.
- [x] The first broad chromaticity witness called the generator integration helper. The test now independently parses and integrates the official observer data before comparing both frozen anchors and generator output.
- [x] The original linear-versus-sRGB discriminator used 6500 K, whose near-white channels differ by only `0.03169`. The fixed 3000 K witness gives a clear `0.27475` distinction without changing palette semantics.
## Stop Point

Stop after the reference/LUT checkpoint. Slice B runtime integration is preplanned but requires a fresh viewer-first contract; no runtime mutation is authorized by this plan.
