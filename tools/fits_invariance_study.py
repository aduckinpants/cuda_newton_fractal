"""FITS invariance field formal comparison study.

Produces:
  1. Field-by-field comparison table across all datasets
  2. Epsilon threshold sweep for persistence fields
  3. Normalization sensitivity analysis
  4. Per-channel decomposition for eigenvalue-encoding datasets
  5. Cross-correlation between fields
  6. Canonical mesh recipe recommendation data

Outputs to D:\salt-output\results\fits_invariance_study\
"""
import numpy as np
from astropy.io import fits
import json
import csv
import os
import time

CAMPAIGN_BASE = os.path.join(
    r"D:\salt-output\results\godel_fits_entropy_campaign",
    "full_round_20260407",
)
OUT_DIR = r"D:\salt-output\results\fits_invariance_study"

CORE_DATASETS = {
    "godel_g13": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g13", "frames_delta_stack.fits"),
    "godel_g4": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g4", "frames_delta_stack.fits"),
    "godel_g11": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g11", "frames_delta_stack.fits"),
    "godel_g12": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g12", "frames_delta_stack.fits"),
    "godel_g1_long": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g1_long", "frames_delta_stack.fits"),
    "godel_g10": os.path.join(CAMPAIGN_BASE, "core", "fits", "godel_g10", "frames_delta_stack.fits"),
}

HIRES_DATASETS = {
    "godel_g4_hires": os.path.join(CAMPAIGN_BASE, "expansion", "fits", "godel_g4_hires", "frames_delta_stack.fits"),
    "godel_g13_hires": os.path.join(CAMPAIGN_BASE, "expansion", "fits", "godel_g13_hires", "frames_delta_stack.fits"),
    "godel_g10_hires": os.path.join(CAMPAIGN_BASE, "expansion", "fits", "godel_g10_hires", "frames_delta_stack.fits"),
}


def reconstruct(path):
    """Load FITS delta stack and reconstruct full trajectory."""
    f = fits.open(path)
    kf = f[0].data.astype(np.float32)
    C, H, W = kf.shape
    T = len(f)
    traj = np.zeros((C, H, W, T), dtype=np.float32)
    traj[:, :, :, 0] = kf
    for t in range(1, T):
        traj[:, :, :, t] = traj[:, :, :, t - 1] + f[t].data.astype(np.float32)
    f.close()
    return traj


def compute_fields(traj):
    """Compute all derived invariance fields from a trajectory."""
    C, H, W, T = traj.shape
    deltas = np.diff(traj, axis=3)  # (C, H, W, T-1)

    fields = {}

    # Temporal variance (channel-mean)
    fields["variance"] = np.mean(np.var(traj, axis=3), axis=0)

    # Log variance
    fields["log_variance"] = np.log1p(fields["variance"])

    # Stability (1 - normalized variance)
    vmax = fields["variance"].max()
    fields["stability"] = 1.0 - fields["variance"] / (vmax + 1e-10)

    # Delta exact-zero fraction (all channels zero)
    fields["zero_frac"] = np.mean(np.all(deltas == 0, axis=0), axis=2)

    # Temporal range (max-min per channel, averaged)
    fields["temporal_range"] = np.mean(
        np.max(traj, axis=3) - np.min(traj, axis=3), axis=0
    )

    # Channel-mean trajectory for occupancy
    chan_mean = np.mean(traj, axis=0)  # (H, W, T)
    threshold = np.median(chan_mean)
    fields["occupancy"] = np.mean(chan_mean > threshold, axis=2)

    # Per-channel variance
    per_ch_var = np.var(traj, axis=3)  # (C, H, W)
    for c in range(C):
        fields[f"variance_ch{c}"] = per_ch_var[c]

    return fields, deltas


def field_stats(field):
    """Compute summary statistics for a scalar field."""
    return {
        "min": float(np.min(field)),
        "max": float(np.max(field)),
        "mean": float(np.mean(field)),
        "median": float(np.median(field)),
        "std": float(np.std(field)),
        "cov": float(np.std(field) / (np.mean(field) + 1e-10)),
        "p5": float(np.percentile(field, 5)),
        "p25": float(np.percentile(field, 25)),
        "p75": float(np.percentile(field, 75)),
        "p95": float(np.percentile(field, 95)),
        "iqr": float(np.percentile(field, 75) - np.percentile(field, 25)),
        "dynamic_range": float(
            (np.max(field) - np.min(field)) / (np.mean(field) + 1e-10)
        ),
    }


def epsilon_sweep(deltas, epsilons):
    """Sweep epsilon thresholds for persistence fields.

    deltas: (C, H, W, T-1)
    Returns dict mapping epsilon -> persistence field (H, W)
    """
    # L2 norm over channels
    delta_mag = np.sqrt(np.sum(deltas ** 2, axis=0))  # (H, W, T-1)
    results = {}
    for eps in epsilons:
        results[eps] = np.mean(delta_mag < eps, axis=2)
    return results


def normalization_sensitivity(field, methods):
    """Test different normalization strategies on a field.

    Returns dict of method_name -> (normalized_field_stats, correlation_with_raw).
    """
    results = {}
    raw_flat = field.flatten()

    for name, norm_fn in methods.items():
        normed = norm_fn(field)
        corr = float(np.corrcoef(raw_flat, normed.flatten())[0, 1])
        results[name] = {
            "stats": field_stats(normed),
            "correlation_with_raw": corr,
        }
    return results


def cross_correlate_fields(fields_dict):
    """Compute pairwise Pearson correlation between all fields."""
    names = sorted(fields_dict.keys())
    n = len(names)
    corr_matrix = {}
    flat = {k: fields_dict[k].flatten() for k in names}
    for i in range(n):
        for j in range(i + 1, n):
            r = float(np.corrcoef(flat[names[i]], flat[names[j]])[0, 1])
            corr_matrix[f"{names[i]} vs {names[j]}"] = r
    return corr_matrix


def run_study():
    os.makedirs(OUT_DIR, exist_ok=True)
    all_datasets = {**CORE_DATASETS, **HIRES_DATASETS}

    # === Part 1: Field-by-field comparison ===
    print("=== Part 1: Field-by-field comparison ===")
    comparison_rows = []
    all_fields_cache = {}

    for ds_name, ds_path in sorted(all_datasets.items()):
        t0 = time.time()
        print(f"  Loading {ds_name}...")
        traj = reconstruct(ds_path)
        C, H, W, T = traj.shape
        fields, deltas = compute_fields(traj)
        elapsed = time.time() - t0
        print(f"    {C}ch x {H}x{W} x {T}f in {elapsed:.1f}s")

        all_fields_cache[ds_name] = (fields, deltas, traj.shape)

        for fname in ["variance", "log_variance", "stability", "zero_frac",
                       "temporal_range", "occupancy",
                       "variance_ch0", "variance_ch1", "variance_ch2"]:
            if fname in fields:
                stats = field_stats(fields[fname])
                row = {"dataset": ds_name, "field": fname, **stats}
                comparison_rows.append(row)

    # Write comparison CSV
    comp_csv = os.path.join(OUT_DIR, "field_comparison.csv")
    keys = list(comparison_rows[0].keys())
    with open(comp_csv, "w", newline="") as f:
        w = csv.DictWriter(f, keys)
        w.writeheader()
        w.writerows(comparison_rows)
    print(f"  Wrote {comp_csv}")

    # === Part 2: Epsilon threshold sweep ===
    print("\n=== Part 2: Epsilon threshold sweep ===")
    epsilons = [0.1, 0.25, 0.5, 1.0, 2.0, 3.0, 5.0, 7.0, 10.0, 15.0, 20.0]
    sweep_rows = []

    for ds_name in ["godel_g13", "godel_g4", "godel_g10", "godel_g1_long"]:
        fields, deltas, shape = all_fields_cache[ds_name]
        eps_fields = epsilon_sweep(deltas, epsilons)
        for eps, ef in eps_fields.items():
            stats = field_stats(ef)
            sweep_rows.append({"dataset": ds_name, "epsilon": eps, **stats})
        print(f"  {ds_name}: swept {len(epsilons)} thresholds")

    sweep_csv = os.path.join(OUT_DIR, "epsilon_sweep.csv")
    keys = list(sweep_rows[0].keys())
    with open(sweep_csv, "w", newline="") as f:
        w = csv.DictWriter(f, keys)
        w.writeheader()
        w.writerows(sweep_rows)
    print(f"  Wrote {sweep_csv}")

    # === Part 3: Normalization sensitivity ===
    print("\n=== Part 3: Normalization sensitivity ===")
    norm_methods = {
        "raw": lambda f: f,
        "min_max": lambda f: (f - f.min()) / (f.max() - f.min() + 1e-10),
        "log1p": lambda f: np.log1p(f),
        "log1p_minmax": lambda f: (lambda g: (g - g.min()) / (g.max() - g.min() + 1e-10))(np.log1p(f)),
        "percentile_clip_1_99": lambda f: np.clip(
            (f - np.percentile(f, 1)) / (np.percentile(f, 99) - np.percentile(f, 1) + 1e-10),
            0, 1
        ),
        "percentile_clip_5_95": lambda f: np.clip(
            (f - np.percentile(f, 5)) / (np.percentile(f, 95) - np.percentile(f, 5) + 1e-10),
            0, 1
        ),
        "rank": lambda f: (lambda s: np.argsort(np.argsort(s)).reshape(f.shape) / (s.size - 1))(f.flatten()),
        "zscore": lambda f: (f - np.mean(f)) / (np.std(f) + 1e-10),
    }

    norm_rows = []
    for ds_name in ["godel_g13", "godel_g4", "godel_g10"]:
        fields, _, _ = all_fields_cache[ds_name]
        for fname in ["variance", "zero_frac", "occupancy"]:
            results = normalization_sensitivity(fields[fname], norm_methods)
            for method, data in results.items():
                norm_rows.append({
                    "dataset": ds_name,
                    "field": fname,
                    "method": method,
                    "corr_with_raw": data["correlation_with_raw"],
                    **{f"norm_{k}": v for k, v in data["stats"].items()},
                })
    norm_csv = os.path.join(OUT_DIR, "normalization_sensitivity.csv")
    keys = list(norm_rows[0].keys())
    with open(norm_csv, "w", newline="") as f:
        w = csv.DictWriter(f, keys)
        w.writeheader()
        w.writerows(norm_rows)
    print(f"  Wrote {norm_csv}")

    # === Part 4: Cross-field correlation ===
    print("\n=== Part 4: Cross-field correlation ===")
    corr_rows = []
    for ds_name in ["godel_g13", "godel_g4", "godel_g10", "godel_g1_long"]:
        fields, _, _ = all_fields_cache[ds_name]
        core_fields = {k: fields[k] for k in ["variance", "zero_frac", "occupancy",
                                                "temporal_range", "stability"]}
        corrs = cross_correlate_fields(core_fields)
        for pair, r in corrs.items():
            corr_rows.append({"dataset": ds_name, "pair": pair, "pearson_r": r})
    corr_csv = os.path.join(OUT_DIR, "field_cross_correlation.csv")
    keys = list(corr_rows[0].keys())
    with open(corr_csv, "w", newline="") as f:
        w = csv.DictWriter(f, keys)
        w.writeheader()
        w.writerows(corr_rows)
    print(f"  Wrote {corr_csv}")

    # === Part 5: Per-channel decomposition deep dive ===
    print("\n=== Part 5: Per-channel decomposition ===")
    ch_rows = []
    for ds_name in ["godel_g11", "godel_g12", "godel_g13"]:
        fields, _, _ = all_fields_cache[ds_name]
        # Cross-correlate per-channel variance
        for i in range(3):
            for j in range(i + 1, 3):
                fi = fields[f"variance_ch{i}"].flatten()
                fj = fields[f"variance_ch{j}"].flatten()
                r = float(np.corrcoef(fi, fj)[0, 1])
                ch_rows.append({
                    "dataset": ds_name,
                    "pair": f"ch{i}_vs_ch{j}",
                    "pearson_r": r,
                })
        # Compare channel variance CoV
        for c in range(3):
            stats = field_stats(fields[f"variance_ch{c}"])
            ch_rows.append({
                "dataset": ds_name,
                "pair": f"ch{c}_stats",
                "pearson_r": stats["cov"],  # reusing column for CoV
            })
    ch_csv = os.path.join(OUT_DIR, "per_channel_decomposition.csv")
    keys = list(ch_rows[0].keys())
    with open(ch_csv, "w", newline="") as f:
        w = csv.DictWriter(f, keys)
        w.writeheader()
        w.writerows(ch_rows)
    print(f"  Wrote {ch_csv}")

    # === Save master summary JSON ===
    summary = {
        "generated_utc": time.strftime("%Y-%m-%dT%H:%M:%S+00:00", time.gmtime()),
        "schema_version": "fits_invariance_study_v1",
        "datasets_analyzed": len(all_datasets),
        "core_datasets": list(CORE_DATASETS.keys()),
        "hires_datasets": list(HIRES_DATASETS.keys()),
        "fields_computed": ["variance", "log_variance", "stability", "zero_frac",
                           "temporal_range", "occupancy", "variance_ch0",
                           "variance_ch1", "variance_ch2"],
        "epsilon_values_swept": epsilons,
        "normalization_methods_tested": list(norm_methods.keys()),
        "artifacts": {
            "field_comparison": comp_csv,
            "epsilon_sweep": sweep_csv,
            "normalization_sensitivity": norm_csv,
            "field_cross_correlation": corr_csv,
            "per_channel_decomposition": ch_csv,
        },
    }
    summary_json = os.path.join(OUT_DIR, "fits_invariance_study_summary.json")
    with open(summary_json, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"\n  Wrote {summary_json}")
    print("Done.")


if __name__ == "__main__":
    run_study()
