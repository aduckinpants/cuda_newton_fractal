#include "finding_archive_actions.h"

#include <Windows.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::string SanitizeFindingLabel(const std::string& text) {
    std::string cleaned;
    cleaned.reserve(text.size());
    for (char ch : text) {
        const bool isAlphaNum =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9');
        if (isAlphaNum || ch == '_' || ch == '-' || ch == '.') {
            cleaned.push_back(ch);
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            cleaned.push_back('_');
        } else {
            cleaned.push_back('_');
        }
    }
    while (!cleaned.empty() && (cleaned.front() == '_' || cleaned.front() == '-' || cleaned.front() == '.')) cleaned.erase(cleaned.begin());
    while (!cleaned.empty() && (cleaned.back() == '_' || cleaned.back() == '-' || cleaned.back() == '.')) cleaned.pop_back();
    return cleaned.empty() ? "finding" : cleaned;
}

const char* FractalTypeSlug(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::newton: return "newton";
    case FractalType::nova: return "nova";
    case FractalType::mandelbrot: return "mandelbrot";
    case FractalType::julia: return "julia";
    case FractalType::burning_ship: return "burning_ship";
    case FractalType::multibrot: return "multibrot";
    case FractalType::phoenix: return "phoenix";
    case FractalType::explaino: return "explaino";
    case FractalType::explaino_y: return "explaino_y";
    case FractalType::explaino_fp: return "explaino_fp";
    case FractalType::explaino_nova: return "explaino_nova";
    case FractalType::explaino_halley: return "explaino_halley";
    case FractalType::explaino_dual: return "explaino_dual";
    case FractalType::explaino_mult: return "explaino_mult";
    case FractalType::explaino_phoenix: return "explaino_phoenix";
    case FractalType::explaino_transcendental: return "explaino_transcendental";
    case FractalType::explaino_inertial: return "explaino_inertial";
    case FractalType::explaino_julia: return "explaino_julia";
    case FractalType::explaino_rational: return "explaino_rational";
    case FractalType::multicorn: return "multicorn";
    case FractalType::halley: return "halley";
    case FractalType::collatz: return "collatz";
    case FractalType::explaino_collatz: return "explaino_collatz";
    case FractalType::mcmullen: return "mcmullen";
    case FractalType::lambda_map: return "lambda";
    case FractalType::explaino_lambda: return "explaino_lambda";
    case FractalType::explaino_rational_escape: return "explaino_rational_escape";
    }
    return "unknown";
}

std::filesystem::path RuntimeRepoRootMetadataPath(const std::filesystem::path& runtimeDir) {
    return runtimeDir / "fractal_ui_repo_root.txt";
}

std::wstring WidenAscii(const std::string& text) {
    return std::wstring(text.begin(), text.end());
}

std::wstring WidenUtf8(const std::string& text) {
    if (text.empty()) return std::wstring();

    const int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (needed <= 0) return WidenAscii(text);

    std::wstring wide(static_cast<size_t>(needed) - 1u, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), needed);
    return wide;
}

std::string TrimWhitespace(const std::string& text) {
    size_t start = 0;
    while (start < text.size()) {
        const char ch = text[start];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
        ++start;
    }

    size_t end = text.size();
    while (end > start) {
        const char ch = text[end - 1];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
        --end;
    }
    return text.substr(start, end - start);
}

bool NeedsCommandLineQuotes(const std::wstring& text) {
    if (text.empty()) return true;
    for (wchar_t ch : text) {
        if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\v' || ch == L'"') return true;
    }
    return false;
}

void AppendCommandLineArg(std::wstring* commandLine, const std::wstring& arg) {
    if (!commandLine) return;
    if (!commandLine->empty()) commandLine->push_back(L' ');

    if (!NeedsCommandLineQuotes(arg)) {
        commandLine->append(arg);
        return;
    }

    commandLine->push_back(L'"');
    size_t backslashCount = 0;
    for (wchar_t ch : arg) {
        if (ch == L'\\') {
            ++backslashCount;
            continue;
        }
        if (ch == L'"') {
            commandLine->append(backslashCount * 2 + 1, L'\\');
            commandLine->push_back(L'"');
            backslashCount = 0;
            continue;
        }

        commandLine->append(backslashCount, L'\\');
        backslashCount = 0;
        commandLine->push_back(ch);
    }

    commandLine->append(backslashCount * 2, L'\\');
    commandLine->push_back(L'"');
}

bool PathExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) && !ec;
}

std::string FormatWin32Error(DWORD errorCode) {
    LPSTR message = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageA(flags, nullptr, errorCode, 0, reinterpret_cast<LPSTR>(&message), 0, nullptr);
    std::string text;
    if (length != 0 && message) {
        text.assign(message, message + length);
        LocalFree(message);
    }
    text = TrimWhitespace(text);
    if (text.empty()) text = "Win32 error " + std::to_string(errorCode);
    return text;
}

std::string ReadHandleOutput(HANDLE handle) {
    std::string output;
    if (!handle || handle == INVALID_HANDLE_VALUE) return output;

    LARGE_INTEGER begin{};
    begin.QuadPart = 0;
    SetFilePointerEx(handle, begin, nullptr, FILE_BEGIN);

    char buffer[4096] = {};
    DWORD bytesRead = 0;
    while (ReadFile(handle, buffer, static_cast<DWORD>(sizeof(buffer)), &bytesRead, nullptr) && bytesRead != 0) {
        output.append(buffer, buffer + bytesRead);
    }
    return output;
}

std::filesystem::path CreateArchiveOutputLogFile(const SECURITY_ATTRIBUTES& sa, HANDLE* outHandle, std::string* outError) {
    if (outHandle) *outHandle = INVALID_HANDLE_VALUE;

    wchar_t tempDir[MAX_PATH] = {};
    const DWORD tempDirLen = GetTempPathW(MAX_PATH, tempDir);
    if (tempDirLen == 0 || tempDirLen >= MAX_PATH) {
        if (outError) *outError = "GetTempPathW failed for finding archive capture: " + FormatWin32Error(GetLastError());
        return {};
    }

    wchar_t tempFile[MAX_PATH] = {};
    if (!GetTempFileNameW(tempDir, L"far", 0, tempFile)) {
        if (outError) *outError = "GetTempFileNameW failed for finding archive capture: " + FormatWin32Error(GetLastError());
        return {};
    }

    HANDLE logHandle = CreateFileW(
        tempFile,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        const_cast<SECURITY_ATTRIBUTES*>(&sa),
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr);
    if (logHandle == INVALID_HANDLE_VALUE) {
        const std::string error = "CreateFileW failed for finding archive capture log: " + FormatWin32Error(GetLastError());
        DeleteFileW(tempFile);
        if (outError) *outError = error;
        return {};
    }

    if (outHandle) *outHandle = logHandle;
    return std::filesystem::path(tempFile);
}

bool ResolvePythonLauncherPath(std::filesystem::path* outPath, std::string* outError) {
    if (!outPath) return false;

    wchar_t windowsDir[MAX_PATH] = {};
    const UINT windowsDirLen = GetWindowsDirectoryW(windowsDir, MAX_PATH);
    if (windowsDirLen > 0 && windowsDirLen < MAX_PATH) {
        const std::filesystem::path defaultPy = std::filesystem::path(windowsDir) / "py.exe";
        if (PathExists(defaultPy)) {
            *outPath = defaultPy;
            return true;
        }
    }

    const DWORD required = SearchPathW(nullptr, L"py.exe", nullptr, 0, nullptr, nullptr);
    if (required > 0) {
        std::wstring resolved(static_cast<size_t>(required), L'\0');
        const DWORD actual = SearchPathW(nullptr, L"py.exe", nullptr, required, resolved.data(), nullptr);
        if (actual > 0 && actual < required) {
            resolved.resize(actual);
            *outPath = std::filesystem::path(resolved);
            return true;
        }
    }

    if (outError) *outError = "Could not resolve py.exe for finding archive capture.";
    return false;
}

std::filesystem::path ArchiveScriptRelativePath() {
    return std::filesystem::path("tools") / "reality_toolkit" / "scripts" / "run_fractal_explorer_archive_finding.py";
}

std::filesystem::path RepoRootFromSourceFile() {
    return std::filesystem::path(__FILE__).lexically_normal().parent_path().parent_path().parent_path();
}

std::filesystem::path ResolveRepoRoot(const std::filesystem::path& runtimeDir) {
    const std::filesystem::path metadataResolved = FindRepoRootFromRuntimeMetadata(runtimeDir);
    if (!metadataResolved.empty()) return metadataResolved;

    const std::filesystem::path sourceRoot = RepoRootFromSourceFile();
    const std::filesystem::path sourceResolved = FindRepoRootContainingArchiveScript(sourceRoot);
    if (!sourceResolved.empty()) return sourceResolved;

    std::error_code cwdError;
    const std::filesystem::path cwd = std::filesystem::current_path(cwdError);
    if (!cwdError) {
        const std::filesystem::path cwdResolved = FindRepoRootContainingArchiveScript(cwd);
        if (!cwdResolved.empty()) return cwdResolved;
    }

    return {};
}

std::filesystem::path PublishRootFromExeDir(const std::string& exeDir) {
    return std::filesystem::path(exeDir).lexically_normal().parent_path();
}

std::string DateLabelNow() {
    SYSTEMTIME now{};
    GetLocalTime(&now);
    char buffer[32] = {};
    std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u", now.wYear, now.wMonth, now.wDay);
    return buffer;
}

std::string TimeLabelNow() {
    SYSTEMTIME now{};
    GetLocalTime(&now);
    char buffer[32] = {};
    std::snprintf(buffer, sizeof(buffer), "%02u%02u%02u_%03u", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    return buffer;
}

bool RunArchiveScript(
    const std::filesystem::path& repoRoot,
    const std::filesystem::path& diagnosticsDir,
    const std::filesystem::path& outRoot,
    const std::string& findingId,
    const std::string& why,
    const std::string& reproCommand,
    std::string* outError) {
    const std::filesystem::path resolvedRepoRoot = repoRoot.empty() ? ResolveRepoRoot({}) : repoRoot;
    if (resolvedRepoRoot.empty()) {
        if (outError) *outError = "Could not resolve an absolute repository root for finding archive capture.";
        return false;
    }

    const std::filesystem::path scriptPath = resolvedRepoRoot / ArchiveScriptRelativePath();
    if (!PathExists(scriptPath)) {
        if (outError) *outError = "Archive finding script missing: " + scriptPath.string();
        return false;
    }

    std::filesystem::path pythonLauncher;
    if (!ResolvePythonLauncherPath(&pythonLauncher, outError)) {
        return false;
    }

    const std::wstring commandLine = BuildArchiveScriptCommandLine(
        pythonLauncher,
        scriptPath,
        resolvedRepoRoot,
        diagnosticsDir,
        outRoot,
        findingId,
        why,
        reproCommand);

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE logHandle = INVALID_HANDLE_VALUE;
    std::string logError;
    const std::filesystem::path logPath = CreateArchiveOutputLogFile(sa, &logHandle, &logError);
    if (logHandle == INVALID_HANDLE_VALUE) {
        if (outError) *outError = logError;
        return false;
    }

    HANDLE nullInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (nullInput == INVALID_HANDLE_VALUE) {
        const std::string error = "Failed to open NUL stdin for finding archive capture: " + FormatWin32Error(GetLastError());
        CloseHandle(logHandle);
        DeleteFileW(logPath.c_str());
        if (outError) *outError = error;
        return false;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = nullInput;
    startup.hStdOutput = logHandle;
    startup.hStdError = logHandle;

    PROCESS_INFORMATION process{};
    std::wstring mutableCommandLine = commandLine;
    std::wstring pythonLauncherPath = pythonLauncher.wstring();
    std::wstring workingDirectory = resolvedRepoRoot.wstring();
    const BOOL created = CreateProcessW(
        pythonLauncherPath.c_str(),
        mutableCommandLine.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
        &startup,
        &process);

    CloseHandle(nullInput);

    if (!created) {
        const std::string error = "CreateProcessW failed for finding archive capture: " + FormatWin32Error(GetLastError());
        CloseHandle(logHandle);
        DeleteFileW(logPath.c_str());
        if (outError) *outError = error;
        return false;
    }

    WaitForSingleObject(process.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    const std::string childOutput = TrimWhitespace(ReadHandleOutput(logHandle));

    CloseHandle(logHandle);
    DeleteFileW(logPath.c_str());
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    if (exitCode != 0) {
        if (outError) {
            *outError = "Archive finding script failed with exit code " + std::to_string(exitCode);
            if (!childOutput.empty()) *outError += ": " + childOutput;
        }
        return false;
    }

    return true;
}

} // namespace

std::filesystem::path FindRepoRootContainingArchiveScript(const std::filesystem::path& startPath) {
    if (startPath.empty()) return {};

    std::filesystem::path current = startPath.lexically_normal();
    if (!current.is_absolute()) return {};

    const std::filesystem::path relativeScriptPath = ArchiveScriptRelativePath();
    for (;;) {
        if (PathExists(current / relativeScriptPath)) return current;

        const std::filesystem::path parent = current.parent_path();
        if (parent.empty() || parent == current) break;
        current = parent;
    }
    return {};
}

std::filesystem::path FindRepoRootFromRuntimeMetadata(const std::filesystem::path& runtimeDir) {
    if (runtimeDir.empty()) return {};

    const std::filesystem::path metadataPath = RuntimeRepoRootMetadataPath(runtimeDir.lexically_normal());
    std::ifstream file(metadataPath, std::ios::in | std::ios::binary);
    if (!file) return {};

    std::string text;
    std::getline(file, text);
    const std::string trimmed = TrimWhitespace(text);
    if (trimmed.empty()) return {};

    const std::filesystem::path candidate = std::filesystem::path(trimmed).lexically_normal();
    if (!candidate.is_absolute()) return {};
    if (!PathExists(candidate / ArchiveScriptRelativePath())) return {};
    return candidate;
}

std::wstring BuildArchiveScriptCommandLine(
    const std::filesystem::path& pythonLauncher,
    const std::filesystem::path& scriptPath,
    const std::filesystem::path& repoRoot,
    const std::filesystem::path& diagnosticsDir,
    const std::filesystem::path& outRoot,
    const std::string& findingId,
    const std::string& why,
    const std::string& reproCommand) {
    std::wstring commandLine;
    AppendCommandLineArg(&commandLine, pythonLauncher.wstring());
    AppendCommandLineArg(&commandLine, L"-3.14");
    AppendCommandLineArg(&commandLine, scriptPath.wstring());
    AppendCommandLineArg(&commandLine, L"--repo-root");
    AppendCommandLineArg(&commandLine, repoRoot.wstring());
    AppendCommandLineArg(&commandLine, L"--diagnostics-dir");
    AppendCommandLineArg(&commandLine, diagnosticsDir.wstring());
    AppendCommandLineArg(&commandLine, L"--out-root");
    AppendCommandLineArg(&commandLine, outRoot.wstring());
    AppendCommandLineArg(&commandLine, L"--finding-id");
    AppendCommandLineArg(&commandLine, WidenUtf8(findingId));
    AppendCommandLineArg(&commandLine, L"--why");
    AppendCommandLineArg(&commandLine, WidenUtf8(why));
    AppendCommandLineArg(&commandLine, L"--repro-command");
    AppendCommandLineArg(&commandLine, WidenUtf8(reproCommand));
    return commandLine;
}

FindingArchiveIdentity BuildUniqueFindingIdentity(
    const std::filesystem::path& findingsRoot,
    const std::string& group,
    const std::string& dateLabel,
    const std::string& timeLabel,
    FractalType fractalType) {
    const std::string safeGroup = SanitizeFindingLabel(group.empty() ? "manual_capture" : group);
    const std::string baseId = SanitizeFindingLabel(timeLabel) + "__" + FractalTypeSlug(fractalType);

    FindingArchiveIdentity result;
    result.out_root = findingsRoot / safeGroup / SanitizeFindingLabel(dateLabel);

    std::string candidateId = baseId;
    std::filesystem::path candidateDir = result.out_root / candidateId;
    int suffix = 2;
    while (std::filesystem::exists(candidateDir)) {
        char buffer[16] = {};
        std::snprintf(buffer, sizeof(buffer), "__%02d", suffix);
        candidateId = baseId + buffer;
        candidateDir = result.out_root / candidateId;
        ++suffix;
    }

    result.finding_id = candidateId;
    result.output_dir = candidateDir;
    return result;
}

bool CaptureAndArchiveFindingBundle(
    const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const std::string& group,
    const std::string& why,
    std::string* outFindingDir,
    std::string* outError) {
    if (outError) outError->clear();

    DiagnosticsCaptureResult capture;
    std::string captureError;
    if (!CaptureDiagnosticsLastBundle(exeDir, view, params, render, stats, rgba, rgbaPixelCount, &capture, &captureError)) {
        if (outError) *outError = captureError;
        return false;
    }

    const std::filesystem::path findingsRoot = PublishRootFromExeDir(exeDir) / "findings";
    const FindingArchiveIdentity identity = BuildUniqueFindingIdentity(findingsRoot, group, DateLabelNow(), TimeLabelNow(), view.fractal_type);
    std::error_code mkdirError;
    std::filesystem::create_directories(identity.out_root, mkdirError);
    if (mkdirError) {
        if (outError) *outError = "Failed to create finding output root: " + identity.out_root.string();
        return false;
    }

    const std::filesystem::path launcherPath = std::filesystem::path(exeDir) / "fractal_ui.cmd";
    const std::filesystem::path publishedExePath = std::filesystem::path(exeDir) / "fractal_ui.exe";
    const std::filesystem::path reproRuntimePath = PathExists(launcherPath) ? launcherPath : publishedExePath;
    const std::string reproCommand = reproRuntimePath.string() + " --load-state-json " + (identity.output_dir / "state.json").string() + " --capture-diagnostic";
    const std::filesystem::path repoRoot = ResolveRepoRoot(std::filesystem::path(exeDir));

    if (!RunArchiveScript(repoRoot, capture.output_dir, identity.out_root, identity.finding_id, why, reproCommand, outError)) {
        return false;
    }

    if (outFindingDir) *outFindingDir = identity.output_dir.string();
    return true;
}