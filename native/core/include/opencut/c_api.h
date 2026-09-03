#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OcTimeline OcTimeline;

typedef enum OcTimelineStatus {
    OC_TIMELINE_OK = 0,
    OC_TIMELINE_INVALID_ARGUMENT = 1,
    OC_TIMELINE_INVALID_TIME_RANGE = 2,
    OC_TIMELINE_MISSING_TRACK = 3,
    OC_TIMELINE_MISSING_CLIP = 4,
    OC_TIMELINE_DUPLICATE_TRACK = 5,
    OC_TIMELINE_DUPLICATE_CLIP = 6,
    OC_TIMELINE_OVERLAP = 7,
} OcTimelineStatus;

// Timeline Lifecycle
OcTimeline* oc_timeline_create(void);
void oc_timeline_destroy(OcTimeline* timeline);

// Timeline Commands
OcTimelineStatus oc_timeline_add_track(OcTimeline* timeline, const char* track_id);
OcTimelineStatus oc_timeline_insert_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id,
                                         int64_t start,
                                         int64_t duration);
OcTimelineStatus oc_timeline_delete_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id);
OcTimelineStatus oc_timeline_move_clip(OcTimeline* timeline,
                                       const char* source_track_id,
                                       const char* clip_id,
                                       const char* target_track_id,
                                       int64_t new_start);
OcTimelineStatus oc_timeline_trim_clip(OcTimeline* timeline,
                                       const char* track_id,
                                       const char* clip_id,
                                       int64_t new_start,
                                       int64_t new_duration);
OcTimelineStatus oc_timeline_split_clip(OcTimeline* timeline,
                                        const char* track_id,
                                        const char* clip_id,
                                        int64_t split_time,
                                        const char* new_clip_id);
int oc_timeline_can_place(OcTimeline* timeline,
                          const char* track_id,
                          int64_t start,
                          int64_t duration,
                          const char* exclude_clip_id);
uint32_t oc_timeline_count_clips_in_range(OcTimeline* timeline, int64_t start, int64_t duration);

// Web-compatible helpers (double args for JS numbers)
OcTimelineStatus ocw_timeline_insert_clip(OcTimeline* timeline,
                                          const char* track_id,
                                          const char* clip_id,
                                          double start,
                                          double duration);
OcTimelineStatus ocw_timeline_delete_clip(OcTimeline* timeline,
                                          const char* track_id,
                                          const char* clip_id);
OcTimelineStatus ocw_timeline_move_clip(OcTimeline* timeline,
                                        const char* source_track_id,
                                        const char* clip_id,
                                        const char* target_track_id,
                                        double new_start);
OcTimelineStatus ocw_timeline_trim_clip(OcTimeline* timeline,
                                        const char* track_id,
                                        const char* clip_id,
                                        double new_start,
                                        double new_duration);
OcTimelineStatus ocw_timeline_split_clip(OcTimeline* timeline,
                                         const char* track_id,
                                         const char* clip_id,
                                         double split_time,
                                         const char* new_clip_id);
int ocw_timeline_can_place(OcTimeline* timeline,
                           const char* track_id,
                           double start,
                           double duration,
                           const char* exclude_clip_id);
uint32_t ocw_timeline_count_clips_in_range(OcTimeline* timeline, double start, double duration);

// Snapping
int ocw_timeline_snap(OcTimeline* timeline,
                      double target_time,
                      double playhead,
                      double threshold,
                      double* out_snapped_time,
                      double* out_delta);

// Time & Timecode
int64_t oc_time_ticks_per_second(void);
double ocw_time_from_seconds(double seconds);
double ocw_time_to_seconds(double ticks);
double ocw_time_round_to_frame(double ticks, uint32_t fps_num, uint32_t fps_den);
double ocw_time_floor_to_frame(double ticks, uint32_t fps_num, uint32_t fps_den);
double ocw_time_last_frame(double duration, uint32_t fps_num, uint32_t fps_den);
double ocw_time_snapped_seek(double time, double duration, uint32_t fps_num, uint32_t fps_den);
int ocw_time_format_timecode(double ticks, int format_type, uint32_t fps_num, uint32_t fps_den, char* out_buf, size_t out_len);
double ocw_time_parse_timecode(const char* timecode_str, int format_type, uint32_t fps_num, uint32_t fps_den);

// Audio
int ocw_audio_decimate_waveform(const float* samples, size_t num_samples, size_t num_bins, float* out_peaks);
float ocw_audio_evaluate_fade(double offset_ticks, double duration_ticks, double fade_in_ticks, double fade_out_ticks);

// Compositor
void ocw_compositor_blend(float b_r, float b_g, float b_b, float b_a,
                          float l_r, float l_g, float l_b, float l_a,
                          uint32_t blend_mode, float opacity, float* out_rgba);

// Masks & Effects
float ocw_mask_evaluate_alpha(float px, float py, int mask_type, float cx, float cy, float sx, float sy, float rot, float feather, int inverted);
void ocw_effects_apply(float r, float g, float b, float a,
                       float brightness, float contrast, float saturation, float exposure,
                       float temp, float tint, float hue, float gamma, float* out_rgba);

// Animation & Keyframes
double ocw_animation_solve_bezier(double time, double t0, double t1, double t2, double t3);
double ocw_animation_evaluate_bezier_point(double progress, double p0, double p1, double p2, double p3);
double ocw_animation_evaluate_channel(const double* key_times, const double* key_values, const int* key_interp, size_t num_keys, double eval_time);

// Speed Ramping
double ocw_speed_map_timeline_to_source(double timeline_offset, double timeline_duration,
                                        const double* speed_ratios, const double* speed_multipliers,
                                        size_t num_points, double constant_speed);

// Gap Search & Ripple Shift
double ocw_timeline_find_available_gap(OcTimeline* timeline, const char* track_id, double duration, double min_start);
int ocw_timeline_apply_ripple(OcTimeline* timeline, const char* track_id, double after_time, double delta_ticks);

// Scene Graph & Display List
typedef struct OcScene OcScene;
OcScene* oc_scene_create(void);
void oc_scene_destroy(OcScene* scene);
void ocw_scene_add_item(OcScene* scene, const char* id, uint32_t type, int32_t z_index,
                        float opacity, uint32_t blend_mode,
                        double cx, double cy, double w, double h, double rot,
                        int flip_x, int flip_y, const char* asset_id);
uint32_t ocw_scene_build_display_list(OcScene* scene, double viewport_w, double viewport_h, int enable_culling);

// Subtitles Engine
double ocw_subtitles_parse_timestamp(const char* ts);
int ocw_subtitles_format_timestamp(double ticks, char* out_buf, size_t out_len);

// Text Layout & Line Breaking
double ocw_text_measure_line_height(double font_size, double line_height_ratio);
int ocw_text_break_lines_count(const char* text, double max_width, double avg_char_width);

// Geometry & Hit-Testing
int ocw_geometry_point_in_rotated_rect(double px, double py, double cx, double cy, double w, double h, double rot);
int ocw_geometry_test_snap(double source_val, double target_val, double threshold, double* out_snapped, double* out_delta);

#ifdef __cplusplus
}
#endif
