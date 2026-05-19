from tools.viewer_host_validate_fractal_toolkit_scan import build_report


def test_fractal_toolkit_scan_accepts_bounded_magnet_lane() -> None:
    report = build_report()
    assert report["ok"] is True, report["findings"]
    assert report["findings"] == []
