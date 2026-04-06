#include "viewer_shutdown.h"

bool NoteQuitMessage(const MSG& msg, bool* ioQuitRequested) {
    if (msg.message != WM_QUIT) return false;
    if (ioQuitRequested) *ioQuitRequested = true;
    return true;
}

bool ShouldExitUiLoop(bool quitRequested) {
    return quitRequested;
}