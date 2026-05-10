from __future__ import annotations

import argparse
import json
from pathlib import Path

from tools.viewer_host_salt_ndepend import build_parser, main


def _write_json(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


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
    code_quality_path = tmp_path / "artifacts" / "code_quality.json"
    code_quality_path.parent.mkdir(parents=True, exist_ok=True)
    _write_json(code_quality_path, {"score": 91, "checks": {"baseline": True}})

    runtime_ui_log = tmp_path / "artifacts" / "runtime_ui.log"
    runtime_ui_log.write_text("line-1\nviewer_host_runtime_pytest_lane: summary=passed=2 failed=0\n", encoding="utf-8")

    policy_path = tmp_path / "policy.json"
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {
                    "producer_id": "code_quality",
                    "artifact_path": str(code_quality_path),
                },
                {
                    "producer_id": "runtime_ui",
                    "artifact_path": str(runtime_ui_log),
                },
                {
                    "producer_id": "missing_ui",
                    "artifact_path": str(tmp_path / "artifacts" / "missing.log"),
                },
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "doctor"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    out_dir = tmp_path / "audit"
    rc = main(["audit", "--policy", str(policy_path), "--out-dir", str(out_dir)])

    assert rc == 0
    suite_index = json.loads((out_dir / "suite_index.json").read_text(encoding="utf-8"))
    producer_index = json.loads((out_dir / "producer_index.json").read_text(encoding="utf-8"))
    assert suite_index["status"] == "producer_bound_seed"
    assert suite_index["command"] == "audit"
    assert "advanced_color" in suite_index["gated_product_threads"]
    assert suite_index["bound_producer_count"] == 2
    assert suite_index["missing_producer_count"] == 1
    assert producer_index["producer_count"] == 3
    producer_status = {item["producer_id"]: item["status"] for item in producer_index["producers"]}
    assert producer_status == {
        "code_quality": "bound",
        "runtime_ui": "bound",
        "missing_ui": "missing",
    }
    summaries = {item["producer_id"]: item["summary"] for item in producer_index["producers"]}
    assert summaries["code_quality"]["score"] == 91
    assert summaries["runtime_ui"]["tail"] == "viewer_host_runtime_pytest_lane: summary=passed=2 failed=0"
    assert summaries["missing_ui"]["reason"] == "artifact_missing"


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
    policy_path = tmp_path / "policy.json"
    ui_log = tmp_path / "artifacts" / "runtime_ui.log"
    ui_log.parent.mkdir(parents=True, exist_ok=True)
    ui_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=1 failed=0\n", encoding="utf-8")
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {
                    "producer_id": "runtime_ui",
                    "artifact_path": str(ui_log),
                },
                {
                    "producer_id": "missing_ui",
                    "artifact_path": str(tmp_path / "artifacts" / "missing.log"),
                },
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "doctor"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )
    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    out_json = tmp_path / "doctor.json"
    out_md = tmp_path / "doctor.md"

    rc = main([
        "doctor",
        "--packet-dir",
        str(packet_dir),
        "--suite-index",
        str(packet_dir / "suite_index.json"),
        "--producer-index",
        str(packet_dir / "producer_index.json"),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["status"] == "producer_bound_seed"
    assert payload["freeze_ready"] is False
    finding_codes = {item["code"] for item in payload["findings"]}
    assert "audit_missing_producers" in finding_codes
    assert "packet_command_surfaces_missing" in finding_codes
    assert "advanced_color_slider_family_missing" in finding_codes
    assert "producer_missing:missing_ui" in finding_codes
    assert payload["contract_count"] == 3
    assert payload["baseline_case_count"] == 8
    assert payload["bound_producer_count"] == 1
    assert payload["missing_producer_count"] == 1
    assert out_md.exists()
