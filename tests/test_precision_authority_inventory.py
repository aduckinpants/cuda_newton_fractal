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


def test_general_schema_inventory_exposes_repaired_double_display_identity() -> None:
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
    assert {item["input_format"] for item in doubles} == {"%.17g"}
    combined = next(item for item in doubles if item["control_id"] == "explaino_seed")
    assert combined["classification"] == "INTENTIONAL_MIXED_PRECISION"
    assert combined["classification_basis"] == (
        "roundtrip_double_editor_projects_fraction_to_float_backed_seed_drift"
    )
    assert combined["authorability_status"] == "schema_ui_numeric_authoring_route"
    assert combined["classification_confidence"] == "source_proven_authoring_route"
    assert combined["state_load_conversion"] == "not_joined_requires_phase3_owner_trace"
    assert combined["runtime_consumption"] == "not_proven_by_ui_binding"
    direct = next(item for item in doubles if item["control_id"] == "explaino_seed_b")
    assert direct["classification"] == "TRUTHFUL_FLOAT64"
    assert direct["classification_basis"] == "double_binding_and_roundtrip_capable_input"


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


def test_state_io_inventory_joins_every_float_conversion_to_declared_storage() -> None:
    inventory = build_inventory(REPO_ROOT)
    state_io = inventory["state_io"]

    assert state_io["serializer_precision"] == "std::numeric_limits<double>::max_digits10"
    assert state_io["float_cast_site_count"] == 149
    assert state_io["float_conversion_owner_function_count"] == 8
    assert state_io["float_conversion_owner_counts"] == {
        "ApplyColorShapeStackEntryNumbers": 10,
        "LoadDiagnosticsStateJson": 106,
        "ParseColorGradingStackEntry": 7,
        "ParseColorPaletteStackEntry": 8,
        "ParseColorSourceStackEntry": 15,
        "ParseFixedFloatArray": 1,
        "ParseOptionalExplainoRoots": 1,
        "ParseOptionalLensFloat": 1,
    }
    assert state_io["unresolved_float_cast_site_count"] == 0
    assert {site["destination_scalar_type"] for site in state_io["float_cast_sites"]} == {"float"}
    assert {site["classification"] for site in state_io["float_cast_sites"]} == {"TRUTHFUL_FLOAT32"}
    assert state_io["classification"] == "INTENTIONAL_MIXED_PRECISION"


def test_state_io_inventory_distinguishes_exact_and_normalized_double_owners() -> None:
    owners = build_inventory(REPO_ROOT)["state_io"]["double_state_owners"]
    assert [(item["owner_struct"], item["owner_member"]) for item in owners] == [
        ("ViewState", "center_hp_x"),
        ("ViewState", "center_hp_y"),
        ("ViewState", "log2_zoom"),
        ("KernelParams", "explaino_secondary_root_pattern_seed"),
        ("KernelParams", "explaino_seed"),
        ("KernelParams", "explaino_seed_b"),
    ]
    assert {item["storage_type"] for item in owners} == {"double"}
    exact = [item for item in owners if item["classification"] == "TRUTHFUL_FLOAT64"]
    normalized = [item for item in owners if item["classification"] == "INTENTIONAL_MIXED_PRECISION"]
    assert len(exact) == 5
    assert [(item["owner_struct"], item["owner_member"]) for item in normalized] == [
        ("KernelParams", "explaino_seed")
    ]
    assert normalized[0]["normalization_owner"] == "ExplainoSeedNormalize"


def test_runtime_tier_inventory_separates_universal_claim_from_static_branch_evidence() -> None:
    inventory = build_inventory(REPO_ROOT)
    runtime = inventory["runtime_tiers"]

    assert runtime["standard_support_policy"] == "advertised_for_all_selectors"
    assert runtime["standard_resolves_to"] == "float64_direct"
    assert runtime["selector_count"] == 51
    assert runtime["branch_records"]
    assert runtime["dispatch_owner_count"] < runtime["selector_count"]
    assert runtime["dispatch_owners_needing_execution_witness"] == []
    assert runtime["selectors_without_static_execution_owner"] == []
    assert runtime["static_evidence_disclaimer"]
    assert {record["classification_confidence"] for record in runtime["branch_records"]} == {
        "static_execution_marker_only"
    }

    repaired_owners = {
        record["owner_id"]: record
        for record in runtime["dispatch_owner_records"]
        if record["owner_id"] in {
            "branch:explaino_y",
            "branch:explaino_julia",
            "branch:explaino_lambda",
            "branch:multicorn",
            "predicate:UsesSpecializedEscapeTimeFormula",
        }
    }
    assert set(repaired_owners) == {
        "branch:explaino_y",
        "branch:explaino_julia",
        "branch:explaino_lambda",
        "branch:multicorn",
        "predicate:UsesSpecializedEscapeTimeFormula",
    }
    assert repaired_owners["predicate:UsesSpecializedEscapeTimeFormula"]["selectors"] == ["collatz", "mcmullen"]
    assert all(record["float64_iteration_execution_marker"] for record in repaired_owners.values())


def test_markdown_and_cli_outputs_are_stable(tmp_path: Path) -> None:
    inventory = build_inventory(REPO_ROOT)
    markdown = render_markdown(inventory)
    assert "# Precision Authority Inventory" in markdown
    assert "AUTHORING_IDENTITY_LOSS" in markdown
    assert "INTENTIONAL_MIXED_PRECISION" in markdown

    out_json = tmp_path / "matrix.json"
    out_md = tmp_path / "matrix.md"
    write_outputs(inventory, out_json=out_json, out_md=out_md)
    assert json.loads(out_json.read_text(encoding="utf-8")) == inventory
    assert out_md.read_text(encoding="utf-8") == markdown
    assert out_json.read_bytes().endswith(b"\n")
    assert out_md.read_bytes().endswith(b"\n")
