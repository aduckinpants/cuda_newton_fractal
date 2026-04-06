#include "../src/viewer_shutdown.h"

#include <iostream>

int main() {
    bool quitRequested = false;
    MSG msg{};
    msg.message = WM_NULL;
    if (NoteQuitMessage(msg, &quitRequested)) {
        std::cerr << "WM_NULL should not be treated as a quit message\n";
        return 1;
    }
    if (ShouldExitUiLoop(quitRequested)) {
        std::cerr << "WM_NULL should not exit the UI loop\n";
        return 1;
    }

    msg.message = WM_CLOSE;
    if (NoteQuitMessage(msg, &quitRequested)) {
        std::cerr << "WM_CLOSE should not be treated as a quit message\n";
        return 1;
    }
    if (ShouldExitUiLoop(quitRequested)) {
        std::cerr << "WM_CLOSE should not exit the UI loop before WM_QUIT is posted\n";
        return 1;
    }

    msg.message = WM_QUIT;
    if (!NoteQuitMessage(msg, &quitRequested)) {
        std::cerr << "WM_QUIT must be recognized as a quit message\n";
        return 1;
    }
    if (!ShouldExitUiLoop(quitRequested)) {
        std::cerr << "WM_QUIT must stop the UI loop before rendering another frame\n";
        return 1;
    }

    msg.message = WM_NULL;
    if (NoteQuitMessage(msg, &quitRequested)) {
        std::cerr << "Later WM_NULL should not clear or re-trigger quit\n";
        return 1;
    }
    if (!ShouldExitUiLoop(quitRequested)) {
        std::cerr << "Later messages must not overwrite a latched quit request\n";
        return 1;
    }

    std::cout << "test_viewer_shutdown: all passed\n";
    return 0;
}