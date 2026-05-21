# Multibrot Exponent QoL Plan

## Current Phase

Branch 1 closed: Multibrot real/complex exponent QoL is implemented and validated. Branch 2 resolution/pacing remains a separate future branch and plan.

## Phase Checklist

- [x] Phase 1 - open and lock a bounded Multibrot exponent slice on `codex/qol-multibrot-exponent`
- [x] Phase 2 - RED: prove current real exponent rejects below 2 / above 12 and schema lacks complex exponent
- [x] Phase 3 - Green slice 1: real exponent hard range `[0.01,32]`, UI range `[0.01,12]`, logarithmic slider
- [x] Phase 4 - Green slice 2: add `multibrot_power_imag` and principal-branch complex exponent math while preserving zero-imag real behavior
- [x] Phase 5 - validate native/runtime rails, hostile audit, checkpoint, receipts, push, and merge to `master`

## Explicit User Asks

- [done] Branch 1 must lower real Multibrot exponent below `2`.
- [done] Branch 1 must allow above-old-cap real values through hard validation up to `32`.
- [done] Branch 1 must add bounded complex exponent support on the existing `multibrot` lane.
- [done] Do not add a new `FractalType`.
- [done] Use no physical mouse automation.
- [done] Branch 2 resolution/pacing work was not started in this branch and must use its own branch/plan after Branch 1 is on `master`.

## Proof Ledger

- Bootstrap: complete on clean repo at `8b47be5`; previous active contract was the closed merge/pause contract.
- Branch setup: current branch renamed to `codex/qol-multibrot-exponent` from the earlier planning branch.
- RED real range proof: `qol_multibrot_exponent_red_native_full_entry.log` failed before implementation because the requested `multibrot_power_imag` surface did not exist.
- RED complex exponent proof: the first native rail failed compiling `test_fractal_runtime_validation.cpp` with no `KernelParams::multibrot_power_imag`, proving the schema/runtime surface was absent.
- First green proof: `qol_multibrot_exponent_native_3.log` reached `All helper tests passed`; the wrapper timed out before child completion, so a clean final native command remained required.
- Focused renderer proof: `qol_multibrot_exponent_native_renderer.log` passed with `test_fractal_renderer: passed=76 failed=0`.
- Clean full native proof: `qol_multibrot_exponent_native_final.log` passed with `All helper tests passed` and wrapper exit 0.
- Runtime publish proof: `qol_multibrot_exponent_runtime_publish_final.log` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Published no-mouse proof: `qol_multibrot_exponent_runtime_pytest_final.log` passed `tests/test_fractal_runtime_multibrot_exponent.py` with one in-process automation test and no physical mouse path.

## Hostile Audit

- Status: done
- Questions before closeout:
  - Does real Multibrot validation accept `0.01`, `1.5`, `12`, and `32`?
  - Does real Multibrot validation reject `0`, negative real power, and values above `32`?
  - Does schema expose hard max `32` but normal widget max `12`?
  - Does imaginary exponent default to `0` and preserve the existing real-power path?
  - Does nonzero imaginary exponent actually change sample/render output?
  - Did this stay on the existing `multibrot` lane without physical mouse automation?
  - Is Branch 2 still untouched?

## Audit Passes

- [done] Pass 1 - review schema/binding/runtime validation for inconsistent exponent bounds.
- [done] Pass 2 - review formula math for nonfinite/overflow handling and zero-imag compatibility.
- [done] Pass 3 - clean re-read of runtime test, diff scope, branch-2 boundaries, and no-mouse proof found no additional real issue.

## Audit Findings

- [done] Audit finding 1: direct formula coverage could pass even if the imaginary exponent silently fell back to the real-only path; tightened the helper and step assertions to compare zero-imag and nonzero-imag behavior directly.
- [done] Audit finding 2: binding docs still advertised an integer-only Multibrot surface; updated them to document real and imaginary exponent controls plus the legacy alias boundary.
- [done] Audit finding 3: the contract listed an invalid multi-target native helper invocation; replaced it with the supported focused renderer target plus the full native helper.
- [done] Clean re-read confirmed Branch 1 scope only: no resolution/pacing files changed, no physical mouse automation was added, and no stale closeout wording remains in this plan.
