# Explaino RTK v3 Measurement Lane

## Current Phase

Complete - entry-profile RTK v3 measurement lane is landed and validated; next follow-on is the full-profile performance ratchet and broader family expansion

## Phase Checklist

- [x] Phase 1 - add the repo-local RTK v3 wrapper, isolated output root contract, and harvested-family manifests
- [x] Phase 2 - add focused tool tests for FITS discovery, harvest classification, and fake-tool end-to-end execution
- [x] Phase 3 - run a real hostile audit against the landed wrapper, repair at least one real defect if found, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - the repo now has a saved-runtime walker plus local trace export, but no isolated external measurement lane that stages FITS through `nine` Reality Toolkit v3 and normalizes the results for later `salts` consumption
  - the user clarified two important shape constraints for this slice:
    - use Reality Toolkit v3 only
    - treat both invariance PLY outputs as first-class harvest artifacts, not just one file
- Locked implementation stance for this slice:
  - keep `state.json` and the walker contracts unchanged; this is an external measurement lane, not a new replay authority
  - use `C:\Users\Adam\Desktop\b3\whatisthis\nine\scripts\utilities\prepare_fits_frame_groups.py` and `C:\Users\Adam\Desktop\b3\whatisthis\nine\scripts\batch_runners\run_reality_toolkit_unknown.py` as the primary v3 tools
  - write all generated outputs under an isolated root:
    - `D:\salt-output\results\explaino_rtk_v3\<run-stamp>`
  - emit machine-readable local contracts first:
    - `rtk_v3_run_manifest.json`
    - `rtk_v3_selection_manifest.json`
    - `rtk_v3_receipt.json`
    - `rtk_v3_harvest_summary.json`
  - make the lane easy to extend:
    - explicit harvest-family registry
    - explicit tool path overrides for tests and future plugin wiring
- Landed behavior in this slice:
  - new repo-local wrapper:
    - `tools/explaino_rtk_v3_measurement_lane.py`
  - isolated run-root contract under:
    - `D:\salt-output\results\explaino_rtk_v3\<run-stamp>`
  - normalized machine-readable outputs:
    - `rtk_v3_selection_manifest.json`
    - `rtk_v3_run_manifest.json`
    - `rtk_v3_receipt.json`
    - `rtk_v3_harvest_summary.json`
    - `rtk_v3_external_family_catalog.json`
  - entry-profile live smoke now succeeds against the real `nine` tools and harvests:
    - invariance summary/metadata/fields
    - both invariance-sculpture PLY outputs
    - external ghost/entropy/invariance catalogs from `D:\salt-output`
- Hostile-review repairs already folded into this slice:
  - the first live smoke failed because the wrapper seeded `PYTHONPATH` with the wrong root; `prepare_fits_frame_groups.py` needs `nine/scripts` so it can import `frame_io.py`
  - the second live smoke proved the `nine` runner does not preserve the wrapper's source dataset id; harvesting now reads actual runner dataset ids from `out_unknown_toolkit_eval_all.csv` before locating per-dataset output roots
- Bounded gap kept explicit:
  - a live `--profile full` pass on a real `salts` checkpoint exceeded the outer shell timeout during this slice
  - the wrapper therefore defaults to `entry` today, while keeping `full` available as an explicit slower second pass and harvesting external ghost/entropy families from existing `D:\salt-output` results
- Required harvested families in the first slice:
  - `ghost_plate`
  - entropy campaign summaries when discovered in existing external result roots
  - invariance summaries
  - invariance-sculpture outputs including:
    - `out_invariance_sculpture_stability.ply`
    - `out_invariance_sculpture_churn.ply`
- Explicit non-goals for this slice:
  - no direct coupling to `salts_session.py`
  - no new UI surface
  - no attempt to replace the `nine` runner with local reimplementations
  - no expansion to broader v3 batteries beyond the named-family harvest surface

## Validation

- focused tool pytest coverage for the new RTK v3 wrapper
- existing focused walker/runtime tests as needed if shared helpers move
- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_runtime_pytest_lane.py`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`

## Evidence / References

- `C:\Users\Adam\Desktop\b3\whatisthis\nine\scripts\utilities\prepare_fits_frame_groups.py`
- `C:\Users\Adam\Desktop\b3\whatisthis\nine\scripts\batch_runners\run_reality_toolkit_unknown.py`
- `C:\Users\Adam\Desktop\b3\whatisthis\nine\scripts\one_off_analyses\fits_invariance_sculpture.py`
- `C:\code\hat-rack-v2\salts\scripts\salts_session.py`
- `C:\code\cuda_newton_fractal_clone\tools\explaino_runtime_walk.py`
