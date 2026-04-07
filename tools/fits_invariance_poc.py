"""FITS trajectory invariance POC - generate heightfield PLY meshes."""
import numpy as np
from astropy.io import fits
import os


OUT_DIR = r"D:\salt-output\results\fits_invariance_poc"
CAMPAIGN_BASE = (
    r"D:\salt-output\results\godel_fits_entropy_campaign"
    r"\full_round_20260407"
)


def reconstruct(path):
    f = fits.open(path)
    kf = f[0].data.astype(np.float32)
    C, H, W = kf.shape
    traj = np.zeros((C, H, W, len(f)), dtype=np.float32)
    traj[:, :, :, 0] = kf
    for t in range(1, len(f)):
        traj[:, :, :, t] = traj[:, :, :, t - 1] + f[t].data.astype(np.float32)
    f.close()
    return traj


def write_ply(filepath, height_field, color_field=None, z_scale=1.0):
    H, W = height_field.shape
    n_verts = H * W
    n_faces = (H - 1) * (W - 1) * 2

    hmin, hmax = height_field.min(), height_field.max()
    if hmax - hmin < 1e-10:
        hnorm = np.zeros_like(height_field)
    else:
        hnorm = (height_field - hmin) / (hmax - hmin)

    with open(filepath, "w") as f:
        f.write("ply\nformat ascii 1.0\n")
        f.write(f"element vertex {n_verts}\n")
        f.write("property float x\nproperty float y\nproperty float z\n")
        f.write("property uchar red\nproperty uchar green\nproperty uchar blue\n")
        f.write(f"element face {n_faces}\n")
        f.write("property list uchar int vertex_indices\nend_header\n")

        for y in range(H):
            for x in range(W):
                z = hnorm[y, x] * z_scale
                if color_field is not None:
                    r = int(color_field[y, x, 0])
                    g = int(color_field[y, x, 1])
                    b = int(color_field[y, x, 2])
                else:
                    val = hnorm[y, x]
                    r = int(min(val * 3, 1.0) * 255)
                    g = int(max(min((val - 0.33) * 3, 1.0), 0) * 255)
                    b = int(max(min((val - 0.66) * 3, 1.0), 0) * 255)
                f.write(f"{x / (W - 1):.6f} {y / (H - 1):.6f} {z:.6f} {r} {g} {b}\n")

        for y in range(H - 1):
            for x in range(W - 1):
                v00 = y * W + x
                v10 = y * W + (x + 1)
                v01 = (y + 1) * W + x
                v11 = (y + 1) * W + (x + 1)
                f.write(f"3 {v00} {v10} {v01}\n3 {v10} {v11} {v01}\n")


def process_dataset(ds_name, fits_path):
    print(f"Processing {ds_name}...")
    traj = reconstruct(fits_path)
    C, H, W, T = traj.shape

    ds_dir = os.path.join(OUT_DIR, ds_name)
    os.makedirs(ds_dir, exist_ok=True)

    # Derived fields
    var_field = np.mean(np.var(traj, axis=3), axis=0)
    stability = 1.0 - var_field / (var_field.max() + 1e-10)
    mean_rgb = np.clip(
        np.mean(traj, axis=3).transpose(1, 2, 0), 0, 255
    ).astype(np.uint8)

    # Subsample for manageable mesh
    step = max(1, H // 128)
    stab_sub = stability[::step, ::step]
    color_sub = mean_rgb[::step, ::step]

    # Stability sculpture (height = stability, color = mean frame)
    ply_path = os.path.join(ds_dir, "invariance_sculpture.ply")
    write_ply(ply_path, stab_sub, color_sub, z_scale=0.3)
    sz = os.path.getsize(ply_path) / 1024 / 1024
    print(f"  {ply_path} ({sz:.1f} MB, {stab_sub.shape[0]}x{stab_sub.shape[1]})")

    # Churn sculpture (height = log-variance)
    log_var = np.log1p(var_field)
    lv_sub = log_var[::step, ::step]
    ply2 = os.path.join(ds_dir, "churn_sculpture.ply")
    write_ply(ply2, lv_sub, color_sub, z_scale=0.3)
    print(f"  {ply2}")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    core_fits = os.path.join(CAMPAIGN_BASE, "core", "fits")
    for ds_name in ["g13", "g4"]:
        fits_path = os.path.join(
            core_fits, f"godel_{ds_name}", "frames_delta_stack.fits"
        )
        process_dataset(ds_name, fits_path)

    print("Done.")


if __name__ == "__main__":
    main()
