from __future__ import annotations

import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_append_handoff import MAINLINE_HANDOFF_APPEND, build_handoff_append_commands


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