cmake_minimum_required(VERSION 3.22)

project(opencut_core LANGUAGES CXX)

set(OPENCUT_CORE_SOURCES
    src/animation.cpp
    src/audio.cpp
    src/c_api.cpp
    src/compositor.cpp
    src/effects.cpp
    src/geometry.cpp
    src/masks.cpp
    src/scene.cpp
    src/speed.cpp
    src/subtitles.cpp
    src/text_layout.cpp
    src/time.cpp
    src/timeline.cpp
)

add_library(opencut_core STATIC
    ${OPENCUT_CORE_SOURCES}
)

target_compile_features(opencut_core PUBLIC cxx_std_20)

target_include_directories(opencut_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(opencut_core PRIVATE /W4 /permissive- /O2)
else()
    target_compile_options(opencut_core PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()

# Tests
enable_testing()

macro(add_opencut_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE opencut_core)
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

add_opencut_test(opencut_core_timeline_tests tests/timeline_tests.cpp)
add_opencut_test(opencut_core_time_tests tests/time_tests.cpp)
add_opencut_test(opencut_core_snapping_tests tests/snapping_tests.cpp)
add_opencut_test(opencut_core_compositor_tests tests/compositor_tests.cpp)
add_opencut_test(opencut_core_masks_tests tests/masks_tests.cpp)
add_opencut_test(opencut_core_effects_tests tests/effects_tests.cpp)
add_opencut_test(opencut_core_audio_tests tests/audio_tests.cpp)
add_opencut_test(opencut_core_animation_tests tests/animation_tests.cpp)
add_opencut_test(opencut_core_speed_tests tests/speed_tests.cpp)
add_opencut_test(opencut_core_scene_tests tests/scene_tests.cpp)
add_opencut_test(opencut_core_subtitles_tests tests/subtitles_tests.cpp)
add_opencut_test(opencut_core_text_layout_tests tests/text_layout_tests.cpp)
add_opencut_test(opencut_core_geometry_tests tests/geometry_tests.cpp)

if(EMSCRIPTEN)
    add_executable(opencut_core_wasm
        ${OPENCUT_CORE_SOURCES}
    )

    target_compile_features(opencut_core_wasm PUBLIC cxx_std_20)
    target_include_directories(opencut_core_wasm
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    set_target_properties(opencut_core_wasm PROPERTIES
        OUTPUT_NAME "opencut_core"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../apps/web/public/wasm"
    )

    target_link_options(opencut_core_wasm PRIVATE
        "--no-entry"
        "-O3"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createOpenCutCoreModule"
        "-sENVIRONMENT=web,worker"
        "-sFILESYSTEM=0"
        "-sSINGLE_FILE=1"
        "-sNO_EXIT_RUNTIME=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_RUNTIME_METHODS=['cwrap','ccall','UTF8ToString','stringToUTF8']"
        "-sEXPORTED_FUNCTIONS=['_malloc','_free','_oc_timeline_create','_oc_timeline_destroy','_oc_timeline_add_track','_ocw_timeline_insert_clip','_ocw_timeline_delete_clip','_ocw_timeline_move_clip','_ocw_timeline_trim_clip','_ocw_timeline_split_clip','_ocw_timeline_can_place','_ocw_timeline_count_clips_in_range','_ocw_timeline_snap','_oc_time_ticks_per_second','_ocw_time_from_seconds','_ocw_time_to_seconds','_ocw_time_round_to_frame','_ocw_time_floor_to_frame','_ocw_time_last_frame','_ocw_time_snapped_seek','_ocw_time_format_timecode','_ocw_time_parse_timecode','_ocw_audio_decimate_waveform','_ocw_audio_evaluate_fade','_ocw_compositor_blend','_ocw_mask_evaluate_alpha','_ocw_effects_apply','_ocw_animation_solve_bezier','_ocw_animation_evaluate_bezier_point','_ocw_animation_evaluate_channel','_ocw_speed_map_timeline_to_source','_ocw_timeline_find_available_gap','_ocw_timeline_apply_ripple','_oc_scene_create','_oc_scene_destroy','_ocw_scene_add_item','_ocw_scene_build_display_list','_ocw_subtitles_parse_timestamp','_ocw_subtitles_format_timestamp','_ocw_text_measure_line_height','_ocw_text_break_lines_count','_ocw_geometry_point_in_rotated_rect','_ocw_geometry_test_snap']"
    )
endif()
