from __future__ import annotations

import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_append_handoff import (
    MAINLINE_HANDOFF_APPEND,
    build_handoff_append_commands,
    capture_repo_status,
    format_repo_status_summary,
    resolve_handoff_checkpoint_token,
)
from tools.viewer_host_repo_status import repo_is_dirty


def test_build_handoff_append_commands_resolves_then_appends_when_message_present() -> None:
    commands = build_handoff_append_commands(
        py="py",
        message="checkpoint message",
        commit="abc1234",
        resolve_last_pending=True,
        score=97,
    )

    assert len(commands) == 2
    assert commands[0] == [
        "py",
        str(MAINLINE_HANDOFF_APPEND),
        "--repo-root",
        str(REPO_ROOT),
        "--commit",
        "abc1234",
        "--resolve-last-pending",
    ]
    assert commands[1] == [
        "py",
        str(MAINLINE_HANDOFF_APPEND),
        "--repo-root",
        str(REPO_ROOT),
        "--commit",
        "abc1234",
        "--score",
        "97",
        "checkpoint message",
    ]


def test_build_handoff_append_commands_uses_single_append_without_resolve() -> None:
    commands = build_handoff_append_commands(
        py="py",
        message="checkpoint message",
        commit="abc1234",
        resolve_last_pending=False,
        no_auto_rotate=True,
    )

    assert commands == [[
        "py",
        str(MAINLINE_HANDOFF_APPEND),
        "--repo-root",
        str(REPO_ROOT),
        "--commit",
        "abc1234",
        "--no-auto-rotate",
        "checkpoint message",
    ]]


def test_resolve_handoff_checkpoint_token_generates_checkpoint_id_for_message_without_commit(monkeypatch) -> None:
    monkeypatch.setattr("tools.viewer_host_append_handoff.uuid.uuid4", lambda: type("_Uuid", (), {"hex": "flow1234abcdef"})())

    token, generated = resolve_handoff_checkpoint_token(commit=None, message="checkpoint message")

    assert token == "ck:flow1234"
    assert generated is True


def test_build_handoff_append_commands_reuses_generated_checkpoint_id_for_resolve_and_append(monkeypatch) -> None:
    monkeypatch.setattr("tools.viewer_host_append_handoff.uuid.uuid4", lambda: type("_Uuid", (), {"hex": "flow1234abcdef"})())

    commands = build_handoff_append_commands(
        py="py",
        message="checkpoint message",
        commit=None,
        resolve_last_pending=True,
        score=97,
    )

    assert commands == [
        [
            "py",
            str(MAINLINE_HANDOFF_APPEND),
            "--repo-root",
            str(REPO_ROOT),
            "--commit",
            "ck:flow1234",
            "--resolve-last-pending",
        ],
        [
            "py",
            str(MAINLINE_HANDOFF_APPEND),
            "--repo-root",
            str(REPO_ROOT),
            "--commit",
            "ck:flow1234",
            "--score",
            "97",
            "checkpoint message",
        ],
    ]


def _init_git_repo(repo_root: Path) -> None:
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)


def test_capture_repo_status_distinguishes_staged_unstaged_and_untracked(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _init_git_repo(repo_root)

    tracked = repo_root / "tracked.txt"
    tracked.write_text("base\n", encoding="utf-8")
    subprocess.run(["git", "add", "tracked.txt"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    tracked.write_text("dirty\n", encoding="utf-8")
    staged = repo_root / "staged.txt"
    staged.write_text("stage me\n", encoding="utf-8")
    subprocess.run(["git", "add", "staged.txt"], cwd=repo_root, check=True, capture_output=True, text=True)
    untracked = repo_root / "untracked.txt"
    untracked.write_text("leave me\n", encoding="utf-8")

    status = capture_repo_status(repo_root)

    assert status == {
        "staged": ["staged.txt"],
        "unstaged": ["tracked.txt"],
        "untracked": ["untracked.txt"],
    }
    assert repo_is_dirty(repo_root) is True


def test_format_repo_status_summary_reports_every_section() -> None:
    summary = format_repo_status_summary(
        {
            "staged": ["HANDOFF_LOG.md", "docs/handoffs/archive/HANDOFF_LOG_20260414_185521.md"],
            "unstaged": [],
            "untracked": ["artifacts/tmp.txt"],
        },
        prefix="viewer_host_append_handoff",
    )

    assert summary == (
        "viewer_host_append_handoff: repo-status "
        "staged=HANDOFF_LOG.md, docs/handoffs/archive/HANDOFF_LOG_20260414_185521.md "
        "| unstaged=none | untracked=artifacts/tmp.txt"
    )