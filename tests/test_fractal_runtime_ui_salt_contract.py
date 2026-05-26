from __future__ import annotations

import json
import subprocess
from pathlib import Path

from tests.runtime_harness import RUNTIME_DIR, active_runtime_exe, runtime_automation_lock


STAGED_CONTRACT = RUNTIME_DIR / "ui_salt" / "generated" / "color_pipeline_function_library.contract.v1.json"


def test_published_runtime_consumes_staged_ui_salt_contract(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    report_path = tmp_path / "ui_salt_contract_report.json"

    with runtime_automation_lock():
        result = subprocess.run(
            [
                str(exe_path),
                "--validate-ui-salt-contract",
                "--ui-salt-contract-report-json",
                str(report_path),
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )

    assert result.returncode == 0, result.stderr or result.stdout
    assert STAGED_CONTRACT.exists(), f"published runtime did not stage {STAGED_CONTRACT}"
    assert report_path.exists(), "runtime contract validation did not write its report"
    report = json.loads(report_path.read_text(encoding="utf-8"))
    assert report["ok"] is True
    assert report["contract_path"] == str(STAGED_CONTRACT)
    assert report["schema_version"] == 1
    assert report["lane_count"] == 4
    assert report["function_count"] == 33
    assert report["compatibility_count"] == 20
    assert report["unsupported_pair_count"] > 0
    assert report["errors"] == []
