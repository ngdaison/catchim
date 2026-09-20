#pragma once

#include "Clip.h"
#include "core/ids/Ids.h"
#include <vector>
#include <memory>
#include <optional>
#include <algorithm>

namespace catchim::editor {

enum class TrackType {
    Video,
    Audio,
    Text,
    Graphic,
    Effect
};

inline const char* trackTypeToString(TrackType type) {
    switch (type) {
        case TrackType::Video: return "video";
        case TrackType::Audio: return "audio";
        case TrackType::Text: return "text";
        case TrackType::Graphic: return "graphic";
        case TrackType::Effect: return "effect";
    }
    return "video";
}

inline TrackType stringToTrackType(std::string_view sv) {
    if (sv == "audio") return TrackType::Audio;
    if (sv == "text") return TrackType::Text;
    if (sv == "graphic") return TrackType::Graphic;
    if (sv == "effect") return TrackType::Effect;
    return TrackType::Video;
}

class Track {
public:
    Track(core::TrackId id, TrackType type, std::string name);

    const core::TrackId& id() const noexcept { return id_; }
    TrackType type() const noexcept { return type_; }
    const std::string& name() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    bool isMuted() const noexcept { return muted_; }
    void setMuted(bool muted) noexcept { muted_ = muted; }

    bool isHidden() const noexcept { return hidden_; }
    void setHidden(bool hidden) noexcept { hidden_ = hidden; }

    // Clip operations
    const std::vector<Clip>& clips() const noexcept { return clips_; }
    std::vector<Clip>& clips() noexcept { return clips_; }

    const Clip* findClip(const core::ClipId& clipId) const noexcept;
    Clip* findClip(const core::ClipId& clipId) noexcept;

    bool canPlace(
        core::TimelineTime start,
        core::TimelineTime duration,
        const std::optional<core::ClipId>& excludeClipId = std::nullopt
    ) const noexcept;

    bool insertClip(Clip clip);
    std::optional<Clip> removeClip(const core::ClipId& clipId);

    bool acceptsClipType(ClipType clipType) const noexcept;

    core::TimelineTime maxEndTime() const noexcept;

    void sortClips();

private:
    core::TrackId id_;
    TrackType type_;
    std::string name_;
    bool muted_{false};
    bool hidden_{false};
    std::vector<Clip> clips_;
};

} // namespace catchim::editor
