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
contract(kind="signal_type_registry", contract_id="viewer.signal_type_registry_contract.v1", version=1)
signal_type(id="scalar.unit", kind="scalar", domain="unit", topology="linear", arity=1, default_adapter_policy="safe")
signal_type(id="scalar.sdf_signed_distance", kind="scalar", domain="signed_distance", topology="linear", arity=1, units="field_px", default_adapter_policy="explicit_only")
signal_type(id="phase.radians", kind="phase", domain="angle", topology="circular", arity=1, units="radians", period=6.283185307179586, default_adapter_policy="explicit_only")
signal_type(id="category.root_index", kind="category", domain="root_index", topology="discrete", arity=1, default_adapter_policy="forbidden")
signal_type(id="palette.discrete_index", kind="palette", domain="discrete_index", topology="discrete", arity=1, default_adapter_policy="explicit_only")
row_applicator(id="none", label="None", target_lane="source", required_signal_kind="any", requires_sdf_field=False, storage_param="signal.sdf_gate", fail_closed_reason="ungated source row contribution")
row_applicator(id="sdf_boundary_band", label="SDF Boundary Band", target_lane="source", required_signal_kind="any", requires_sdf_field=True, storage_param="signal.sdf_gate", width_param="signal.sdf_gate_width_px", fail_closed_reason="requires an SDF field for boundary-band row masking")
row_applicator(id="sdf_inside", label="SDF Inside", target_lane="source", required_signal_kind="any", requires_sdf_field=True, storage_param="signal.sdf_gate", fail_closed_reason="requires an SDF field for inside row masking")
row_applicator(id="sdf_outside", label="SDF Outside", target_lane="source", required_signal_kind="any", requires_sdf_field=True, storage_param="signal.sdf_gate", fail_closed_reason="requires an SDF field for outside row masking")
lane(id="source", label="Source", default="smooth_escape_ramp")
lane(id="palette", label="Palette", default="heatmap")
function(lane="source", id="smooth_escape_ramp", label="Smooth Escape Ramp", taxonomy_group="escape", signal_kind="scalar", typed_signal="scalar.unit", runtime_backed=True, params=[["signal.scale", "float", "Scale", 0.25, 4.0, 0.01, 1.0], ["signal.bias", "float", "Bias", -1.0, 1.0, 0.01, 0.0]])
function(lane="source", id="sdf_signed_distance", label="SDF Signed Distance", taxonomy_group="sdf", signal_kind="scalar", typed_signal="scalar.sdf_signed_distance", runtime_backed=True, params=[["signal.scale", "float", "Distance Scale", -2.0, 2.0, 0.01, 0.05]])
function(lane="source", id="sdf_normal_angle", label="SDF Normal Angle", taxonomy_group="sdf_phase", signal_kind="phase", typed_signal="phase.radians", runtime_backed=True, params=[["signal.scale", "float", "Angle Scale", -2.0, 2.0, 0.01, 1.0]])
function(lane="source", id="root_index", label="Root Index", taxonomy_group="basin", signal_kind="categorical", typed_signal="category.root_index", runtime_backed=True)
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
        "signal_type_registry",
    ]
    signal_types = {item["id"]: item for item in payload["signal_type_registry"]["types"]}
    assert set(signal_types) == {
        "scalar.unit",
        "scalar.sdf_signed_distance",
        "phase.radians",
        "category.root_index",
        "palette.discrete_index",
    }
    assert signal_types["scalar.unit"]["kind"] == "scalar"
    assert signal_types["scalar.sdf_signed_distance"]["units"] == "field_px"
    assert signal_types["phase.radians"]["topology"] == "circular"
    assert signal_types["phase.radians"]["period"] == 6.283185307179586
    assert signal_types["category.root_index"]["default_adapter_policy"] == "forbidden"
    source_lane = payload["function_library"]["lanes"][0]
    assert source_lane["id"] == "source"
    assert [fn["id"] for fn in source_lane["functions"]] == [
        "smooth_escape_ramp",
        "sdf_signed_distance",
        "sdf_normal_angle",
        "root_index",
    ]
    typed_signals = {fn["id"]: fn.get("typed_signal") for fn in source_lane["functions"]}
    assert typed_signals == {
        "smooth_escape_ramp": "scalar.unit",
        "sdf_signed_distance": "scalar.sdf_signed_distance",
        "sdf_normal_angle": "phase.radians",
        "root_index": "category.root_index",
    }
    assert source_lane["functions"][2]["signal_kind"] == "phase"
    assert source_lane["functions"][0]["taxonomy_group"] == "escape"
    assert source_lane["functions"][2]["taxonomy_group"] == "sdf_phase"
    assert payload["composition_recipe_contract"]["compatibility"][0]["mode"] == "smooth_escape"
    assert payload["explaino_contract"]["entries"][0]["proof"] == "color_pipeline_metadata_parity"

    applicators = payload["composition_recipe_contract"]["row_applicators"]
    assert [item["id"] for item in applicators] == [
        "none",
        "sdf_boundary_band",
        "sdf_inside",
        "sdf_outside",
    ]
    assert all(item["target_lane"] == "source" for item in applicators)
    assert applicators[0]["requires_sdf_field"] is False
    assert applicators[1]["requires_sdf_field"] is True
    assert applicators[1]["width_param"] == "signal.sdf_gate_width_px"
    assert all(item["storage_param"] == "signal.sdf_gate" for item in applicators)



def test_materializer_rejects_duplicate_signal_type_ids(tmp_path):
    text = VALID_UI_SALT + 'signal_type(id="scalar.unit", kind="scalar", domain="unit", topology="linear", arity=1, default_adapter_policy="safe")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "duplicate signal_type id" in proc.stderr


def test_materializer_rejects_unknown_signal_type_kind(tmp_path):
    text = VALID_UI_SALT.replace('kind="phase"', 'kind="vector"', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "invalid signal_type kind" in proc.stderr


def test_materializer_rejects_ambiguous_palette_category_domain(tmp_path):
    text = VALID_UI_SALT.replace('kind="category", domain="root_index"', 'kind="category", domain="discrete_index"', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "palette discrete_index must use kind palette" in proc.stderr


def test_materializer_rejects_missing_signal_type_policy(tmp_path):
    text = VALID_UI_SALT.replace(', default_adapter_policy="safe"', '', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "signal_type scalar.unit requires default_adapter_policy" in proc.stderr


def test_materializer_rejects_function_typed_signal_unknown_type(tmp_path):
    text = VALID_UI_SALT.replace('typed_signal="scalar.unit"', 'typed_signal="scalar.missing"', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "typed_signal references unknown signal type" in proc.stderr

def test_materializer_rejects_duplicate_row_applicator_ids(tmp_path):
    text = VALID_UI_SALT + 'row_applicator(id="none", label="Duplicate", target_lane="source", required_signal_kind="any", requires_sdf_field=False, storage_param="signal.sdf_gate", fail_closed_reason="duplicate")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "duplicate row_applicator id" in proc.stderr


def test_materializer_rejects_invalid_row_applicator_target_lane(tmp_path):
    text = VALID_UI_SALT.replace('target_lane="source"', 'target_lane="palette"', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "row_applicator none has invalid target_lane" in proc.stderr


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


def test_materializer_rejects_invalid_row_applicator_signal_kind(tmp_path):
    text = VALID_UI_SALT.replace('required_signal_kind="any"', 'required_signal_kind="vector"', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "row_applicator none has invalid required_signal_kind" in proc.stderr


def test_materializer_requires_row_applicator_fail_closed_reason(tmp_path):
    text = VALID_UI_SALT.replace(', fail_closed_reason="ungated source row contribution"', '', 1)
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "row_applicator none requires fail_closed_reason" in proc.stderr


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
    assert len(lanes["source"]["functions"]) == 13
    assert len(lanes["shape"]["functions"]) == 7
    assert len(lanes["palette"]["functions"]) == 6
    assert len(lanes["grading"]["functions"]) == 8
    signal_kinds = {fn["id"]: fn.get("signal_kind") for fn in lanes["source"]["functions"]}
    assert signal_kinds["sdf_normal_angle"] == "phase"
    assert signal_kinds["sdf_inside_outside"] == "categorical"
    assert signal_kinds["lens_field_v2_distance"] == "scalar"
    signal_types = {item["id"]: item for item in actual["signal_type_registry"]["types"]}
    assert {
        "scalar.unit",
        "scalar.signed",
        "scalar.sdf_signed_distance",
        "phase.radians",
        "category.root_index",
        "category.inside_outside",
        "palette.discrete_index",
        "mask.alpha",
        "color.linear_rgb",
        "field.sdf_signed_distance",
    }.issubset(signal_types)
    assert signal_types["field.sdf_signed_distance"]["kind"] == "field"
    assert signal_types["scalar.sdf_signed_distance"]["kind"] == "scalar"
    typed_signals = {fn["id"]: fn.get("typed_signal") for fn in lanes["source"]["functions"]}
    assert typed_signals["smooth_escape_ramp"] == "scalar.unit"
    assert typed_signals["sdf_signed_distance"] == "scalar.sdf_signed_distance"
    assert typed_signals["sdf_normal_angle"] == "phase.radians"
    assert typed_signals["root_index"] == "category.root_index"
    assert typed_signals["sdf_inside_outside"] == "category.inside_outside"
    taxonomy_groups = {
        fn["id"]: fn.get("taxonomy_group")
        for lane in actual["function_library"]["lanes"]
        for fn in lane["functions"]
    }
    assert all(taxonomy_groups.values())
    assert taxonomy_groups["smooth_escape_ramp"] == "escape"
    assert taxonomy_groups["sdf_normal_angle"] == "sdf_phase"
    assert taxonomy_groups["lens_field_v2_distance"] == "lens_field_v2"
    assert taxonomy_groups["identity"] == "identity"
    assert taxonomy_groups["phase_wheel_palette"] == "palette_phase"
    assert taxonomy_groups["balance_void_grade"] == "grade_manifold"
    lens_v2 = next(fn for fn in lanes["source"]["functions"] if fn["id"] == "lens_field_v2_distance")
    lens_v2_params = {param["path"]: param for param in lens_v2["params"]}
    assert lens_v2_params["signal.sign_contrast"]["default"] == 0.35
    assert lens_v2_params["signal.sign_contrast"]["min"] == 0.0
    assert lens_v2_params["signal.sign_contrast"]["max"] == 1.0
    row_applicators = actual["composition_recipe_contract"]["row_applicators"]
    assert [item["id"] for item in row_applicators] == [
        "none",
        "sdf_boundary_band",
        "sdf_inside",
        "sdf_outside",
    ]
    assert row_applicators[0]["requires_sdf_field"] is False
    assert row_applicators[1]["requires_sdf_field"] is True
    assert row_applicators[1]["required_signal_kind"] == "any"
    assert row_applicators[1]["width_param"] == "signal.sdf_gate_width_px"
    assert all(item["target_lane"] == "source" for item in row_applicators)
    assert all(item["fail_closed_reason"] for item in row_applicators)
    assert len(actual["composition_recipe_contract"]["compatibility"]) == 22
