"""Read and format the invariance study results."""
import csv
import json
import os

BASE = r"D:\salt-output\results\fits_invariance_study"


def fmt(val, width=8, decimals=3):
    return f"{float(val):{width}.{decimals}f}"


def read_csv(name):
    with open(os.path.join(BASE, name)) as f:
        return list(csv.DictReader(f))


def section(title):
    print(f"\n{'=' * 70}")
    print(f"  {title}")
    print(f"{'=' * 70}\n")


def main():
    # 1. Field-by-field comparison
    section("FIELD-BY-FIELD COMPARISON (CoV = spatial structure strength)")
    rows = read_csv("field_comparison.csv")
    core_fields = ["variance", "log_variance", "zero_frac", "occupancy",
                   "temporal_range", "stability"]
    for field in core_fields:
        field_rows = [r for r in rows if r["field"] == field]
        print(f"  {field}:")
        for r in sorted(field_rows, key=lambda x: -float(x["cov"])):
            ds = r["dataset"].replace("godel_", "")
            cov = fmt(r["cov"], 6, 3)
            rng = f"[{fmt(r['min'], 8, 2)}, {fmt(r['max'], 8, 2)}]"
            iqr = fmt(r["iqr"], 8, 3)
            drng = fmt(r["dynamic_range"], 8, 2)
            print(f"    {ds:14s} CoV={cov}  range={rng}  IQR={iqr}  dynRng={drng}")
        print()

    # 2. Epsilon sweep
    section("EPSILON THRESHOLD SWEEP")
    rows = read_csv("epsilon_sweep.csv")
    datasets_in_sweep = sorted(set(r["dataset"] for r in rows))
    for ds in datasets_in_sweep:
        ds_rows = [r for r in rows if r["dataset"] == ds]
        ds_short = ds.replace("godel_", "")
        print(f"  {ds_short}:")
        print(f"    {'eps':>6s}  {'mean':>8s}  {'CoV':>8s}  {'IQR':>8s}  {'p5':>8s}  {'p95':>8s}")
        for r in sorted(ds_rows, key=lambda x: float(x["epsilon"])):
            eps = fmt(r["epsilon"], 6, 1)
            mean = fmt(r["mean"])
            cov = fmt(r["cov"])
            iqr = fmt(r["iqr"])
            p5 = fmt(r["p5"])
            p95 = fmt(r["p95"])
            print(f"    {eps}  {mean}  {cov}  {iqr}  {p5}  {p95}")
        print()

    # 3. Normalization sensitivity
    section("NORMALIZATION SENSITIVITY")
    rows = read_csv("normalization_sensitivity.csv")
    datasets_in_norm = sorted(set(r["dataset"] for r in rows))
    fields_in_norm = ["variance", "zero_frac", "occupancy"]
    for ds in datasets_in_norm:
        ds_short = ds.replace("godel_", "")
        for field in fields_in_norm:
            frows = [r for r in rows if r["dataset"] == ds and r["field"] == field]
            print(f"  {ds_short} / {field}:")
            print(f"    {'method':>22s}  {'corr':>7s}  {'CoV':>7s}  {'IQR':>8s}")
            for r in sorted(frows, key=lambda x: -float(x["corr_with_raw"])):
                method = r["method"]
                corr = fmt(r["corr_with_raw"], 7, 4)
                cov = fmt(r["norm_cov"], 7, 4)
                iqr = fmt(r["norm_iqr"], 8, 4)
                print(f"    {method:>22s}  {corr}  {cov}  {iqr}")
            print()

    # 4. Cross-field correlation
    section("CROSS-FIELD CORRELATION")
    rows = read_csv("field_cross_correlation.csv")
    datasets_in_corr = sorted(set(r["dataset"] for r in rows))
    for ds in datasets_in_corr:
        ds_short = ds.replace("godel_", "")
        ds_rows = [r for r in rows if r["dataset"] == ds]
        print(f"  {ds_short}:")
        for r in sorted(ds_rows, key=lambda x: -abs(float(x["pearson_r"]))):
            pair = r["pair"]
            pr = fmt(r["pearson_r"], 7, 4)
            print(f"    {pair:40s}  r={pr}")
        print()

    # 5. Per-channel decomposition
    section("PER-CHANNEL DECOMPOSITION (eigenvalue datasets)")
    rows = read_csv("per_channel_decomposition.csv")
    datasets_in_ch = sorted(set(r["dataset"] for r in rows))
    for ds in datasets_in_ch:
        ds_short = ds.replace("godel_", "")
        ds_rows = [r for r in rows if r["dataset"] == ds]
        print(f"  {ds_short}:")
        corr_rows = [r for r in ds_rows if "vs" in r["pair"]]
        stat_rows = [r for r in ds_rows if "stats" in r["pair"]]
        for r in corr_rows:
            print(f"    {r['pair']:20s}  r={fmt(r['pearson_r'], 7, 4)}")
        for r in stat_rows:
            print(f"    {r['pair']:20s}  CoV={fmt(r['pearson_r'], 7, 4)}")
        print()


if __name__ == "__main__":
    main()
