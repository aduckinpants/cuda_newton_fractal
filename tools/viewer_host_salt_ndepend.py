from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_POLICY = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_POLICY.v1.json"
DEFAULT_BASELINES = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_BASELINES.v1.json"
DEFAULT_CONTRACTS = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_CONTRACTS.v1.json"
DEFAULT_FREEZE_GATE = REPO_ROOT / "docs" / "VIEWER_HOST_SALT_NDEPEND_FREEZE_GATE.v1.json"


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


def _summarize_text_artifact(path: Path) -> dict[str, object]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    tail = next((line.strip() for line in reversed(lines) if line.strip()), "")
    return {
        "line_count": len(lines),
        "tail": tail,
    }


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

    bound["status"] = "bound"
    bound["summary"] = summary
    return bound


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
    parity.add_argument("--out", required=True)
    parity.add_argument("--out-md", required=True)

    family_parity = subparsers.add_parser("family-parity", help="Emit the seeded family-parity packet surface.")
    family_parity.add_argument("--baseline-index", required=True)
    family_parity.add_argument("--candidate-suite", required=True)
    family_parity.add_argument("--out", required=True)
    family_parity.add_argument("--out-md", required=True)

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
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    payload = {
        "schema_version": "viewer_host_salt_ndepend.parity.v1",
        "status": "surface_seed_only",
        "command": "parity",
        "comparison_ready": False,
        "baseline_suite_path": _repo_rel(baseline_path),
        "candidate_suite_path": _repo_rel(candidate_path),
        "baseline_status": baseline_payload.get("status", "unknown"),
        "candidate_status": candidate_payload.get("status", "unknown"),
        "reason": "C2 seeds the local parity command surface only; real drift analysis lands in a later slice.",
        "packet_meta": _packet_meta("parity", {"baseline_suite": _repo_rel(baseline_path), "candidate_suite": _repo_rel(candidate_path)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Parity",
            "",
            "Status: surface_seed_only",
            f"Baseline suite: {_repo_rel(baseline_path)}",
            f"Candidate suite: {_repo_rel(candidate_path)}",
            "",
            "Real drift analysis lands in a later slice.",
        ],
    )
    print(json.dumps({"parity_json": _repo_rel(out_path), "parity_md": _repo_rel(out_md)}, indent=2))
    return 0


def _run_family_parity(args: argparse.Namespace) -> int:
    baseline_index_path, baseline_index = _load_json(args.baseline_index)
    candidate_path, candidate_payload = _load_json(args.candidate_suite)
    out_path = _resolve_path(args.out)
    out_md = _resolve_path(args.out_md)
    payload = {
        "schema_version": "viewer_host_salt_ndepend.family_parity.v1",
        "status": "surface_seed_only",
        "command": "family-parity",
        "comparison_ready": False,
        "baseline_index_path": _repo_rel(baseline_index_path),
        "candidate_suite_path": _repo_rel(candidate_path),
        "baseline_case_count": baseline_index.get("case_count", 0),
        "candidate_status": candidate_payload.get("status", "unknown"),
        "reason": "C2 seeds the family-parity surface only; representative family comparison lands in a later slice.",
        "packet_meta": _packet_meta("family-parity", {"baseline_index": _repo_rel(baseline_index_path), "candidate_suite": _repo_rel(candidate_path)}),
    }
    _write_json(out_path, payload)
    _write_markdown(
        out_md,
        [
            "# Viewer Host salt_ndepend Family Parity",
            "",
            "Status: surface_seed_only",
            f"Baseline index: {_repo_rel(baseline_index_path)}",
            f"Candidate suite: {_repo_rel(candidate_path)}",
            "",
            "Real family comparison lands in a later slice.",
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
            if producer.get("status") != "bound":
                findings.append(
                    {
                        "code": f"producer_missing:{producer.get('producer_id', 'unknown')}",
                        "reason": f"producer artifact missing for {producer.get('producer_id', 'unknown')}",
                    }
                )
    for blocker in freeze_gate.get("intentional_open_blockers", []):
        findings.append({"code": blocker.get("blocker_id"), "reason": blocker.get("reason")})

    payload = {
        "schema_version": "viewer_host_salt_ndepend.doctor.v1",
        "status": "producer_bound_seed",
        "command": "doctor",
        "freeze_ready": False,
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

    parser.error(f"unknown command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
