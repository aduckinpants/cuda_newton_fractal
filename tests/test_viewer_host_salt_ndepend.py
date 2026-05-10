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
        "freeze-gate",
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
    assert payload["case_count"] == 11
    assert payload["planned_blocker_cases"] == [
        "advanced_color_slider_contract",
        "advanced_color_function_switch_contract",
        "advanced_color_shape_stack_contract",
        "advanced_color_draft_resync_contract",
    ]
    assert all(case.get("required_contract") for case in payload["cases"])
    blocker_case = next(case for case in payload["cases"] if case["case_id"] == "advanced_color_slider_contract")
    assert blocker_case["required_contract"] == "advanced_color_slider_contract.v1"
    function_switch_case = next(case for case in payload["cases"] if case["case_id"] == "advanced_color_function_switch_contract")
    assert function_switch_case["required_contract"] == "advanced_color_function_switch_contract.v1"
    shape_stack_case = next(case for case in payload["cases"] if case["case_id"] == "advanced_color_shape_stack_contract")
    assert shape_stack_case["required_contract"] == "advanced_color_shape_stack_contract.v1"
    runtime_ui_case = next(case for case in payload["cases"] if case["case_id"] == "runtime_ui_harness_baseline")
    assert runtime_ui_case["required_contract"] == "runtime_ui_harness_contract.v1"
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
    assert "producer_missing:missing_ui" in finding_codes
    assert "packet_command_surfaces_missing" not in finding_codes
    assert "critical_family_backfill_incomplete" not in finding_codes
    assert "workflow_hard_denial_missing" not in finding_codes
    assert payload["contract_count"] == 12
    assert payload["baseline_case_count"] == 11
    assert payload["bound_producer_count"] == 1
    assert payload["missing_producer_count"] == 1
    assert out_md.exists()


def test_doctor_surface_reports_required_blocker_contracts_that_are_not_green(tmp_path: Path) -> None:
    policy_path = tmp_path / "policy.json"
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {
                    "producer_id": "runtime_publish",
                    "artifact_path": str(tmp_path / "artifacts" / "runtime_publish.log"),
                },
                {
                    "producer_id": "runtime_ui_harness",
                    "artifact_path": str(tmp_path / "artifacts" / "missing_ui.log"),
                },
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "doctor"},
                {"command_id": "family-parity"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )
    runtime_publish_log = tmp_path / "artifacts" / "runtime_publish.log"
    runtime_publish_log.parent.mkdir(parents=True, exist_ok=True)
    runtime_publish_log.write_text("published runtime available\n", encoding="utf-8")

    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "runtime_ui_harness_baseline",
                    "family": "runtime_ui_behavior",
                    "status": "seeded",
                    "required_contract": "runtime_ui_harness_contract.v1",
                }
            ]
        },
    )
    baseline_dir = tmp_path / "baseline"
    baseline_json = baseline_dir / "baseline_index.json"
    baseline_md = baseline_dir / "baseline_index.md"
    assert main([
        "baselines",
        "--manifest",
        str(manifest_path),
        "--out-dir",
        str(baseline_dir),
        "--out",
        str(baseline_json),
        "--out-md",
        str(baseline_md),
    ]) == 0

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "runtime_ui_harness_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": [
                        "runtime_publish",
                        "runtime_ui_harness",
                    ],
                }
            ]
        },
    )

    family_parity_json = tmp_path / "family_parity.json"
    family_parity_md = tmp_path / "family_parity.md"
    assert main([
        "family-parity",
        "--baseline-index",
        str(baseline_json),
        "--candidate-suite",
        str(packet_dir / "suite_index.json"),
        "--contract-registry",
        str(registry_path),
        "--out",
        str(family_parity_json),
        "--out-md",
        str(family_parity_md),
    ]) == 0

    freeze_gate_path = tmp_path / "freeze_gate.json"
    _write_json(
        freeze_gate_path,
        {
            "required_packet_commands": ["audit", "doctor", "family-parity"],
            "required_blocker_contracts": ["runtime_ui_harness_contract.v1"],
            "intentional_open_blockers": [],
        },
    )

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
        "--contract-registry",
        str(registry_path),
        "--freeze-gate",
        str(freeze_gate_path),
        "--baseline-manifest",
        str(manifest_path),
        "--baseline-index",
        str(baseline_json),
        "--family-parity",
        str(family_parity_json),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    finding_codes = {item["code"] for item in payload["findings"]}
    assert "required_blocker_contract_not_green:runtime_ui_harness_contract.v1" in finding_codes


def test_doctor_surface_sets_freeze_ready_when_findings_are_empty(tmp_path: Path) -> None:
    policy_path = tmp_path / "policy.json"
    code_quality_path = tmp_path / "artifacts" / "code_quality.json"
    code_quality_path.parent.mkdir(parents=True, exist_ok=True)
    _write_json(code_quality_path, {"score": 99, "checks": {"baseline": True}})

    coverage_report = tmp_path / "artifacts" / "test_coverage_report.json"
    _write_json(coverage_report, {"status": "ok", "matrix": []})

    runtime_publish_log = tmp_path / "artifacts" / "runtime_publish.log"
    runtime_publish_log.write_text("published runtime available\n", encoding="utf-8")

    runtime_probe_log = tmp_path / "artifacts" / "runtime_probe.log"
    runtime_probe_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=5 failed=0\n", encoding="utf-8")

    runtime_ui_log = tmp_path / "artifacts" / "runtime_ui.log"
    runtime_ui_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=3 failed=0\n", encoding="utf-8")

    runtime_artifact_log = tmp_path / "artifacts" / "runtime_artifact.log"
    runtime_artifact_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=2 failed=0\n", encoding="utf-8")

    native_helper_log = tmp_path / "artifacts" / "native_helper.log"
    native_helper_log.write_text("All helper tests passed.\n", encoding="utf-8")

    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {"producer_id": "code_quality_audit", "artifact_path": str(code_quality_path)},
                {"producer_id": "test_coverage_audit", "artifact_path": str(coverage_report)},
                {"producer_id": "native_helper_tests", "artifact_path": str(native_helper_log)},
                {"producer_id": "runtime_publish", "artifact_path": str(runtime_publish_log)},
                {"producer_id": "runtime_probe_session", "artifact_path": str(runtime_probe_log)},
                {"producer_id": "runtime_artifact_tools", "artifact_path": str(runtime_artifact_log)},
                {"producer_id": "runtime_ui_harness", "artifact_path": str(runtime_ui_log)},
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "doctor"},
                {"command_id": "family-parity"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "advanced_color_slider_contract",
                    "family": "critical_ui_behavior",
                    "status": "seeded",
                    "required_contract": "advanced_color_slider_contract.v1",
                }
            ]
        },
    )
    baseline_json = packet_dir / "baseline_index.json"
    baseline_md = packet_dir / "baseline_index.md"
    assert main([
        "baselines",
        "--manifest",
        str(manifest_path),
        "--out-dir",
        str(packet_dir),
        "--out",
        str(baseline_json),
        "--out-md",
        str(baseline_md),
    ]) == 0

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "advanced_color_slider_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": [
                        "code_quality_audit",
                        "test_coverage_audit",
                        "native_helper_tests",
                        "runtime_publish",
                        "runtime_probe_session",
                        "runtime_ui_harness",
                    ],
                }
            ]
        },
    )

    family_parity_json = packet_dir / "family_parity.json"
    family_parity_md = packet_dir / "family_parity.md"
    assert main([
        "family-parity",
        "--baseline-index",
        str(baseline_json),
        "--candidate-suite",
        str(packet_dir / "suite_index.json"),
        "--contract-registry",
        str(registry_path),
        "--out",
        str(family_parity_json),
        "--out-md",
        str(family_parity_md),
    ]) == 0

    freeze_gate_path = tmp_path / "freeze_gate.json"
    _write_json(
        freeze_gate_path,
        {
            "required_packet_commands": ["audit", "doctor", "family-parity"],
            "required_blocker_contracts": ["advanced_color_slider_contract.v1"],
            "intentional_open_blockers": [],
        },
    )

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
        "--contract-registry",
        str(registry_path),
        "--freeze-gate",
        str(freeze_gate_path),
        "--baseline-manifest",
        str(manifest_path),
        "--baseline-index",
        str(baseline_json),
        "--family-parity",
        str(family_parity_json),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["findings"] == []
    assert payload["freeze_ready"] is True


def test_parity_surface_evaluates_contract_backed_case_results(tmp_path: Path) -> None:
    artifacts_dir = tmp_path / "artifacts"
    artifacts_dir.mkdir(parents=True, exist_ok=True)

    code_quality_path = artifacts_dir / "code_quality.json"
    _write_json(
        code_quality_path,
        {
            "score": 97,
            "severity_counts": {"CRITICAL": 0, "ERROR": 0, "WARN": 1, "NOTE": 0, "OK": 10},
        },
    )
    coverage_report = artifacts_dir / "test_coverage_report.json"
    _write_json(
        coverage_report,
        {
            "coverage": [
                {"module": "viewer_cli", "status": "COVERED"},
                {"module": "main", "status": "UNCOVERED"},
            ]
        },
    )
    native_helper_log = artifacts_dir / "native_helper.log"
    native_helper_log.write_text("All helper tests passed.\n", encoding="utf-8")
    runtime_publish_log = artifacts_dir / "runtime_publish.log"
    runtime_publish_log.write_text("[build_vsdevcmd] Active runtime: D:/salt/runtime/fractal_ui.exe\n", encoding="utf-8")
    runtime_probe_log = artifacts_dir / "runtime_probe.log"
    runtime_probe_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=5 failed=0 skipped=0 errors=0\n", encoding="utf-8")
    runtime_ui_log = artifacts_dir / "runtime_ui.log"
    runtime_ui_log.write_text("viewer_host_runtime_pytest_lane: summary=passed=3 failed=0 skipped=0 errors=0\n", encoding="utf-8")

    policy_path = tmp_path / "policy.json"
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {"producer_id": "code_quality_audit", "artifact_path": str(code_quality_path)},
                {"producer_id": "test_coverage_audit", "artifact_path": str(coverage_report)},
                {"producer_id": "native_helper_tests", "artifact_path": str(native_helper_log)},
                {"producer_id": "runtime_publish", "artifact_path": str(runtime_publish_log)},
                {"producer_id": "runtime_probe_session", "artifact_path": str(runtime_probe_log)},
                {"producer_id": "runtime_ui_harness", "artifact_path": str(runtime_ui_log)},
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "parity"},
                {"command_id": "family-parity"},
                {"command_id": "doctor"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "code_quality_baseline",
                    "family": "structural_quality",
                    "status": "seeded",
                    "required_contract": "code_quality_baseline_contract.v1",
                    "source_artifact": str(code_quality_path),
                    "expected_result": "baseline_check_passes",
                },
                {
                    "case_id": "test_coverage_matrix_baseline",
                    "family": "structural_quality",
                    "status": "seeded",
                    "required_contract": "test_coverage_matrix_contract.v1",
                    "source_artifact": str(coverage_report),
                    "expected_result": "no_direct_coverage_regression",
                },
                {
                    "case_id": "native_helper_regression_baseline",
                    "family": "native_helper_surface",
                    "status": "seeded",
                    "required_contract": "native_helper_regression_contract.v1",
                    "source_artifact": str(native_helper_log),
                    "expected_result": "all_helper_tests_pass",
                },
                {
                    "case_id": "runtime_publish_baseline",
                    "family": "runtime_publish_surface",
                    "status": "seeded",
                    "required_contract": "runtime_publish_contract.v1",
                    "source_artifact": str(runtime_publish_log),
                    "expected_result": "published_runtime_available",
                },
                {
                    "case_id": "runtime_probe_session_baseline",
                    "family": "runtime_probe_session",
                    "status": "seeded",
                    "required_contract": "runtime_probe_session_contract.v1",
                    "source_artifact": str(runtime_probe_log),
                    "expected_result": "green_published_runtime_lane",
                },
                {
                    "case_id": "runtime_ui_harness_baseline",
                    "family": "runtime_ui_behavior",
                    "status": "seeded",
                    "required_contract": "runtime_ui_harness_contract.v1",
                    "source_artifact": str(runtime_ui_log),
                    "expected_result": "green_published_runtime_lane",
                },
                {
                    "case_id": "advanced_color_slider_contract",
                    "family": "critical_ui_behavior",
                    "status": "planned_blocker",
                    "required_contract": "advanced_color_slider_contract.v1",
                },
            ]
        },
    )
    baseline_dir = tmp_path / "baseline"
    baseline_json = baseline_dir / "baseline_index.json"
    baseline_md = baseline_dir / "baseline_index.md"
    assert main([
        "baselines",
        "--manifest",
        str(manifest_path),
        "--out-dir",
        str(baseline_dir),
        "--out",
        str(baseline_json),
        "--out-md",
        str(baseline_md),
    ]) == 0

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "advanced_color_slider_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": [
                        "code_quality_audit",
                        "test_coverage_audit",
                        "native_helper_tests",
                        "runtime_publish",
                        "runtime_probe_session",
                        "runtime_ui_harness",
                    ],
                }
            ]
        },
    )

    out_json = tmp_path / "parity.json"
    out_md = tmp_path / "parity.md"
    rc = main([
        "parity",
        "--baseline-suite",
        str(baseline_json),
        "--candidate-suite",
        str(packet_dir / "producer_index.json"),
        "--contract-registry",
        str(registry_path),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["status"] == "case_result_parity"
    assert payload["comparison_ready"] is True
    assert payload["matched_case_count"] == 7
    packets = {item["case_id"]: item for item in payload["case_packets"]}
    assert packets["code_quality_baseline"]["status"] == "expected_result_matched"
    assert packets["advanced_color_slider_contract"]["status"] == "contract_case_matched"
    assert out_md.exists()


def test_family_parity_emits_advanced_color_slider_packet(tmp_path: Path) -> None:
    policy_path = tmp_path / "policy.json"
    native_helper_log = tmp_path / "artifacts" / "native_helper.log"
    native_helper_log.parent.mkdir(parents=True, exist_ok=True)
    native_helper_log.write_text("All helper tests passed\n", encoding="utf-8")
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {
                    "producer_id": "native_helper_tests",
                    "artifact_path": str(native_helper_log),
                },
                {
                    "producer_id": "runtime_ui_harness",
                    "artifact_path": str(tmp_path / "artifacts" / "missing_ui.log"),
                },
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "family-parity"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "advanced_color_slider_contract",
                    "family": "critical_ui_behavior",
                    "status": "planned_blocker",
                    "required_contract": "advanced_color_slider_contract.v1",
                }
            ]
        },
    )
    baseline_dir = tmp_path / "baseline"
    baseline_json = baseline_dir / "baseline_index.json"
    baseline_md = baseline_dir / "baseline_index.md"
    assert main([
        "baselines",
        "--manifest",
        str(manifest_path),
        "--out-dir",
        str(baseline_dir),
        "--out",
        str(baseline_json),
        "--out-md",
        str(baseline_md),
    ]) == 0

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "advanced_color_slider_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": [
                        "native_helper_tests",
                        "runtime_ui_harness",
                    ],
                }
            ]
        },
    )

    out_json = tmp_path / "family_parity.json"
    out_md = tmp_path / "family_parity.md"
    rc = main([
        "family-parity",
        "--baseline-index",
        str(baseline_json),
        "--candidate-suite",
        str(packet_dir / "suite_index.json"),
        "--contract-registry",
        str(registry_path),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["status"] == "family_case_parity"
    assert payload["comparison_ready"] is True
    packet = payload["critical_family_packets"][0]
    assert packet["case_id"] == "advanced_color_slider_contract"
    assert packet["contract_id"] == "advanced_color_slider_contract.v1"
    assert packet["status"] == "missing_required_producers"
    assert packet["missing_producers"] == ["runtime_ui_harness"]
    assert out_md.exists()


def test_family_parity_requires_green_parity_case_when_contract_names_parity(tmp_path: Path) -> None:
    artifacts_dir = tmp_path / "artifacts"
    artifacts_dir.mkdir(parents=True, exist_ok=True)

    producer_paths = {
        "code_quality_audit": artifacts_dir / "code_quality.json",
        "test_coverage_audit": artifacts_dir / "coverage.json",
        "native_helper_tests": artifacts_dir / "native_helper.log",
        "runtime_publish": artifacts_dir / "runtime_publish.log",
        "runtime_probe_session": artifacts_dir / "runtime_probe.log",
        "runtime_ui_harness": artifacts_dir / "runtime_ui.log",
    }
    _write_json(producer_paths["code_quality_audit"], {"severity_counts": {"CRITICAL": 0, "ERROR": 0}})
    _write_json(producer_paths["test_coverage_audit"], {"coverage": [{"module": "viewer_cli", "status": "COVERED"}]})
    producer_paths["native_helper_tests"].write_text("All helper tests passed.\n", encoding="utf-8")
    producer_paths["runtime_publish"].write_text("[build_vsdevcmd] Active runtime: D:/salt/runtime/fractal_ui.exe\n", encoding="utf-8")
    producer_paths["runtime_probe_session"].write_text("viewer_host_runtime_pytest_lane: summary=passed=5 failed=0 skipped=0 errors=0\n", encoding="utf-8")
    producer_paths["runtime_ui_harness"].write_text("viewer_host_runtime_pytest_lane: summary=passed=3 failed=0 skipped=0 errors=0\n", encoding="utf-8")

    policy_path = tmp_path / "policy.json"
    _write_json(
        policy_path,
        {
            "producer_surfaces": [
                {"producer_id": producer_id, "artifact_path": str(path)}
                for producer_id, path in producer_paths.items()
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "parity"},
                {"command_id": "family-parity"},
                {"command_id": "doctor"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    packet_dir = tmp_path / "packet"
    packet_dir.mkdir()
    assert main(["audit", "--policy", str(policy_path), "--out-dir", str(packet_dir)]) == 0

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "advanced_color_slider_contract",
                    "family": "critical_ui_behavior",
                    "status": "planned_blocker",
                    "required_contract": "advanced_color_slider_contract.v1",
                }
            ]
        },
    )
    baseline_dir = tmp_path / "baseline"
    baseline_json = baseline_dir / "baseline_index.json"
    baseline_md = baseline_dir / "baseline_index.md"
    assert main([
        "baselines",
        "--manifest",
        str(manifest_path),
        "--out-dir",
        str(baseline_dir),
        "--out",
        str(baseline_json),
        "--out-md",
        str(baseline_md),
    ]) == 0

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "advanced_color_slider_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": list(producer_paths.keys()),
                    "required_packet_surfaces": ["audit", "parity", "family-parity", "doctor"],
                }
            ]
        },
    )

    parity_json = tmp_path / "parity.json"
    _write_json(
        parity_json,
        {
            "comparison_ready": False,
            "case_packets": [
                {
                    "case_id": "advanced_color_slider_contract",
                    "status": "required_producer_parity_mismatch",
                }
            ]
        },
    )

    out_json = tmp_path / "family_parity.json"
    out_md = tmp_path / "family_parity.md"
    rc = main([
        "family-parity",
        "--baseline-index",
        str(baseline_json),
        "--candidate-suite",
        str(packet_dir / "suite_index.json"),
        "--contract-registry",
        str(registry_path),
        "--parity",
        str(parity_json),
        "--out",
        str(out_json),
        "--out-md",
        str(out_md),
    ])

    assert rc == 0
    payload = json.loads(out_json.read_text(encoding="utf-8"))
    packet = payload["critical_family_packets"][0]
    assert packet["status"] == "missing_required_surfaces"
    assert packet["missing_required_surfaces"] == ["parity"]
    assert packet["parity_case_status"] == "required_producer_parity_mismatch"
    assert out_md.exists()


def test_freeze_gate_surface_materializes_current_packet_set(tmp_path: Path) -> None:
    code_quality_path = tmp_path / "artifacts" / "code_quality.json"
    code_quality_path.parent.mkdir(parents=True, exist_ok=True)
    _write_json(code_quality_path, {"score": 97, "checks": {"baseline": True}})

    runtime_publish_log = tmp_path / "artifacts" / "runtime_publish.log"
    runtime_publish_log.write_text("published runtime available\n", encoding="utf-8")

    policy_path = tmp_path / "policy.json"
    _write_json(
        policy_path,
        {
            "packet_root": "artifacts/salt_ndepend",
            "producer_surfaces": [
                {
                    "producer_id": "code_quality_audit",
                    "artifact_path": str(code_quality_path),
                },
                {
                    "producer_id": "runtime_publish",
                    "artifact_path": str(runtime_publish_log),
                },
                {
                    "producer_id": "runtime_ui_harness",
                    "artifact_path": str(tmp_path / "artifacts" / "missing_ui.log"),
                },
            ],
            "planned_command_surfaces": [
                {"command_id": "audit"},
                {"command_id": "structural"},
                {"command_id": "review"},
                {"command_id": "baselines"},
                {"command_id": "contracts"},
                {"command_id": "parity"},
                {"command_id": "family-parity"},
                {"command_id": "doctor"},
            ],
            "gated_product_threads": ["advanced_color"],
        },
    )

    manifest_path = tmp_path / "baselines.json"
    _write_json(
        manifest_path,
        {
            "cases": [
                {
                    "case_id": "runtime_ui_harness_baseline",
                    "family": "runtime_ui_behavior",
                    "status": "seeded",
                    "required_contract": "runtime_ui_harness_contract.v1",
                }
            ]
        },
    )

    registry_path = tmp_path / "contracts.json"
    _write_json(
        registry_path,
        {
            "contracts": [
                {
                    "contract_id": "runtime_ui_harness_contract.v1",
                    "blocked_product_threads": ["advanced_color"],
                    "required_producers": [
                        "runtime_publish",
                        "runtime_ui_harness",
                    ],
                }
            ]
        },
    )

    freeze_gate_path = tmp_path / "freeze_gate.json"
    _write_json(
        freeze_gate_path,
        {
            "required_packet_commands": [
                "audit",
                "structural",
                "review",
                "baselines",
                "contracts",
                "parity",
                "family-parity",
                "doctor",
            ],
            "required_blocker_contracts": ["runtime_ui_harness_contract.v1"],
            "intentional_open_blockers": [],
        },
    )

    out_dir = tmp_path / "packet"
    rc = main([
        "freeze-gate",
        "--policy",
        str(policy_path),
        "--manifest",
        str(manifest_path),
        "--contract-registry",
        str(registry_path),
        "--freeze-gate",
        str(freeze_gate_path),
        "--out-dir",
        str(out_dir),
    ])

    assert rc == 0
    assert (out_dir / "suite_index.json").exists()
    assert (out_dir / "producer_index.json").exists()
    assert (out_dir / "structural_index.json").exists()
    assert (out_dir / "review.json").exists()
    assert (out_dir / "baseline_index.json").exists()
    assert (out_dir / "contracts.json").exists()
    assert (out_dir / "parity.json").exists()
    assert (out_dir / "family_parity.json").exists()
    doctor_payload = json.loads((out_dir / "doctor.json").read_text(encoding="utf-8"))
    assert doctor_payload["freeze_ready"] is False
    finding_codes = {item["code"] for item in doctor_payload["findings"]}
    assert "required_blocker_contract_not_green:runtime_ui_harness_contract.v1" in finding_codes
