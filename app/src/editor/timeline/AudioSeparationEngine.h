#pragma once

#include "editor/timeline/Clip.h"
#include "media/MediaAsset.h"
#include <string>
#include <string_view>
#include <optional>

namespace catchim::editor {

class AudioSeparationEngine {
public:
    /**
     * @brief Checks whether a video clip currently has its source audio enabled.
     * Mirrors web/src/timeline/audio-separation/index.ts isSourceAudioEnabled.
     */
    static bool isSourceAudioEnabled(const Clip& clip) noexcept;

    /**
     * @brief Checks whether a video clip has had its source audio separated.
     * Mirrors web/src/timeline/audio-separation/index.ts isSourceAudioSeparated.
     */
    static bool isSourceAudioSeparated(const Clip& clip) noexcept;

    /**
     * @brief Checks if source audio can be extracted from a clip.
     * Mirrors web/src/timeline/audio-separation/index.ts canExtractSourceAudio.
     */
    static bool canExtractSourceAudio(
        const Clip& clip,
        const media::MediaAsset* mediaAsset = nullptr
    ) noexcept;

    /**
     * @brief Checks if separated audio can be recovered back into the video clip.
     * Mirrors web/src/timeline/audio-separation/index.ts canRecoverSourceAudio.
     */
    static bool canRecoverSourceAudio(const Clip& clip) noexcept;

    /**
     * @brief Checks if source audio can be toggled (either extracted or recovered).
     * Mirrors web/src/timeline/audio-separation/index.ts canToggleSourceAudio.
     */
    static bool canToggleSourceAudio(
        const Clip& clip,
        const media::MediaAsset* mediaAsset = nullptr
    ) noexcept;

    /**
     * @brief Checks if an element has active, enabled audio output.
     * Mirrors web/src/timeline/audio-separation/index.ts doesElementHaveEnabledAudio.
     */
    static bool doesElementHaveEnabledAudio(
        const Clip& clip,
        const media::MediaAsset* mediaAsset = nullptr
    ) noexcept;

    /**
     * @brief Gets the UI action label ("Extract audio" or "Recover audio").
     * Mirrors web/src/timeline/audio-separation/index.ts getSourceAudioActionLabel.
     */
    static std::string_view getSourceAudioActionLabel(const Clip& clip) noexcept;

    /**
     * @brief Builds an independent audio Clip from a source video Clip.
     * Copies duration, trim boundaries, source duration, volume, retime config,
     * and clones the volume animation channel.
     * Mirrors web/src/timeline/audio-separation/index.ts buildSeparatedAudioElement.
     */
    static Clip buildSeparatedAudioClip(const Clip& sourceClip);
};

} // namespace catchim::editor
