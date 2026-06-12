from __future__ import annotations

import json
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
CONTRACT_PATH = REPO_ROOT / "docs" / "contracts" / "explaino_slime_trace_receipt_v1.contract.json"


def _load_contract() -> dict:
    return json.loads(CONTRACT_PATH.read_text(encoding="utf-8"))


def test_trace_receipt_contract_records_current_legacy_history_inventory() -> None:
    contract = _load_contract()
    inventory = contract["current_sidecar_mutation_history_inventory"]

    assert inventory["storage_path"] == "state_json.sidecar_mutation_history"
    assert inventory["record_type"] == "SidecarAutoDemoMutationRecord"
    assert inventory["record_fields"] == [
        "label",
        "path",
        "type",
        "target_value",
        "utility",
    ]
    assert any("WriteSidecarMutationHistoryJson" in seam for seam in inventory["write_seams"])
    assert any("ReplayLoadedSidecarMutationHistory" in seam for seam in inventory["read_replay_seams"])


def test_trace_receipt_v1_requires_golden_thread_root_and_policy_fields() -> None:
    contract = _load_contract()
    fields = {field["id"]: field for field in contract["v1_step_record_required_fields"]}

    required = {
        "step_index",
        "path",
        "type",
        "previous_value",
        "target_value",
        "applied_value",
        "utility",
        "selection_reason",
        "pre_state_hash",
        "post_state_hash",
        "measurement_hash",
        "root_authority",
        "roots_at_step",
        "scene_id",
        "rng_seed",
        "policy_id",
    }
    assert required <= set(fields)
    for field_id in required:
        assert fields[field_id]["required"] is True
        assert fields[field_id]["reason"]


def test_trace_receipt_contract_keeps_legacy_history_from_becoming_trace_authority() -> None:
    contract = _load_contract()
    rules = "\n".join(contract["compatibility_rules"])

    assert contract["authority_boundary"]["state_json_remains_replay_authority"] is True
    assert contract["authority_boundary"]["sidecar_mutation_history_remains_legacy_ordered_replay_input"] is True
    assert "not promoted to rigorous trace authority" in rules
    assert "fail closed" in rules


def test_trace_receipt_contract_defers_runner_and_policy_growth() -> None:
    contract = _load_contract()
    deferred = set(contract["deferred_until_after_receipt_contract"])

    assert contract["status"] == "draft_contract_only"
    assert "headless parameter-space slime trace runner" in deferred
    assert "genetic algorithm over slime policy" in deferred
    assert "FITS/flashlight corpus or visualization reuse" in deferred
