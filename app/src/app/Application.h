#pragma once

#include "AppConfig.h"
#include "editor/EditorEngine.h"
#include "media/MediaLibrary.h"
#include "render/RenderEngine.h"
#include "audio/AudioEngine.h"
#include <memory>

namespace catchim::app {

class Application {
public:
    Application(int argc, char** argv);
    ~Application();

    int run();

    editor::EditorEngine& editor() noexcept { return *editorEngine_; }
    media::MediaLibrary& media() noexcept { return *mediaLibrary_; }
    render::RenderEngine& render() noexcept { return *renderEngine_; }
    audio::AudioEngine& audio() noexcept { return *audioEngine_; }

private:
    void initSubsystems();
    void shutdownSubsystems();

    int argc_;
    char** argv_;
    AppConfig config_;

    std::unique_ptr<media::MediaLibrary> mediaLibrary_;
    std::unique_ptr<audio::AudioEngine> audioEngine_;
    std::unique_ptr<render::RenderEngine> renderEngine_;
    std::unique_ptr<editor::EditorEngine> editorEngine_;
};

} // namespace catchim::app
