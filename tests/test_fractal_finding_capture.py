from __future__ import annotations

import json
import struct
import zlib
from pathlib import Path

from tools.reality_toolkit.fractal_explorer.finding_capture import archive_finding_bundle, validate_finding_id


def _write_test_bmp24(path: Path) -> None:
    width = 2
    height = 1
    row_stride = width * 3
    row_padded = (row_stride + 3) & ~3
    pixel_offset = 54
    file_size = pixel_offset + row_padded * height

    header = bytearray()
    header.extend(b"BM")
    header.extend(struct.pack("<I", file_size))
    header.extend(b"\x00\x00\x00\x00")
    header.extend(struct.pack("<I", pixel_offset))
    header.extend(struct.pack("<I", 40))
    header.extend(struct.pack("<i", width))
    header.extend(struct.pack("<i", height))
    header.extend(struct.pack("<H", 1))
    header.extend(struct.pack("<H", 24))
    header.extend(struct.pack("<I", 0))
    header.extend(struct.pack("<I", row_padded * height))
    header.extend(struct.pack("<i", 2835))
    header.extend(struct.pack("<i", 2835))
    header.extend(struct.pack("<I", 0))
    header.extend(struct.pack("<I", 0))

    # BMP stores pixels BGR, bottom-up. Pixel 0 = red, pixel 1 = green.
    row = bytes([
        0, 0, 255,
        0, 255, 0,
        0, 0,
    ])
    path.write_bytes(bytes(header) + row)


def _read_png_rgb_rows(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    assert data.startswith(b"\x89PNG\r\n\x1a\n")

    offset = 8
    width = 0
    height = 0
    idat = bytearray()
    while offset < len(data):
        length = struct.unpack_from(">I", data, offset)[0]
        chunk_type = data[offset + 4:offset + 8]
        chunk_data = data[offset + 8:offset + 8 + length]
        offset += 12 + length

        if chunk_type == b"IHDR":
            width = struct.unpack_from(">I", chunk_data, 0)[0]
            height = struct.unpack_from(">I", chunk_data, 4)[0]
        elif chunk_type == b"IDAT":
            idat.extend(chunk_data)
        elif chunk_type == b"IEND":
            break

    raw = zlib.decompress(bytes(idat))
    rows = bytearray()
    stride = width * 3
    for row_index in range(height):
        start = row_index * (stride + 1)
        assert raw[start] == 0
        rows.extend(raw[start + 1:start + 1 + stride])
    return width, height, bytes(rows)


def test_validate_finding_id_rejects_spaces() -> None:
    try:
        validate_finding_id("nova finding")
    except ValueError as exc:
        assert "finding_id" in str(exc)
    else:
        raise AssertionError("Expected invalid finding_id to raise ValueError")


def test_archive_finding_bundle_writes_png_state_fractal_state_and_sidecar(tmp_path: Path) -> None:
    diagnostics_dir = tmp_path / "diagnostics_last"
    diagnostics_dir.mkdir()
    _write_test_bmp24(diagnostics_dir / "frame.bmp")
    (diagnostics_dir / "state.json").write_text(
        json.dumps({"fractal_type": "nova", "params": {"max_iter": 300}}, indent=2),
        encoding="utf-8",
    )
    fractal_state_source = tmp_path / "capture-fractal-state.json"
    fractal_state_source.write_text(
        json.dumps(
            {
                "schema_id": "viewer.finding_fractal_state.v1",
                "fractal_type": "nova",
                "active_fractal_controls": [{"id": "max_iter", "value": 300}],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    output_dir = tmp_path / "findings" / "nova_escape_default"
    archive_finding_bundle(
        diagnostics_dir=diagnostics_dir,
        output_dir=output_dir,
        finding_id="nova_escape_default",
        why="Nova renders through the escape-time path after the rules fix.",
        repro_command="fractal_ui.exe --capture-diagnostic --fractal-type nova",
        fractal_state_json_path=fractal_state_source,
    )

    png_path = output_dir / "frame.png"
    state_path = output_dir / "state.json"
    fractal_state_path = output_dir / "fractal-state.json"
    sidecar_path = output_dir / "finding.md"
    json_path = output_dir / "finding.json"

    assert png_path.exists()
    assert state_path.exists()
    assert fractal_state_path.exists()
    assert sidecar_path.exists()
    assert json_path.exists()

    width, height, rows = _read_png_rgb_rows(png_path)
    assert (width, height) == (2, 1)
    assert rows == bytes([
        255, 0, 0,
        0, 255, 0,
    ])

    sidecar_text = sidecar_path.read_text(encoding="utf-8")
    assert "nova_escape_default" in sidecar_text
    assert "escape-time path" in sidecar_text
    assert "--fractal-type nova" in sidecar_text
    assert "fractal-state.json" in sidecar_text

    metadata = json.loads(json_path.read_text(encoding="utf-8"))
    assert metadata["finding_id"] == "nova_escape_default"
    assert metadata["fractal_type"] == "nova"
    assert metadata["state_file"] == "state.json"
    assert metadata["fractal_state_file"] == "fractal-state.json"
    assert json.loads(fractal_state_path.read_text(encoding="utf-8"))["schema_id"] == "viewer.finding_fractal_state.v1"
