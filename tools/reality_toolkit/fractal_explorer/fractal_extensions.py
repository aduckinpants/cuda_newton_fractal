from __future__ import annotations

import json
import subprocess
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Any, Callable, Mapping, Sequence

from .finding_analyzer import analyze_finding
from .finding_capture import archive_finding_bundle, validate_finding_id
from .finding_charts import generate_all
from .paths import diagnostics_last_dir, findings_root, runtime_root
from .probe_client import resolve_probe_runtime_path, run_sample_request


DEFAULT_EXTENSION_SIDECAR_METRICS = [
    "iterations",
    "status",
    "value",
    "abs2",
    "derivative",
]

DEFAULT_FINDING_GROUP = "fractal_extensions_gallery"


def _validate_label(label: str, field_name: str) -> str:
    try:
        return validate_finding_id(label)
    except ValueError as exc:
        raise ValueError(f"{field_name} {exc}") from exc


def _require_mapping(value: object, context: str) -> Mapping[str, object]:
    if not isinstance(value, Mapping):
        raise ValueError(f"{context} must be a JSON object")
    return value


def _require_sequence(value: object, context: str) -> Sequence[object]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes, bytearray)):
        raise ValueError(f"{context} must be a JSON array")
    return value


def _require_string(mapping: Mapping[str, object], key: str, context: str) -> str:
    value = mapping.get(key)
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{context}.{key} must be a non-empty string")
    return value


def _optional_string(mapping: Mapping[str, object], key: str) -> str | None:
    value = mapping.get(key)
    if value is None:
        return None
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{key} must be a non-empty string when provided")
    return value


def _require_float(mapping: Mapping[str, object], key: str, context: str) -> float:
    value = mapping.get(key)
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{context}.{key} must be a number")
    return float(value)


def _require_int(mapping: Mapping[str, object], key: str, context: str) -> int:
    value = mapping.get(key)
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{context}.{key} must be an integer")
    return value


def _runtime_command(exe_path: Path, *args: str) -> list[str]:
    command = [str(exe_path), *args]
    if exe_path.suffix.lower() in {".cmd", ".bat"}:
        return ["cmd.exe", "/d", "/c", *command]
    return command


def _write_json(path: Path, payload: Mapping[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


@dataclass(frozen=True)
class FractalExtensionRegion:
    center_x: float
    center_y: float
    span_x: float
    span_y: float
    grid_width: int
    grid_height: int

    def __post_init__(self) -> None:
        if self.span_x <= 0.0 or self.span_y <= 0.0:
            raise ValueError("region spans must be > 0")
        if self.grid_width <= 0 or self.grid_height <= 0:
            raise ValueError("region grid size must be > 0")

    def to_dict(self) -> dict[str, object]:
        return {
            "center_x": self.center_x,
            "center_y": self.center_y,
            "span_x": self.span_x,
            "span_y": self.span_y,
            "grid_width": self.grid_width,
            "grid_height": self.grid_height,
        }


@dataclass(frozen=True)
class FractalExtensionSidecar:
    sidecar_id: str
    expression: str
    params: Mapping[str, float] | None = None
    epsilon: float | None = None
    escape_radius: float | None = None
    metrics: Sequence[str] = tuple(DEFAULT_EXTENSION_SIDECAR_METRICS)
    notes: str = ""

    def __post_init__(self) -> None:
        _validate_label(self.sidecar_id, "sidecar_id")
        if not self.expression.strip():
            raise ValueError("expression must be a non-empty string")
        if not self.metrics:
            raise ValueError("metrics must not be empty")

    def to_dict(self) -> dict[str, object]:
        payload: dict[str, object] = {
            "sidecar_id": self.sidecar_id,
            "expression": self.expression,
            "metrics": list(self.metrics),
            "notes": self.notes,
        }
        if self.params:
            payload["params"] = dict(self.params)
        if self.epsilon is not None:
            payload["epsilon"] = self.epsilon
        if self.escape_radius is not None:
            payload["escape_radius"] = self.escape_radius
        return payload


@dataclass(frozen=True)
class FractalExtensionScene:
    scene_id: str
    state_json: Path
    why: str
    region: FractalExtensionRegion
    sidecars: Sequence[FractalExtensionSidecar] = ()
    finding_id: str | None = None

    def __post_init__(self) -> None:
        _validate_label(self.scene_id, "scene_id")
        if self.finding_id is not None:
            _validate_label(self.finding_id, "finding_id")
        if not self.why.strip():
            raise ValueError("why must be a non-empty string")

    @property
    def resolved_finding_id(self) -> str:
        return self.finding_id or self.scene_id

    def to_dict(self) -> dict[str, object]:
        payload: dict[str, object] = {
            "scene_id": self.scene_id,
            "state_json": str(self.state_json),
            "why": self.why,
            "region": self.region.to_dict(),
            "sidecars": [sidecar.to_dict() for sidecar in self.sidecars],
        }
        if self.finding_id is not None:
            payload["finding_id"] = self.finding_id
        return payload


@dataclass(frozen=True)
class FractalExtensionsManifest:
    finding_group: str = DEFAULT_FINDING_GROUP
    scenes: Sequence[FractalExtensionScene] = ()

    def __post_init__(self) -> None:
        _validate_label(self.finding_group, "finding_group")
        if not self.scenes:
            raise ValueError("manifest must define at least one scene")

    def to_dict(self) -> dict[str, object]:
        return {
            "finding_group": self.finding_group,
            "scenes": [scene.to_dict() for scene in self.scenes],
        }


def default_fractal_extensions_out_dir(repo_root: Path, finding_group: str) -> Path:
    date_label = datetime.now().strftime("%Y-%m-%d")
    return findings_root(repo_root) / _validate_label(finding_group, "finding_group") / date_label


def _parse_region(payload: Mapping[str, object], context: str) -> FractalExtensionRegion:
    return FractalExtensionRegion(
        center_x=_require_float(payload, "center_x", context),
        center_y=_require_float(payload, "center_y", context),
        span_x=_require_float(payload, "span_x", context),
        span_y=_require_float(payload, "span_y", context),
        grid_width=_require_int(payload, "grid_width", context),
        grid_height=_require_int(payload, "grid_height", context),
    )


def _parse_sidecar(payload: Mapping[str, object], context: str) -> FractalExtensionSidecar:
    params_value = payload.get("params")
    params: dict[str, float] | None = None
    if params_value is not None:
        params_mapping = _require_mapping(params_value, f"{context}.params")
        params = {}
        for key, value in params_mapping.items():
            if not isinstance(key, str) or not key:
                raise ValueError(f"{context}.params keys must be non-empty strings")
            if isinstance(value, bool) or not isinstance(value, (int, float)):
                raise ValueError(f"{context}.params.{key} must be a number")
            params[key] = float(value)

    metrics_value = payload.get("metrics", list(DEFAULT_EXTENSION_SIDECAR_METRICS))
    metrics_raw = _require_sequence(metrics_value, f"{context}.metrics")
    metrics = [metric for metric in metrics_raw if isinstance(metric, str) and metric.strip()]
    if len(metrics) != len(metrics_raw):
        raise ValueError(f"{context}.metrics entries must be non-empty strings")

    epsilon = payload.get("epsilon")
    if epsilon is not None and (isinstance(epsilon, bool) or not isinstance(epsilon, (int, float))):
        raise ValueError(f"{context}.epsilon must be numeric when provided")
    escape_radius = payload.get("escape_radius")
    if escape_radius is not None and (isinstance(escape_radius, bool) or not isinstance(escape_radius, (int, float))):
        raise ValueError(f"{context}.escape_radius must be numeric when provided")

    notes = payload.get("notes", "")
    if not isinstance(notes, str):
        raise ValueError(f"{context}.notes must be a string when provided")

    return FractalExtensionSidecar(
        sidecar_id=_require_string(payload, "sidecar_id", context),
        expression=_require_string(payload, "expression", context),
        params=params,
        epsilon=float(epsilon) if epsilon is not None else None,
        escape_radius=float(escape_radius) if escape_radius is not None else None,
        metrics=tuple(metrics),
        notes=notes,
    )


def _parse_scene(payload: Mapping[str, object], manifest_path: Path, index: int) -> FractalExtensionScene:
    context = f"scenes[{index}]"
    state_value = _require_string(payload, "state_json", context)
    state_json = Path(state_value)
    if not state_json.is_absolute():
        state_json = (manifest_path.parent / state_json).resolve()
    if not state_json.exists():
        raise FileNotFoundError(f"{context}.state_json does not exist: {state_json}")

    sidecars_value = payload.get("sidecars", [])
    sidecars_raw = _require_sequence(sidecars_value, f"{context}.sidecars")
    sidecars = tuple(
        _parse_sidecar(_require_mapping(sidecar_value, f"{context}.sidecars[{sidecar_index}]"), f"{context}.sidecars[{sidecar_index}]")
        for sidecar_index, sidecar_value in enumerate(sidecars_raw)
    )

    return FractalExtensionScene(
        scene_id=_require_string(payload, "scene_id", context),
        state_json=state_json,
        why=_require_string(payload, "why", context),
        region=_parse_region(_require_mapping(payload.get("region"), f"{context}.region"), f"{context}.region"),
        sidecars=sidecars,
        finding_id=_optional_string(payload, "finding_id"),
    )


def load_fractal_extensions_manifest(manifest_path: Path) -> FractalExtensionsManifest:
    payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    root = _require_mapping(payload, "manifest")
    scenes_raw = _require_sequence(root.get("scenes"), "manifest.scenes")
    scenes = tuple(
        _parse_scene(_require_mapping(scene_value, f"manifest.scenes[{index}]"), manifest_path, index)
        for index, scene_value in enumerate(scenes_raw)
    )
    finding_group = root.get("finding_group", DEFAULT_FINDING_GROUP)
    if not isinstance(finding_group, str) or not finding_group.strip():
        raise ValueError("manifest.finding_group must be a non-empty string when provided")
    return FractalExtensionsManifest(finding_group=finding_group, scenes=scenes)


def build_sidecar_request(scene: FractalExtensionScene, sidecar: FractalExtensionSidecar) -> dict[str, object]:
    function: dict[str, object] = {
        "expression": sidecar.expression,
    }
    if sidecar.params:
        function["params"] = dict(sidecar.params)
    if sidecar.epsilon is not None:
        function["epsilon"] = sidecar.epsilon
    if sidecar.escape_radius is not None:
        function["escape_radius"] = sidecar.escape_radius

    return {
        "request_version": 1,
        "request_id": f"{scene.scene_id}-{sidecar.sidecar_id}",
        "function_id": "generic.sample",
        "mode": "grid",
        "function": function,
        "region": scene.region.to_dict(),
        "metrics": list(sidecar.metrics),
        "operator_context": {
            "source": "reality_toolkit",
            "operator": "fractal_extensions",
            "why": sidecar.notes or scene.why,
        },
    }


def run_headless_capture_diagnostic(
    repo_root: Path,
    state_json: Path,
    *,
    exe_path: Path | None = None,
    timeout_seconds: float = 300.0,
) -> Path:
    resolved_exe = exe_path or resolve_probe_runtime_path(repo_root)
    result = subprocess.run(
        _runtime_command(resolved_exe, "--load-state-json", str(state_json), "--capture-diagnostic"),
        cwd=str(runtime_root(repo_root)),
        text=True,
        capture_output=True,
        timeout=timeout_seconds,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or result.stdout.strip() or f"capture-diagnostic failed with exit code {result.returncode}")

    diagnostics_dir = diagnostics_last_dir(repo_root)
    if not (diagnostics_dir / "frame.bmp").exists():
        raise RuntimeError(f"capture-diagnostic did not produce frame.bmp under {diagnostics_dir}")
    if not (diagnostics_dir / "state.json").exists():
        raise RuntimeError(f"capture-diagnostic did not produce state.json under {diagnostics_dir}")
    return diagnostics_dir


def _append_extension_sidecars_to_markdown(
    finding_md_path: Path,
    field_notes_path: Path,
    sidecars: Sequence[Mapping[str, object]],
) -> None:
    if not sidecars:
        return

    lines = [
        "",
        "## Extension Sidecars",
        "",
    ]
    for sidecar in sidecars:
        lines.append(f"- `{sidecar['sidecar_id']}`")
        lines.append(f"  - Expression: `{sidecar['expression']}`")
        notes = sidecar.get("notes")
        if isinstance(notes, str) and notes.strip():
            lines.append(f"  - Why: {notes.strip()}")
        request_path = sidecar.get("request_path")
        if isinstance(request_path, str):
            lines.append(f"  - Request: {request_path}")
        response_path = sidecar.get("response_path")
        if isinstance(response_path, str):
            lines.append(f"  - Response: {response_path}")
    finding_md_path.write_text(finding_md_path.read_text(encoding="utf-8") + "\n".join(lines) + "\n", encoding="utf-8")

    notes_lines = [
        "",
        "## Extension Sidecars",
        "",
    ]
    for sidecar in sidecars:
        notes_lines.append(f"- {sidecar['sidecar_id']}: {sidecar.get('notes') or sidecar['expression']}")
    field_notes_path.write_text(field_notes_path.read_text(encoding="utf-8") + "\n".join(notes_lines) + "\n", encoding="utf-8")


def _scene_metadata(scene: FractalExtensionScene) -> dict[str, object]:
    payload = scene.to_dict()
    payload["finding_id"] = scene.resolved_finding_id
    return payload


def run_fractal_extensions_composite(
    *,
    repo_root: Path,
    manifest: FractalExtensionsManifest | None = None,
    manifest_path: Path | None = None,
    out_dir: Path | None = None,
    finding_group: str | None = None,
    analyze_findings: bool = True,
    dry_run: bool = False,
    timeout_seconds: float = 300.0,
    exe_path: Path | None = None,
    overwrite: bool = False,
    capture_runner: Callable[..., Path] = run_headless_capture_diagnostic,
    sample_runner: Callable[..., dict[str, object]] = run_sample_request,
    analysis_runner: Callable[[Path], Any] = analyze_finding,
    analysis_writer: Callable[[Any, Path], Mapping[str, str]] = generate_all,
) -> dict[str, object]:
    if manifest is None:
        if manifest_path is None:
            raise ValueError("Either manifest or manifest_path is required")
        manifest = load_fractal_extensions_manifest(manifest_path)

    resolved_group = finding_group or manifest.finding_group
    resolved_out_dir = out_dir or default_fractal_extensions_out_dir(repo_root, resolved_group)
    resolved_out_dir.mkdir(parents=True, exist_ok=True)

    summary: dict[str, object] = {
        "tool": "fractal_extensions",
        "finding_group": resolved_group,
        "out_dir": str(resolved_out_dir),
        "dry_run": dry_run,
        "scene_count": len(manifest.scenes),
        "scene_summaries": [],
    }
    if manifest_path is not None:
        summary["manifest_path"] = str(manifest_path)

    for scene in manifest.scenes:
        if not scene.state_json.exists():
            raise FileNotFoundError(f"Scene state_json does not exist: {scene.state_json}")

        scene_dir = resolved_out_dir / scene.resolved_finding_id
        sidecar_records: list[dict[str, object]] = []
        analysis_manifest: Mapping[str, str] | None = None

        if not dry_run:
            resolved_exe = exe_path or resolve_probe_runtime_path(repo_root)
            diagnostics_dir = capture_runner(
                repo_root,
                scene.state_json,
                exe_path=resolved_exe,
                timeout_seconds=timeout_seconds,
            )
            repro_command = f'"{resolved_exe}" --load-state-json "{scene_dir / "state.json"}" --capture-diagnostic'
            archive_finding_bundle(
                diagnostics_dir=diagnostics_dir,
                output_dir=scene_dir,
                finding_id=scene.resolved_finding_id,
                why=scene.why,
                repro_command=repro_command,
                overwrite=overwrite,
            )
        else:
            scene_dir.mkdir(parents=True, exist_ok=True)

        _write_json(scene_dir / "scene.json", _scene_metadata(scene))

        for sidecar in scene.sidecars:
            request = build_sidecar_request(scene, sidecar)
            sidecar_dir = scene_dir / "sidecars" / sidecar.sidecar_id
            _write_json(sidecar_dir / "request.json", request)

            response_path = sidecar_dir / "response.json"
            response_summary: Mapping[str, object] | None = None
            if not dry_run:
                resolved_exe = exe_path or resolve_probe_runtime_path(repo_root)
                response = sample_runner(
                    repo_root,
                    request,
                    exe_path=resolved_exe,
                    timeout_seconds=timeout_seconds,
                )
                _write_json(response_path, response)
                response_summary_raw = response.get("summary")
                if isinstance(response_summary_raw, Mapping):
                    response_summary = dict(response_summary_raw)

            sidecar_records.append({
                "sidecar_id": sidecar.sidecar_id,
                "expression": sidecar.expression,
                "notes": sidecar.notes,
                "request_path": str(sidecar_dir / "request.json"),
                "response_path": str(response_path) if response_path.exists() else None,
                "summary": response_summary,
            })

        _write_json(
            scene_dir / "extension_sidecars.json",
            {
                "scene_id": scene.scene_id,
                "finding_id": scene.resolved_finding_id,
                "sidecars": sidecar_records,
            },
        )

        if not dry_run and scene.sidecars:
            _append_extension_sidecars_to_markdown(
                scene_dir / "finding.md",
                scene_dir / "field-notes.md",
                sidecar_records,
            )

        if not dry_run and analyze_findings:
            analysis = analysis_runner(scene_dir)
            analysis_manifest = dict(analysis_writer(analysis, scene_dir / "analysis"))

        summary["scene_summaries"].append({
            "scene_id": scene.scene_id,
            "finding_id": scene.resolved_finding_id,
            "finding_dir": str(scene_dir),
            "captured": not dry_run,
            "sidecar_count": len(scene.sidecars),
            "sidecars": sidecar_records,
            "analysis_manifest": dict(analysis_manifest) if analysis_manifest is not None else None,
        })

    summary_path = resolved_out_dir / "fractal_extensions_summary.json"
    _write_json(summary_path, summary)
    summary["summary_path"] = str(summary_path)
    return summary
