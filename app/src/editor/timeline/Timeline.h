#pragma once

#include "Track.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "Bookmark.h"

namespace catchim::editor {

struct TimelineTracksSnapshot {
    std::vector<Track> overlayTracks;
    Track mainTrack;
    std::vector<Track> audioTracks;
};

class Timeline {
public:
    Timeline();

    // Track hierarchy (SceneTracks in web)
    const std::vector<Track>& overlayTracks() const noexcept { return overlayTracks_; }
    std::vector<Track>& overlayTracks() noexcept { return overlayTracks_; }

    const Track& mainTrack() const noexcept { return mainTrack_; }
    Track& mainTrack() noexcept { return mainTrack_; }

    const std::vector<Track>& audioTracks() const noexcept { return audioTracks_; }
    std::vector<Track>& audioTracks() noexcept { return audioTracks_; }

    // Combined tracks list (order: overlay -> main -> audio, matching web view)
    std::vector<Track*> allTracks();
    std::vector<const Track*> allTracks() const;

    Track* findTrack(const core::TrackId& trackId);
    const Track* findTrack(const core::TrackId& trackId) const;

    Track* findTrackContainingClip(const core::ClipId& clipId);
    const Track* findTrackContainingClip(const core::ClipId& clipId) const;

    Track& addTrack(TrackType type, std::string name);
    Track& insertTrack(
        TrackType type,
        std::string name,
        std::optional<size_t> index = std::nullopt,
        std::optional<core::TrackId> specificId = std::nullopt
    );
    bool removeTrack(const core::TrackId& trackId);
    std::optional<Track> extractTrack(const core::TrackId& trackId);

    // Clip operations
    bool addClip(const core::TrackId& trackId, Clip clip);
    std::optional<Clip> removeClip(const core::ClipId& clipId);
    const Clip* findClip(const core::ClipId& clipId) const;
    Clip* findClip(const core::ClipId& clipId);

    bool moveClip(
        const core::ClipId& clipId,
        const core::TrackId& targetTrackId,
        core::TimelineTime targetTime
    );

    bool trimClipStart(const core::ClipId& clipId, core::TimelineTime newStartTime);
    bool trimClipEnd(const core::ClipId& clipId, core::TimelineTime newDuration);

    std::pair<std::optional<core::ClipId>, std::optional<core::ClipId>> splitClip(
        const core::ClipId& clipId,
        core::TimelineTime splitTime
    );

    // Bookmarks
    const std::vector<Bookmark>& bookmarks() const noexcept { return bookmarks_; }
    std::vector<Bookmark>& bookmarks() noexcept { return bookmarks_; }
    void setBookmarks(std::vector<Bookmark> bms) { bookmarks_ = std::move(bms); }
    void addBookmark(Bookmark bm) { bookmarks_.push_back(std::move(bm)); }
    bool removeBookmark(const core::BookmarkId& id);
    bool hasBookmarkAt(core::TimelineTime time, core::TimelineTime threshold) const;
    bool toggleBookmark(core::TimelineTime time, core::TimelineTime threshold = core::TimelineTime::fromTicks(1000));
    std::optional<Bookmark> findNextBookmark(core::TimelineTime time) const;
    std::optional<Bookmark> findPrevBookmark(core::TimelineTime time) const;
    void sortBookmarks();

    // Snapshots (SceneTracks in web)
    TimelineTracksSnapshot createSnapshot() const;
    void restoreSnapshot(const TimelineTracksSnapshot& snapshot);

    // Timeline duration
    core::TimelineTime totalDuration() const noexcept;

    // Ripple
    void applyRipple(const core::TrackId& trackId, core::TimelineTime afterTime, core::TimelineTime delta);

private:
    std::vector<Track> overlayTracks_;
    Track mainTrack_;
    std::vector<Track> audioTracks_;
    std::vector<Bookmark> bookmarks_;
};

} // namespace catchim::editor
