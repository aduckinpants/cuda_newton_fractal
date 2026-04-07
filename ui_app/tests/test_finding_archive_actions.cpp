#include "../src/finding_archive_actions.h"

#include <Windows.h>

#include <shellapi.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")

namespace {

std::wstring WideAscii(const char* text) {
    std::wstring wide;
    if (!text) return wide;
    while (*text) {
        wide.push_back(static_cast<wchar_t>(*text));
        ++text;
    }
    return wide;
}

std::vector<std::wstring> ParseWindowsCommandLine(const std::wstring& commandLine) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(commandLine.c_str(), &argc);
    if (!argv) return {};

    std::vector<std::wstring> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    LocalFree(argv);
    return args;
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return false;
    std::ostringstream text;
    text << file.rdbuf();
    *outText = text.str();
    return true;
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_finding_archive_tests";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot);

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual_capture",
            "2026-04-05",
            "112233_456",
            FractalType::newton);

        if (identity.out_root != tempRoot / "manual_capture" / "2026-04-05") {
            std::cerr << "Expected dated out_root under manual_capture\n";
            return 1;
        }
        if (identity.finding_id != "112233_456__newton") {
            std::cerr << "Unexpected base finding id\n";
            return 1;
        }
        if (identity.output_dir != identity.out_root / identity.finding_id) {
            std::cerr << "Expected output_dir to match out_root / finding_id\n";
            return 1;
        }
    }

    {
        fs::create_directories(tempRoot / "manual_capture" / "2026-04-05" / "112233_456__newton");
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual_capture",
            "2026-04-05",
            "112233_456",
            FractalType::newton);

        if (identity.finding_id != "112233_456__newton__02") {
            std::cerr << "Expected collision suffix on second finding id\n";
            return 1;
        }
    }

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual sweep!",
            "2026-04-05",
            "235959_999",
            FractalType::explaino_fp);

        if (identity.out_root != tempRoot / "manual_sweep" / "2026-04-05") {
            std::cerr << "Expected sanitized group folder\n";
            return 1;
        }
        if (identity.finding_id != "235959_999__explaino_fp") {
            std::cerr << "Expected fractal type suffix in finding id\n";
            return 1;
        }
    }

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual sweep!",
            "2026-04-05",
            "235959_999",
            FractalType::explaino_nova);

        if (identity.finding_id != "235959_999__explaino_nova") {
            std::cerr << "Expected explaino_nova fractal type suffix in finding id\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_runtime";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::explaino_dual;
        view.explaino_phase_strength = -2.5f;
        view.auto_max_iter = true;
        KernelParams params{};
        params.explaino_seed = -3.0;
        params.explaino_seed_b = -7.5;
        params.explaino_root_spread = 1.75f;
        RenderSettings render{};
        render.resolution = {64, 48};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff336699u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &capture, &error)) {
            std::cerr << "Expected diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"explaino_phase_strength\": -2.5") == std::string::npos ||
            stateJson.find("\"explaino_root_spread\": 1.75") == std::string::npos ||
            stateJson.find("\"auto_max_iter\": true") == std::string::npos ||
            stateJson.find("\"interaction_debounce_ms\": 200") == std::string::npos ||
            stateJson.find("\"preview_target_fps\": 30") == std::string::npos ||
            stateJson.find("\"preview_min_scale\": 0.5") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist Explaino phase strength, root spread, auto_max_iter, and adaptive preview pacing fields\n";
            return 1;
        }

        if (CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size() - 1, &capture, &error)) {
            std::cerr << "Expected diagnostics capture to reject mismatched pixel counts\n";
            return 1;
        }
        if (error.find("pixel count") == std::string::npos) {
            std::cerr << "Expected pixel-count validation error for mismatched capture buffer\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1536, 1024};
        render.block_size = 128;
        render.device_id = 2;
        render.preview_target_fps = 48.0f;

        const RenderSettings captureRender = BuildFindingArchiveCaptureRender(render);
        if (captureRender.resolution.x != 4096 || captureRender.resolution.y != 4096) {
            std::cerr << "Expected finding archive capture to force a 4k square frame\n";
            return 1;
        }
        if (captureRender.block_size != render.block_size || captureRender.device_id != render.device_id ||
            captureRender.preview_target_fps != render.preview_target_fps) {
            std::cerr << "Expected 4k finding archive render to preserve the non-resolution render settings\n";
            return 1;
        }
    }

    {
        const fs::path repoRoot = tempRoot / "repo_root_probe";
        const fs::path scriptPath = repoRoot / "tools" / "reality_toolkit" / "scripts" / "run_fractal_explorer_archive_finding.py";
        fs::create_directories(scriptPath.parent_path());
        std::ofstream scriptFile(scriptPath, std::ios::out | std::ios::binary | std::ios::trunc);
        scriptFile << "# archive stub\n";
        scriptFile.close();

        const fs::path nestedStart = repoRoot / "nested" / "deeper" / "still_deeper";
        fs::create_directories(nestedStart);

        const fs::path resolved = FindRepoRootContainingArchiveScript(nestedStart);
        if (resolved != repoRoot) {
            std::cerr << "Expected repo-root discovery to walk upward until archive script is found\n";
            return 1;
        }
    }

    {
        const fs::path repoRoot = tempRoot / "repo_root_metadata_probe";
        const fs::path scriptPath = repoRoot / "tools" / "reality_toolkit" / "scripts" / "run_fractal_explorer_archive_finding.py";
        fs::create_directories(scriptPath.parent_path());
        std::ofstream scriptFile(scriptPath, std::ios::out | std::ios::binary | std::ios::trunc);
        scriptFile << "# archive stub\n";
        scriptFile.close();

        const fs::path runtimeDir = tempRoot / "runtime_root_probe" / "runtime";
        fs::create_directories(runtimeDir);
        std::ofstream metadataFile(runtimeDir / "fractal_ui_repo_root.txt", std::ios::out | std::ios::binary | std::ios::trunc);
        metadataFile << repoRoot.string() << "\n";
        metadataFile.close();

        const fs::path resolved = FindRepoRootFromRuntimeMetadata(runtimeDir);
        if (resolved != repoRoot) {
            std::cerr << "Expected runtime metadata file to resolve the absolute repo root\n";
            return 1;
        }
    }

    {
        const fs::path pythonLauncher = R"(C:\Windows\py.exe)";
        const fs::path scriptPath = R"(C:\code\cuda newton fractal clone\tools\reality_toolkit\scripts\run_fractal_explorer_archive_finding.py)";
        const fs::path repoRoot = R"(C:\code\cuda newton fractal clone)";
        const fs::path diagnosticsDir = R"(D:\salt fractal\cuda_newton_fractal_clone\runtime\diagnostics\last)";
        const fs::path outRoot = R"(D:\salt fractal\cuda_newton_fractal_clone\findings\manual capture\2026-04-05)";
        const std::string findingId = "235959_999__explaino_fp";
        const std::string why = "Slice \"smoke\" capture.";
        const std::string reproCommand =
            R"(D:\salt fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json D:\salt fractal\cuda_newton_fractal_clone\findings\manual capture\2026-04-05\235959_999__explaino_fp\state.json --capture-diagnostic)";

        const std::wstring commandLine = BuildArchiveScriptCommandLine(
            pythonLauncher,
            scriptPath,
            repoRoot,
            diagnosticsDir,
            outRoot,
            findingId,
            why,
            reproCommand);
        const std::vector<std::wstring> argv = ParseWindowsCommandLine(commandLine);

        const std::vector<std::wstring> expected = {
            pythonLauncher.wstring(),
            L"-3.14",
            scriptPath.wstring(),
            L"--repo-root",
            repoRoot.wstring(),
            L"--diagnostics-dir",
            diagnosticsDir.wstring(),
            L"--out-root",
            outRoot.wstring(),
            L"--finding-id",
            WideAscii(findingId.c_str()),
            L"--why",
            WideAscii(why.c_str()),
            L"--repro-command",
            WideAscii(reproCommand.c_str()),
        };

        if (argv != expected) {
            std::cerr << "Expected archive script command line to round-trip through CommandLineToArgvW\n";
            return 1;
        }
    }

    std::cout << "test_finding_archive_actions: all passed\n";
    return 0;
}