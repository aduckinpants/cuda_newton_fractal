# CUDA Newton Fractal (Artifacts-Only)

This is a minimal CUDA smoke-test that renders a Newton fractal to a `.ppm` image.

- Output format: binary PPM (`P6`) so there are no extra dependencies.
- Workspace hygiene: this project intentionally lives under `C:\artifacts`.

## Prerequisites (Windows)

`nvcc` needs a host C++ toolchain on Windows.

- Install Visual Studio (or Build Tools) with **Desktop development with C++**
- Ensure a **Windows 10/11 SDK** component is installed

Note: this machine uses Visual Studio 2026 (v18). CUDA 13.1 will warn about an unsupported host compiler; the build scripts include `-allow-unsupported-compiler` for the smoke-test.

## Diagnostics

```powershell
py .\doctor.py
# or (after enabling script execution)
.\doctor.ps1
```

## Build

Recommended (works even if your current PowerShell does not have `cl.exe` on PATH):

```powershell
cmd /s /c .\build_vsdevcmd.cmd
```

If you are already in **Developer PowerShell for VS (x64)**, you can also run:

```powershell
.\build.ps1
```

## Run

```powershell
.\newton_fractal.exe --width 1024 --height 1024 --out newton.ppm
```

Optional knobs:

- `--max-iter 80`
- `--eps 1e-5`
- `--xmin -2 --xmax 2 --ymin -2 --ymax 2`

## View the output

Windows cant always preview `.ppm` directly. Easy options:

- Open in an image tool that supports PPM, or
- Convert to PNG using ImageMagick if installed:

```powershell
magick newton.ppm newton.png
```
