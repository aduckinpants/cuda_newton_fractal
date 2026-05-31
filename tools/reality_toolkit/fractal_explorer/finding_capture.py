from __future__ import annotations

import json
import re
import shutil
import struct
import zlib
from pathlib import Path


_VALID_FINDING_ID = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]*$")
_PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def validate_finding_id(finding_id: str) -> str:
    if not _VALID_FINDING_ID.fullmatch(finding_id):
        raise ValueError("finding_id must match ^[A-Za-z0-9][A-Za-z0-9._-]*$")
    return finding_id


def _load_bmp24_pixels(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"Unsupported BMP header in {path}")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError(f"Unsupported DIB header in {path}")

    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    if bits_per_pixel != 24 or compression != 0:
        raise ValueError(f"Only 24-bit BI_RGB BMPs are supported, got {bits_per_pixel}-bit compression={compression}")

    abs_height = abs(height)
    row_stride = width * 3
    row_padded = (row_stride + 3) & ~3
    pixel_bytes = bytearray(width * abs_height * 3)

    top_down = height < 0
    for row_index in range(abs_height):
        source_row = row_index if top_down else (abs_height - 1 - row_index)
        source_offset = pixel_offset + source_row * row_padded
        target_offset = row_index * row_stride
        pixel_bytes[target_offset:target_offset + row_stride] = data[source_offset:source_offset + row_stride]

    return width, abs_height, bytes(pixel_bytes)


def _write_png_chunk(chunk_type: bytes, chunk_data: bytes) -> bytes:
    payload = chunk_type + chunk_data
    checksum = zlib.crc32(payload) & 0xFFFFFFFF
    return struct.pack(">I", len(chunk_data)) + payload + struct.pack(">I", checksum)


def _write_png_rgb(path: Path, width: int, height: int, rgb_pixels: bytes) -> None:
    scanlines = bytearray()
    row_stride = width * 3
    for row_index in range(height):
        scanlines.append(0)
        row_offset = row_index * row_stride
        scanlines.extend(rgb_pixels[row_offset:row_offset + row_stride])

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    png_bytes = bytearray(_PNG_SIGNATURE)
    png_bytes.extend(_write_png_chunk(b"IHDR", ihdr))
    png_bytes.extend(_write_png_chunk(b"IDAT", zlib.compress(bytes(scanlines), level=9)))
    png_bytes.extend(_write_png_chunk(b"IEND", b""))
    path.write_bytes(bytes(png_bytes))


def convert_bmp24_to_png(source_bmp: Path, destination_png: Path) -> None:
    width, height, bgr_pixels = _load_bmp24_pixels(source_bmp)
    rgb_pixels = bytearray(len(bgr_pixels))
    for offset in range(0, len(bgr_pixels), 3):
        blue = bgr_pixels[offset + 0]
        green = bgr_pixels[offset + 1]
        red = bgr_pixels[offset + 2]
        rgb_pixels[offset + 0] = red
        rgb_pixels[offset + 1] = green
        rgb_pixels[offset + 2] = blue
    _write_png_rgb(destination_png, width, height, bytes(rgb_pixels))


def _load_state_data(state_json_path: Path) -> dict[str, object] | None:
    try:
        data = json.loads(state_json_path.read_text(encoding="utf-8"))
    except Exception:
        return None
    return data if isinstance(data, dict) else None


def _write_sidecar(
    path: Path,
    *,
    finding_id: str,
    why: str,
    repro_command: str,
    fractal_type: str | None,
    fractal_state_file: str | None = None,
) -> None:
    lines = [
        f"# Fractal Finding: {finding_id}",
        "",
    ]
    if fractal_type:
        lines.extend([
            f"- Fractal type: {fractal_type}",
            "",
        ])
    lines.extend([
        "## Why it matters",
        why.strip(),
        "",
        "## Reproduce",
        "```powershell",
        repro_command.strip(),
        "```",
        "",
        "## Artifacts",
        "- frame.png",
        "- state.json",
    ])
    if fractal_state_file:
        lines.append(f"- {fractal_state_file}")
    lines.extend([
        "- finding.json",
        "",
    ])
    path.write_text("\n".join(lines), encoding="utf-8")


def _write_field_notes_template(path: Path, *, finding_id: str) -> None:
    lines = [
        f"# Field Notes: {finding_id}",
        "",
        "## What caught my eye",
        "",
        "(describe what you saw that made you stop and capture this)",
        "",
        "## Observations",
        "",
        "- ",
        "",
        "## Follow-up ideas",
        "",
        "- ",
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")


def archive_finding_bundle(
    *,
    diagnostics_dir: Path,
    output_dir: Path,
    finding_id: str,
    why: str,
    repro_command: str,
    fractal_state_json_path: Path | None = None,
    overwrite: bool = False,
) -> Path:
    validate_finding_id(finding_id)

    frame_bmp_path = diagnostics_dir / "frame.bmp"
    state_json_path = diagnostics_dir / "state.json"
    if not diagnostics_dir.exists():
        raise FileNotFoundError(f"Diagnostics directory does not exist: {diagnostics_dir}")
    if not frame_bmp_path.exists():
        raise FileNotFoundError(f"Missing frame.bmp in {diagnostics_dir}")
    if not state_json_path.exists():
        raise FileNotFoundError(f"Missing state.json in {diagnostics_dir}")
    if fractal_state_json_path is not None and not fractal_state_json_path.exists():
        raise FileNotFoundError(f"Missing fractal-state sidecar: {fractal_state_json_path}")

    if output_dir.exists():
        if not overwrite:
            raise FileExistsError(f"Finding output already exists: {output_dir}")
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    frame_png_path = output_dir / "frame.png"
    archived_state_path = output_dir / "state.json"
    archived_fractal_state_path = output_dir / "fractal-state.json"
    sidecar_path = output_dir / "finding.md"
    metadata_path = output_dir / "finding.json"

    convert_bmp24_to_png(frame_bmp_path, frame_png_path)
    shutil.copy2(state_json_path, archived_state_path)
    if fractal_state_json_path is not None:
        shutil.copy2(fractal_state_json_path, archived_fractal_state_path)

    state_data = _load_state_data(state_json_path)
    fractal_type = state_data.get("fractal_type") if isinstance(state_data, dict) else None
    metadata = {
        "finding_id": finding_id,
        "why": why,
        "repro_command": repro_command,
        "diagnostics_dir": str(diagnostics_dir),
        "fractal_type": fractal_type if isinstance(fractal_type, str) else None,
        "frame_file": frame_png_path.name,
        "state_file": archived_state_path.name,
        "sidecar_file": sidecar_path.name,
    }
    if fractal_state_json_path is not None:
        metadata["fractal_state_file"] = archived_fractal_state_path.name

    _write_sidecar(
        sidecar_path,
        finding_id=finding_id,
        why=why,
        repro_command=repro_command,
        fractal_type=metadata["fractal_type"],
        fractal_state_file=metadata.get("fractal_state_file") if isinstance(metadata.get("fractal_state_file"), str) else None,
    )
    metadata_path.write_text(json.dumps(metadata, indent=2), encoding="utf-8")

    field_notes_path = output_dir / "field-notes.md"
    _write_field_notes_template(field_notes_path, finding_id=finding_id)

    return output_dir
