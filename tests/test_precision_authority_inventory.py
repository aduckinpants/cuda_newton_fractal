from __future__ import annotations

import json
from pathlib import Path

from tools.precision_authority_inventory import (
    SCHEMA_VERSION,
    build_inventory,
    render_markdown,
    write_outputs,
)


REPO_ROOT = Path(__file__).resolve().parents[1]


def test_inventory_is_deterministic_and_source_grounded() -> None:
    first = build_inventory(REPO_ROOT)
    second = build_inventory(REPO_ROOT)

    assert first == second
    assert first["schema_version"] == SCHEMA_VERSION
    assert first["authority"]["ui_schema"]["path"] == "ui/fractal_binding_surface_v1.ui_schema.json"
    assert first["authority"]["color_pipeline_contract"]["path"] == (
        "docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json"
    )
    assert first["authority"]["source_commit"]
    assert "generated_at" not in first
    assert "branch" not in first["authority"]


def test_general_schema_inventory_exposes_double_display_identity_risk() -> None:
    inventory = build_inventory(REPO_ROOT)
    controls = inventory["general_schema"]["numeric_controls"]
    doubles = [item for item in controls if item["control_type"] in {"slider_double", "drag_double"}]

    assert [item["control_id"] for item in doubles] == [
        "explaino_seed",
        "explaino_seed_b",
        "dynamics_root_field_seed",
        "color_root_field_seed",
    ]
    assert {item["binding_storage"] for item in doubles} == {"double"}
    assert {item["input_format"] for item in doubles} == {"%.6f"}
    combined = next(item for item in doubles if item["control_id"] == "explaino_seed")
    assert combined["classification"] == "AUTHORING_IDENTITY_LOSS"
    assert combined["classification_basis"] == "combined_double_seed_uses_fixed_six_decimal_edit_format"
    assert combined["authorability_status"] == "schema_ui_numeric_authoring_route"
    assert combined["classification_confidence"] == "source_proven_authoring_route"
    assert combined["state_load_conversion"] == "not_joined_requires_phase3_owner_trace"
    assert combined["runtime_consumption"] == "not_proven_by_ui_binding"


def test_general_schema_inventory_exposes_specialized_camera_authority_routes() -> None:
    inventory = build_inventory(REPO_ROOT)
    controls = {
        item["control_id"]: item
        for item in inventory["general_schema"]["numeric_controls"]
    }

    for control_id in ("center_x", "center_y"):
        control = controls[control_id]
        assert control["binding_storage"] == "float"
        assert control["authoritative_storage"] == "double"
        assert control["editor_carrier"] == "float"
        assert control["edit_route"] == "camera_hp_double_via_float_editor"
        assert control["classification"] == "AUTHORING_IDENTITY_LOSS"
        assert control["classification_basis"] == (
            "camera_hp_double_authority_roundtrips_through_float_editor_and_fixed_five_decimal_input"
        )

    zoom = controls["zoom"]
    assert zoom["binding_storage"] == "float"
    assert zoom["authoritative_storage"] == "double_log2"
    assert zoom["editor_carrier"] == "double"
    assert zoom["edit_route"] == "camera_log2_double_via_double_editor"
    assert zoom["classification"] == "AUTHORING_IDENTITY_LOSS"
    assert zoom["classification_basis"] == (
        "camera_log2_double_authority_uses_nine_significant_digit_linear_zoom_input"
    )


def test_general_schema_inventory_exposes_composite_numeric_routes() -> None:
    inventory = build_inventory(REPO_ROOT)
    controls = {
        item["control_id"]: item
        for item in inventory["general_schema"]["numeric_controls"]
    }

    resolution = controls["resolution_long_edge"]
    assert resolution["edit_route"] == "resolution_long_edge_to_int2"
    assert resolution["authoritative_storage"] == "int2"
    assert resolution["editor_carrier"] == "int"
    assert resolution["classification"] == "INTENTIONAL_MIXED_PRECISION"

    for control_id in ("explaino_seed", "dynamics_root_field_seed"):
        seed = controls[control_id]
        assert seed["edit_route"] == "combined_explaino_seed_double"
        assert seed["authoritative_storage"] == "double_plus_derived_float_fields"
        assert seed["editor_carrier"] == "double"


def test_color_pipeline_inventory_uses_compiled_contract_and_double_carrier() -> None:
    inventory = build_inventory(REPO_ROOT)
    pipeline = inventory["color_pipeline"]

    assert pipeline["contract_parameter_count"] > 0
    assert pipeline["parameter_type_counts"]["float"] > 0
    assert pipeline["runtime_number_carrier"] == "double"
    assert pipeline["classification"] == "NEEDS_RUNTIME_WITNESS"
    assert pipeline["authority_kind"] == "compiled_ui_salt_contract"
    assert {item["classification_confidence"] for item in pipeline["parameters"]} == {
        "contract_and_carrier_proven_runtime_consumer_unresolved"
    }


def test_state_io_inventory_records_roundtrip_and_narrowing_sites_without_overclaiming() -> None:
    inventory = build_inventory(REPO_ROOT)
    state_io = inventory["state_io"]

    assert state_io["serializer_precision"] == "std::numeric_limits<double>::max_digits10"
    assert state_io["float_cast_site_count"] > 0
    assert all("line" in site and "snippet" in site for site in state_io["float_cast_sites"])
    assert state_io["classification"] == "NEEDS_RUNTIME_WITNESS"


def test_runtime_tier_inventory_separates_universal_claim_from_static_branch_evidence() -> None:
    inventory = build_inventory(REPO_ROOT)
    runtime = inventory["runtime_tiers"]

    assert runtime["standard_support_policy"] == "advertised_for_all_selectors"
    assert runtime["standard_resolves_to"] == "float64_direct"
    assert runtime["selector_count"] == 51
    assert runtime["branch_records"]
    assert runtime["selectors_needing_runtime_witness"]
    assert "explaino_rational_escape" not in runtime["selectors_needing_runtime_witness"]
    assert runtime["static_evidence_disclaimer"]
    assert {record["classification_confidence"] for record in runtime["branch_records"]} == {
        "static_token_evidence_only"
    }


def test_markdown_and_cli_outputs_are_stable(tmp_path: Path) -> None:
    inventory = build_inventory(REPO_ROOT)
    markdown = render_markdown(inventory)
    assert "# Precision Authority Inventory" in markdown
    assert "AUTHORING_IDENTITY_LOSS" in markdown
    assert "NEEDS_RUNTIME_WITNESS" in markdown

    out_json = tmp_path / "matrix.json"
    out_md = tmp_path / "matrix.md"
    write_outputs(inventory, out_json=out_json, out_md=out_md)
    assert json.loads(out_json.read_text(encoding="utf-8")) == inventory
    assert out_md.read_text(encoding="utf-8") == markdown
    assert out_json.read_bytes().endswith(b"\n")
    assert out_md.read_bytes().endswith(b"\n")
