#pragma once

#include <Windows.h>

bool NoteQuitMessage(const MSG& msg, bool* ioQuitRequested);
bool ShouldExitUiLoop(bool quitRequested);