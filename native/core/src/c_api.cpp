#define _CRT_SECURE_NO_WARNINGS
#include "opencut/c_api.h"

#include "opencut/animation.hpp"
#include "opencut/audio.hpp"
#include "opencut/compositor.hpp"
#include "opencut/effects.hpp"
#include "opencut/masks.hpp"
#include "opencut/scene.hpp"
#include "opencut/speed.hpp"
#include "opencut/time.hpp"
#include "opencut/timeline.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct OcTimeline {
    opencut::TimelineIndex index;
};

namespace {

bool missing(const char* value)
{
    return value == nullptr || value[0] == '\0';
}

std::string_view view(const char* value)
{
    return value == nullptr ? std::string_view{} : std::string_view{value};
}

OcTimelineStatus to_status(const opencut::TimelineError& error)
{
    using opencut::TimelineErrorCode;

    switch (error.code) {
        case TimelineErrorCode::None:
            return OC_TIMELINE_OK;
        case TimelineErrorCode::InvalidTimeRange:
            return OC_TIMELINE_INVALID_TIME_RANGE;
        case TimelineErrorCode::MissingTrack:
            return OC_TIMELINE_MISSING_TRACK;
        case TimelineErrorCode::MissingClip:
            return OC_TIMELINE_MISSING_CLIP;
        case TimelineErrorCode::DuplicateTrack:
            return OC_TIMELINE_DUPLICATE_TRACK;
        case TimelineErrorCode::DuplicateClip:
            return OC_TIMELINE_DUPLICATE_CLIP;
        case TimelineErrorCode::Overlap:
            return OC_TIMELINE_OVERLAP;
    }

    return OC_TIMELINE_INVALID_ARGUMENT;
}

opencut::TimeRange range(int64_t start, int64_t duration)
{
    return opencut::TimeRange{.start = start, .duration = duration};
}

std::optional<opencut::TimelineTick> tick_from_double(double value)
{
    if (!std::isfinite(value)) {
        return std::nullopt;
    }

    constexpr auto min_tick = static_cast<double>(std::numeric_limits<opencut::TimelineTick>::min());
    constexpr auto max_tick = static_cast<double>(std::numeric_limits<opencut::TimelineTick>::max());
    if (value < min_tick || value > max_tick) {
        return std::nullopt;
    }

    return static_cast<opencut::TimelineTick>(std::round(value));
}

std::optional<opencut::TimeRange> range_from_double(double start, double duration)
{
    const auto start_tick = tick_from_double(start);
    const auto duration_tick = tick_from_double(duration);
    if (!start_tick || !duration_tick) {
        return std::nullopt;
    }

    return range(*start_tick, *duration_tick);
}

} // namespace

extern "C" {

OcTimeline* oc_timeline_create(void)
{
    return new OcTimeline{};
}

void oc_timeline_destroy(OcTimeline* timeline)
{
    delete timeline;
}

OcTimelineStatus oc_timeline_add_track(OcTimeline* timeline, const char* track_id)
{
    if (timeline == nullptr || missing(track_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.add_track(std::string(track_id));
    return to_status(result.error);
}

OcTimelineStatus oc_timeline_insert_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id,
                                         int64_t start,
                                         int64_t duration)
{
    if (timeline == nullptr || missing(track_id) || missing(clip_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.insert_clip(
        view(track_id),
        opencut::Clip{
            .id = std::string(clip_id),
            .range = range(start, duration),
        });

    return to_status(result.error);
}

OcTimelineStatus oc_timeline_delete_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id)
{
    if (timeline == nullptr || missing(track_id) || missing(clip_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.delete_clip(
        opencut::ClipRef{.track_id = view(track_id), .clip_id = view(clip_id)});

    return to_status(result.error);
}

OcTimelineStatus oc_timeline_move_clip(OcTimeline* timeline,
                                       const char* source_track_id,
                                       const char* clip_id,
                                       const char* target_track_id,
                                       int64_t new_start)
{
    if (timeline == nullptr || missing(source_track_id) || missing(clip_id) || missing(target_track_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.move_clip(
        opencut::ClipRef{.track_id = view(source_track_id), .clip_id = view(clip_id)},
        view(target_track_id),
        new_start);

    return to_status(result.error);
}

OcTimelineStatus oc_timeline_trim_clip(OcTimeline* timeline,
                                       const char* track_id,
                                       const char* clip_id,
                                       int64_t new_start,
                                       int64_t new_duration)
{
    if (timeline == nullptr || missing(track_id) || missing(clip_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.trim_clip(
        opencut::ClipRef{.track_id = view(track_id), .clip_id = view(clip_id)},
        new_start,
        new_duration);

    return to_status(result.error);
}

OcTimelineStatus oc_timeline_split_clip(OcTimeline* timeline,
                                        const char* track_id,
                                        const char* clip_id,
                                        int64_t split_time,
                                        const char* new_clip_id)
{
    if (timeline == nullptr || missing(track_id) || missing(clip_id) || missing(new_clip_id)) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    const auto result = timeline->index.split_clip(
        opencut::ClipRef{.track_id = view(track_id), .clip_id = view(clip_id)},
        split_time,
        std::string(new_clip_id));

    return to_status(result.error);
}

int oc_timeline_can_place(OcTimeline* timeline,
                          const char* track_id,
                          int64_t start,
                          int64_t duration,
                          const char* exclude_clip_id)
{
    if (timeline == nullptr || missing(track_id)) {
        return 0;
    }

    const auto exclude = missing(exclude_clip_id)
                             ? std::nullopt
                             : std::optional<std::string_view>{view(exclude_clip_id)};

    return timeline->index.can_place(view(track_id), range(start, duration), exclude) ? 1 : 0;
}

uint32_t oc_timeline_count_clips_in_range(OcTimeline* timeline, int64_t start, int64_t duration)
{
    if (timeline == nullptr) {
        return 0;
    }

    return static_cast<uint32_t>(timeline->index.clips_in_range(range(start, duration)).size());
}

OcTimelineStatus ocw_timeline_insert_clip(OcTimeline* timeline,
                                          const char* track_id,
                                          const char* clip_id,
                                          double start,
                                          double duration)
{
    const auto start_tick = tick_from_double(start);
    const auto duration_tick = tick_from_double(duration);
    if (!start_tick || !duration_tick) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    return oc_timeline_insert_clip(timeline, track_id, clip_id, *start_tick, *duration_tick);
}

OcTimelineStatus ocw_timeline_delete_clip(OcTimeline* timeline,
                                          const char* track_id,
                                          const char* clip_id)
{
    return oc_timeline_delete_clip(timeline, track_id, clip_id);
}

OcTimelineStatus ocw_timeline_move_clip(OcTimeline* timeline,
                                        const char* source_track_id,
                                        const char* clip_id,
                                        const char* target_track_id,
                                        double new_start)
{
    const auto new_start_tick = tick_from_double(new_start);
    if (!new_start_tick) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    return oc_timeline_move_clip(timeline, source_track_id, clip_id, target_track_id, *new_start_tick);
}

OcTimelineStatus ocw_timeline_trim_clip(OcTimeline* timeline,
                                        const char* track_id,
                                        const char* clip_id,
                                        double new_start,
                                        double new_duration)
{
    const auto start_tick = tick_from_double(new_start);
    const auto duration_tick = tick_from_double(new_duration);
    if (!start_tick || !duration_tick) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    return oc_timeline_trim_clip(timeline, track_id, clip_id, *start_tick, *duration_tick);
}

OcTimelineStatus ocw_timeline_split_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id,
                                         double split_time,
                                         const char* new_clip_id)
{
    const auto split_tick = tick_from_double(split_time);
    if (!split_tick) {
        return OC_TIMELINE_INVALID_ARGUMENT;
    }

    return oc_timeline_split_clip(timeline, track_id, clip_id, *split_tick, new_clip_id);
}

int ocw_timeline_can_place(OcTimeline* timeline,
                           const char* track_id,
                           double start,
                           double duration,
                           const char* exclude_clip_id)
{
    const auto start_tick = tick_from_double(start);
    const auto duration_tick = tick_from_double(duration);
    if (!start_tick || !duration_tick) {
        return 0;
    }

    return oc_timeline_can_place(timeline, track_id, *start_tick, *duration_tick, exclude_clip_id);
}

uint32_t ocw_timeline_count_clips_in_range(OcTimeline* timeline, double start, double duration)
{
    const auto query_range = range_from_double(start, duration);
    if (!query_range) {
        return 0;
    }

    return static_cast<uint32_t>(timeline->index.clips_in_range(*query_range).size());
}

int ocw_timeline_snap(OcTimeline* timeline,
                      double target_time,
                      double playhead,
                      double threshold,
                      double* out_snapped_time,
                      double* out_delta)
{
    if (timeline == nullptr || out_snapped_time == nullptr || out_delta == nullptr) {
        return 0;
    }

    auto target_tick = tick_from_double(target_time).value_or(0);
    auto playhead_tick = std::isfinite(playhead) && playhead >= 0 ? tick_from_double(playhead) : std::nullopt;
    auto thresh_tick = tick_from_double(threshold).value_or(20);

    auto points = timeline->index.collect_snap_points(playhead_tick, {});
    auto result = timeline->index.snap_time(target_tick, points, thresh_tick);

    *out_snapped_time = static_cast<double>(result.snapped_time);
    *out_delta = static_cast<double>(result.delta);
    return result.snapped ? 1 : 0;
}

// Time & Timecode
int64_t oc_time_ticks_per_second(void)
{
    return opencut::TICKS_PER_SECOND;
}

double ocw_time_from_seconds(double seconds)
{
    auto mt = opencut::MediaTime::from_seconds_f64(seconds);
    return mt ? static_cast<double>(mt->as_ticks()) : 0.0;
}

double ocw_time_to_seconds(double ticks)
{
    auto t = tick_from_double(ticks).value_or(0);
    return opencut::MediaTime(t).to_seconds_f64();
}

double ocw_time_round_to_frame(double ticks, uint32_t fps_num, uint32_t fps_den)
{
    auto t = tick_from_double(ticks).value_or(0);
    opencut::FrameRate fps{fps_num, fps_den};
    auto res = opencut::MediaTime(t).round_to_frame(fps);
    return res ? static_cast<double>(res->as_ticks()) : ticks;
}

double ocw_time_floor_to_frame(double ticks, uint32_t fps_num, uint32_t fps_den)
{
    auto t = tick_from_double(ticks).value_or(0);
    opencut::FrameRate fps{fps_num, fps_den};
    auto res = opencut::MediaTime(t).floor_to_frame(fps);
    return res ? static_cast<double>(res->as_ticks()) : ticks;
}

double ocw_time_last_frame(double duration, uint32_t fps_num, uint32_t fps_den)
{
    auto dur = tick_from_double(duration).value_or(0);
    opencut::FrameRate fps{fps_num, fps_den};
    auto res = opencut::MediaTime(dur).last_frame_time(fps);
    return res ? static_cast<double>(res->as_ticks()) : duration;
}

double ocw_time_snapped_seek(double time, double duration, uint32_t fps_num, uint32_t fps_den)
{
    auto t = tick_from_double(time).value_or(0);
    auto d = tick_from_double(duration).value_or(0);
    opencut::FrameRate fps{fps_num, fps_den};
    auto res = opencut::MediaTime(t).snapped_seek_time(opencut::MediaTime(d), fps);
    return res ? static_cast<double>(res->as_ticks()) : time;
}

int ocw_time_format_timecode(double ticks, int format_type, uint32_t fps_num, uint32_t fps_den, char* out_buf, size_t out_len)
{
    if (out_buf == nullptr || out_len == 0) return 0;
    auto t = tick_from_double(ticks).value_or(0);
    auto fmt = static_cast<opencut::TimeCodeFormat>(format_type);
    opencut::FrameRate fps{fps_num, fps_den};

    auto str = opencut::format_timecode(opencut::MediaTime(t), fmt, fps);
    if (!str) {
        out_buf[0] = '\0';
        return 0;
    }

    std::strncpy(out_buf, str->c_str(), out_len - 1);
    out_buf[out_len - 1] = '\0';
    return 1;
}

double ocw_time_parse_timecode(const char* timecode_str, int format_type, uint32_t fps_num, uint32_t fps_den)
{
    if (missing(timecode_str)) return -1.0;
    auto fmt = static_cast<opencut::TimeCodeFormat>(format_type);
    opencut::FrameRate fps{fps_num, fps_den};

    auto res = opencut::parse_timecode(timecode_str, fmt, fps);
    return res ? static_cast<double>(res->as_ticks()) : -1.0;
}

// Audio
int ocw_audio_decimate_waveform(const float* samples, size_t num_samples, size_t num_bins, float* out_peaks)
{
    if (samples == nullptr || out_peaks == nullptr || num_samples == 0 || num_bins == 0) {
        return 0;
    }

    auto peaks = opencut::decimate_waveform_peaks(std::span<const float>(samples, num_samples), num_bins);
    for (size_t i = 0; i < peaks.size(); ++i) {
        out_peaks[2 * i] = peaks[i].min_val;
        out_peaks[2 * i + 1] = peaks[i].max_val;
    }
    return 1;
}

float ocw_audio_evaluate_fade(double offset_ticks, double duration_ticks, double fade_in_ticks, double fade_out_ticks)
{
    auto off = tick_from_double(offset_ticks).value_or(0);
    auto dur = tick_from_double(duration_ticks).value_or(0);
    auto fi = tick_from_double(fade_in_ticks).value_or(0);
    auto fo = tick_from_double(fade_out_ticks).value_or(0);

    opencut::AudioFade fade{.fade_in_ticks = fi, .fade_out_ticks = fo};
    return opencut::evaluate_fade_multiplier(off, dur, fade);
}

// Compositor
void ocw_compositor_blend(float b_r, float b_g, float b_b, float b_a,
                          float l_r, float l_g, float l_b, float l_a,
                          uint32_t blend_mode, float opacity, float* out_rgba)
{
    if (out_rgba == nullptr) return;
    opencut::ColorRGBA base{b_r, b_g, b_b, b_a};
    opencut::ColorRGBA layer{l_r, l_g, l_b, l_a};
    auto mode = static_cast<opencut::BlendMode>(blend_mode);
    auto result = opencut::blend_colors(base, layer, mode, opacity);
    out_rgba[0] = result.r;
    out_rgba[1] = result.g;
    out_rgba[2] = result.b;
    out_rgba[3] = result.a;
}

// Masks & Effects
float ocw_mask_evaluate_alpha(float px, float py, int mask_type, float cx, float cy, float sx, float sy, float rot, float feather, int inverted)
{
    opencut::MaskDefinition def{
        .type = static_cast<opencut::MaskShapeType>(mask_type),
        .center = {cx, cy},
        .size = {sx, sy},
        .rotation_degrees = rot,
        .feather = feather,
        .inverted = inverted != 0,
    };
    return opencut::evaluate_mask_alpha({px, py}, def);
}

void ocw_effects_apply(float r, float g, float b, float a,
                       float brightness, float contrast, float saturation, float exposure,
                       float temp, float tint, float hue, float gamma, float* out_rgba)
{
    if (out_rgba == nullptr) return;
    opencut::ColorAdjustments adj{
        .brightness = brightness,
        .contrast = contrast,
        .saturation = saturation,
        .exposure = exposure,
        .temperature = temp,
        .tint = tint,
        .hue_degrees = hue,
        .gamma = gamma,
    };
    auto result = opencut::apply_color_adjustments({r, g, b, a}, adj);
    out_rgba[0] = result.r;
    out_rgba[1] = result.g;
    out_rgba[2] = result.b;
    out_rgba[3] = result.a;
}

double ocw_animation_solve_bezier(double time, double t0, double t1, double t2, double t3) {
    return opencut::animation::CubicBezier::solve_progress_for_time(time, t0, t1, t2, t3);
}

double ocw_animation_evaluate_bezier_point(double progress, double p0, double p1, double p2, double p3) {
    return opencut::animation::CubicBezier::evaluate_point(progress, p0, p1, p2, p3);
}

double ocw_animation_evaluate_channel(const double* key_times, const double* key_values, const int* key_interp, size_t num_keys, double eval_time) {
    if (key_times == nullptr || key_values == nullptr || num_keys == 0) {
        return 0.0;
    }
    opencut::animation::KeyframeChannel channel;
    for (size_t i = 0; i < num_keys; ++i) {
        opencut::animation::Keyframe kf{
            .time = static_cast<int64_t>(std::round(key_times[i])),
            .value = key_values[i],
            .interpolation = key_interp != nullptr
                ? static_cast<opencut::animation::InterpolationType>(key_interp[i])
                : opencut::animation::InterpolationType::Linear
        };
        channel.insert_or_update_keyframe(kf);
    }
    return channel.evaluate(static_cast<int64_t>(std::round(eval_time)));
}

double ocw_speed_map_timeline_to_source(double timeline_offset, double timeline_duration,
                                        const double* speed_ratios, const double* speed_multipliers,
                                        size_t num_points, double constant_speed) {
    opencut::speed::SpeedCurve curve;
    if (speed_ratios != nullptr && speed_multipliers != nullptr && num_points > 0) {
        for (size_t i = 0; i < num_points; ++i) {
            curve.add_point(speed_ratios[i], speed_multipliers[i]);
        }
    }
    return static_cast<double>(curve.map_timeline_to_source_offset(
        static_cast<int64_t>(std::round(timeline_offset)),
        static_cast<int64_t>(std::round(timeline_duration)),
        constant_speed
    ));
}

double ocw_timeline_find_available_gap(OcTimeline* timeline, const char* track_id, double duration, double min_start) {
    if (timeline == nullptr || track_id == nullptr) return min_start;
    auto gap = timeline->index.find_first_available_gap(
        track_id,
        static_cast<opencut::TimelineTick>(std::round(duration)),
        static_cast<opencut::TimelineTick>(std::round(min_start))
    );
    return gap ? static_cast<double>(*gap) : min_start;
}

int ocw_timeline_apply_ripple(OcTimeline* timeline, const char* track_id, double after_time, double delta_ticks) {
    if (timeline == nullptr || track_id == nullptr) return 0;
    auto modified = timeline->index.apply_ripple_shift(
        track_id,
        static_cast<opencut::TimelineTick>(std::round(after_time)),
        static_cast<opencut::TimelineTick>(std::round(delta_ticks))
    );
    return static_cast<int>(modified.size());
}

struct OcScene {
    opencut::scene::SceneGraph graph;
    std::vector<opencut::scene::RenderItem> last_display_list;
};

OcScene* oc_scene_create(void) {
    return new OcScene();
}

void oc_scene_destroy(OcScene* scene) {
    delete scene;
}

void ocw_scene_add_item(OcScene* scene, const char* id, uint32_t type, int32_t z_index,
                        float opacity, uint32_t blend_mode,
                        double cx, double cy, double w, double h, double rot,
                        int flip_x, int flip_y, const char* asset_id)
{
    if (scene == nullptr || id == nullptr) return;
    opencut::scene::RenderItem item;
    item.id = id;
    item.type = static_cast<opencut::scene::NodeType>(type);
    item.z_index = z_index;
    item.opacity = opacity;
    item.blend_mode = blend_mode;
    item.local_transform = opencut::QuadTransform{
        .center_x = static_cast<float>(cx),
        .center_y = static_cast<float>(cy),
        .width = static_cast<float>(w),
        .height = static_cast<float>(h),
        .rotation_degrees = static_cast<float>(rot),
        .flip_x = flip_x != 0,
        .flip_y = flip_y != 0,
    };
    if (asset_id != nullptr) {
        item.asset_id = asset_id;
    }
    scene->graph.add_item(std::move(item));
}

uint32_t ocw_scene_build_display_list(OcScene* scene, double viewport_w, double viewport_h, int enable_culling) {
    if (scene == nullptr) return 0;
    scene->last_display_list = scene->graph.build_display_list(viewport_w, viewport_h, enable_culling != 0);
    return static_cast<uint32_t>(scene->last_display_list.size());
}

} // extern "C"
