from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
COLOR_PIPELINE_WINDOW = REPO_ROOT / "ui_app" / "src" / "color_pipeline_window.h"


def test_color_pipeline_preset_copy_does_not_reintroduce_internal_bridge_leakage() -> None:
    text = COLOR_PIPELINE_WINDOW.read_text(encoding="utf-8")
    assert '"Draft Source' not in text
    assert '"Draft ' not in text
    assert '"Live bridge:' not in text
