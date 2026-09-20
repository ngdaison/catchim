add_executable(catchim_tests
    catchim_tests.cpp
)
target_link_libraries(catchim_tests PRIVATE
    catchim_core
    catchim_editor
    catchim_media
    catchim_subtitles
    catchim_storage
    catchim_render
    catchim_audio
    catchim_export
)
add_test(NAME CatchimCoreTests COMMAND catchim_tests)
