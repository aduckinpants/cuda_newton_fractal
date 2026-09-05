from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import struct
from fractions import Fraction
from pathlib import Path
from typing import Iterable, Sequence


CIE_CSV_SHA256 = "fa663e3535a7e0763a745993a1f0a192eb0275ac46ad2d1befd7626841e713c1"
CIE_METADATA_SOURCE_SHA256 = "03abcaecf4e63d77045ef57c4514b52c8bb1a46dd18e1f93a50044a0f4f481c8"
CIE_METADATA_CANONICAL_SHA256 = "88a821deaf1e425ac5300d4b9be8ccc8d8b029bb7a0d41eb7e7a266299409774"
PLANCK_C2_M_K = 1.438776877e-2
TEMPERATURE_MIN_K = 800.0
TEMPERATURE_MAX_K = 40000.0
LUT_ENTRY_COUNT = 1024
WITNESS_TEMPERATURES_K = (800.0, 1600.0, 3000.0, 6500.0, 12000.0, 40000.0)
XYZ_TO_LINEAR_SRGB_RATIONAL = (
    ("12831/3959", "-329/214", "-1974/3959"),
    ("-851781/878810", "1648619/878810", "36519/878810"),
    ("705/12673", "-2585/12673", "705/667"),
)
XYZ_TO_LINEAR_SRGB = tuple(
    tuple(float(Fraction(value)) for value in row)
    for row in XYZ_TO_LINEAR_SRGB_RATIONAL
)

CieRow = tuple[int, float, float, float]
Rgb = tuple[float, float, float]


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_cie_rows(path: Path) -> list[CieRow]:
    if sha256_file(path) != CIE_CSV_SHA256:
        raise ValueError(f"CIE CSV checksum mismatch: {path}")
    rows: list[CieRow] = []
    with path.open("r", encoding="utf-8", newline="") as handle:
        for raw in csv.reader(handle):
            if len(raw) != 4:
                raise ValueError(f"expected four CIE columns, got {len(raw)}")
            wavelength = int(raw[0])
            values = tuple(float(value) for value in raw[1:])
            if not all(math.isfinite(value) and value >= 0.0 for value in values):
                raise ValueError(f"invalid CIE sample at {wavelength} nm")
            rows.append((wavelength, values[0], values[1], values[2]))
    if [row[0] for row in rows] != list(range(360, 831)):
        raise ValueError("CIE wavelength domain must be 360..830 nm at 1 nm")
    return rows


def load_cie_metadata(path: Path) -> dict[str, object]:
    metadata = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(metadata, dict):
        raise ValueError("CIE metadata must be a JSON object")
    canonical = json.dumps(
        metadata, sort_keys=True, separators=(",", ":"), ensure_ascii=True
    ).encode("utf-8")
    if hashlib.sha256(canonical).hexdigest() != CIE_METADATA_CANONICAL_SHA256:
        raise ValueError(f"CIE metadata content mismatch: {path}")
    return metadata

def reciprocal_temperature_q(temperature_k: float) -> float:
    if not math.isfinite(temperature_k) or temperature_k <= 0.0:
        raise ValueError("temperature must be finite and positive")
    numerator = 1.0 / temperature_k - 1.0 / TEMPERATURE_MIN_K
    denominator = 1.0 / TEMPERATURE_MAX_K - 1.0 / TEMPERATURE_MIN_K
    return numerator / denominator


def temperature_for_lut_index(index: int) -> float:
    if index < 0 or index >= LUT_ENTRY_COUNT:
        raise ValueError("LUT index out of range")
    u = index / float(LUT_ENTRY_COUNT - 1)
    reciprocal = (1.0 - u) / TEMPERATURE_MIN_K + u / TEMPERATURE_MAX_K
    return 1.0 / reciprocal


def integrate_blackbody_xyz(rows: Sequence[CieRow], temperature_k: float) -> Rgb:
    if not math.isfinite(temperature_k) or temperature_k <= 0.0:
        raise ValueError("temperature must be finite and positive")
    accum = [0.0, 0.0, 0.0]
    last = len(rows) - 1
    for index, (wavelength_nm, x_bar, y_bar, z_bar) in enumerate(rows):
        wavelength_m = wavelength_nm * 1.0e-9
        exponent = PLANCK_C2_M_K / (wavelength_m * temperature_k)
        spectral = 1.0 / (wavelength_m**5 * math.expm1(exponent))
        weight = 0.5 if index == 0 or index == last else 1.0
        accum[0] += weight * spectral * x_bar
        accum[1] += weight * spectral * y_bar
        accum[2] += weight * spectral * z_bar
    if not all(math.isfinite(value) for value in accum) or accum[1] <= 0.0:
        raise ValueError("black-body XYZ integration did not produce finite positive Y")
    inv_y = 1.0 / accum[1]
    return accum[0] * inv_y, 1.0, accum[2] * inv_y


def xyz_to_pre_gamut_linear_rgb(xyz: Sequence[float]) -> Rgb:
    return tuple(
        sum(matrix_value * xyz_value for matrix_value, xyz_value in zip(row, xyz))
        for row in XYZ_TO_LINEAR_SRGB
    )  # type: ignore[return-value]


def apply_neutral_lift_and_peak_normalize(rgb: Sequence[float]) -> tuple[Rgb, float]:
    if len(rgb) != 3 or not all(math.isfinite(value) for value in rgb):
        raise ValueError("pre-gamut RGB must contain three finite values")
    lift = max(0.0, -min(rgb))
    lifted = tuple(value + lift for value in rgb)
    peak = max(lifted)
    if not math.isfinite(peak) or peak <= 0.0:
        raise ValueError("gamut policy requires a finite positive peak")
    normalized = tuple(max(0.0, min(1.0, value / peak)) for value in lifted)
    return normalized, lift  # type: ignore[return-value]


def blackbody_reference(rows: Sequence[CieRow], temperature_k: float) -> dict[str, object]:
    xyz = integrate_blackbody_xyz(rows, temperature_k)
    xyz_sum = sum(xyz)
    xy = (xyz[0] / xyz_sum, xyz[1] / xyz_sum)
    pre_gamut = xyz_to_pre_gamut_linear_rgb(xyz)
    final_rgb, lift = apply_neutral_lift_and_peak_normalize(pre_gamut)
    return {
        "temperature_k": float(temperature_k),
        "xyz_y1": list(xyz),
        "xy": list(xy),
        "pre_gamut_linear_rgb": list(pre_gamut),
        "neutral_lift": lift,
        "final_linear_rgb": list(final_rgb),
    }


def blackbody_linear_rgb(rows: Sequence[CieRow], temperature_k: float) -> Rgb:
    return tuple(blackbody_reference(rows, temperature_k)["final_linear_rgb"])  # type: ignore[arg-type,return-value]


def _float32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def generate_lut(rows: Sequence[CieRow]) -> list[Rgb]:
    return [
        tuple(_float32(value) for value in blackbody_linear_rgb(rows, temperature_for_lut_index(index)))
        for index in range(LUT_ENTRY_COUNT)
    ]  # type: ignore[return-value]


def sample_lut_coordinate(lut: Sequence[Rgb], coordinate: float) -> Rgb:
    x = max(0.0, min(float(LUT_ENTRY_COUNT - 1), coordinate))
    if x >= LUT_ENTRY_COUNT - 1:
        return tuple(lut[-1])
    lower = int(math.floor(x))
    fraction = x - lower
    return tuple(
        lut[lower][channel] + (lut[lower + 1][channel] - lut[lower][channel]) * fraction
        for channel in range(3)
    )  # type: ignore[return-value]


def sample_lut_temperature(lut: Sequence[Rgb], temperature_k: float) -> Rgb:
    coordinate = (LUT_ENTRY_COUNT - 1) * reciprocal_temperature_q(temperature_k)
    return sample_lut_coordinate(lut, coordinate)


def _float_literal(value: float) -> str:
    literal = f"{_float32(value):.9g}"
    if "." not in literal and "e" not in literal:
        literal += ".0"
    return literal + "f"


def render_lut_initializer(lut: Sequence[Rgb]) -> str:
    lines = [
        "// Generated by tools/blackbody_palette_generator.py. Do not edit.",
        f"#define BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT {LUT_ENTRY_COUNT}",
        "#define BLACKBODY_PALETTE_LUT_V1_INITIALIZER \\",
        "{ \\",
    ]
    for index, rgb in enumerate(lut):
        suffix = ", \\" if index + 1 < len(lut) else " \\"
        lines.append(
            "    { " + ", ".join(_float_literal(value) for value in rgb) + " }" + suffix
        )
    lines.append("}")
    return "\n".join(lines) + "\n"

def build_metadata(rows: Sequence[CieRow], lut: Sequence[Rgb], cie_csv: Path, cie_metadata: Path) -> dict[str, object]:
    lifts: list[tuple[float, float]] = []
    for index in range(LUT_ENTRY_COUNT):
        temperature = temperature_for_lut_index(index)
        lift = float(blackbody_reference(rows, temperature)["neutral_lift"])
        if lift > 0.0:
            lifts.append((lift, temperature))
    maximum_lift, maximum_temperature = max(lifts, default=(0.0, TEMPERATURE_MIN_K))

    witnesses: list[dict[str, object]] = []
    for temperature in WITNESS_TEMPERATURES_K:
        witness = blackbody_reference(rows, temperature)
        witness["lut_rgb"] = list(sample_lut_temperature(lut, temperature))
        witnesses.append(witness)

    return {
        "schema_id": "viewer.blackbody_palette_lut.v1",
        "generator_version": 1,
        "cie_csv_file": cie_csv.name,
        "cie_csv_sha256": sha256_file(cie_csv),
        "cie_metadata_file": cie_metadata.name,
        "cie_metadata_source_sha256": CIE_METADATA_SOURCE_SHA256,
        "cie_metadata_canonical_sha256": CIE_METADATA_CANONICAL_SHA256,
        "wavelength_nm": {"minimum": 360, "maximum": 830, "step": 1, "samples": len(rows)},
        "planck_c2_m_k": PLANCK_C2_M_K,
        "xyz_to_linear_srgb_matrix": {
            "rational": [list(row) for row in XYZ_TO_LINEAR_SRGB_RATIONAL],
            "float64": [list(row) for row in XYZ_TO_LINEAR_SRGB],
            "provenance": "W3C CSS Color 4 XYZ-D65 to linear-sRGB matrix",
        },
        "gamut_policy": "neutral_axis_lift_then_peak_normalize_then_clamp",
        "color_encoding": "internal_linear_rgb_no_srgb_transfer",
        "temperature_k": {"minimum": TEMPERATURE_MIN_K, "maximum": TEMPERATURE_MAX_K},
        "entry_count": LUT_ENTRY_COUNT,
        "spacing": "reciprocal_temperature",
        "storage": "float32",
        "neutral_lift": {
            "entry_count": len(lifts),
            "maximum": maximum_lift,
            "temperature_k": maximum_temperature,
        },
        "witnesses": witnesses,
    }


def generate_artifacts(cie_csv: Path, cie_metadata: Path, out_inc: Path, out_metadata: Path) -> None:
    rows = load_cie_rows(cie_csv)
    load_cie_metadata(cie_metadata)
    lut = generate_lut(rows)
    metadata = build_metadata(rows, lut, cie_csv, cie_metadata)
    out_inc.parent.mkdir(parents=True, exist_ok=True)
    out_metadata.parent.mkdir(parents=True, exist_ok=True)
    out_inc.write_text(render_lut_initializer(lut), encoding="utf-8", newline="\n")
    out_metadata.write_text(
        json.dumps(metadata, indent=2, sort_keys=True, allow_nan=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )

def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate the deterministic Black Body palette LUT")
    parser.add_argument("--cie-csv", required=True, type=Path)
    parser.add_argument("--cie-metadata", required=True, type=Path)
    parser.add_argument("--out-inc", required=True, type=Path)
    parser.add_argument("--out-json", required=True, type=Path)
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    generate_artifacts(args.cie_csv, args.cie_metadata, args.out_inc, args.out_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
