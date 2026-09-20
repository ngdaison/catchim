#include "TimelineDragSource.h"
#include "editor/timeline/PlacementEngine.h"

namespace catchim::editor {

DragPayloadType TimelineDragSource::getActiveType() const {
    if (!active_) {
        return DragPayloadType::Media;
    }
    return std::visit([](const auto& arg) -> DragPayloadType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, MediaDragPayload>) return DragPayloadType::Media;
        else if constexpr (std::is_same_v<T, TextDragPayload>) return DragPayloadType::Text;
        else if constexpr (std::is_same_v<T, StickerDragPayload>) return DragPayloadType::Sticker;
        else if constexpr (std::is_same_v<T, GraphicDragPayload>) return DragPayloadType::Graphic;
        else return DragPayloadType::Effect;
    }, *active_);
}

bool TimelineDragSource::canDropOnTrack(const Track& track, const TimelineDragData& data) noexcept {
    return std::visit([&](const auto& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, MediaDragPayload>) {
            if (payload.mediaType == ClipType::Audio) {
                return track.type() == TrackType::Audio;
            }
            return track.type() == TrackType::Video;
        } else if constexpr (std::is_same_v<T, TextDragPayload>) {
            return track.type() == TrackType::Text || track.type() == TrackType::Video;
        } else if constexpr (std::is_same_v<T, StickerDragPayload>) {
            return track.type() == TrackType::Graphic || track.type() == TrackType::Video;
        } else if constexpr (std::is_same_v<T, GraphicDragPayload>) {
            return track.type() == TrackType::Graphic || track.type() == TrackType::Video;
        } else {
            return track.type() == TrackType::Effect;
        }
    }, data);
}

std::optional<core::ClipId> TimelineDragSource::resolveDropOnTrack(
    Timeline& timeline,
    const core::TrackId& trackId,
    core::TimelineTime dropTime,
    const TimelineDragData& data
) {
    auto* track = timeline.findTrack(trackId);
    if (!track || !canDropOnTrack(*track, data)) {
        return std::nullopt;
    }

    std::optional<Clip> createdClip = std::visit([&](const auto& payload) -> std::optional<Clip> {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, MediaDragPayload>) {
            core::ClipId cid = core::ClipId::generate();
            Clip clip(cid, payload.mediaType, payload.name, dropTime, payload.duration);
            clip.setMediaId(core::MediaId(payload.id));
            return clip;
        } else if constexpr (std::is_same_v<T, TextDragPayload>) {
            core::ClipId cid = core::ClipId::generate();
            Clip clip(cid, ClipType::Text, payload.name, dropTime, core::TimelineTime::fromSeconds(3.0));
            clip.setParam<std::string>("content", payload.content);
            return clip;
        } else if constexpr (std::is_same_v<T, StickerDragPayload>) {
            core::ClipId cid = core::ClipId::generate();
            Clip clip(cid, ClipType::Sticker, payload.name, dropTime, core::TimelineTime::fromSeconds(3.0));
            clip.setParam<std::string>("stickerId", payload.stickerId);
            return clip;
        } else if constexpr (std::is_same_v<T, GraphicDragPayload>) {
            core::ClipId cid = core::ClipId::generate();
            Clip clip(cid, ClipType::Graphic, payload.name, dropTime, core::TimelineTime::fromSeconds(3.0));
            clip.setParam<std::string>("definitionId", payload.definitionId);
            clip.setParams(payload.params);
            return clip;
        } else if constexpr (std::is_same_v<T, EffectDragPayload>) {
            core::ClipId cid = core::ClipId::generate();
            Clip clip(cid, ClipType::Effect, payload.name, dropTime, core::TimelineTime::fromSeconds(3.0));
            clip.setParam<std::string>("effectType", payload.effectType);
            return clip;
        }
        return std::nullopt;
    }, data);

    if (!createdClip.has_value()) {
        return std::nullopt;
    }

    if (!PlacementEngine::canPlaceClipOnTrack(*track, createdClip->startTime(), createdClip->duration())) {
        return std::nullopt;
    }

    core::ClipId resultId = createdClip->id();
    if (timeline.addClip(trackId, std::move(*createdClip))) {
        return resultId;
    }

    return std::nullopt;
}

bool TimelineDragSource::canDropOnClip(const Clip& targetClip, const TimelineDragData& data) noexcept {
    return std::visit([&](const auto& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, EffectDragPayload>) {
            for (const auto& allowed : payload.targetElementTypes) {
                if (targetClip.type() == allowed) {
                    return true;
                }
            }
        }
        return false;
    }, data);
}

bool TimelineDragSource::resolveDropOnClip(Clip& targetClip, const TimelineDragData& data) {
    if (!canDropOnClip(targetClip, data)) {
        return false;
    }

    return std::visit([&](const auto& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, EffectDragPayload>) {
            EffectInstance eff;
            eff.id = core::ClipId::generate().str();
            eff.type = payload.effectType;
            eff.params = nlohmann::json::object();
            eff.enabled = true;
            targetClip.effects().push_back(std::move(eff));
            return true;
        }
        return false;
    }, data);
}

} // namespace catchim::editor
