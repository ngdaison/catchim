// Feature 229 -- mirrors web/src/editor/editor-store.ts
#include "editor/core/EditorAppStoreEngine.h"

namespace catchim::editor {

EditorAppStoreEngine::EditorAppStoreEngine() {
    resetState();
}

void EditorAppStoreEngine::initializeApp() {
    isInitializing_ = false;
    isPanelsReady_ = true;
}

void EditorAppStoreEngine::resetState() {
    isInitializing_ = true;
    isPanelsReady_ = false;
    canvasPresets_ = {
        {"16:9 Landscape", 1920, 1080, "16:9"},
        {"9:16 Portrait", 1080, 1920, "9:16"},
        {"1:1 Square", 1080, 1080, "1:1"},
        {"4:5 Vertical", 1080, 1350, "4:5"},
        {"21:9 Ultrawide", 2560, 1080, "21:9"}
    };
}

} // namespace catchim::editor
