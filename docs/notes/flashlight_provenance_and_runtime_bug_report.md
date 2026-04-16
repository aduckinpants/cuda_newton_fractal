# Flashlight Provenance And Runtime Bug Report

## Scope

This note records the provenance and regression findings for the historical `FlashlightProbe` feature family, with emphasis on why the currently published runtime at:

- `D:\salt-fractal\cuda_newton_fractal_clone\runtime`

does not expose flashlight/bridge behavior even though the feature did originate in this repo's history.

## Executive Summary

The flashlight-bearing code did come from this repo.

The key correction is that the feature still exists in this repo's object graph on:

- `origin/feature/seed-refactor`
- commit tip `5fe0339f6867ef48837679da4b1c94409f0a9f38`

But the active published runtime is built from the current viewer-host line, not from that historical flashlight branch. Those two lines share the same root commit:

- merge-base: `d997f3f847ef32ae1f380ea8798bcf475947b85a`

and have diverged heavily since then.

Measured branch divergence at time of review:

- `git rev-list --left-right --count origin/feature/seed-refactor...HEAD`
- result: `3    270`

Interpretation:

- the flashlight branch is only three commits away from the common root
- the current viewer-host line has ~270 commits on a different descendant path
- flashlight was not proven to be removed by a later commit in the current line
- instead, the current publish path simply follows a branch that never merged the flashlight implementation

## Proof That Flashlight Lived In This Repo

Current repo remote:

- `origin https://github.com/aduckinpants/cuda_newton_fractal.git`

Historical flashlight branch visible in this repo:

- `origin/feature/seed-refactor`

Historical flashlight commits present in this repo object graph:

- `cbfe6a2` `updates`
- `3180247` `initial`
- `5fe0339` `ui tweak`

The current repo can show flashlight-bearing source directly from that branch without leaving this repo:

- `git show origin/feature/seed-refactor:ui_app/src/main.cpp`

Measured source size:

- flashlight branch `ui_app/src/main.cpp`: `5558` lines, `282184` characters
- current branch `ui_app/src/main.cpp`: far smaller, no flashlight references

Direct flashlight strings present in `origin/feature/seed-refactor:ui_app/src/main.cpp`:

- `--flashlight-probe`
- `--flashlight-live`
- `flashlight_probe.json`
- `flashlight_bridge_request.json`

So the claim "the copy in `salticid-cuda` came from this repo" is consistent with the git evidence. The relevant code lineage is real; it is just not on the active publish branch.

## Why The Published Runtime Lacks Flashlight

The runtime deployment metadata at:

- `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui_repo_root.txt`

points to:

- `C:\code\cuda_newton_fractal_clone`

This proves the publish destination is fed by the current repo and current build script.

Current publish script:

- `C:\code\cuda_newton_fractal_clone\ui_app\build_vsdevcmd.cmd`

That build line publishes the current viewer-host runtime into:

- `D:\salt-fractal\cuda_newton_fractal_clone\runtime`

The current branch source tree does not include the flashlight implementation surface, so the publish destination cannot expose flashlight behavior.

Observed runtime symptoms from the published runtime:

- `fractal_ui.exe` in the published runtime did not expose flashlight strings
- `diagnostics/last` did not regenerate flashlight artifacts
- flashlight bridge files were absent

This is therefore a branch-lineage mismatch, not an imaginary feature and not a false artifact.

## Bisect Result: No Single Removal Commit Proven On The Current Line

A narrow `git log -S "--flashlight-probe" -- ui_app/src/main.cpp` search points at:

- `cbfe6a2` `updates`

That means the flashlight CLI was introduced on the old flashlight branch.

The current line and the flashlight line diverge from `d997f3f`. The available evidence in this repo does not show a later commit on the current line that removed flashlight after inheriting it. The more accurate statement is:

- flashlight was added on one historical branch
- the modern viewer-host line evolved on another branch
- the active runtime publish follows the modern line

## Seed-Refactor Runtime Recovery Attempt

To determine whether flashlight is only missing from the active publish line or also broken in its own historical branch, an isolated worktree was created at:

- `C:\code\_flashlight_seed_refactor`

based on:

- `origin/feature/seed-refactor`

### Initial run result

The checked-in `ui_app/fractal_ui.exe` and a first rebuild attempt did not regenerate flashlight outputs. Using `Start-Process -Wait` showed the app was parsing the CLI and exiting with code `1`.

Updated diagnostic file:

- `C:\code\_flashlight_seed_refactor\ui\diagnostics\last\cli_debug.json`

Recorded error:

- `C:\code\_flashlight_seed_refactor\ui\last_ui_error.json`

Content:

- `stage = render.cuda`
- `failing_path = RenderFractalCUDA`
- `detail = CUDA kernel launch failed`

So the flashlight path was executing, but the render failed before the artifact bundle could be rewritten.

## Build Regression Found During Recovery

Both the current viewer-host build script and the old seed-refactor build script hardcode:

- `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat`

On this machine that path exists, but it does not produce a usable `cl.exe` toolchain in the session.

Observed build failure from both scripts:

- `cl.exe not found after VsDevCmd`

Meanwhile, the working MSVC toolchain is installed under:

- `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`

Verified working compiler:

- `cl.exe` from `VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe`

This is an independent build-tooling regression that now affects reproduction for both the current line and the historical flashlight branch.

## CUDA Architecture Regression Found During Recovery

The historical seed-refactor build script compiled only:

- `sm_120`
- `sm_121`

The current viewer-host build script, by contrast, includes:

- `sm_86`
- `sm_120`
- `sm_121`

A reproduction helper was created for investigation only:

- `C:\code\cuda_newton_fractal_clone\artifacts\build_flashlight_seed_refactor_2022.cmd`

The first 2022-based rebuild preserved the historical architecture list and still failed at runtime with:

- `CUDA kernel launch failed`

After adding:

- `-gencode=arch=compute_86,code=sm_86`

to the seed-refactor rebuild, the flashlight headless probe started working again.

## Fresh Probe Reproduction Result

Seed used for the successful recovery run:

- `C:\code\cuda_newton_fractal_clone\artifacts\flashlight_seed_prompt_alt.txt`

Command shape used:

- `fractal_ui_2022.exe --flashlight-probe <seed> --flashlight-ticks 9 --flashlight-radius 0.61 --flashlight-zoom-radius 0.19 --no-messagebox`

Recovered runtime result after the `sm_86` rebuild:

- process exit code: `0`

Freshly rewritten artifacts:

- `C:\code\_flashlight_seed_refactor\ui\diagnostics\last\flashlight_probe.json`
- `C:\code\_flashlight_seed_refactor\ui\diagnostics\last\frame.bmp`
- `C:\code\_flashlight_seed_refactor\ui\diagnostics\last\lens_sdf.bmp`
- `C:\code\_flashlight_seed_refactor\ui\diagnostics\last\state.json`

Measured regenerated probe fields:

- `conversation_seed32 = 1305844223`
- `ticks = 9`
- `radius = 0.61000001`
- `zoom_radius = 0.19000000`
- `fractal_type = explaino_fp`

Observed first trace sample from the regenerated JSON:

- `camera.center_hp_x = -0.79445844`
- `camera.center_hp_y = -0.65117155`
- `camera.log2_zoom = -0.13857958`
- center `signed_px = 53.84062576`
- center `inside = false`

This is enough to say the historical flashlight headless probe still works today on this machine once the CUDA architecture target is corrected.

## Why A New STL Did Not Yet Appear

Even after the successful recovered headless run, these historical artifacts did not change:

- `flashlight_trace.stl`
- `flashlight_trace_overlay.bmp`

Interpretation:

- the headless `--flashlight-probe` path rewrites the probe JSON and proof bundle
- the trace/STL export is not produced by that headless path directly
- the app source clearly exposes a bridge producer:
  - writes `flashlight_bridge_request.json`
  - reads `flashlight_bridge_prefill.json`
  - reads status from `flashlight_bridge_status.json`
- but the trace/STL consumer/exporter still appears to be external watcher-side logic rather than code currently found in the app source tree

The old diagnostics contain evidence that such a watcher/export path existed:

- `flash_prefill_000.json` ... `flash_prefill_011.json`
- `flashlight_trace_lines.obj`
- `flashlight_trace.stl`
- `flashlight_trace_overlay.bmp`
- `flashlight_hashes.json`

But the exporter itself was not located in the reviewed repo surfaces during this slice.

## Current Conclusions

1. The flashlight feature really did originate in this repo.
2. The current deployment runtime lacks flashlight because the active publish branch is a different descendant line that never merged the historical flashlight branch.
3. Reproduction was also blocked by a separate environment/tooling regression:
   - both old and current build scripts assume a VS 2026 toolchain that does not provide `cl.exe` here
4. The historical flashlight runtime itself was additionally broken on this machine until `sm_86` was added to the CUDA build.
5. After that CUDA-target correction, the headless flashlight probe worked again and regenerated fresh JSON/frame/SDF/state outputs.
6. Fresh STL generation is still unresolved because STL export appears to depend on an external bridge consumer / watcher path that has not yet been recovered in this repo.

## Next Useful Steps

1. Patch the current repo build rails to locate an installed MSVC toolchain instead of hardcoding the VS 2026 Community path.
2. Preserve the historical flashlight branch as a formal provenance reference in docs so it is not mistaken for a fabricated feature again.
3. Recover or reimplement the bridge consumer that turns:
   - `flashlight_bridge_request.json`
   - headless probe bundles
   - optional prefill/status files
   into:
   - `flashlight_trace_overlay.bmp`
   - `flashlight_trace_lines.obj`
   - `flashlight_trace.stl`
4. If reintegration is desired, lift the flashlight engine seams from `origin/feature/seed-refactor` into the current viewer-host line instead of trying to drive the deployment path from the obsolete branch directly.
