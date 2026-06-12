from __future__ import annotations

import json
from pathlib import Path

from tools.reality_toolkit.fractal_explorer import explaino_capability_atlas as atlas_mod


REPO_ROOT = Path(__file__).resolve().parents[1]
ATLAS_JSON = REPO_ROOT / "docs" / "manifests" / "explaino_capability_atlas.json"
ATLAS_MD = REPO_ROOT / "docs" / "manifests" / "explaino_capability_atlas.md"


def _entry_by_id(payload: dict) -> dict[str, dict]:
    return {entry["fractal_id"]: entry for entry in payload["fractal_capabilities"]}


def test_generated_explaino_capability_atlas_is_current() -> None:
    generated = atlas_mod.build_atlas(REPO_ROOT)
    written = json.loads(ATLAS_JSON.read_text(encoding="utf-8"))

    assert written == generated
    assert generated["schema_id"] == "viewer.explaino_capability_atlas.v1"
    assert generated["generation_mode"] == "repo_derived_static_analysis"
    assert generated["fractal_capabilities"]


def test_atlas_covers_every_current_explaino_selector() -> None:
    generated = atlas_mod.build_atlas(REPO_ROOT)
    entries = _entry_by_id(generated)
    selector_ids = atlas_mod.parse_explaino_selector_registry(REPO_ROOT)
    enum_ids = atlas_mod.parse_explaino_enum_ids(REPO_ROOT)

    assert set(selector_ids) == set(enum_ids)
    assert set(entries) == set(selector_ids)
    assert len(entries) >= 20

    for fractal_id, entry in entries.items():
        assert entry["fractal_id"] == fractal_id
        assert entry["selector_role"]
        assert entry["generated_root_support"] is True
        assert entry["captured_root_behavior"]["analysis_authority"] == "params.explaino_roots_when_present"
        assert entry["source_evidence"]
        assert entry["analyzer_limitations"]
        assert entry["lens_mask_semantics"]["semantic_id"] != "unknown"


def test_atlas_records_conservative_root_authority_and_secondary_notes() -> None:
    entries = _entry_by_id(atlas_mod.build_atlas(REPO_ROOT))

    assert entries["explaino"]["custom_root_editor_support"] is True
    assert entries["explaino_julia"]["custom_root_editor_support"] is False
    assert entries["explaino_lambda"]["custom_root_editor_support"] is False
    assert entries["explaino_rational_escape"]["custom_root_editor_support"] is False
    assert entries["explaino_collatz_direct"]["custom_root_editor_support"] is False
    assert entries["explaino_counterfactual_pair"]["generated_root_count"] == "3_or_4_by_root_family"
    assert entries["explaino_projection_and_flow"]["generated_root_count"] == "3_or_4_by_root_family"
    assert entries["explaino_mult"]["conjugate_semantics"] == "cluster_split_not_strict_conjugate_pairs"
    assert any("secondary splice polynomial" in note for note in entries["explaino_splice"]["secondary_root_family_notes"])
    assert any("captured roots" in note for note in entries["explaino_julia"]["analyzer_limitations"])


def test_generated_explaino_capability_markdown_is_current() -> None:
    generated = atlas_mod.build_atlas(REPO_ROOT)
    assert ATLAS_MD.read_text(encoding="utf-8") == atlas_mod.render_markdown(generated)
