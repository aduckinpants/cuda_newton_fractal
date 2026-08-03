from __future__ import annotations

import json
from pathlib import Path

from tools.precision_authority_inventory import build_inventory


REPO_ROOT = Path(__file__).resolve().parents[1]
REVIEW_PATH = REPO_ROOT / "docs" / "notes" / "precision_authority_promotion_review_p4.json"


def test_phase4_promotion_review_is_joined_to_current_inventory() -> None:
    review = json.loads(REVIEW_PATH.read_text(encoding="utf-8"))
    inventory = build_inventory(REPO_ROOT)
    joined = review["inventory_join"]

    assert review["schema_version"] == "viewer_host.precision_authority_promotion_review.v1"
    assert review["overall_disposition"] == "NO_PRECISION_PROMOTION_AUTHORIZED"
    assert review["product_mutation_performed"] is False
    assert joined["general_schema_classification_counts"] == inventory["general_schema"]["classification_counts"]
    assert joined["general_schema_authoring_identity_loss_count"] == 0
    assert joined["state_io_float_cast_site_count"] == inventory["state_io"]["float_cast_site_count"]
    assert joined["state_io_unresolved_float_cast_site_count"] == inventory["state_io"]["unresolved_float_cast_site_count"]
    assert joined["color_pipeline_float_parameter_count"] == inventory["color_pipeline"]["parameter_type_counts"]["float"]
    assert joined["color_pipeline_double_parameter_count"] == inventory["color_pipeline"]["compiled_double_parameter_count"]
    assert joined["runtime_dispatch_owners_needing_execution_witness"] == inventory["runtime_tiers"]["dispatch_owners_needing_execution_witness"]


def test_phase4_promotion_review_authorizes_no_speculative_widening() -> None:
    review = json.loads(REVIEW_PATH.read_text(encoding="utf-8"))
    candidates = review["candidate_classes"]
    candidate_ids = [item["candidate_id"] for item in candidates]

    assert len(candidates) == 6
    assert len(candidate_ids) == len(set(candidate_ids))
    assert all(item["promotion_authorized"] is False for item in candidates)
    assert all(item["failed_gates"] for item in candidates)
    assert {item["disposition"] for item in candidates} <= {
        "REJECTED_CURRENT_EVIDENCE",
        "REJECTED_NOT_AUTHORITY",
        "DEFERRED_NEEDS_REPRODUCIBLE_DEFECT",
    }
    assert not any(item["disposition"].startswith("APPROVED") for item in candidates)
