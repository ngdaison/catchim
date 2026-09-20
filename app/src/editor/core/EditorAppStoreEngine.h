#pragma once
// Feature 229 -- mirrors web/src/editor/editor-store.ts
#include <string>
#include <vector>

namespace catchim::editor {

struct CanvasPresetSpec {
    std::string name;
    int width{1920};
    int height{1080};
    std::string aspectRatio{"16:9"};
};

class EditorAppStoreEngine {
public:
    EditorAppStoreEngine();

    bool isInitializing() const noexcept { return isInitializing_; }
    bool isPanelsReady() const noexcept { return isPanelsReady_; }

    void setInitializing(bool loading) noexcept { isInitializing_ = loading; }
    void setPanelsReady(bool ready) noexcept { isPanelsReady_ = ready; }

    void initializeApp();
    void resetState();

    const std::vector<CanvasPresetSpec>& getCanvasPresets() const noexcept { return canvasPresets_; }
    void setCanvasPresets(std::vector<CanvasPresetSpec> presets) { canvasPresets_ = std::move(presets); }

private:
    bool isInitializing_{true};
    bool isPanelsReady_{false};
    std::vector<CanvasPresetSpec> canvasPresets_;
};

} // namespace catchim::editor
