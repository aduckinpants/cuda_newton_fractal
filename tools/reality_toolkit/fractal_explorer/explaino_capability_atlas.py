from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


DEFAULT_REPO_ROOT = Path(__file__).resolve().parents[3]
FRACTAL_TYPES_PATH = Path("ui_app/src/fractal_types.h")
FAMILY_RULES_PATH = Path("ui_app/src/fractal_family_rules.h")
DERIVED_FIELDS_PATH = Path("ui_app/src/fractal_derived_fields.cpp")
ANALYZER_PATH = Path("tools/reality_toolkit/fractal_explorer/finding_analyzer.py")
JSON_OUTPUT_PATH = Path("docs/manifests/explaino_capability_atlas.json")
MARKDOWN_OUTPUT_PATH = Path("docs/manifests/explaino_capability_atlas.md")


ROLE_RE = re.compile(
    r'\{FractalType::(?P<enum>explaino[_a-z0-9]*),\s*"(?P<id>explaino[_a-z0-9]*)",\s*ExplainoSelectorRole::(?P<role>[a-zA-Z0-9_]+)\}'
)
LENS_RE = re.compile(
    r'X\((?P<name>explaino[_a-z0-9]*),\s*FractalType::(?P<enum>explaino[_a-z0-9]*),\s*LensMaskPartition::(?P<partition>[a-zA-Z0-9_]+),\s*"(?P<semantic>[^"]+)",\s*"(?P<label>[^"]+)"\)'
)
CASE_RE = re.compile(r'case\s+FractalType::(?P<id>explaino[_a-z0-9]*):')


def _read(repo_root: Path, rel_path: Path) -> str:
    return (repo_root / rel_path).read_text(encoding="utf-8")


def _section(text: str, start_marker: str, end_marker: str) -> str:
    start = text.index(start_marker)
    end = text.index(end_marker, start)
    return text[start:end]


def parse_explaino_enum_ids(repo_root: Path = DEFAULT_REPO_ROOT) -> list[str]:
    text = _read(repo_root, FRACTAL_TYPES_PATH)
    enum_block = _section(text, "enum class FractalType", "};")
    ids = []
    for line in enum_block.splitlines():
        match = re.match(r"\s*(explaino[_a-z0-9]*)\s*=", line)
        if match:
            ids.append(match.group(1))
    return ids


def parse_explaino_selector_registry(repo_root: Path = DEFAULT_REPO_ROOT) -> dict[str, str]:
    text = _read(repo_root, FAMILY_RULES_PATH)
    selectors: dict[str, str] = {}
    for match in ROLE_RE.finditer(text):
        if match.group("enum") != match.group("id"):
            raise ValueError(f"selector enum/id mismatch: {match.group(0)}")
        selectors[match.group("id")] = match.group("role")
    return selectors


def parse_lens_mask_semantics(repo_root: Path = DEFAULT_REPO_ROOT) -> dict[str, dict[str, str]]:
    text = _read(repo_root, FAMILY_RULES_PATH)
    result: dict[str, dict[str, str]] = {}
    for match in LENS_RE.finditer(text):
        result[match.group("name")] = {
            "partition": match.group("partition"),
            "semantic_id": match.group("semantic"),
            "label": match.group("label"),
        }
    return result


def parse_root_editor_exclusions(repo_root: Path = DEFAULT_REPO_ROOT) -> set[str]:
    text = _read(repo_root, DERIVED_FIELDS_PATH)
    body = _section(text, "static bool IsExplainoRootEditorFractalType", "static int ClampExplainoCustomRootCount")
    return {match.group("id") for match in CASE_RE.finditer(body)}


def parse_composed_variant_ids(repo_root: Path = DEFAULT_REPO_ROOT) -> set[str]:
    text = _read(repo_root, DERIVED_FIELDS_PATH)
    body = _section(text, "static bool IsExplainoComposedVariantType", "void UpdateExplainoPolynomial")
    return {match.group("id") for match in CASE_RE.finditer(body)}


def _root_generation_model(fractal_id: str) -> str:
    if fractal_id == "explaino_dual":
        return "dual_seed_blended_seed_shape"
    if fractal_id == "explaino_mult":
        return "cluster_radius_split_seed_shape"
    if fractal_id in {"explaino_counterfactual_pair", "explaino_projection_and_flow"}:
        return "root_family_selector_driven_seed_shape"
    return "seed_shape_quartic"


def _generated_root_count(fractal_id: str) -> str:
    if fractal_id in {"explaino_counterfactual_pair", "explaino_projection_and_flow"}:
        return "3_or_4_by_root_family"
    return "4"


def _conjugate_semantics(fractal_id: str) -> str:
    if fractal_id == "explaino_mult":
        return "cluster_split_not_strict_conjugate_pairs"
    if fractal_id in {"explaino_counterfactual_pair", "explaino_projection_and_flow"}:
        return "root_family_dependent_quartic_pairs_or_cubic_pair_plus_real"
    return "two_seed_shape_conjugate_pairs"


def _secondary_notes(fractal_id: str, composed_variants: set[str]) -> list[str]:
    notes: list[str] = []
    if fractal_id == "explaino_dual":
        notes.append("Uses explaino_seed_b and explaino_mix to blend a secondary seed surface before root generation.")
    if fractal_id == "explaino_mult":
        notes.append("Uses explaino_cluster_radius to split the generated seed shape into clustered roots.")
    if fractal_id == "explaino_splice":
        notes.append("Always derives a secondary splice polynomial from splice_offset into poly_coeffs_b.")
    elif fractal_id in composed_variants:
        notes.append("May derive a secondary splice polynomial when splice_offset is nonzero under the composed-variant path.")
    if fractal_id == "explaino_counterfactual_pair":
        notes.append("counterfactual_pair_root_family selects cubic or quartic generated root layouts.")
    if fractal_id == "explaino_projection_and_flow":
        notes.append("projection_and_flow_root_family selects cubic or quartic generated root layouts.")
    return notes


def _analyzer_limitations(fractal_id: str) -> list[str]:
    return [
        "Finding analysis must use captured roots from params.explaino_roots when explaino_root_count is positive.",
        "Coefficient solving is fallback-only for legacy states without captured roots.",
        "This atlas records static capability evidence; it does not recompute per-capture root trajectories.",
        f"{fractal_id} semantics are only as specific as the checked-in static authority surfaces expose.",
    ]


def build_atlas(repo_root: Path = DEFAULT_REPO_ROOT) -> dict[str, Any]:
    repo_root = Path(repo_root)
    enum_ids = parse_explaino_enum_ids(repo_root)
    selectors = parse_explaino_selector_registry(repo_root)
    if set(enum_ids) != set(selectors):
        missing_from_registry = sorted(set(enum_ids) - set(selectors))
        missing_from_enum = sorted(set(selectors) - set(enum_ids))
        raise ValueError(
            "ExplainO enum/selector registry mismatch: "
            f"missing_from_registry={missing_from_registry}, missing_from_enum={missing_from_enum}"
        )

    root_editor_exclusions = parse_root_editor_exclusions(repo_root)
    composed_variants = parse_composed_variant_ids(repo_root)
    lens_semantics = parse_lens_mask_semantics(repo_root)
    missing_lens_semantics = sorted(set(enum_ids) - set(lens_semantics))
    if missing_lens_semantics:
        raise ValueError(f"ExplainO lens-mask semantics missing for: {missing_lens_semantics}")

    capabilities: list[dict[str, Any]] = []
    for fractal_id in enum_ids:
        custom_editor = fractal_id not in root_editor_exclusions
        capabilities.append({
            "fractal_id": fractal_id,
            "selector_role": selectors[fractal_id],
            "generated_root_support": True,
            "generated_root_count": _generated_root_count(fractal_id),
            "root_generation_model": _root_generation_model(fractal_id),
            "custom_root_editor_support": custom_editor,
            "custom_root_count": "3_or_4" if custom_editor else "unsupported_by_editor",
            "captured_root_behavior": {
                "runtime_fields": [
                    "params.explaino_root_authority",
                    "params.explaino_root_count",
                    "params.explaino_roots",
                ],
                "analysis_authority": "params.explaino_roots_when_present",
                "fallback": "legacy_coefficients_when_captured_roots_missing",
                "malformed_captured_roots": "fail_closed",
            },
            "conjugate_semantics": _conjugate_semantics(fractal_id),
            "secondary_root_family_notes": _secondary_notes(fractal_id, composed_variants),
            "lens_mask_semantics": lens_semantics[fractal_id],
            "source_evidence": [
                "ui_app/src/fractal_types.h:enum class FractalType",
                "ui_app/src/fractal_family_rules.h:kExplainoSelectorRegistry",
                "ui_app/src/fractal_derived_fields.cpp:UpdateExplainoPolynomial",
                "tools/reality_toolkit/fractal_explorer/finding_analyzer.py:resolve_analysis_roots",
            ],
            "analyzer_limitations": _analyzer_limitations(fractal_id),
        })

    return {
        "schema_id": "viewer.explaino_capability_atlas.v1",
        "version": 1,
        "generation_mode": "repo_derived_static_analysis",
        "source_files": [
            FRACTAL_TYPES_PATH.as_posix(),
            FAMILY_RULES_PATH.as_posix(),
            DERIVED_FIELDS_PATH.as_posix(),
            ANALYZER_PATH.as_posix(),
        ],
        "authority_notes": [
            "Captured params.explaino_roots are analysis authority when explaino_root_count is positive.",
            "Coefficient-derived roots are fallback-only for legacy states missing captured roots.",
            "The atlas is a static capability map, not a per-seed or per-capture trajectory measurement.",
        ],
        "fractal_capabilities": capabilities,
    }


def render_markdown(payload: dict[str, Any]) -> str:
    lines = [
        "# ExplainO Capability Atlas",
        "",
        "Generated from checked-in enum, family-rule, derived-field, and analyzer authority surfaces.",
        "",
        "## Authority Notes",
        "",
    ]
    for note in payload["authority_notes"]:
        lines.append(f"- {note}")
    lines.extend([
        "",
        "## Fractal Capabilities",
        "",
        "| Fractal | Selector Role | Generated Roots | Custom Root Editor | Root Model | Conjugate Semantics | Lens Mask |",
        "| --- | --- | --- | --- | --- | --- | --- |",
    ])
    for entry in payload["fractal_capabilities"]:
        lens = entry["lens_mask_semantics"]["semantic_id"]
        lines.append(
            "| {fractal_id} | {selector_role} | {generated_root_count} | {custom_root_editor_support} | {root_generation_model} | {conjugate_semantics} | {lens} |".format(
                lens=lens,
                **entry,
            )
        )
    lines.extend(["", "## Analyzer Limitations", ""])
    for entry in payload["fractal_capabilities"]:
        notes = "; ".join(entry["analyzer_limitations"])
        lines.append(f"- `{entry['fractal_id']}`: {notes}")
    return "\n".join(lines) + "\n"


def write_atlas(repo_root: Path = DEFAULT_REPO_ROOT) -> tuple[Path, Path]:
    payload = build_atlas(repo_root)
    json_path = repo_root / JSON_OUTPUT_PATH
    markdown_path = repo_root / MARKDOWN_OUTPUT_PATH
    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    markdown_path.write_text(render_markdown(payload), encoding="utf-8")
    return json_path, markdown_path


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate the ExplainO capability atlas")
    parser.add_argument("--repo-root", type=Path, default=DEFAULT_REPO_ROOT)
    parser.add_argument("--write", action="store_true", help="Write docs/manifests atlas outputs")
    args = parser.parse_args(argv)

    payload = build_atlas(args.repo_root)
    if args.write:
        json_path, markdown_path = write_atlas(args.repo_root)
        print(f"wrote {json_path}")
        print(f"wrote {markdown_path}")
    else:
        print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
