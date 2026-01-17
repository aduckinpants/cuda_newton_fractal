from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


def _run(cmd: list[str]) -> tuple[int, str]:
    try:
        p = subprocess.run(cmd, capture_output=True, text=True)
        out = (p.stdout or "") + (p.stderr or "")
        return p.returncode, out.strip()
    except FileNotFoundError:
        return 127, ""


def _ok(name: str, details: str = "") -> None:
    print(f"[OK]   {name} {details}".rstrip())


def _fail(name: str, details: str = "", hint: str = "") -> None:
    print(f"[FAIL] {name} {details}".rstrip())
    if hint:
        print(f"       Hint: {hint}")


def _exists(path: str | Path) -> bool:
    return Path(path).exists()


def main() -> int:
    print("CUDA Newton Fractal doctor (Python)")
    print(f"Python: {sys.version.split()[0]}")
    print(f"Folder: {Path(__file__).resolve().parent}")
    print("")

    nvcc = shutil.which("nvcc")
    if nvcc:
        _ok("nvcc on PATH", nvcc)
        code, ver = _run(["nvcc", "--version"])
        if code == 0 and ver:
            for line in ver.splitlines()[-4:]:
                print(f"       {line}")
    else:
        _fail("nvcc on PATH", hint="Install CUDA Toolkit and ensure nvcc.exe is on PATH.")

    print("")
    cl = shutil.which("cl")
    if cl:
        _ok("cl.exe on PATH", cl)
    else:
        _fail(
            "cl.exe on PATH",
            hint=(
                "Install Visual Studio/Build Tools with 'Desktop development with C++' and a Windows 10/11 SDK, "
                "then run builds from a 'x64 Native Tools Command Prompt for VS' or 'Developer PowerShell for VS'."
            ),
        )

    print("")
    vs_root = Path(r"C:\Program Files\Microsoft Visual Studio\18\Community")
    if vs_root.exists():
        _ok("VS folder exists", str(vs_root))
    else:
        _fail("VS folder exists", str(vs_root), hint="If you use a different VS version/edition, install it (or Build Tools).")

    vsdevcmd = vs_root / "Common7" / "Tools" / "VsDevCmd.bat"
    if vsdevcmd.exists():
        _ok("VsDevCmd.bat present", str(vsdevcmd))
    else:
        _fail("VsDevCmd.bat present", str(vsdevcmd))

    vcvarsall = vs_root / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
    if vcvarsall.exists():
        _ok("vcvarsall.bat present", str(vcvarsall))
    else:
        _fail(
            "vcvarsall.bat present",
            str(vcvarsall),
            hint=(
                "Your VS install appears incomplete for C++ builds. In Visual Studio Installer, add 'Desktop development with C++'."
            ),
        )

    msvc_tools = vs_root / "VC" / "Tools" / "MSVC"
    if msvc_tools.exists():
        _ok("MSVC toolset root", str(msvc_tools))
        versions = sorted([p for p in msvc_tools.iterdir() if p.is_dir()], reverse=True)
        if versions:
            latest = versions[0]
            _ok("MSVC toolset version", str(latest))
            include_dir = latest / "include"
            lib_x64_dir = latest / "lib" / "x64"
            if include_dir.exists():
                _ok("MSVC include dir", str(include_dir))
            else:
                _fail("MSVC include dir", str(include_dir), hint="MSVC headers missing; install MSVC toolset components.")
            if lib_x64_dir.exists():
                _ok("MSVC x64 libs", str(lib_x64_dir))
            else:
                _fail("MSVC x64 libs", str(lib_x64_dir), hint="MSVC libs missing; install MSVC toolset components.")
        else:
            _fail("MSVC toolset version", hint="No MSVC toolset directories found; install MSVC v14x toolset.")
    else:
        _fail("MSVC toolset root", str(msvc_tools), hint="Install MSVC toolset via Visual Studio Installer.")

    print("")
    kits10 = Path(r"C:\Program Files (x86)\Windows Kits\10\Include")
    if kits10.exists():
        _ok("Windows SDK include root", str(kits10))
        sdk_versions = sorted([p.name for p in kits10.iterdir() if p.is_dir()], reverse=True)
        if sdk_versions:
            _ok("Windows SDK versions", ", ".join(sdk_versions[:3]))
    else:
        _fail(
            "Windows SDK include root",
            str(kits10),
            hint="Install a Windows 10/11 SDK via Visual Studio Installer (Individual components).",
        )

    print("")
    print("Next steps:")
    print("  - If cl/vcvarsall/Windows SDK checks failed: open 'Visual Studio Installer' -> Modify")
    print("    and select 'Desktop development with C++' plus a Windows 10/11 SDK component.")
    print("  - After install, open 'Developer PowerShell for VS' (x64) and run: .\\build.ps1")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
