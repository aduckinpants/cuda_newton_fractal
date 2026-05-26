from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
COLOR_PIPELINE_WINDOW = REPO_ROOT / "ui_app" / "src" / "color_pipeline_window.h"


def test_color_pipeline_preset_copy_does_not_reintroduce_internal_bridge_leakage() -> None:
    text = COLOR_PIPELINE_WINDOW.read_text(encoding="utf-8")
    assert '"Draft Source' not in text
    assert '"Draft ' not in text
    assert '"Live bridge:' not in text


def test_color_pipeline_composition_copy_hides_internal_workflow_terms() -> None:
    text = COLOR_PIPELINE_WINDOW.read_text(encoding="utf-8")
    forbidden_visible_fragments = [
        '"Reset Draft From Live"',
        '"Current live bridge',
        '"color pipeline draft/live bridge rejected visible control:',
        '"This fixed %s row has no tunable parameters; choosing it changes the live bridge tuple directly."',
        '"Current live runtime tuple is outside the shipped advanced catalog; keep editing the programmable draft',
        '" (draft only)"',
        '"Live selection:',
        '"The row editor keeps its own starter state until live runtime maps onto',
        '"Live selection: current runtime color state',
        '"reset-from-live',
        '"Supported runtime presets right now:',
    ]
    for fragment in forbidden_visible_fragments:
        assert fragment not in text, fragment
