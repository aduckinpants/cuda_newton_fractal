from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np


def _load_primary_plane(path: Path) -> np.ndarray:
    try:
        from astropy.io import fits
    except ImportError as exc:  # pragma: no cover - environment-specific guard
        raise RuntimeError("astropy is required to extract runtime-walk FITS orientation inputs") from exc

    with fits.open(path) as hdul:
        if not hdul:
            raise RuntimeError(f"FITS file has no HDUs: {path}")
        data = hdul[0].data
        if data is None:
            raise RuntimeError(f"FITS primary HDU has no data: {path}")
        plane = np.asarray(data, dtype=np.float64)
    while plane.ndim > 2:
        plane = plane[0]
    if plane.ndim == 1:
        plane = plane.reshape((1, plane.shape[0]))
    if plane.ndim != 2:
        raise RuntimeError(f"Unsupported FITS primary shape for runtime-walk orientation extraction: {plane.shape!r}")
    if not np.isfinite(plane).any():
        raise RuntimeError(f"FITS primary plane contains no finite values: {path}")
    return np.nan_to_num(plane, nan=0.0, posinf=0.0, neginf=0.0)


def _normalize_unit(value: float) -> float:
    return float(max(0.0, min(1.0, value)))


def _normalize_signed(value: float) -> float:
    return float(max(-1.0, min(1.0, value)))


def _extract_signals(plane: np.ndarray) -> dict[str, float]:
    height, width = plane.shape
    abs_plane = np.abs(plane)
    mean = float(np.mean(plane))
    stddev = float(np.std(plane))
    residual_energy = float(np.mean(abs_plane))

    cy0 = max(0, height // 4)
    cy1 = min(height, height - cy0)
    cx0 = max(0, width // 4)
    cx1 = min(width, width - cx0)
    center = plane[cy0:cy1, cx0:cx1]
    if center.size == 0:
        center = plane

    mask = np.ones_like(plane, dtype=bool)
    mask[cy0:cy1, cx0:cx1] = False
    edge = plane[mask]
    if edge.size == 0:
        edge = plane.reshape(-1)

    center_mean = float(np.mean(center))
    edge_mean = float(np.mean(edge))
    center_bias = center_mean - edge_mean

    left_mean = float(np.mean(plane[:, : max(1, width // 2)]))
    right_mean = float(np.mean(plane[:, width // 2 :]))
    top_mean = float(np.mean(plane[: max(1, height // 2), :]))
    bottom_mean = float(np.mean(plane[height // 2 :, :]))
    x_bias = right_mean - left_mean
    y_bias = top_mean - bottom_mean

    grad_y, grad_x = np.gradient(plane)
    grad_energy = float(np.mean(np.sqrt(grad_x * grad_x + grad_y * grad_y)))

    total_abs = float(np.sum(abs_plane))
    focus_ratio = float(np.sum(np.abs(center))) / total_abs if total_abs > 1.0e-12 else 0.0
    frame_delta = abs(center_bias) + 0.5 * grad_energy
    edge_balance = (edge_mean - mean) if np.isfinite(edge_mean) else 0.0

    mean_norm = _normalize_unit(0.5 + 0.5 * np.tanh(mean))
    stddev_norm = _normalize_unit(np.tanh(stddev))
    residual_norm = _normalize_unit(np.tanh(residual_energy))
    frame_delta_norm = _normalize_unit(np.tanh(frame_delta))
    focus_norm = _normalize_unit(focus_ratio)
    center_bias_norm = _normalize_signed(np.tanh(center_bias))
    edge_balance_norm = _normalize_signed(np.tanh(edge_balance))
    x_bias_norm = _normalize_signed(np.tanh(x_bias))
    y_bias_norm = _normalize_signed(np.tanh(y_bias))

    return {
        "mean": mean_norm,
        "stddev": stddev_norm,
        "residual_energy": residual_norm,
        "frame_delta": frame_delta_norm,
        "focus_ratio": focus_norm,
        "center_bias": center_bias_norm,
        "edge_balance": edge_balance_norm,
        "x_bias": x_bias_norm,
        "y_bias": y_bias_norm,
        "center_mean": center_mean,
        "edge_mean": edge_mean,
        "gradient_energy": grad_energy,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Extract runtime-walk FITS orientation inputs for Explaino synthesis.")
    parser.add_argument("--fits", type=Path, required=True)
    parser.add_argument("--out-json", type=Path, required=True)
    args = parser.parse_args(argv)

    fits_path = args.fits.resolve()
    plane = _load_primary_plane(fits_path)
    payload = {
        "version": 1,
        "fits_path": str(fits_path),
        "signals": _extract_signals(plane),
    }
    args.out_json.parent.mkdir(parents=True, exist_ok=True)
    args.out_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
