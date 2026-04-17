from __future__ import annotations

import argparse
import json
import math
import struct
import subprocess
import sys
import time
from pathlib import Path


def _active_runtime_exe(runtime_dir: Path) -> Path:
    active_file = runtime_dir / "fractal_ui_active.txt"
    if not active_file.exists():
        raise RuntimeError(f"missing active runtime metadata: {active_file}")
    active_name = active_file.read_text(encoding="utf-8").strip()
    if not active_name:
        raise RuntimeError(f"empty active runtime metadata: {active_file}")
    exe_path = runtime_dir / active_name
    if not exe_path.exists():
        raise RuntimeError(f"active runtime missing: {exe_path}")
    return exe_path


def _read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _write_json(path: Path, payload: dict) -> None:
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _read_bmp32(path: Path) -> tuple[int, int, list[int]]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise RuntimeError(f"unsupported BMP file: {path}")
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    header_size = struct.unpack_from("<I", data, 14)[0]
    if header_size < 40:
        raise RuntimeError(f"unsupported BMP header: {path}")
    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bit_count = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    if compression != 0 or bit_count not in (24, 32):
        raise RuntimeError(f"expected 24-bit or 32-bit uncompressed BMP: {path}")
    top_down = height < 0
    abs_height = abs(height)
    row_stride = ((width * bit_count + 31) // 32) * 4
    pixels = [0] * (width * abs_height)
    for row in range(abs_height):
        src_row = row if top_down else (abs_height - 1 - row)
        start = pixel_offset + src_row * row_stride
        chunk = data[start:start + row_stride]
        for x in range(width):
            if bit_count == 32:
                b, g, r, a = chunk[x * 4:x * 4 + 4]
            else:
                b, g, r = chunk[x * 3:x * 3 + 3]
                a = 0xFF
            pixels[row * width + x] = r | (g << 8) | (b << 16) | (a << 24)
    return width, abs_height, pixels


def _write_bmp32(path: Path, width: int, height: int, pixels: list[int]) -> None:
    row_bytes = width * 4
    data_bytes = row_bytes * height
    file_header = struct.pack("<HIHHI", 0x4D42, 14 + 40 + data_bytes, 0, 0, 14 + 40)
    info_header = struct.pack("<IiiHHIIiiII", 40, width, -height, 1, 32, 0, data_bytes, 2835, 2835, 0, 0)
    rows = bytearray()
    for y in range(height):
        for x in range(width):
            pixel = pixels[y * width + x]
            rows.extend((
                (pixel >> 16) & 0xFF,
                (pixel >> 8) & 0xFF,
                pixel & 0xFF,
                (pixel >> 24) & 0xFF,
            ))
    path.write_bytes(file_header + info_header + bytes(rows))


def _blend(base: int, over: int) -> int:
    br, bg, bb, ba = base & 0xFF, (base >> 8) & 0xFF, (base >> 16) & 0xFF, (base >> 24) & 0xFF
    or_, og, ob, oa = over & 0xFF, (over >> 8) & 0xFF, (over >> 16) & 0xFF, (over >> 24) & 0xFF
    alpha = oa / 255.0
    inv = 1.0 - alpha
    r = int(round(or_ * alpha + br * inv))
    g = int(round(og * alpha + bg * inv))
    b = int(round(ob * alpha + bb * inv))
    a = int(round(oa + ba * inv))
    return r | (g << 8) | (b << 16) | (a << 24)


def _draw_disc(pixels: list[int], width: int, height: int, cx: float, cy: float, radius: float, color: int) -> None:
    radius_sq = radius * radius
    x0 = max(0, int(math.floor(cx - radius)))
    x1 = min(width - 1, int(math.ceil(cx + radius)))
    y0 = max(0, int(math.floor(cy - radius)))
    y1 = min(height - 1, int(math.ceil(cy + radius)))
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            dx = x + 0.5 - cx
            dy = y + 0.5 - cy
            if dx * dx + dy * dy <= radius_sq:
                idx = y * width + x
                pixels[idx] = _blend(pixels[idx], color)


def _draw_segment(pixels: list[int], width: int, height: int, a: tuple[float, float], b: tuple[float, float], radius: float, color: int) -> None:
    ax, ay = a
    bx, by = b
    length = max(abs(bx - ax), abs(by - ay))
    steps = max(1, int(math.ceil(length * 1.5)))
    for i in range(steps + 1):
        t = i / steps
        x = ax + (bx - ax) * t
        y = ay + (by - ay) * t
        _draw_disc(pixels, width, height, x, y, radius, color)


def _pressure_from_trace_entry(entry: dict) -> float:
    ref = entry.get("reference_trace", {})
    saddle = ref.get("saddle", {})
    min_abs = float(saddle.get("min_abs_signed_px", 999.0))
    return math.exp(-min_abs / 18.0)


def _build_trace_artifacts(probe_path: Path, diagnostics_dir: Path) -> dict[str, str]:
    probe = _read_json(probe_path)
    reference_view = probe.get("reference_view", {})
    reference_frame = diagnostics_dir / reference_view.get("frame_bmp", "flashlight_reference_frame.bmp")
    if not reference_frame.exists():
        reference_frame = diagnostics_dir / "frame.bmp"
    width, height, base_pixels = _read_bmp32(reference_frame)
    trace_pixels = [0xFF000000 for _ in range(width * height)]
    overlay_pixels = list(base_pixels)

    points: list[tuple[float, float]] = []
    pressures: list[float] = []
    for entry in probe.get("trace", []):
        ref = entry.get("reference_trace", {})
        render_xy = ref.get("render_xy")
        if not isinstance(render_xy, list) or len(render_xy) != 2:
            continue
        points.append((float(render_xy[0]), float(render_xy[1])))
        pressures.append(_pressure_from_trace_entry(entry))

    for i in range(len(points) - 1):
        a = points[i]
        b = points[i + 1]
        pressure = max(pressures[i], pressures[i + 1])
        radius = 1.5 + 5.0 * pressure
        glow = 0xA0FFFF40
        core = 0xFF60F0FF
        _draw_segment(trace_pixels, width, height, a, b, radius + 1.5, glow)
        _draw_segment(trace_pixels, width, height, a, b, radius, core)
        _draw_segment(overlay_pixels, width, height, a, b, radius + 1.5, glow)
        _draw_segment(overlay_pixels, width, height, a, b, radius, core)

    for point, pressure in zip(points, pressures):
        radius = 2.0 + 4.0 * pressure
        head = 0xFFFFE880
        _draw_disc(trace_pixels, width, height, point[0], point[1], radius, head)
        _draw_disc(overlay_pixels, width, height, point[0], point[1], radius, head)

    trace_frame_path = diagnostics_dir / "flashlight_trace_frame.bmp"
    trace_overlay_path = diagnostics_dir / "flashlight_trace_overlay.bmp"
    _write_bmp32(trace_frame_path, width, height, trace_pixels)
    _write_bmp32(trace_overlay_path, width, height, overlay_pixels)

    stl_path = diagnostics_dir / "flashlight_trace.stl"
    _write_trace_stl(stl_path, points, pressures)
    obj_path = stl_path.with_suffix(".obj")
    csv_path = stl_path.with_suffix(".csv")
    _write_trace_obj(obj_path, points, pressures)
    _write_trace_csv(csv_path, points, pressures)

    return {
        "reference_frame_bmp": str(reference_frame),
        "trace_frame_bmp": str(trace_frame_path),
        "trace_overlay_bmp": str(trace_overlay_path),
        "trace_stl": str(stl_path),
        "trace_obj": str(obj_path),
        "trace_csv": str(csv_path),
    }


def _write_trace_stl(path: Path, points: list[tuple[float, float]], pressures: list[float]) -> None:
    triangles = _build_trace_triangles(points, pressures)
    header = b"flashlight_trace".ljust(80, b"\0")
    payload = bytearray(header)
    payload.extend(struct.pack("<I", len(triangles)))
    for tri in triangles:
        ax, ay, az = tri[0]
        bx, by, bz = tri[1]
        cx, cy, cz = tri[2]
        payload.extend(struct.pack("<fff", 0.0, 0.0, 1.0))
        payload.extend(struct.pack("<fff", ax, ay, az))
        payload.extend(struct.pack("<fff", bx, by, bz))
        payload.extend(struct.pack("<fff", cx, cy, cz))
        payload.extend(struct.pack("<H", 0))
    path.write_bytes(bytes(payload))


def _write_trace_obj(path: Path, points: list[tuple[float, float]], pressures: list[float]) -> None:
    triangles = _build_trace_triangles(points, pressures)
    verts: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int]] = []
    for tri in triangles:
        base = len(verts) + 1
        verts.extend(tri)
        faces.append((base, base + 1, base + 2))
    with path.open("w", encoding="utf-8") as handle:
        for v in verts:
            handle.write(f"v {v[0]} {v[1]} {v[2]}\n")
        for f in faces:
            handle.write(f"f {f[0]} {f[1]} {f[2]}\n")


def _write_trace_csv(path: Path, points: list[tuple[float, float]], pressures: list[float]) -> None:
    triangles = _build_trace_triangles(points, pressures)
    with path.open("w", encoding="utf-8") as handle:
        handle.write("type,i,x,y,z\n")
        idx = 1
        for tri in triangles:
            for v in tri:
                handle.write(f"v,{idx},{v[0]},{v[1]},{v[2]}\n")
                idx += 1


def _build_trace_triangles(points: list[tuple[float, float]], pressures: list[float]) -> list[list[tuple[float, float, float]]]:
    if len(points) < 2:
        return []
    left: list[tuple[float, float, float]] = []
    right: list[tuple[float, float, float]] = []
    for i, point in enumerate(points):
        if i == 0:
            dx = points[1][0] - points[0][0]
            dy = points[1][1] - points[0][1]
        elif i == len(points) - 1:
            dx = points[i][0] - points[i - 1][0]
            dy = points[i][1] - points[i - 1][1]
        else:
            dx = points[i + 1][0] - points[i - 1][0]
            dy = points[i + 1][1] - points[i - 1][1]
        length = math.hypot(dx, dy) or 1.0
        nx = -dy / length
        ny = dx / length
        width = 1.5 + 4.5 * pressures[i]
        z = 6.0 + 10.0 * (i / max(1, len(points) - 1)) + 2.0 * pressures[i]
        left.append((point[0] + nx * width, point[1] + ny * width, z))
        right.append((point[0] - nx * width, point[1] - ny * width, z))
    triangles: list[list[tuple[float, float, float]]] = []
    for i in range(len(points) - 1):
        triangles.append([left[i], right[i], left[i + 1]])
        triangles.append([right[i], right[i + 1], left[i + 1]])
    return triangles


def run_bridge_request(runtime_dir: Path, request_path: Path, *, write_status: bool = True) -> dict:
    request = _read_json(request_path)
    if request.get("kind") != "flashlight_probe_headless":
        raise RuntimeError(f"unsupported request kind: {request.get('kind')!r}")

    exe_path = _active_runtime_exe(runtime_dir)
    seed_path = Path(request["seed_path"])
    ticks = int(request.get("ticks", 8))
    warp = float(request.get("warp", 0.0))
    closure_last = bool(request.get("closure_last", False))
    radius = float(request.get("radius", 0.75))
    zoom_radius = float(request.get("zoom_radius", 0.25))
    diagnostics_dir = runtime_dir / "diagnostics" / "last"
    probe_path = diagnostics_dir / "flashlight_probe.json"

    before_probe = probe_path.stat().st_mtime if probe_path.exists() else 0.0
    time.sleep(1.05)
    command = [
        str(exe_path),
        "--flashlight-probe",
        str(seed_path),
        "--flashlight-ticks",
        str(ticks),
        "--flashlight-radius",
        str(radius),
        "--flashlight-zoom-radius",
        str(zoom_radius),
        "--flashlight-warp",
        str(warp),
    ]
    if closure_last:
        command.append("--flashlight-closure-last")

    result = subprocess.run(
        command,
        cwd=str(runtime_dir),
        text=True,
        capture_output=True,
        check=False,
    )
    status_path = diagnostics_dir / "flashlight_bridge_status.json"

    if result.returncode != 0:
        payload = {
            "ok": False,
            "request_id": request.get("request_id"),
            "error": result.stderr or result.stdout or "flashlight probe execution failed",
        }
        if write_status:
            _write_json(status_path, payload)
        return payload

    if not probe_path.exists() or probe_path.stat().st_mtime <= before_probe:
        payload = {
            "ok": False,
            "request_id": request.get("request_id"),
            "error": "flashlight probe did not regenerate flashlight_probe.json",
        }
        if write_status:
            _write_json(status_path, payload)
        return payload

    artifacts = {}
    if not bool(request.get("no_export", False)):
        artifacts = _build_trace_artifacts(probe_path, diagnostics_dir)

    payload = {
        "ok": True,
        "request_id": request.get("request_id"),
        "seed_path": str(seed_path),
        "probe_json": str(probe_path),
        "artifacts": artifacts,
    }
    if write_status:
        _write_json(status_path, payload)
    return payload


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Process a flashlight bridge request and emit trace artifacts.")
    parser.add_argument("--runtime-dir", type=Path, default=Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime"))
    parser.add_argument("--request-json", type=Path, default=None)
    parser.add_argument("--watch", action="store_true", help="Watch the runtime diagnostics folder for new bridge requests")
    parser.add_argument("--poll-seconds", type=float, default=0.5)
    args = parser.parse_args(argv)

    runtime_dir = args.runtime_dir
    diagnostics_dir = runtime_dir / "diagnostics" / "last"
    request_path = args.request_json or diagnostics_dir / "flashlight_bridge_request.json"

    if args.watch:
        last_request_id = None
        while True:
            if request_path.exists():
                try:
                    request = _read_json(request_path)
                    request_id = request.get("request_id")
                    if request_id != last_request_id:
                        last_request_id = request_id
                        payload = run_bridge_request(runtime_dir, request_path, write_status=True)
                        print(json.dumps(payload, indent=2))
                except Exception as exc:  # pragma: no cover - watch mode convenience
                    error = {"ok": False, "error": str(exc)}
                    _write_json(diagnostics_dir / "flashlight_bridge_status.json", error)
                    print(json.dumps(error, indent=2))
            time.sleep(max(0.1, args.poll_seconds))
    else:
        payload = run_bridge_request(runtime_dir, request_path, write_status=True)
        print(json.dumps(payload, indent=2))
        return 0 if payload.get("ok") else 1


if __name__ == "__main__":
    raise SystemExit(main())
