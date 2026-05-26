import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
TOOL = REPO_ROOT / "tools" / "viewer_host_materialize_ui_salt.py"
COLOR_PIPELINE_UI_SALT = REPO_ROOT / "docs" / "ui_salt" / "color_pipeline_function_library.ui.salt"
COLOR_PIPELINE_GENERATED = (
    REPO_ROOT
    / "docs"
    / "ui_salt"
    / "generated"
    / "color_pipeline_function_library.contract.v1.json"
)


VALID_UI_SALT = '''
contract(kind="function_library", contract_id="viewer.function_library_contract.v1", version=1)
contract(kind="composition_recipe", contract_id="viewer.composition_recipe_contract.v1", version=1)
contract(kind="explaino", contract_id="viewer.explaino_contract.v1", version=1)
lane(id="source", label="Source", default="smooth_escape_ramp")
lane(id="palette", label="Palette", default="heatmap")
function(lane="source", id="smooth_escape_ramp", label="Smooth Escape Ramp", taxonomy_group="escape", signal_kind="scalar", runtime_backed=True, params=[["signal.scale", "float", "Scale", 0.25, 4.0, 0.01, 1.0], ["signal.bias", "float", "Bias", -1.0, 1.0, 0.01, 0.0]])
function(lane="source", id="sdf_normal_angle", label="SDF Normal Angle", taxonomy_group="sdf_phase", signal_kind="phase", runtime_backed=True, params=[["signal.scale", "float", "Angle Scale", -2.0, 2.0, 0.01, 1.0]])
function(lane="palette", id="heatmap", label="Heatmap", taxonomy_group="palette_escape", runtime_backed=True, params=[["palette.cycle_scale", "float", "Cycle Scale", 0.25, 4.0, 0.01, 1.0]])
compat(source="smooth_escape_ramp", palette="heatmap", signal="smooth_escape_ramp", palette_runtime="heatmap", grading="contrast_lift", mode="smooth_escape")
explaino_contract(id="color_pipeline.explaino_cmap", hypothesis_space="color_pipeline_source_signal", authority="palette_row", lens="source_signal_to_explaino_cmap", invariant="fail_closed_runtime_backing", proof="color_pipeline_metadata_parity", fallback="fail_closed", product_facing=False, diagnostic=True)
'''


def run_materializer(tmp_path: Path, text: str):
    source = tmp_path / "case.ui.salt"
    out = tmp_path / "out.json"
    source.write_text(text, encoding="utf-8")
    proc = subprocess.run(
        [sys.executable, str(TOOL), "--ui-salt", str(source), "--out", str(out)],
        cwd=str(REPO_ROOT),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    return proc, out


def test_materializer_accepts_valid_contract(tmp_path):
    proc, out = run_materializer(tmp_path, VALID_UI_SALT)
    assert proc.returncode == 0, proc.stderr
    payload = json.loads(out.read_text(encoding="utf-8"))

    assert payload["schema_version"] == 1
    assert [item["kind"] for item in payload["contracts"]] == [
        "function_library",
        "composition_recipe",
        "explaino",
    ]
    source_lane = payload["function_library"]["lanes"][0]
    assert source_lane["id"] == "source"
    assert [fn["id"] for fn in source_lane["functions"]] == [
        "smooth_escape_ramp",
        "sdf_normal_angle",
    ]
    assert source_lane["functions"][1]["signal_kind"] == "phase"
    assert source_lane["functions"][0]["taxonomy_group"] == "escape"
    assert source_lane["functions"][1]["taxonomy_group"] == "sdf_phase"
    assert payload["composition_recipe_contract"]["compatibility"][0]["mode"] == "smooth_escape"
    assert payload["explaino_contract"]["entries"][0]["proof"] == "color_pipeline_metadata_parity"


def test_materializer_rejects_unknown_statement(tmp_path):
    proc, _ = run_materializer(tmp_path, 'surprise(id="bad")\n')
    assert proc.returncode != 0
    assert "unknown statement" in proc.stderr


def test_materializer_rejects_duplicate_function_ids(tmp_path):
    text = VALID_UI_SALT + 'function(lane="source", id="smooth_escape_ramp", label="Duplicate", signal_kind="scalar", runtime_backed=True)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "duplicate function id" in proc.stderr


def test_materializer_rejects_invalid_signal_kind(tmp_path):
    text = VALID_UI_SALT.replace('signal_kind="phase"', 'signal_kind="vector"')
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "invalid signal_kind" in proc.stderr


def test_materializer_requires_taxonomy_group(tmp_path):
    text = VALID_UI_SALT.replace('taxonomy_group="escape", ', '')
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "function smooth_escape_ramp requires taxonomy_group" in proc.stderr


def test_materializer_rejects_invalid_param_range(tmp_path):
    text = VALID_UI_SALT.replace('0.25, 4.0, 0.01, 1.0', '4.0, 0.25, 0.01, 1.0')
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "min greater than max" in proc.stderr


def test_materializer_requires_explaino_proof_fields(tmp_path):
    text = VALID_UI_SALT.replace('proof="color_pipeline_metadata_parity", ', '')
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "explaino_contract requires proof" in proc.stderr


def test_checked_in_color_pipeline_contract_is_fresh(tmp_path):
    out = tmp_path / "materialized.json"
    proc = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--ui-salt",
            str(COLOR_PIPELINE_UI_SALT),
            "--out",
            str(out),
        ],
        cwd=str(REPO_ROOT),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    assert proc.returncode == 0, proc.stderr
    actual = json.loads(out.read_text(encoding="utf-8"))
    checked_in = json.loads(COLOR_PIPELINE_GENERATED.read_text(encoding="utf-8"))

    assert actual == checked_in
    lanes = {lane["id"]: lane for lane in actual["function_library"]["lanes"]}
    assert [lane["id"] for lane in actual["function_library"]["lanes"]] == [
        "source",
        "shape",
        "palette",
        "grading",
    ]
    assert len(lanes["source"]["functions"]) == 12
    assert len(lanes["shape"]["functions"]) == 7
    assert len(lanes["palette"]["functions"]) == 6
    assert len(lanes["grading"]["functions"]) == 8
    signal_kinds = {fn["id"]: fn.get("signal_kind") for fn in lanes["source"]["functions"]}
    assert signal_kinds["sdf_normal_angle"] == "phase"
    assert signal_kinds["sdf_inside_outside"] == "categorical"
    taxonomy_groups = {
        fn["id"]: fn.get("taxonomy_group")
        for lane in actual["function_library"]["lanes"]
        for fn in lane["functions"]
    }
    assert all(taxonomy_groups.values())
    assert taxonomy_groups["smooth_escape_ramp"] == "escape"
    assert taxonomy_groups["sdf_normal_angle"] == "sdf_phase"
    assert taxonomy_groups["identity"] == "identity"
    assert taxonomy_groups["phase_wheel_palette"] == "palette_phase"
    assert taxonomy_groups["balance_void_grade"] == "grade_manifold"
    assert len(actual["composition_recipe_contract"]["compatibility"]) == 20
