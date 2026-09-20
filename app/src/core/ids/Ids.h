#pragma once

#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace catchim::core {

namespace detail {
inline std::string generateRandomId(size_t length = 21) {
    static const char alphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz-";
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, sizeof(alphabet) - 2);

    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        s.push_back(alphabet[dis(gen)]);
    }
    return s;
}
} // namespace detail

template <typename Tag>
class Id {
public:
    Id() : value_(detail::generateRandomId()) {}
    explicit Id(std::string value) : value_(std::move(value)) {}
    explicit Id(const char* value) : value_(value ? value : "") {}

    static Id generate() { return Id(); }
    static Id empty() { return Id(""); }

    const std::string& str() const { return value_; }
    const char* c_str() const { return value_.c_str(); }
    bool isEmpty() const { return value_.empty(); }

    bool operator==(const Id& other) const { return value_ == other.value_; }
    bool operator!=(const Id& other) const { return value_ != other.value_; }
    bool operator<(const Id& other) const { return value_ < other.value_; }

private:
    std::string value_;
};

struct ProjectTag {};
struct SceneTag {};
struct TrackTag {};
struct ClipTag {};
struct MediaTag {};
struct EffectTag {};
struct BookmarkTag {};

using ProjectId  = Id<ProjectTag>;
using SceneId    = Id<SceneTag>;
using TrackId    = Id<TrackTag>;
using ClipId     = Id<ClipTag>;
using MediaId    = Id<MediaTag>;
using EffectId   = Id<EffectTag>;
using BookmarkId = Id<BookmarkTag>;

} // namespace catchim::core

namespace std {
template <typename Tag>
struct hash<catchim::core::Id<Tag>> {
    size_t operator()(const catchim::core::Id<Tag>& id) const noexcept {
        return hash<string>()(id.str());
    }
};
} // namespace std
