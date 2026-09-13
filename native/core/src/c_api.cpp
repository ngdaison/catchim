#define _CRT_SECURE_NO_WARNINGS
#include "opencut/c_api.h"

#include "opencut/animation.hpp"
#include "opencut/audio.hpp"
#include "opencut/compositor.hpp"
#include "opencut/effects.hpp"
#include "opencut/geometry.hpp"
#include "opencut/masks.hpp"
#include "opencut/scene.hpp"
#include "opencut/speed.hpp"
#include "opencut/subtitles.hpp"
#include "opencut/text_layout.hpp"
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

int ocw_snap_points_sorted(double target_time,
                           const double* points_times,
                           size_t num_points,
                           double max_distance,
                           double* out_snapped_time,
                           int* out_matched_index)
{
    if (points_times == nullptr || num_points == 0 || out_snapped_time == nullptr || out_matched_index == nullptr) {
        if (out_snapped_time != nullptr) *out_snapped_time = target_time;
        if (out_matched_index != nullptr) *out_matched_index = -1;
        return 0;
    }

    double closest_distance = max_distance;
    int closest_index = -1;
    double snapped_time = target_time;

    const double min_bound = target_time - max_distance;
    const double max_bound = target_time + max_distance;

    const double* first = points_times;
    const double* last = points_times + num_points;
    const double* it = std::lower_bound(first, last, min_bound);

    for (; it != last && *it <= max_bound; ++it) {
        double dist = std::abs(target_time - *it);
        if (dist <= closest_distance) {
            closest_distance = dist;
            closest_index = static_cast<int>(it - first);
            snapped_time = *it;
        }
    }

    *out_snapped_time = snapped_time;
    *out_matched_index = closest_index;
    return closest_index >= 0 ? 1 : 0;
}

int ocw_snap_points_linear(double target_time,
                           const double* points_times,
                           size_t num_points,
                           double max_distance,
                           double* out_snapped_time,
                           int* out_matched_index)
{
    if (points_times == nullptr || num_points == 0 || out_snapped_time == nullptr || out_matched_index == nullptr) {
        if (out_snapped_time != nullptr) *out_snapped_time = target_time;
        if (out_matched_index != nullptr) *out_matched_index = -1;
        return 0;
    }

    double closest_distance = max_distance;
    int closest_index = -1;
    double snapped_time = target_time;

    for (size_t i = 0; i < num_points; ++i) {
        double dist = std::abs(target_time - points_times[i]);
        if (dist <= closest_distance) {
            closest_distance = dist;
            closest_index = static_cast<int>(i);
            snapped_time = points_times[i];
        }
    }

    *out_snapped_time = snapped_time;
    *out_matched_index = closest_index;
    return closest_index >= 0 ? 1 : 0;
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

void ocw_compositor_clear_buffer(uint32_t* buffer, int width, int height, float r, float g, float b, float a)
{
    opencut::clear_buffer_rgba(buffer, width, height, {r, g, b, a});
}

void ocw_compositor_composite_layer(uint32_t* dest, int dest_w, int dest_h,
                                    const uint32_t* src, int src_w, int src_h,
                                    float cx, float cy, float w, float h, float rot,
                                    int flip_x, int flip_y,
                                    float opacity, uint32_t blend_mode,
                                    const float* mask_alpha)
{
    opencut::QuadTransform transform{
        .center_x = cx,
        .center_y = cy,
        .width = w,
        .height = h,
        .rotation_degrees = rot,
        .flip_x = flip_x != 0,
        .flip_y = flip_y != 0,
    };
    auto mode = static_cast<opencut::BlendMode>(blend_mode);
    opencut::composite_layer_rgba(dest, dest_w, dest_h, src, src_w, src_h, transform, opacity, mode, mask_alpha);
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

void ocw_mask_apply_to_buffer(uint32_t* pixels, int width, int height,
                              int mask_type, float cx, float cy, float sx, float sy,
                              float rot, float feather, int inverted)
{
    opencut::MaskDefinition def{
        .type = static_cast<opencut::MaskShapeType>(mask_type),
        .center = {cx, cy},
        .size = {sx, sy},
        .rotation_degrees = rot,
        .feather = feather,
        .inverted = inverted != 0,
    };
    opencut::apply_mask_rgba(pixels, width, height, def);
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

void ocw_effects_apply_color_grading(uint32_t* pixels, int width, int height,
                                     float brightness, float contrast, float saturation,
                                     float exposure, float temp, float tint,
                                     float hue, float gamma)
{
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
    opencut::apply_color_grading_rgba(pixels, width, height, adj);
}

void ocw_effects_apply_gaussian_blur(uint32_t* pixels, int width, int height, int radius, float sigma)
{
    opencut::apply_gaussian_blur_rgba(pixels, width, height, radius, sigma);
}

void ocw_effects_apply_vignette(uint32_t* pixels, int width, int height, float amount, float softness, float roundness)
{
    opencut::VignetteParams params{
        .amount = amount,
        .softness = softness,
        .roundness = roundness,
    };
    opencut::apply_vignette_rgba(pixels, width, height, params);
}

void ocw_effects_apply_chroma_key(uint32_t* pixels, int width, int height,
                                  float key_r, float key_g, float key_b,
                                  float similarity, float smoothness, float spill)
{
    opencut::apply_chroma_key_rgba(pixels, width, height, key_r, key_g, key_b, similarity, smoothness, spill);
}

// Audio Buffer DSP
void ocw_audio_apply_gain_ramp(float* samples, size_t num_samples, float start_gain, float end_gain)
{
    opencut::apply_gain_ramp(samples, num_samples, start_gain, end_gain);
}

void ocw_audio_mix_buffers(float* dest, const float* src, size_t num_samples, float volume)
{
    opencut::mix_audio_buffers(dest, src, num_samples, volume);
}

void ocw_audio_resample_linear(const float* src, size_t src_len, float* dst, size_t dst_len)
{
    opencut::resample_audio_linear(src, src_len, dst, dst_len);
}

void ocw_audio_compute_peak_buckets(const float* channel_data,
                                    const uint32_t* bucket_starts,
                                    const uint32_t* bucket_ends,
                                    size_t num_buckets,
                                    float* out_peaks)
{
    if (channel_data == nullptr || bucket_starts == nullptr || bucket_ends == nullptr || out_peaks == nullptr || num_buckets == 0) {
        return;
    }
    const float* const channels[] = { channel_data };
    opencut::compute_peak_buckets(
        std::span<const float* const>(channels, 1),
        std::span<const uint32_t>(bucket_starts, num_buckets),
        std::span<const uint32_t>(bucket_ends, num_buckets),
        out_peaks
    );
}

void ocw_audio_compute_rms_buckets(const float* channel_data,
                                   uint32_t max_window_length,
                                   const uint32_t* bucket_starts,
                                   const uint32_t* bucket_ends,
                                   size_t num_buckets,
                                   float* out_rms)
{
    if (channel_data == nullptr || bucket_starts == nullptr || bucket_ends == nullptr || out_rms == nullptr || num_buckets == 0) {
        return;
    }
    const float* const channels[] = { channel_data };
    opencut::compute_rms_buckets(
        std::span<const float* const>(channels, 1),
        max_window_length,
        std::span<const uint32_t>(bucket_starts, num_buckets),
        std::span<const uint32_t>(bucket_ends, num_buckets),
        out_rms
    );
}

void ocw_audio_mix_channel_retime(float* output_data,
                                  size_t output_start_sample,
                                  size_t rendered_length,
                                  size_t output_length,
                                  double sample_rate,
                                  const float* source_data,
                                  size_t source_length,
                                  double source_sample_rate,
                                  double trim_start,
                                  double retime_rate,
                                  float gain)
{
    opencut::mix_audio_channel_retime(
        output_data,
        output_start_sample,
        rendered_length,
        output_length,
        sample_rate,
        source_data,
        source_length,
        source_sample_rate,
        trim_start,
        retime_rate,
        gain
    );
}

float ocw_audio_compute_buffer_peak(const float* samples, size_t num_samples)
{
    return opencut::compute_buffer_peak(samples, num_samples);
}

void ocw_audio_clamp_buffer_samples(float* samples, size_t num_samples, float max_peak)
{
    opencut::clamp_buffer_samples(samples, num_samples, max_peak);
}

void ocw_audio_downmix_stereo(const float* left, const float* right, float* out, size_t num_samples)
{
    opencut::downmix_stereo_to_mono(left, right, out, num_samples);
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

double ocw_subtitles_parse_timestamp(const char* ts) {
    if (ts == nullptr) return 0.0;
    return static_cast<double>(opencut::subtitles::SrtParser::parse_timestamp(ts));
}

int ocw_subtitles_format_timestamp(double ticks, char* out_buf, size_t out_len) {
    if (out_buf == nullptr || out_len == 0) return 0;
    std::string formatted = opencut::subtitles::SrtParser::format_timestamp(static_cast<opencut::TimelineTick>(std::round(ticks)));
    if (formatted.size() >= out_len) return 0;
    std::memcpy(out_buf, formatted.c_str(), formatted.size() + 1);
    return 1;
}

double ocw_text_measure_line_height(double font_size, double line_height_ratio) {
    if (font_size <= 0.0) font_size = 16.0;
    if (line_height_ratio <= 0.0) line_height_ratio = 1.2;
    return font_size * line_height_ratio;
}

int ocw_text_break_lines_count(const char* text, double max_width, double avg_char_width) {
    if (text == nullptr) return 0;
    auto lines = opencut::text::TextLayoutEngine::break_lines(text, max_width, avg_char_width);
    return static_cast<int>(lines.size());
}

int ocw_geometry_point_in_rotated_rect(double px, double py, double cx, double cy, double w, double h, double rot) {
    return opencut::geometry::GeometryEngine::point_in_rotated_rect(px, py, cx, cy, w, h, rot) ? 1 : 0;
}

int ocw_geometry_test_snap(double source_val, double target_val, double threshold, double* out_snapped, double* out_delta) {
    return opencut::geometry::GeometryEngine::test_snap_axis(source_val, target_val, threshold, out_snapped, out_delta) ? 1 : 0;
}

} // extern "C"
