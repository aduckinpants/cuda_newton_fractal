from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path

import pytest

from tools import blackbody_palette_generator as blackbody


ROOT = Path(__file__).resolve().parents[1]
REFERENCE_DIR = ROOT / "docs" / "reference" / "blackbody_chromaticity"
CIE_CSV = REFERENCE_DIR / "CIE_xyz_1931_2deg.csv"
CIE_METADATA = REFERENCE_DIR / "CIE_xyz_1931_2deg.csv_metadata.json"
CHECKED_INC = REFERENCE_DIR / "blackbody_palette_lut_v1.inc"
CHECKED_METADATA = REFERENCE_DIR / "blackbody_palette_lut_v1.metadata.json"
EXPECTED_CIE_SHA256 = "fa663e3535a7e0763a745993a1f0a192eb0275ac46ad2d1befd7626841e713c1"
EXPECTED_LUT_COUNT = 1024


def _independent_xy(temperature_k: float) -> tuple[float, float]:
    accum = [0.0, 0.0, 0.0]
    with CIE_CSV.open("r", encoding="utf-8", newline="") as handle:
        samples = [
            (int(row[0]), float(row[1]), float(row[2]), float(row[3]))
            for row in csv.reader(handle)
        ]
    for index, (wavelength_nm, x_bar, y_bar, z_bar) in enumerate(samples):
        wavelength_m = wavelength_nm * 1.0e-9
        spectral = 1.0 / (
            wavelength_m**5
            * math.expm1(1.438776877e-2 / (wavelength_m * temperature_k))
        )
        weight = 0.5 if index in (0, len(samples) - 1) else 1.0
        accum[0] += weight * spectral * x_bar
        accum[1] += weight * spectral * y_bar
        accum[2] += weight * spectral * z_bar
    total = sum(accum)
    return accum[0] / total, accum[1] / total


def _srgb_encode(value: float) -> float:
    value = max(0.0, min(1.0, value))
    if value <= 0.0031308:
        return 12.92 * value
    return 1.055 * value ** (1.0 / 2.4) - 0.055


def test_checked_in_blackbody_lut_is_fresh(tmp_path: Path) -> None:
    assert hashlib.sha256(CIE_CSV.read_bytes()).hexdigest() == EXPECTED_CIE_SHA256
    rows = blackbody.load_cie_rows(CIE_CSV)
    assert len(rows) == 471
    assert [row[0] for row in rows] == list(range(360, 831))

    first_inc = tmp_path / "first.inc"
    first_metadata = tmp_path / "first.json"
    second_inc = tmp_path / "second.inc"
    second_metadata = tmp_path / "second.json"
    blackbody.generate_artifacts(CIE_CSV, CIE_METADATA, first_inc, first_metadata)
    blackbody.generate_artifacts(CIE_CSV, CIE_METADATA, second_inc, second_metadata)

    assert first_inc.read_bytes() == second_inc.read_bytes()
    assert first_metadata.read_bytes() == second_metadata.read_bytes()
    assert first_inc.read_text(encoding="utf-8") == CHECKED_INC.read_text(encoding="utf-8")
    assert json.loads(first_metadata.read_text(encoding="utf-8")) == json.loads(
        CHECKED_METADATA.read_text(encoding="utf-8")
    )


def test_generator_rejects_source_provenance_drift(tmp_path: Path) -> None:
    bad_csv = tmp_path / "bad.csv"
    bad_csv.write_bytes(CIE_CSV.read_bytes() + b"\r\n")
    with pytest.raises(ValueError, match="checksum mismatch"):
        blackbody.load_cie_rows(bad_csv)

    bad_metadata = tmp_path / "bad_metadata.json"
    metadata = json.loads(CIE_METADATA.read_text(encoding="utf-8"))
    metadata["identifier"] = "altered"
    bad_metadata.write_text(json.dumps(metadata), encoding="utf-8")
    with pytest.raises(ValueError, match="metadata content mismatch"):
        blackbody.load_cie_metadata(bad_metadata)

    assert blackbody._float_literal(0.0) == "0.0f"
    assert blackbody._float_literal(1.0) == "1.0f"


def test_blackbody_lut_reference_accuracy_and_independent_witnesses() -> None:
    rows = blackbody.load_cie_rows(CIE_CSV)
    lut = blackbody.generate_lut(rows)
    assert len(lut) == EXPECTED_LUT_COUNT
    assert all(math.isfinite(channel) and 0.0 <= channel <= 1.0 for rgb in lut for channel in rgb)

    assert blackbody.reciprocal_temperature_q(800.0) == pytest.approx(0.0, abs=1.0e-14)
    assert blackbody.reciprocal_temperature_q(40000.0) == pytest.approx(1.0, abs=1.0e-14)
    prior_q = -1.0
    for index in range(EXPECTED_LUT_COUNT):
        temperature = blackbody.temperature_for_lut_index(index)
        q = blackbody.reciprocal_temperature_q(temperature)
        assert q == pytest.approx(index / (EXPECTED_LUT_COUNT - 1), abs=2.0e-13)
        assert q > prior_q
        prior_q = q

    # Independently encoded broad Planckian-locus anchors. These values are not
    # obtained from generator metadata or helper output.
    expected_xy = {
        1600.0: (0.5732295853, 0.3992641988),
        2856.0: (0.4475351652, 0.4074283255),
        6500.0: (0.3135260346, 0.3236286620),
        12000.0: (0.2717840580, 0.2775638860),
    }
    for temperature, expected in expected_xy.items():
        independent = _independent_xy(temperature)
        xyz = blackbody.integrate_blackbody_xyz(rows, temperature)
        total = sum(xyz)
        actual = (xyz[0] / total, xyz[1] / total)
        assert independent == pytest.approx(expected, abs=3.0e-6)
        assert actual == pytest.approx(independent, abs=2.0e-13)

    max_error = 0.0
    for index in range(4096):
        temperature = 800.0 + (40000.0 - 800.0) * index / 4095.0
        expected = blackbody.blackbody_linear_rgb(rows, temperature)
        actual = blackbody.sample_lut_temperature(lut, temperature)
        max_error = max(max_error, *(abs(a - b) for a, b in zip(actual, expected)))
    assert max_error <= 0.002

    metadata = json.loads(CHECKED_METADATA.read_text(encoding="utf-8"))
    assert metadata["schema_id"] == "viewer.blackbody_palette_lut.v1"
    assert metadata["cie_csv_sha256"] == EXPECTED_CIE_SHA256
    assert (
        metadata["cie_metadata_source_sha256"]
        == blackbody.CIE_METADATA_SOURCE_SHA256
    )
    assert (
        metadata["cie_metadata_canonical_sha256"]
        == blackbody.CIE_METADATA_CANONICAL_SHA256
    )
    assert "cie_metadata_sha256" not in metadata
    assert metadata["entry_count"] == EXPECTED_LUT_COUNT
    assert metadata["spacing"] == "reciprocal_temperature"
    assert metadata["neutral_lift"]["entry_count"] > 0
    assert metadata["neutral_lift"]["maximum"] > 0.0
    assert 800.0 <= metadata["neutral_lift"]["temperature_k"] <= 40000.0
    assert [witness["temperature_k"] for witness in metadata["witnesses"]] == [
        800.0,
        1600.0,
        3000.0,
        6500.0,
        12000.0,
        40000.0,
    ]
    for witness in metadata["witnesses"]:
        assert set(("xyz_y1", "xy", "pre_gamut_linear_rgb", "neutral_lift", "final_linear_rgb", "lut_rgb")) <= set(witness)

    linear = blackbody.blackbody_linear_rgb(rows, 3000.0)
    encoded = tuple(_srgb_encode(channel) for channel in linear)
    assert max(abs(a - b) for a, b in zip(linear, encoded)) > 0.05
