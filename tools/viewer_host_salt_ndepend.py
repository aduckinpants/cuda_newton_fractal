from __future__ import annotations

import argparse
import contextlib
import io
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_POLICY = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_POLICY.v1.json"
DEFAULT_BASELINES = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_BASELINES.v1.json"
DEFAULT_CONTRACTS = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_CONTRACTS.v1.json"
DEFAULT_FREEZE_GATE = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_FREEZE_GATE.v1.json"
GREEN_PARITY_CASE_STATUSES = {"expected_result_matched", "contract_case_matched"}
GREEN_FAMILY_PARITY_STATUSES = {"critical_family_seed", "critical_family_parity_match"}


def _resolve_path(path_text: str) -> Path:
    path = Path(path_text)
    if not path.is_absolute():
        path = REPO_ROOT / path
    return path.resolve()


def _repo_rel(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return str(path.resolve())


def _normalize_path_text(path_text: str) -> str:
    return path_text.replace("\\", "/").strip()


def _string_or_empty(value: object) -> str:
    return "" if value is None else str(value).strip()


def _load_json(path_text: str) -> tuple[Path, dict[str, object]]:
    path = _resolve_path(path_text)
    payload = json.loads(path.read_text(encoding="utf-8"))
    return path, payload


def _load_optional_json(path: Path) -> dict[str, object] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def _write_json(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _write_markdown(path: Path, lines: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _load_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def _packet_meta(command_id: str, inputs: dict[str, object]) -> dict[str, object]:
    return {
        "tool": "viewer_host_salt_ndepend",
        "command": command_id,
        "schema_version": "viewer_host_salt_ndepend.packet_meta.v1",
        "repo_root": REPO_ROOT.as_posix(),
        "inputs": inputs,
    }


def _planned_command(policy: dict[str, object], command_id: str) -> dict[str, object]:
    for item in policy.get("planned_command_surfaces", []):
        if item.get("command_id") == command_id:
            return item
    raise KeyError(f"missing planned command surface: {command_id}")


def _summarize_json_artifact(payload: dict[str, object]) -> dict[str, object]:
    summary: dict[str, object] = {}
    if "score" in payload:
        summary["score"] = payload["score"]
    if "checks" in payload and isinstance(payload["checks"], dict):
        summary["checks"] = payload["checks"]
    if "severity_counts" in payload and isinstance(payload["severity_counts"], dict):
        summary["severity_counts"] = payload["severity_counts"]
    if "case_count" in payload:
        summary["case_count"] = payload["case_count"]
    if "contract_count" in payload:
        summary["contract_count"] = payload["contract_count"]
    if "ok" in payload:
        summary["ok"] = payload["ok"]
    if not summary:
        summary["keys"] = sorted(payload.keys())[:8]
    return summary


def _text_artifact_interruption_reason(lines: list[str]) -> str:
    tail = next((line.strip() for line in reversed(lines) if line.strip()), "")
    if tail == "^C":
        return "tail_ctrl_c"
    lowered = "\n".join(lines).lower()
    for marker in ("keyboardinterrupt", "user cancelled", "operation canceled", "operation cancelled"):
        if marker in lowered:
            return marker.replace(" ", "_")
    return ""


def _summarize_text_artifact(path: Path) -> dict[str, object]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    tail = next((line.strip() for line in reversed(lines) if line.strip()), "")
    interruption_reason = _text_artifact_interruption_reason(lines)
    summary: dict[str, object] = {
        "line_count": len(lines),
        "tail": tail,
    }
    if interruption_reason:
        summary["interrupted"] = True
        summary["interruption_reason"] = interruption_reason
    return summary


def _bind_producer_surface(producer: dict[str, object]) -> dict[str, object]:
    artifact_text = producer.get("artifact_path")
    artifact_path = _resolve_path(str(artifact_text)) if artifact_text else None
    bound = dict(producer)
    bound["artifact_exists"] = bool(artifact_path and artifact_path.exists())
    if artifact_path is not None:
        bound["artifact_path"] = _repo_rel(artifact_path)
    if artifact_path is None or not artifact_path.exists():
        bound["status"] = "missing"
        bound["summary"] = {"reason": "artifact_missing"}
        return bound

    if artifact_path.suffix.lower() == ".json":
        payload = json.loads(artifact_path.read_text(encoding="utf-8"))
        summary = _summarize_json_artifact(payload if isinstance(payload, dict) else {"value_type": type(payload).__name__})
    else:
        summary = _summarize_text_artifact(artifact_path)

    if bool(summary.get("interrupted", False)):
        bound["status"] = "interrupted"
        bound["summary"] = summary
        return bound

    bound["status"] = "bound"
    bound["summary"] = summary
    return bound


def _producer_maps(producer_payload: dict[str, object]) -> tuple[dict[str, dict[str, object]], dict[str, dict[str, object]]]:
    by_id: dict[str, dict[str, object]] = {}
    by_artifact: dict[str, dict[str, object]] = {}
    for producer in producer_payload.get("producers", []):
        if not isinstance(producer, dict):
            continue
        producer_id = str(producer.get("producer_id", "")).strip()
        artifact_path = str(producer.get("artifact_path", "")).strip()
        if producer_id:
            by_id[producer_id] = producer
        if artifact_path:
            by_artifact[_normalize_path_text(artifact_path)] = producer
    return by_id, by_artifact


def _evaluate_expected_result(expected_result: str, artifact_path: Path) -> tuple[bool, str]:
    if expected_result == "baseline_check_passes":
        payload = _load_optional_json(artifact_path)
        severity_counts = {} if not isinstance(payload, dict) else payload.get("severity_counts", {})
        critical = 0 if not isinstance(severity_counts, dict) else int(severity_counts.get("CRITICAL", 0))
        errors = 0 if not isinstance(severity_counts, dict) else int(severity_counts.get("ERROR", 0))
        ok = critical == 0 and errors == 0
        return ok, f"critical={critical} errors={errors}"
    if expected_result == "no_direct_coverage_regression":
        payload = _load_optional_json(artifact_path)
        coverage = [] if not isinstance(payload, dict) else payload.get("coverage", [])
        covered_count = sum(1 for item in coverage if isinstance(item, dict) and str(item.get("status", "")).strip() == "COVERED")
        ok = covered_count > 0
        return ok, f"covered_count={covered_count}"
    if expected_result == "all_helper_tests_pass":
        text = _load_text(artifact_path)
        ok = "All helper tests passed." in text
        return ok, "tail_contains=All helper tests passed."
    if expected_result == "published_runtime_available":
        text = _load_text(artifact_path)
        ok = "Active runtime:" in text
        return ok, "tail_contains=Active runtime:"
    if expected_result == "green_published_runtime_lane":
        text = _load_text(artifact_path)
        summary_lines = [line.strip() for line in text.splitlines() if "viewer_host_runtime_pytest_lane: summary=" in line]
        summary_line = summary_lines[-1] if summary_lines else ""
        ok = bool(summary_line) and "failed=0" in summary_line and "errors=0" in summary_line
        return ok, summary_line or "summary_line_missing"
    return False, f"unsupported_expected_result:{expected_result}"


def _evaluate_seeded_parity_case(case: dict[str, object], producer_by_artifact: dict[str, dict[str, object]]) -> dict[str, object]:
    source_artifact = _string_or_empty(case.get("source_artifact"))
    expected_result = _string_or_empty(case.get("expected_result"))
    packet = {
        "case_id": case.get("case_id"),
        "family": case.get("family"),
        "contract_id": case.get("required_contract"),
        "expected_result": expected_result,
        "matched_producers": [],
    }
    if not source_artifact or not expected_result:
        packet["status"] = "case_definition_incomplete"
        packet["detail"] = "source_artifact or expected_result missing"
        return packet
    producer = producer_by_artifact.get(_normalize_path_text(source_artifact))
    if producer is None or str(producer.get("status", "")).strip() != "bound":
        packet["status"] = "missing_candidate_producer"
        packet["detail"] = f"candidate producer missing for {source_artifact}"
        return packet
    producer_id = str(producer.get("producer_id", "")).strip()
    artifact_path = _resolve_path(str(producer.get("artifact_path", "")))
    ok, detail = _evaluate_expected_result(expected_result, artifact_path)
    packet["matched_producers"] = [producer_id] if producer_id else []
    packet["detail"] = detail
    packet["status"] = "expected_result_matched" if ok else "expected_result_mismatch"
    return packet


def _evaluate_contract_parity_case(
    case: dict[str, object],
    contract_lookup: dict[str, dict[str, object]],
    producer_by_id: dict[str, dict[str, object]],
    seeded_case_by_artifact: dict[str, dict[str, object]],
) -> dict[str, object]:
    contract_id = _string_or_empty(case.get("required_contract"))
    packet = {
        "case_id": case.get("case_id"),
        "family": case.get("family"),
        "contract_id": contract_id,
        "matched_producers": [],
        "supporting_case_ids": [],
        "missing_required_producers": [],
        "missing_producer_baselines": [],
        "failed_required_producers": [],
    }
    contract = contract_lookup.get(contract_id)
    if not isinstance(contract, dict):
        packet["status"] = "missing_contract_entry"
        packet["detail"] = f"contract registry missing {contract_id}"
        return packet

    required_producers = [str(item).strip() for item in contract.get("required_producers", []) if str(item).strip()]
    packet["required_producers"] = required_producers
    for producer_id in required_producers:
        producer = producer_by_id.get(producer_id)
        if producer is None or str(producer.get("status", "")).strip() != "bound":
            packet["missing_required_producers"].append(producer_id)
            continue
        packet["matched_producers"].append(producer_id)
        artifact_key = _normalize_path_text(str(producer.get("artifact_path", "")))
        supporting_case = seeded_case_by_artifact.get(artifact_key)
        if not isinstance(supporting_case, dict):
            packet["missing_producer_baselines"].append(producer_id)
            continue
        packet["supporting_case_ids"].append(_string_or_empty(supporting_case.get("case_id")))
        expected_result = _string_or_empty(supporting_case.get("expected_result"))
        artifact_path = _resolve_path(str(producer.get("artifact_path", "")))
        ok, detail = _evaluate_expected_result(expected_result, artifact_path)
        if not ok:
            packet["failed_required_producers"].append(
                {
                    "producer_id": producer_id,
                    "supporting_case_id": supporting_case.get("case_id"),
                    "detail": detail,
                }
            )

    if packet["missing_required_producers"]:
        packet["status"] = "missing_required_producers"
        packet["detail"] = "required producer artifacts are not all bound"
    elif packet["missing_producer_baselines"]:
        packet["status"] = "missing_required_producer_baselines"
        packet["detail"] = "no seeded baseline case maps to every required producer"
    elif packet["failed_required_producers"]:
        packet["status"] = "required_producer_parity_mismatch"
        packet["detail"] = "one or more required producers failed their supporting parity checks"
    else:
        packet["status"] = "contract_case_matched"
        packet["detail"] = "all required producers satisfied their supporting parity checks"
    return packet


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Seed local viewer-host salt_ndepend packet surfaces.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    audit = subparsers.add_parser("audit", help="Emit the seeded audit packet surface.")
    audit.add_argument("--policy", default=str(DEFAULT_POLICY))
    audit.add_argument("--out-dir", required=True)

    structural = subparsers.add_parser("structural", help="Emit the seeded structural packet surface.")
    structural.add_argument("--policy", default=str(DEFAULT_POLICY))
    structural.add_argument("--out", required=True)
    structural.add_argument("--out-md", required=True)

    review = subparsers.add_parser("review", help="Emit the seeded review packet surface from a suite index.")
    review.add_argument("--suite-index", required=True)
    review.add_argument("--out", required=True)
    review.add_argument("--out-md", required=True)

    baselines = subparsers.add_parser("baselines", help="Emit the seeded baseline index surface.")
    baselines.add_argument("--manifest", default=str(DEFAULT_BASELINES))
    baselines.add_argument("--out-dir", required=True)
    baselines.add_argument("--out", required=True)
    baselines.add_argument("--out-md", required=True)

    contracts = subparsers.add_parser("contracts", help="Emit the seeded contract-registry packet surface.")
    contracts.add_argument("--registry", default=str(DEFAULT_CONTRACTS))
    contracts.add_argument("--out", required=True)
    contracts.add_argument("--out-md", required=True)

    parity = subparsers.add_parser("parity", help="Emit the seeded parity packet surface.")
    parity.add_argument("--baseline-suite", required=True)
    parity.add_argument("--candidate-suite", required=True)
    parity.add_argument("--contract-registry", default=str(DEFAULT_CONTRACTS))
    parity.add_argument("--out", required=True)
    parity.add_argument("--out-md", required=True)

    family_parity = subparsers.add_parser("family-parity", help="Emit the seeded family-parity packet surface.")
    family_parity.add_argument("--baseline-index", required=True)
    family_parity.add_argument("--candidate-suite", required=True)
    family_parity.add_argument("--contract-registry", default=str(DEFAULT_CONTRACTS))
    family_parity.add_argument("--parity")
    family_parity.add_argument("--out", required=True)
    family_parity.add_argument("--out-md", required=True)

    freeze_gate = subparsers.add_parser("freeze-gate", help="Materialize the current seeded salt_ndepend packet set in one output directory.")
    freeze_gate.add_argument("--policy", default=str(DEFAULT_POLICY))
    freeze_gate.add_argument("--manifest", default=str(DEFAULT_BASELINES))
    freeze_gate.add_argument("--contract-registry", default=str(DEFAULT_CONTRACTS))
    freeze_gate.add_argument("--freeze-gate", default=str(DEFAULT_FREEZE_GATE))
    freeze_gate.add_argument("--out-dir", required=True)

    doctor = subparsers.add_parser("doctor", help="Emit the seeded doctor packet surface.")
    doctor.add_argument("--packet-dir", required=True)
    doctor.add_argument("--suite-index")
    doctor.add_argument("--producer-index")
    doctor.add_argument("--contract-registry", default=str(DEFAULT_CONTRACTS))
    doctor.add_argument("--freeze-gate", default=str(DEFAULT_FREEZE_GATE))
    doctor.add_argument("--baseline-manifest", default=str(DEFAULT_BASELINES))
    doctor.add_argument("--baseline-index")
    doctor.add_argument("--family-parity")
    doctor.add_argument("--out", required=True)
    doctor.add_argument("--out-md", required=True)

    return parser


def _run_audit(args: argparse.Namespace) -> int:
    policy_path, policy = _load_json(args.policy)
    out_dir = _resolve_path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    suite_index_path = out_dir / "suite_index.json"
    producer_index_path = out_dir / "producer_index.json"
    bound_producers = [
        _bind_producer_surface(producer)
        for producer in policy.get("producer_surfaces", [])
        if isinstance(producer, dict)
    ]
    bound_count = sum(1 for producer in bound_producers if producer.get("status") == "bound")
    missing_count = sum(1 for producer in bound_producers if producer.get("status") != "bound")

    suite_payload = {
        "schema_version": "viewer_host_salt_ndepend.audit_suite_index.v1",
        "status": "producer_bound_seed",
        "command": "audit",
        "policy_path": _repo_rel(policy_path),
        "producer_count": len(bound_producers),
        "bound_producer_count": bound_count,
        "missing_producer_count": missing_count,
        "planned_command_ids": [item.get("command_id") for item in policy.get("planned_command_surfaces", [])],
        "gated_product_threads": policy.get("gated_product_threads", []),
        "suite_index_json": _repo_rel(suite_index_path),
        "producer_index_json": _repo_rel(producer_index_path),
        "packet_meta": _packet_meta("audit", {"policy": _repo_rel(policy_path), "out_dir": _repo_rel(out_dir)}),
    }
    producer_payload = {
        "schema_version": "viewer_host_salt_ndepend.producer_index.v1",
        "status": "producer_bound_seed",
        "command": "audit",
        "policy_path": _repo_rel(policy_path),
        "producer_count": len(bound_producers),
        "bound_producer_count": bound_count,
        "missing_producer_count": missing_count,
        "planned_command_count": len(policy.get("planned_command_surfaces", [])),
        "producers": bound_producers,
        "packet_meta": _packet_meta("audit", {"policy": _repo_rel(policy_path), "out_dir": _repo_rel(out_dir)}),
    }
    _write_json(suite_index_path, suite_payload)
    _write_json(producer_index_path, producer_payload)
    print(json.dumps({"suite_index_json": _repo_rel(suite_index_path), "producer_index_json": _repo_rel(producer_index_path)}, indent=2))
    return 0


def _run_structural(args: argparse.Namespace) -> int:
    policy_path, policy = _load_json(args.policy)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    spec = _planned_command(policy, "structural")
    payload = {
        "schema_version": "viewer_host_salt_ndepend.structural_index.v1",
        "status": "surface_seed_only",
        "command": "structural",
        "policy_path": _repo_rel(policy_path),
        "consumed_producers": spec.get("consumes", []),
        "planned_outputs": spec.get("produces", []),
        "packet_meta": _packet_meta("structural", {"policy": _repo_rel(policy_path), "out": _repo_rel(out_path)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Structural",
            "",
            "Status: surface_seed_only",
            f"Policy: {_repo_rel(policy_path)}",
            "",
            "Consumed producers:",
            *[f"- {item}" for item in spec.get("consumes", [])],
        ],
    )
    print(json.dumps({"structural_json": _repo_rel(out_path), "structural_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_review(args: argparse.Namespace) -> int:
    suite_path, suite_payload = _load_json(args.suite_index)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    payload = {
        "schema_version": "viewer_host_salt_ndepend.review.v1",
        "status": "surface_seed_only",
        "command": "review",
        "suite_index_path": _repo_rel(suite_path),
        "suite_status": suite_payload.get("status", "unknown"),
        "next_blocker": "Real issue summarization lands after the audit packet gains actual producer ingestion.",
        "packet_meta": _packet_meta("review", {"suite_index": _repo_rel(suite_path), "out": _repo_rel(out_path)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Review",
            "",
            "Status: surface_seed_only",
            f"Suite index: {_repo_rel(suite_path)}",
            "",
            "Next blocker: actual audit packet ingestion is still pending.",
        ],
    )
    print(json.dumps({"review_json": _repo_rel(out_path), "review_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_baselines(args: argparse.Namespace) -> int:
    manifest_path, manifest = _load_json(args.manifest)
    out_dir = _resolve_path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    cases = manifest.get("cases", [])
    blocker_cases = [case.get("case_id") for case in cases if case.get("status") != "seeded"]
    payload = {
        "schema_version": "viewer_host_salt_ndepend.baseline_index.v1",
        "status": "surface_seed_only",
        "command": "baselines",
        "manifest_path": _repo_rel(manifest_path),
        "case_count": len(cases),
        "seeded_case_count": sum(1 for case in cases if case.get("status") == "seeded"),
        "planned_blocker_cases": blocker_cases,
        "cases": [
            {
                "case_id": case.get("case_id"),
                "family": case.get("family"),
                "status": case.get("status"),
                "required_contract": case.get("required_contract"),
                "source_artifact": case.get("source_artifact"),
                "source_command": case.get("source_command"),
                "source_task": case.get("source_task"),
                "expected_result": case.get("expected_result"),
                "required_by_freeze_gate": case.get("required_by_freeze_gate"),
            }
            for case in cases
        ],
        "packet_meta": _packet_meta("baselines", {"manifest": _repo_rel(manifest_path), "out_dir": _repo_rel(out_dir)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Baselines",
            "",
            "Status: surface_seed_only",
            f"Manifest: {_repo_rel(manifest_path)}",
            f"Seeded cases: {payload['seeded_case_count']}",
            f"Planned blockers: {', '.join(blocker_cases) if blocker_cases else 'none'}",
        ],
    )
    print(json.dumps({"baseline_index_json": _repo_rel(out_path), "baseline_index_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_contracts(args: argparse.Namespace) -> int:
    registry_path, registry = _load_json(args.registry)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    contracts = registry.get("contracts", [])
    payload = {
        "schema_version": "viewer_host_salt_ndepend.contracts.v1",
        "status": "surface_seed_only",
        "command": "contracts",
        "registry_path": _repo_rel(registry_path),
        "contract_count": len(contracts),
        "contract_ids": [contract.get("contract_id") for contract in contracts],
        "packet_meta": _packet_meta("contracts", {"registry": _repo_rel(registry_path), "out": _repo_rel(out_path)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Contracts",
            "",
            "Status: surface_seed_only",
            f"Registry: {_repo_rel(registry_path)}",
            "",
            "Contracts:",
            *[f"- {contract.get('contract_id')}" for contract in contracts],
        ],
    )
    print(json.dumps({"contracts_json": _repo_rel(out_path), "contracts_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_parity(args: argparse.Namespace) -> int:
    baseline_path, baseline_payload = _load_json(args.baseline_suite)
    candidate_path, candidate_payload = _load_json(args.candidate_suite)
    contract_registry_path, contract_registry = _load_json(args.contract_registry)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    producer_by_id, producer_by_artifact = _producer_maps(candidate_payload)
    contract_lookup = {}
    for contract in contract_registry.get("contracts", []):
        if isinstance(contract, dict):
            contract_lookup[str(contract.get("contract_id"))] = contract

    seeded_case_by_artifact = {}
    for case in baseline_payload.get("cases", []):
        if not isinstance(case, dict):
            continue
        source_artifact = _string_or_empty(case.get("source_artifact"))
        expected_result = _string_or_empty(case.get("expected_result"))
        if source_artifact and expected_result:
            seeded_case_by_artifact[_normalize_path_text(source_artifact)] = case

    case_packets = []
    for case in baseline_payload.get("cases", []):
        if not isinstance(case, dict):
            continue
        source_artifact = _string_or_empty(case.get("source_artifact"))
        expected_result = _string_or_empty(case.get("expected_result"))
        if source_artifact or expected_result:
            case_packets.append(_evaluate_seeded_parity_case(case, producer_by_artifact))
            continue
        case_packets.append(_evaluate_contract_parity_case(case, contract_lookup, producer_by_id, seeded_case_by_artifact))

    matched_case_count = sum(1 for packet in case_packets if str(packet.get("status", "")).strip() in GREEN_PARITY_CASE_STATUSES)
    failing_case_count = len(case_packets) - matched_case_count
    payload = {
        "schema_version": "viewer_host_salt_ndepend.parity.v1",
        "status": "case_result_parity",
        "command": "parity",
        "comparison_ready": bool(case_packets) and failing_case_count == 0,
        "baseline_suite_path": _repo_rel(baseline_path),
        "candidate_suite_path": _repo_rel(candidate_path),
        "contract_registry_path": _repo_rel(contract_registry_path),
        "baseline_status": baseline_payload.get("status", "unknown"),
        "candidate_status": candidate_payload.get("status", "unknown"),
        "matched_case_count": matched_case_count,
        "failing_case_count": failing_case_count,
        "case_packets": case_packets,
        "reason": "Parity now evaluates seeded baseline expectations and contract-backed required-producer cases against the current producer packet, instead of emitting a placeholder surface.",
        "packet_meta": _packet_meta(
            "parity",
            {
                "baseline_suite": _repo_rel(baseline_path),
                "candidate_suite": _repo_rel(candidate_path),
                "contract_registry": _repo_rel(contract_registry_path),
            },
        ),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Parity",
            "",
            "Status: case_result_parity",
            f"Baseline suite: {_repo_rel(baseline_path)}",
            f"Candidate suite: {_repo_rel(candidate_path)}",
            f"Matched cases: {matched_case_count}",
            f"Failing cases: {failing_case_count}",
            "",
            "Case packets:",
            *[
                f"- {item['case_id']}: status={item['status']} detail={item.get('detail', 'n/a')}"
                for item in case_packets
            ],
        ],
    )
    print(json.dumps({"parity_json": _repo_rel(out_path), "parity_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_family_parity(args: argparse.Namespace) -> int:
    baseline_index_path, baseline_index = _load_json(args.baseline_index)
    candidate_path, candidate_payload = _load_json(args.candidate_suite)
    contract_registry_path, contract_registry = _load_json(args.contract_registry)
    parity_path = None if args.parity is None else _resolve_path(args.parity)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    producer_index_path = candidate_payload.get("producer_index_json")
    producer_payload = None
    if isinstance(producer_index_path, str):
        producer_payload = _load_optional_json(_resolve_path(producer_index_path))
    producer_status = {}
    if isinstance(producer_payload, dict):
        for producer in producer_payload.get("producers", []):
            if isinstance(producer, dict):
                producer_status[str(producer.get("producer_id"))] = str(producer.get("status"))
    candidate_planned_commands = {
        str(command_id).strip()
        for command_id in candidate_payload.get("planned_command_ids", [])
        if str(command_id).strip()
    }
    parity_payload = None if parity_path is None else _load_optional_json(parity_path)
    parity_cases_by_id = {}
    if isinstance(parity_payload, dict):
        for packet in parity_payload.get("case_packets", []):
            if not isinstance(packet, dict):
                continue
            case_id = str(packet.get("case_id", "")).strip()
            if case_id:
                parity_cases_by_id[case_id] = packet

    contract_lookup = {}
    for contract in contract_registry.get("contracts", []):
        if isinstance(contract, dict):
            contract_lookup[str(contract.get("contract_id"))] = contract

    family_packets = []
    for case in baseline_index.get("cases", []):
        if not isinstance(case, dict):
            continue
        contract_id = case.get("required_contract")
        if not contract_id:
            continue
        contract = contract_lookup.get(str(contract_id))
        required_producers = [] if not isinstance(contract, dict) else [str(item) for item in contract.get("required_producers", [])]
        required_packet_surfaces = [] if not isinstance(contract, dict) else [str(item) for item in contract.get("required_packet_surfaces", []) if str(item).strip()]
        missing_producers = [producer_id for producer_id in required_producers if producer_status.get(producer_id) != "bound"]
        missing_required_surfaces = [surface for surface in required_packet_surfaces if surface != "parity" and surface not in candidate_planned_commands]
        parity_case_status = None
        if "parity" in required_packet_surfaces:
            parity_case = parity_cases_by_id.get(str(case.get("case_id", "")).strip())
            if parity_case is None:
                missing_required_surfaces.append("parity")
            else:
                parity_case_status = str(parity_case.get("status", "")).strip()
                if parity_case_status not in GREEN_PARITY_CASE_STATUSES:
                    missing_required_surfaces.append("parity")
        packet_status = "critical_family_parity_match"
        if missing_producers:
            packet_status = "missing_required_producers"
        elif missing_required_surfaces:
            packet_status = "missing_required_surfaces"
        family_packets.append(
            {
                "case_id": case.get("case_id"),
                "family": case.get("family"),
                "contract_id": contract_id,
                "status": packet_status,
                "required_producers": required_producers,
                "missing_producers": missing_producers,
                "required_packet_surfaces": required_packet_surfaces,
                "missing_required_surfaces": missing_required_surfaces,
                "parity_case_status": parity_case_status,
                "blocked_product_threads": [] if not isinstance(contract, dict) else contract.get("blocked_product_threads", []),
            }
        )

    payload = {
        "schema_version": "viewer_host_salt_ndepend.family_parity.v1",
        "status": "family_case_parity",
        "command": "family-parity",
        "comparison_ready": bool(family_packets),
        "baseline_index_path": _repo_rel(baseline_index_path),
        "candidate_suite_path": _repo_rel(candidate_path),
        "contract_registry_path": _repo_rel(contract_registry_path),
        "parity_path": None if parity_path is None else _repo_rel(parity_path),
        "baseline_case_count": baseline_index.get("case_count", 0),
        "candidate_status": candidate_payload.get("status", "unknown"),
        "critical_family_packets": family_packets,
        "reason": "Family parity now combines contract-required producers, required packet surfaces, and optional parity case results to decide whether a contract-backed family is actually green.",
        "packet_meta": _packet_meta(
            "family-parity",
            {
                "baseline_index": _repo_rel(baseline_index_path),
                "candidate_suite": _repo_rel(candidate_path),
                "contract_registry": _repo_rel(contract_registry_path),
                "parity": None if parity_path is None else _repo_rel(parity_path),
            },
        ),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Family Parity",
            "",
            "Status: family_case_parity",
            f"Baseline index: {_repo_rel(baseline_index_path)}",
            f"Candidate suite: {_repo_rel(candidate_path)}",
            f"Contract registry: {_repo_rel(contract_registry_path)}",
            "",
            "Critical family packets:",
            *[
                f"- {item['case_id']} ({item['contract_id']}): status={item['status']} missing_producers={', '.join(item['missing_producers']) if item['missing_producers'] else 'none'} missing_surfaces={', '.join(item['missing_required_surfaces']) if item['missing_required_surfaces'] else 'none'}"
                for item in family_packets
            ],
        ],
    )
    print(json.dumps({"family_parity_json": _repo_rel(out_path), "family_parity_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_doctor(args: argparse.Namespace) -> int:
    packet_dir = _resolve_path(args.packet_dir)
    suite_index_path = None if args.suite_index is None else _resolve_path(args.suite_index)
    producer_index_path = None if args.producer_index is None else _resolve_path(args.producer_index)
    registry_path, registry = _load_json(args.contract_registry)
    freeze_gate_path, freeze_gate = _load_json(args.freeze_gate)
    manifest_path, manifest = _load_json(args.baseline_manifest)
    baseline_index_path = None if args.baseline_index is None else _resolve_path(args.baseline_index)
    family_parity_path = None if args.family_parity is None else _resolve_path(args.family_parity)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)

    findings: list[dict[str, object]] = []
    if not packet_dir.exists():
        findings.append({"code": "packet_dir_missing", "reason": f"packet_dir does not exist: {_repo_rel(packet_dir)}"})
    if suite_index_path is not None and not suite_index_path.exists():
        findings.append({"code": "suite_index_missing", "reason": f"suite_index does not exist: {_repo_rel(suite_index_path)}"})
    if producer_index_path is not None and not producer_index_path.exists():
        findings.append({"code": "producer_index_missing", "reason": f"producer_index does not exist: {_repo_rel(producer_index_path)}"})
    if baseline_index_path is not None and not baseline_index_path.exists():
        findings.append({"code": "baseline_index_missing", "reason": f"baseline_index does not exist: {_repo_rel(baseline_index_path)}"})
    if family_parity_path is not None and not family_parity_path.exists():
        findings.append({"code": "family_parity_missing", "reason": f"family_parity does not exist: {_repo_rel(family_parity_path)}"})

    suite_payload = None if suite_index_path is None else _load_optional_json(suite_index_path)
    producer_payload = None if producer_index_path is None else _load_optional_json(producer_index_path)
    family_parity_payload = None if family_parity_path is None else _load_optional_json(family_parity_path)
    if isinstance(suite_payload, dict):
        if suite_payload.get("missing_producer_count", 0):
            findings.append(
                {
                    "code": "audit_missing_producers",
                    "reason": f"audit packet reports {suite_payload.get('missing_producer_count', 0)} missing producer artifacts",
                }
            )
    if isinstance(producer_payload, dict):
        for producer in producer_payload.get("producers", []):
            if not isinstance(producer, dict):
                continue
            producer_status = str(producer.get("status", "missing")).strip() or "missing"
            if producer_status != "bound":
                producer_id = producer.get("producer_id", "unknown")
                code_prefix = "producer_missing" if producer_status == "missing" else "producer_unusable"
                reason = (
                    f"producer artifact missing for {producer_id}"
                    if producer_status == "missing"
                    else f"producer artifact unusable for {producer_id}: status={producer_status}"
                )
                findings.append(
                    {
                        "code": f"{code_prefix}:{producer_id}",
                        "reason": reason,
                    }
                )

    family_packets_by_contract: dict[str, dict[str, object]] = {}
    if isinstance(family_parity_payload, dict):
        for packet in family_parity_payload.get("critical_family_packets", []):
            if not isinstance(packet, dict):
                continue
            contract_id = str(packet.get("contract_id", "")).strip()
            if contract_id:
                family_packets_by_contract[contract_id] = packet

    for contract_id in freeze_gate.get("required_blocker_contracts", []):
        contract_text = str(contract_id).strip()
        if not contract_text:
            continue
        packet = family_packets_by_contract.get(contract_text)
        if packet is None:
            findings.append(
                {
                    "code": f"required_blocker_contract_packet_missing:{contract_text}",
                    "reason": f"family-parity does not report required blocker contract {contract_text}",
                }
            )
            continue
        packet_status = str(packet.get("status", "unknown")).strip() or "unknown"
        if packet_status not in GREEN_FAMILY_PARITY_STATUSES:
            missing_producers = [str(item) for item in packet.get("missing_producers", []) if str(item).strip()]
            missing_surfaces = [str(item) for item in packet.get("missing_required_surfaces", []) if str(item).strip()]
            missing_suffix = ""
            if missing_producers:
                missing_suffix = " missing_producers=" + ", ".join(missing_producers)
            if missing_surfaces:
                missing_suffix += " missing_required_surfaces=" + ", ".join(missing_surfaces)
            findings.append(
                {
                    "code": f"required_blocker_contract_not_green:{contract_text}",
                    "reason": f"family-parity reports {contract_text} status={packet_status}.{missing_suffix}".rstrip(),
                }
            )
    for blocker in freeze_gate.get("intentional_open_blockers", []):
        findings.append({"code": blocker.get("blocker_id"), "reason": blocker.get("reason")})

    freeze_ready = not findings

    payload = {
        "schema_version": "viewer_host_salt_ndepend.doctor.v1",
        "status": "producer_bound_seed",
        "command": "doctor",
        "freeze_ready": freeze_ready,
        "packet_dir": _repo_rel(packet_dir),
        "suite_index_path": None if suite_index_path is None else _repo_rel(suite_index_path),
        "producer_index_path": None if producer_index_path is None else _repo_rel(producer_index_path),
        "contract_registry_path": _repo_rel(registry_path),
        "freeze_gate_path": _repo_rel(freeze_gate_path),
        "baseline_manifest_path": _repo_rel(manifest_path),
        "contract_count": len(registry.get("contracts", [])),
        "baseline_case_count": len(manifest.get("cases", [])),
        "bound_producer_count": 0 if not isinstance(producer_payload, dict) else producer_payload.get("bound_producer_count", 0),
        "missing_producer_count": 0 if not isinstance(producer_payload, dict) else producer_payload.get("missing_producer_count", 0),
        "required_packet_commands": freeze_gate.get("required_packet_commands", []),
        "required_blocker_contracts": freeze_gate.get("required_blocker_contracts", []),
        "findings": findings,
        "packet_meta": _packet_meta(
            "doctor",
            {
                "packet_dir": _repo_rel(packet_dir),
                "suite_index": None if suite_index_path is None else _repo_rel(suite_index_path),
                "producer_index": None if producer_index_path is None else _repo_rel(producer_index_path),
                "contract_registry": _repo_rel(registry_path),
                "freeze_gate": _repo_rel(freeze_gate_path),
                "baseline_manifest": _repo_rel(manifest_path),
            },
        ),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Doctor",
            "",
            "Status: surface_seed_only",
            f"Freeze ready: {payload['freeze_ready']}",
            "",
            "Findings:",
            *[f"- {item['code']}: {item['reason']}" for item in findings],
        ],
    )
    print(json.dumps({"doctor_json": _repo_rel(out_path), "doctor_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_freeze_gate(args: argparse.Namespace) -> int:
    out_dir = _resolve_path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    command_results: list[dict[str, object]] = []

    def run_seeded(command_id: str, command_args: argparse.Namespace) -> int:
        with contextlib.redirect_stdout(io.StringIO()):
            rc = {
                "audit": _run_audit,
                "structural": _run_structural,
                "review": _run_review,
                "baselines": _run_baselines,
                "contracts": _run_contracts,
                "parity": _run_parity,
                "family-parity": _run_family_parity,
                "doctor": _run_doctor,
            }[command_id](command_args)
        command_results.append({"command": command_id, "rc": rc})
        return rc

    suite_index = out_dir / "suite_index.json"
    producer_index = out_dir / "producer_index.json"
    structural_index = out_dir / "structural_index.json"
    structural_md = out_dir / "structural_index.md"
    review_json = out_dir / "review.json"
    review_md = out_dir / "review.md"
    baseline_index = out_dir / "baseline_index.json"
    baseline_md = out_dir / "baseline_index.md"
    contracts_json = out_dir / "contracts.json"
    contracts_md = out_dir / "contracts.md"
    parity_json = out_dir / "parity.json"
    parity_md = out_dir / "parity.md"
    family_parity_json = out_dir / "family_parity.json"
    family_parity_md = out_dir / "family_parity.md"
    doctor_json = out_dir / "doctor.json"
    doctor_md = out_dir / "doctor.md"

    if run_seeded("audit", argparse.Namespace(policy=args.policy, out_dir=str(out_dir))) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded("structural", argparse.Namespace(policy=args.policy, out=str(structural_index), out_md=str(structural_md))) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded("review", argparse.Namespace(suite_index=str(suite_index), out=str(review_json), out_md=str(review_md))) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded("baselines", argparse.Namespace(manifest=args.manifest, out_dir=str(out_dir), out=str(baseline_index), out_md=str(baseline_md))) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded("contracts", argparse.Namespace(registry=args.contract_registry, out=str(contracts_json), out_md=str(contracts_md))) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded(
        "parity",
        argparse.Namespace(
            baseline_suite=str(baseline_index),
            candidate_suite=str(producer_index),
            contract_registry=args.contract_registry,
            out=str(parity_json),
            out_md=str(parity_md),
        ),
    ) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded(
        "family-parity",
        argparse.Namespace(
            baseline_index=str(baseline_index),
            candidate_suite=str(suite_index),
            contract_registry=args.contract_registry,
            parity=str(parity_json),
            out=str(family_parity_json),
            out_md=str(family_parity_md),
        ),
    ) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1
    if run_seeded(
        "doctor",
        argparse.Namespace(
            packet_dir=str(out_dir),
            suite_index=str(suite_index),
            producer_index=str(producer_index),
            contract_registry=args.contract_registry,
            freeze_gate=args.freeze_gate,
            baseline_manifest=args.manifest,
            baseline_index=str(baseline_index),
            family_parity=str(family_parity_json),
            out=str(doctor_json),
            out_md=str(doctor_md),
        ),
    ) != 0:
        print(json.dumps({"command_results": command_results}, indent=2))
        return 1

    doctor_payload = json.loads(doctor_json.read_text(encoding="utf-8"))
    print(
        json.dumps(
            {
                "command_results": command_results,
                "suite_index_json": _repo_rel(suite_index),
                "producer_index_json": _repo_rel(producer_index),
                "structural_json": _repo_rel(structural_index),
                "review_json": _repo_rel(review_json),
                "baseline_index_json": _repo_rel(baseline_index),
                "contracts_json": _repo_rel(contracts_json),
                "parity_json": _repo_rel(parity_json),
                "family_parity_json": _repo_rel(family_parity_json),
                "doctor_json": _repo_rel(doctor_json),
                "freeze_ready": bool(doctor_payload.get("freeze_ready", False)),
            },
            indent=2,
        )
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    if args.command == "audit":
        return _run_audit(args)
    if args.command == "structural":
        return _run_structural(args)
    if args.command == "review":
        return _run_review(args)
    if args.command == "baselines":
        return _run_baselines(args)
    if args.command == "contracts":
        return _run_contracts(args)
    if args.command == "parity":
        return _run_parity(args)
    if args.command == "family-parity":
        return _run_family_parity(args)
    if args.command == "doctor":
        return _run_doctor(args)
    if args.command == "freeze-gate":
        return _run_freeze_gate(args)

    parser.error(f"unknown command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
