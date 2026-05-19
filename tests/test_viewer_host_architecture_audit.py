from __future__ import annotations

import json
from pathlib import Path

from tools import viewer_host_architecture_audit as audit


def test_architecture_audit_reports_known_risk_surfaces() -> None:
    report = audit.build_report()
    paths = {row["path"] for row in report["risk_surfaces"]}
    assert "ui_app/src/main.cpp" in paths
    assert "ui_app/src/color_pipeline_window.h" in paths
    assert "ui_app/src/schema_binding.cpp" in paths
    assert report["ok"] is True


def test_architecture_audit_exposes_cppdepend_readiness_gap() -> None:
    report = audit.build_report()
    status = report["mainline_tool_status"]
    assert status["native_architecture_audit_present"] is True
    assert status["cppdepend_status_present"] is True
    assert status["repo_compile_commands_present"] is False
    assert any(row["code"] == "cppdepend_not_ready" for row in report["findings"])


def test_architecture_audit_writes_parseable_json(tmp_path: Path) -> None:
    out = tmp_path / "architecture.json"
    report = audit.build_report()
    audit.write_report(report, out)
    loaded = json.loads(out.read_text(encoding="utf-8"))
    assert loaded["schema_version"] == "viewer_host_architecture_audit.v1"
    assert isinstance(loaded["risk_surfaces"], list)
