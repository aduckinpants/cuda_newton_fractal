# CUDA Newton Fractal

This is a minimal CUDA smoke-test that renders a Newton fractal to a `.ppm` image.

- Output format: binary PPM (`P6`) so there are no extra dependencies.
- Runtime artifacts now publish under `D:\salt-fractal\cuda_newton_fractal_clone\...` by default.

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

Default smoke binary location after build:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton_fractal.exe
```

## Run

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton_fractal.exe --width 1024 --height 1024
```

Optional knobs:

- `--max-iter 80`
- `--eps 1e-5`
- `--xmin -2 --xmax 2 --ymin -2 --ymax 2`
- `--out D:\salt-fractal\cuda_newton_fractal_clone\smoke\custom_name.ppm`

Default output path when `--out` is omitted:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton.ppm
```

## Publish existing repo-local artifacts

To move older in-repo build outputs, diagnostics, and sample renders into the publish tree:

```powershell
pwsh -File .\tools\publish_repo_artifacts.ps1 -Label cleanup
```

## View the output

Windows cant always preview `.ppm` directly. Easy options:

- Open in an image tool that supports PPM, or
- Convert to PNG using ImageMagick if installed:

```powershell
magick D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton.ppm D:\salt-fractal\cuda_newton_fractal_clone\smoke\newton.png
```
