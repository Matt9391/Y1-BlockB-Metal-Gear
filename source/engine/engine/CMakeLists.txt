# The main loop: owns every system, runs the game and draws its frames. Also
# the window, keyboard and mouse (on top of SDL3), sound effects and music
# (on top of miniaudio), and the game classes: game, game_2d, and game_3d
# for the 3D template.
BUZ_MODULE(engine
           PUBLIC_DEPS buz_renderer_2d yesno buz_renderer_3d
           PRIVATE_DEPS args buz_core_internal buz_renderer_2d_internal buz_rhi_internal SDL3::SDL3-static miniaudio stb)

IF(NOT WIN32)
    # miniaudio talks to the audio device from a thread of its own, and
    # loads the Linux audio libraries (ALSA, PulseAudio) at runtime.
    FIND_PACKAGE(Threads REQUIRED)
    TARGET_LINK_LIBRARIES(buz_engine PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
ENDIF()
