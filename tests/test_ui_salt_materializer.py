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
contract(kind="adapter_library", contract_id="viewer.adapter_library_contract.v1", version=1)
signal_type(id="scalar.unit", kind="scalar", domain="unit", topology="linear", arity=1, default_adapter_policy="safe")
signal_type(id="scalar.signed", kind="scalar", domain="signed", topology="linear", arity=1, default_adapter_policy="explicit_only")
signal_type(id="scalar.sdf_signed_distance", kind="scalar", domain="signed_distance", topology="linear", arity=1, units="field_px", default_adapter_policy="explicit_only")
signal_type(id="phase.radians", kind="phase", domain="angle", topology="circular", arity=1, units="radians", period=6.283185307179586, default_adapter_policy="explicit_only")
signal_type(id="category.root_index", kind="category", domain="root_index", topology="discrete", arity=1, default_adapter_policy="forbidden")
signal_type(id="palette.discrete_index", kind="palette", domain="discrete_index", topology="discrete", arity=1, default_adapter_policy="explicit_only")
signal_type(id="mask.alpha", kind="mask", domain="alpha", topology="mask", arity=1, default_adapter_policy="explicit_only")
signal_type(id="color.linear_rgb", kind="color", domain="linear_rgb", topology="color", arity=3, color_space="linear_rgb", default_adapter_policy="forbidden")
signal_type(id="field.sdf_signed_distance", kind="field", domain="signed_distance", topology="field", arity=1, units="field_px", coordinate_space="sdf_field", default_adapter_policy="forbidden")
adapter(id="identity.scalar_unit", source="scalar.unit", target="scalar.unit", policy="safe", lossy=False, reversible=True, cost=0)
adapter(id="normalize.sdf_signed_distance.unit", source="scalar.sdf_signed_distance", target="scalar.unit", policy="explicit_only", lossy=True, reversible=False, cost=2, fail_closed_reason="requires explicit signed-distance normalization")
adapter(id="phase.radians.unit_wrap", source="phase.radians", target="scalar.unit", policy="visible_default", lossy=True, reversible=False, cost=2, fail_closed_reason="phase-to-unit wrapping is a visible projection")
adapter(id="root_index.palette_discrete_index", source="category.root_index", target="palette.discrete_index", policy="visible_default", lossy=False, reversible=False, cost=1, fail_closed_reason="root category palette mapping requires a root palette")
adapter(id="field.sdf_signed_distance.boundary_mask", source="field.sdf_signed_distance", target="mask.alpha", policy="explicit_only", lossy=True, reversible=False, cost=3, fail_closed_reason="raw SDF field to boundary mask requires explicit boundary width")
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


EDGE_RESOLUTION_FIXTURE = VALID_UI_SALT + '''
contract(kind="edge_resolution", contract_id="viewer.edge_resolution_contract.v1", version=1)
contract(kind="resolution_audit", contract_id="viewer.color_pipeline_resolution_audit.v1", version=1)
lane(id="shape", label="Shape", default="identity")
lane(id="grading", label="Grading", default="contrast_lift")
function(lane="shape", id="identity", label="Identity", taxonomy_group="identity", runtime_backed=True)
function(lane="shape", id="repeat", label="Repeat", taxonomy_group="repeat", runtime_backed=True)
function(lane="shape", id="bias_gain_curve", label="Bias + Gain Curve", taxonomy_group="remap", runtime_backed=True)
function(lane="palette", id="phase_wheel_palette", label="Phase Wheel", taxonomy_group="palette_phase", runtime_backed=True)
function(lane="palette", id="root_classic_palette", label="Root Classic Palette", taxonomy_group="palette_basin", runtime_backed=True)
function(lane="grading", id="contrast_lift", label="Contrast Lift", taxonomy_group="grade_escape", runtime_backed=True)
function(lane="grading", id="phase_finish", label="Phase Finish", taxonomy_group="grade_phase", runtime_backed=True)
function(lane="grading", id="basin_default", label="Basin Default", taxonomy_group="grade_basin", runtime_backed=True)
function(lane="source", id="raw_sdf_field_debug", label="Raw SDF Field Debug", taxonomy_group="sdf_debug", typed_signal="field.sdf_signed_distance", runtime_backed=False)
port(function="smooth_escape_ramp", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="sdf_signed_distance", direction="output", id="signal", type="scalar.sdf_signed_distance", canonical=True)
port(function="sdf_normal_angle", direction="output", id="signal", type="phase.radians", canonical=True)
port(function="root_index", direction="output", id="signal", type="category.root_index", canonical=True)
port(function="raw_sdf_field_debug", direction="output", id="signal", type="field.sdf_signed_distance", canonical=True)
port(function="identity", direction="input", id="signal", type="generic.T", generic_group="T")
port(function="identity", direction="output", id="signal", type="generic.T", generic_group="T", canonical=True)
port(function="repeat", direction="input", id="signal", type="scalar.unit")
port(function="repeat", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="bias_gain_curve", direction="input", id="signal", type="scalar.unit")
port(function="bias_gain_curve", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="heatmap", direction="input", id="signal", type="scalar.unit")
port(function="heatmap", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="phase_wheel_palette", direction="input", id="signal", type="phase.radians")
port(function="phase_wheel_palette", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="root_classic_palette", direction="input", id="signal", type="category.root_index")
port(function="root_classic_palette", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="contrast_lift", direction="input", id="color", type="color.linear_rgb")
port(function="contrast_lift", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="phase_finish", direction="input", id="color", type="color.linear_rgb")
port(function="phase_finish", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="basin_default", direction="input", id="color", type="color.linear_rgb")
port(function="basin_default", direction="output", id="color", type="color.linear_rgb", canonical=True)
edge_policy(id="current_linear_color_stack", max_adapter_hops=2, allow_lossy=False, allow_visible_default=True, allow_explicit=False, allow_diagnostic=False, fail_closed_default=True)
edge_link(id="source_to_shape", from_lane="source", to_lane="shape", from_port="signal", to_port="signal", fail_closed_reason="source output cannot feed selected shape")
edge_link(id="shape_to_palette", from_lane="shape", to_lane="palette", from_port="signal", to_port="signal", fail_closed_reason="shape output cannot feed selected palette")
edge_link(id="palette_to_grading", from_lane="palette", to_lane="grading", from_port="color", to_port="color", fail_closed_reason="palette output cannot feed selected grading")
resolution_case(id="smooth_escape_heatmap", source="smooth_escape_ramp", shape="identity", palette="heatmap", grading="contrast_lift", expect="resolved")
resolution_case(id="phase_orbit_wheel", source="sdf_normal_angle", shape="identity", palette="phase_wheel_palette", grading="phase_finish", expect="resolved")
resolution_case(id="root_classic", source="root_index", shape="identity", palette="root_classic_palette", grading="basin_default", expect="resolved")
resolution_case(id="sdf_signed_normalized_heatmap", source="sdf_signed_distance", shape="bias_gain_curve", palette="heatmap", grading="contrast_lift", expect="resolved", allow_lossy=True, explicit_adapter_consent=True)
resolution_case(id="root_repeat_heatmap_bad", source="root_index", shape="repeat", palette="heatmap", grading="contrast_lift", expect="fail_closed", fail_closed_reason="root category cannot enter scalar repeat/heatmap route")
resolution_case(id="phase_root_palette_bad", source="sdf_normal_angle", shape="identity", palette="root_classic_palette", grading="basin_default", expect="fail_closed", fail_closed_reason="phase cannot enter root palette route")
resolution_case(id="raw_sdf_field_phase_palette_bad", source="raw_sdf_field_debug", shape="identity", palette="phase_wheel_palette", grading="phase_finish", expect="fail_closed", fail_closed_reason="raw SDF field cannot enter phase palette route")
'''


COMPAT_OVERRIDE_FIXTURE = EDGE_RESOLUTION_FIXTURE + '''
contract(kind="compat_override_audit", contract_id="viewer.compat_override_audit_contract.v1", version=1)
function(lane="source", id="legacy_escape_shape", label="Legacy Escape Shape", taxonomy_group="legacy_escape", signal_kind="scalar", typed_signal="scalar.unit", runtime_backed=True)
function(lane="palette", id="legacy_escape_palette", label="Legacy Escape Palette", taxonomy_group="legacy_palette", runtime_backed=True)
compat(source="legacy_escape_shape", palette="legacy_escape_palette", signal="legacy_escape_shape", palette_runtime="legacy_escape_palette", grading="contrast_lift", mode="smooth_escape", reason="legacy runtime bridge")
compat_override(id="legacy_escape_palette_bridge", source="legacy_escape_shape", palette="legacy_escape_palette", grading="contrast_lift", classification="runtime_legacy_override", owner_seam="color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds", reason="legacy palette has no typed port signature yet", proof="compat_override_audit")
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
        "adapter_library",
    ]
    signal_types = {item["id"]: item for item in payload["signal_type_registry"]["types"]}
    assert set(signal_types) == {
        "scalar.unit",
        "scalar.signed",
        "scalar.sdf_signed_distance",
        "phase.radians",
        "category.root_index",
        "palette.discrete_index",
        "mask.alpha",
        "color.linear_rgb",
        "field.sdf_signed_distance",
    }
    assert signal_types["scalar.unit"]["kind"] == "scalar"
    assert signal_types["scalar.sdf_signed_distance"]["units"] == "field_px"
    assert signal_types["phase.radians"]["topology"] == "circular"
    assert signal_types["phase.radians"]["period"] == 6.283185307179586
    assert signal_types["category.root_index"]["default_adapter_policy"] == "forbidden"
    adapters = {item["id"]: item for item in payload["adapter_library_contract"]["adapters"]}
    assert adapters["identity.scalar_unit"] == {
        "id": "identity.scalar_unit",
        "source": "scalar.unit",
        "target": "scalar.unit",
        "policy": "safe",
        "lossy": False,
        "reversible": True,
        "cost": 0,
        "fail_closed_reason": "",
    }
    assert adapters["normalize.sdf_signed_distance.unit"]["policy"] == "explicit_only"
    assert adapters["normalize.sdf_signed_distance.unit"]["lossy"] is True
    assert adapters["phase.radians.unit_wrap"]["policy"] == "visible_default"
    assert adapters["root_index.palette_discrete_index"]["target"] == "palette.discrete_index"
    assert adapters["field.sdf_signed_distance.boundary_mask"]["source"] == "field.sdf_signed_distance"
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


def test_materializer_rejects_duplicate_adapter_ids(tmp_path):
    text = VALID_UI_SALT + 'adapter(id="identity.scalar_unit", source="scalar.unit", target="scalar.unit", policy="safe", lossy=False, reversible=True, cost=0)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "duplicate adapter id" in proc.stderr


def test_materializer_rejects_adapter_unknown_source_or_target_type(tmp_path):
    text = VALID_UI_SALT + 'adapter(id="bad.unknown", source="scalar.missing", target="scalar.unit", policy="explicit_only", lossy=False, reversible=False, cost=1, fail_closed_reason="missing source")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.unknown references unknown source type" in proc.stderr

    text = VALID_UI_SALT + 'adapter(id="bad.unknown.target", source="scalar.unit", target="scalar.missing", policy="explicit_only", lossy=False, reversible=False, cost=1, fail_closed_reason="missing target")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.unknown.target references unknown target type" in proc.stderr


def test_materializer_rejects_adapter_missing_or_invalid_policy(tmp_path):
    text = VALID_UI_SALT + 'adapter(id="bad.no_policy", source="scalar.unit", target="scalar.unit", lossy=False, reversible=True, cost=0)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.no_policy requires policy" in proc.stderr

    text = VALID_UI_SALT + 'adapter(id="bad.policy", source="scalar.unit", target="scalar.unit", policy="maybe", lossy=False, reversible=True, cost=0)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.policy has invalid policy" in proc.stderr


def test_materializer_rejects_adapter_invalid_cost_and_missing_reason(tmp_path):
    text = VALID_UI_SALT + 'adapter(id="bad.cost", source="scalar.unit", target="scalar.unit", policy="safe", lossy=False, reversible=True, cost=-1)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.cost requires non-negative integer cost" in proc.stderr

    text = VALID_UI_SALT + 'adapter(id="bad.reason", source="scalar.unit", target="phase.radians", policy="explicit_only", lossy=False, reversible=False, cost=1)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "adapter bad.reason requires fail_closed_reason for non-safe policy" in proc.stderr


def test_materializer_rejects_adapter_lossy_safe_and_risky_safe_conversions(tmp_path):
    text = VALID_UI_SALT + 'adapter(id="bad.lossy.safe", source="phase.radians", target="scalar.unit", policy="safe", lossy=True, reversible=False, cost=1)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "lossy adapters cannot use safe policy" in proc.stderr

    text = VALID_UI_SALT + 'adapter(id="bad.category.scalar", source="category.root_index", target="scalar.unit", policy="visible_default", lossy=False, reversible=False, cost=1, fail_closed_reason="category projection")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "category-to-scalar adapters cannot be safe or visible_default" in proc.stderr

    text = VALID_UI_SALT + 'adapter(id="bad.signed.unit", source="scalar.signed", target="scalar.unit", policy="safe", lossy=False, reversible=False, cost=1)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "signed-to-unit adapters require explicit normalization policy" in proc.stderr

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




def _function_by_id(payload, lane_id, function_id):
    lanes = {lane["id"]: lane for lane in payload["function_library"]["lanes"]}
    lane = lanes[lane_id]
    return next(fn for fn in lane["functions"] if fn["id"] == function_id)


def _ports(payload, lane_id, function_id):
    return _function_by_id(payload, lane_id, function_id).get("ports", [])


def test_materializer_accepts_pilot_port_signatures(tmp_path):
    text = VALID_UI_SALT + """
lane(id="shape", label="Shape", default="identity")
lane(id="grading", label="Grading", default="contrast_lift")
function(lane="shape", id="identity", label="Identity", taxonomy_group="identity", runtime_backed=True)
function(lane="shape", id="repeat", label="Repeat", taxonomy_group="repeat", runtime_backed=True)
function(lane="shape", id="bias_gain_curve", label="Bias + Gain Curve", taxonomy_group="remap", runtime_backed=True)
function(lane="palette", id="root_classic_palette", label="Root Classic Palette", taxonomy_group="palette_basin", runtime_backed=True)
function(lane="grading", id="contrast_lift", label="Contrast Lift", taxonomy_group="grade_escape", runtime_backed=True)
port(function="smooth_escape_ramp", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="sdf_signed_distance", direction="output", id="signal", type="scalar.sdf_signed_distance", canonical=True)
port(function="sdf_normal_angle", direction="output", id="signal", type="phase.radians", canonical=True)
port(function="root_index", direction="output", id="signal", type="category.root_index", canonical=True)
port(function="identity", direction="input", id="signal", type="generic.T", generic_group="T")
port(function="identity", direction="output", id="signal", type="generic.T", generic_group="T", canonical=True)
port(function="repeat", direction="input", id="signal", type="scalar.unit")
port(function="repeat", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="bias_gain_curve", direction="input", id="signal", type="scalar.unit")
port(function="bias_gain_curve", direction="output", id="signal", type="scalar.unit", canonical=True)
port(function="heatmap", direction="input", id="signal", type="scalar.unit")
port(function="heatmap", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="root_classic_palette", direction="input", id="signal", type="category.root_index")
port(function="root_classic_palette", direction="output", id="color", type="color.linear_rgb", canonical=True)
port(function="contrast_lift", direction="input", id="color", type="color.linear_rgb")
port(function="contrast_lift", direction="output", id="color", type="color.linear_rgb", canonical=True)
"""
    proc, out = run_materializer(tmp_path, text)
    assert proc.returncode == 0, proc.stderr
    payload = json.loads(out.read_text(encoding="utf-8"))

    assert _ports(payload, "source", "smooth_escape_ramp") == [
        {"direction": "output", "id": "signal", "type": "scalar.unit", "canonical": True}
    ]
    assert _ports(payload, "shape", "identity") == [
        {"direction": "input", "id": "signal", "type": "generic.T", "generic_group": "T"},
        {"direction": "output", "id": "signal", "type": "generic.T", "generic_group": "T", "canonical": True},
    ]
    assert _ports(payload, "shape", "repeat")[0]["type"] == "scalar.unit"
    assert _ports(payload, "shape", "bias_gain_curve")[1] == {
        "direction": "output",
        "id": "signal",
        "type": "scalar.unit",
        "canonical": True,
    }
    assert _ports(payload, "palette", "heatmap")[-1]["type"] == "color.linear_rgb"
    assert _ports(payload, "grading", "contrast_lift")[-1]["type"] == "color.linear_rgb"


def test_materializer_accepts_edge_resolution_shadow_audit(tmp_path):
    proc, out = run_materializer(tmp_path, EDGE_RESOLUTION_FIXTURE)
    assert proc.returncode == 0, proc.stderr
    payload = json.loads(out.read_text(encoding="utf-8"))

    assert [item["kind"] for item in payload["contracts"]][-2:] == [
        "edge_resolution",
        "resolution_audit",
    ]
    policy = payload["edge_resolution_contract"]["policy"]
    assert policy == {
        "id": "current_linear_color_stack",
        "max_adapter_hops": 2,
        "allow_lossy": False,
        "allow_visible_default": True,
        "allow_explicit": False,
        "allow_diagnostic": False,
        "fail_closed_default": True,
    }
    assert [edge["id"] for edge in payload["edge_resolution_contract"]["edges"]] == [
        "source_to_shape",
        "shape_to_palette",
        "palette_to_grading",
    ]

    cases = {case["id"]: case for case in payload["color_pipeline_resolution_audit"]["cases"]}
    assert cases["smooth_escape_heatmap"]["status"] == "resolved"
    assert cases["smooth_escape_heatmap"]["chosen_adapters"] == []
    assert cases["smooth_escape_heatmap"]["adapter_hops"] == 0
    assert cases["smooth_escape_heatmap"]["adapter_cost"] == 0
    assert cases["smooth_escape_heatmap"]["policy_blockers"] == []
    assert [edge["status"] for edge in cases["smooth_escape_heatmap"]["route_edges"]] == [
        "direct",
        "direct",
        "direct",
    ]
    assert cases["sdf_signed_normalized_heatmap"]["status"] == "resolved"
    assert cases["sdf_signed_normalized_heatmap"]["chosen_adapters"] == [
        "normalize.sdf_signed_distance.unit"
    ]
    assert cases["sdf_signed_normalized_heatmap"]["adapter_hops"] == 1
    assert cases["sdf_signed_normalized_heatmap"]["adapter_cost"] == 2
    assert cases["sdf_signed_normalized_heatmap"]["tie_break_rule"] == (
        "exact_identity_safe_non_lossy_lower_cost_fewer_hops_declaration_order"
    )
    assert cases["sdf_signed_normalized_heatmap"]["route_edges"][0]["adapters"] == [
        "normalize.sdf_signed_distance.unit"
    ]
    assert cases["sdf_signed_normalized_heatmap"]["route_edges"][0]["adapter_hops"] == 1
    assert cases["sdf_signed_normalized_heatmap"]["route_edges"][0]["adapter_cost"] == 2
    assert cases["root_repeat_heatmap_bad"]["status"] == "fail_closed"
    assert "root category" in cases["root_repeat_heatmap_bad"]["fail_closed_reason"]
    assert cases["root_repeat_heatmap_bad"]["policy_blockers"]
    assert cases["phase_root_palette_bad"]["status"] == "fail_closed"
    assert cases["raw_sdf_field_phase_palette_bad"]["status"] == "fail_closed"
    assert "raw SDF field" in cases["raw_sdf_field_phase_palette_bad"]["fail_closed_reason"]


def test_materializer_rejects_resolution_case_without_explicit_consent_for_explicit_adapter(tmp_path):
    text = EDGE_RESOLUTION_FIXTURE.replace(
        ', allow_lossy=True, explicit_adapter_consent=True)',
        ', allow_lossy=True)',
        1,
    )
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "resolution_case sdf_signed_normalized_heatmap expected resolved but got fail_closed" in proc.stderr
    assert "explicit adapter consent" in proc.stderr


def test_materializer_rejects_resolution_case_expected_fail_but_resolves(tmp_path):
    text = EDGE_RESOLUTION_FIXTURE.replace(
        'resolution_case(id="smooth_escape_heatmap", source="smooth_escape_ramp", shape="identity", palette="heatmap", grading="contrast_lift", expect="resolved")',
        'resolution_case(id="smooth_escape_heatmap", source="smooth_escape_ramp", shape="identity", palette="heatmap", grading="contrast_lift", expect="fail_closed", fail_closed_reason="should not resolve")',
    )
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "resolution_case smooth_escape_heatmap expected fail_closed but resolved" in proc.stderr


def test_materializer_accepts_compat_override_audit(tmp_path):
    proc, out = run_materializer(tmp_path, COMPAT_OVERRIDE_FIXTURE)
    assert proc.returncode == 0, proc.stderr
    payload = json.loads(out.read_text(encoding="utf-8"))

    assert payload["contracts"][-1]["kind"] == "compat_override_audit"
    overrides = payload["composition_recipe_contract"]["compat_overrides"]
    assert overrides == [
        {
            "id": "legacy_escape_palette_bridge",
            "source": "legacy_escape_shape",
            "palette": "legacy_escape_palette",
            "grading": "contrast_lift",
            "classification": "runtime_legacy_override",
            "owner_seam": "color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds",
            "reason": "legacy palette has no typed port signature yet",
            "proof": "compat_override_audit",
        }
    ]
    audit = {
        (row["source"], row["palette"], row["grading"]): row
        for row in payload["composition_recipe_contract"]["compatibility_audit"]
    }
    assert audit[("smooth_escape_ramp", "heatmap", "contrast_lift")]["classification"] == "typed_resolved"
    assert audit[("smooth_escape_ramp", "heatmap", "contrast_lift")]["route_case_id"] == "smooth_escape_heatmap"
    legacy_row = audit[("legacy_escape_shape", "legacy_escape_palette", "contrast_lift")]
    assert legacy_row["classification"] == "runtime_legacy_override"
    assert legacy_row["override_id"] == "legacy_escape_palette_bridge"
    assert legacy_row["reason"] == "legacy palette has no typed port signature yet"


def test_materializer_rejects_unclassified_compat_row(tmp_path):
    text = COMPAT_OVERRIDE_FIXTURE.replace(
        'compat_override(id="legacy_escape_palette_bridge", source="legacy_escape_shape", palette="legacy_escape_palette", grading="contrast_lift", classification="runtime_legacy_override", owner_seam="color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds", reason="legacy palette has no typed port signature yet", proof="compat_override_audit")\n',
        "",
    )
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "compat row legacy_escape_shape + legacy_escape_palette + contrast_lift is not typed-resolved and has no compat_override" in proc.stderr


def test_materializer_rejects_override_for_typed_resolved_row(tmp_path):
    text = COMPAT_OVERRIDE_FIXTURE + 'compat_override(id="bad_typed_route_override", source="smooth_escape_ramp", palette="heatmap", grading="contrast_lift", classification="runtime_legacy_override", owner_seam="color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds", reason="typed direct route should not need an override", proof="compat_override_audit")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "compat_override bad_typed_route_override does not map to a compatibility row" in proc.stderr


def test_materializer_rejects_unknown_port_signal_type(tmp_path):
    text = VALID_UI_SALT + 'port(function="smooth_escape_ramp", direction="output", id="signal", type="scalar.missing", canonical=True)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "port references unknown signal type" in proc.stderr


def test_materializer_rejects_any_port_type(tmp_path):
    text = VALID_UI_SALT + 'port(function="smooth_escape_ramp", direction="output", id="signal", type="any", canonical=True)\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "port type 'any' is forbidden" in proc.stderr


def test_materializer_rejects_source_input_port(tmp_path):
    text = VALID_UI_SALT + """
port(function="smooth_escape_ramp", direction="input", id="signal", type="scalar.unit")
port(function="smooth_escape_ramp", direction="output", id="signal", type="scalar.unit", canonical=True)
"""
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "source function smooth_escape_ramp cannot declare input ports" in proc.stderr


def test_materializer_rejects_generic_any_port_type(tmp_path):
    text = VALID_UI_SALT + """
lane(id="shape", label="Shape", default="identity")
function(lane="shape", id="identity", label="Identity", taxonomy_group="identity", runtime_backed=True)
port(function="identity", direction="input", id="signal", type="generic.any", generic_group="any")
port(function="identity", direction="output", id="signal", type="generic.any", generic_group="any", canonical=True)
"""
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "generic_group 'any' is forbidden" in proc.stderr


def test_materializer_rejects_identity_extra_ports(tmp_path):
    text = VALID_UI_SALT + """
lane(id="shape", label="Shape", default="identity")
function(lane="shape", id="identity", label="Identity", taxonomy_group="identity", runtime_backed=True)
port(function="identity", direction="input", id="signal", type="generic.T", generic_group="T")
port(function="identity", direction="output", id="signal", type="generic.T", generic_group="T", canonical=True)
port(function="identity", direction="input", id="aux", type="scalar.unit")
"""
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "identity ports must declare exactly one matching generic input and output" in proc.stderr


def test_materializer_rejects_missing_canonical_output_port(tmp_path):
    text = VALID_UI_SALT + 'port(function="heatmap", direction="input", id="signal", type="scalar.unit")\n'
    proc, _ = run_materializer(tmp_path, text)
    assert proc.returncode != 0
    assert "function heatmap port signatures require exactly one canonical output" in proc.stderr

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
    compat_overrides = actual["composition_recipe_contract"]["compat_overrides"]
    compatibility_audit = actual["composition_recipe_contract"]["compatibility_audit"]
    assert len(compat_overrides) == 18
    assert len(compatibility_audit) == 22
    audit_by_key = {
        (row["source"], row["palette"], row["grading"]): row
        for row in compatibility_audit
    }
    assert audit_by_key[("smooth_escape_ramp", "heatmap", "contrast_lift")]["classification"] == "typed_resolved"
    assert audit_by_key[("smooth_escape_ramp", "heatmap", "contrast_lift")]["route_case_id"] == "smooth_escape_heatmap"
    assert audit_by_key[("phase_orbit", "phase_wheel_palette", "phase_finish")]["classification"] == "typed_resolved"
    assert audit_by_key[("root_index", "root_classic_palette", "basin_default")]["classification"] == "typed_resolved"
    assert audit_by_key[("sdf_normal_angle", "phase_wheel_palette", "phase_finish")]["classification"] == "typed_resolved"
    assert audit_by_key[("sdf_signed_distance", "heatmap", "contrast_lift")]["classification"] == "runtime_legacy_override"
    assert audit_by_key[("sdf_signed_distance", "heatmap", "contrast_lift")]["override_id"] == "legacy_sdf_signed_distance_heatmap_contrast_lift"
    assert audit_by_key[("smooth_escape_ramp", "explaino_cmap", "contrast_lift")]["classification"] == "runtime_legacy_override"
    assert all(row["reason"] for row in compatibility_audit)
    assert all(row["owner_seam"] and row["proof"] for row in compat_overrides)
    assert actual["edge_resolution_contract"]["policy"]["id"] == "current_linear_color_stack"
    assert [edge["id"] for edge in actual["edge_resolution_contract"]["edges"]] == [
        "source_to_shape",
        "shape_to_palette",
        "palette_to_grading",
    ]
    audit_cases = {case["id"]: case for case in actual["color_pipeline_resolution_audit"]["cases"]}
    assert set(audit_cases) == {
        "smooth_escape_heatmap",
        "phase_orbit_wheel",
        "root_classic",
        "sdf_normal_angle_phase_wheel",
        "sdf_signed_distance_normalized_heatmap",
        "root_repeat_heatmap_bad",
        "phase_root_palette_bad",
        "sdf_signed_distance_phase_palette_bad",
    }
    assert audit_cases["smooth_escape_heatmap"]["status"] == "resolved"
    assert audit_cases["smooth_escape_heatmap"]["chosen_adapters"] == []
    assert audit_cases["sdf_signed_distance_normalized_heatmap"]["chosen_adapters"] == [
        "normalize.sdf_signed_distance.unit"
    ]
    assert audit_cases["sdf_signed_distance_normalized_heatmap"]["explicit_adapter_consent"] is True
    assert audit_cases["sdf_signed_distance_normalized_heatmap"]["adapter_hops"] == 1
    assert audit_cases["sdf_signed_distance_normalized_heatmap"]["adapter_cost"] == 2
    assert audit_cases["sdf_signed_distance_normalized_heatmap"]["policy_blockers"] == []
    assert audit_cases["root_repeat_heatmap_bad"]["status"] == "fail_closed"
    assert "root category" in audit_cases["root_repeat_heatmap_bad"]["fail_closed_reason"]
    assert audit_cases["root_repeat_heatmap_bad"]["policy_blockers"]
    assert audit_cases["phase_root_palette_bad"]["status"] == "fail_closed"
    assert audit_cases["sdf_signed_distance_phase_palette_bad"]["status"] == "fail_closed"

    assert _ports(actual, "source", "smooth_escape_ramp") == [
        {"direction": "output", "id": "signal", "type": "scalar.unit", "canonical": True}
    ]
    assert _ports(actual, "source", "sdf_signed_distance") == [
        {"direction": "output", "id": "signal", "type": "scalar.sdf_signed_distance", "canonical": True}
    ]
    assert _ports(actual, "source", "sdf_normal_angle") == [
        {"direction": "output", "id": "signal", "type": "phase.radians", "canonical": True}
    ]
    assert _ports(actual, "source", "sdf_inside_outside") == [
        {"direction": "output", "id": "signal", "type": "category.inside_outside", "canonical": True}
    ]
    assert _ports(actual, "shape", "identity") == [
        {"direction": "input", "id": "signal", "type": "generic.T", "generic_group": "T"},
        {"direction": "output", "id": "signal", "type": "generic.T", "generic_group": "T", "canonical": True},
    ]
    assert _ports(actual, "shape", "repeat") == [
        {"direction": "input", "id": "signal", "type": "scalar.unit"},
        {"direction": "output", "id": "signal", "type": "scalar.unit", "canonical": True},
    ]
    assert _ports(actual, "shape", "bias_gain_curve") == [
        {"direction": "input", "id": "signal", "type": "scalar.unit"},
        {"direction": "output", "id": "signal", "type": "scalar.unit", "canonical": True},
    ]
    assert _ports(actual, "palette", "heatmap") == [
        {"direction": "input", "id": "signal", "type": "scalar.unit"},
        {"direction": "output", "id": "color", "type": "color.linear_rgb", "canonical": True},
    ]
    assert _ports(actual, "palette", "phase_wheel_palette") == [
        {"direction": "input", "id": "signal", "type": "phase.radians"},
        {"direction": "output", "id": "color", "type": "color.linear_rgb", "canonical": True},
    ]
    assert _ports(actual, "palette", "root_classic_palette") == [
        {"direction": "input", "id": "signal", "type": "category.root_index"},
        {"direction": "output", "id": "color", "type": "color.linear_rgb", "canonical": True},
    ]
    assert _ports(actual, "grading", "contrast_lift") == [
        {"direction": "input", "id": "color", "type": "color.linear_rgb"},
        {"direction": "output", "id": "color", "type": "color.linear_rgb", "canonical": True},
    ]
