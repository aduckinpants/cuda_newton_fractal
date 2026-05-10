from __future__ import annotations

import argparse
import json
from pathlib import Path

from tools.viewer_host_salt_ndepend import build_parser, main


def _subcommands(parser: argparse.ArgumentParser) -> set[str]:
    subparsers = next(
        action for action in parser._actions if isinstance(action, argparse._SubParsersAction)
    )
    return set(subparsers.choices.keys())


def test_parser_exposes_expected_command_set() -> None:
    parser = build_parser()
    assert _subcommands(parser) == {
        "audit",
        "structural",
        "review",
        "baselines",
        "contracts",
        "doctor",
        "parity",
        "family-parity",
    }


def test_audit_surface_emits_suite_and_producer_index(tmp_path: Path) -> None:
    out_dir = tmp_path / "audit"
    rc = main(["audit", "--out-dir", str(out_dir)])

    assert rc == 0
    suite_index = json.loads((out_dir / "suite_index.json").read_text(encoding="utf-8"))
    producer_index = json.loads((out_dir / "producer_index.json").read_text(encoding="utf-8"))
    assert suite_index["status"] == "surface_seed_only"
    assert suite_index["command"] == "audit"
    assert "advanced_color" in suite_index["gated_product_threads"]
    assert producer_index["producer_count"] == 7
    assert producer_index["planned_command_count"] == 8


def test_baselines_surface_emits_seeded_index(tmp_path: Path) -> None:
    out_dir = tmp_path / "baselines"
    out_json = out_dir / "baseline_index.json"
    out_md = out_dir / "baseline_index.md"

    rc = main([
        "baselines",
        "--out-dir",
        str(out_dir),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["status"] == "surface_seed_only"
    assert payload["case_count"] == 8
    assert payload["planned_blocker_cases"] == ["advanced_color_slider_contract"]
    assert out_md.exists()


def test_doctor_surface_reports_open_blockers(tmp_path: Path) -> None:
    out_json = tmp_path / "doctor.json"
    out_md = tmp_path / "doctor.md"

    rc = main([
        "doctor",
        "--packet-dir",
        str(tmp_path / "missing-packet-dir"),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["status"] == "surface_seed_only"
    assert payload["freeze_ready"] is False
    finding_codes = {item["code"] for item in payload["findings"]}
    assert "packet_dir_missing" in finding_codes
    assert "packet_command_surfaces_missing" in finding_codes
    assert "advanced_color_slider_family_missing" in finding_codes
    assert payload["contract_count"] == 3
    assert payload["baseline_case_count"] == 8
    assert out_md.exists()
