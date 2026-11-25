# The main loop: owns every system, runs the game and draws its frames. Also
# the window, keyboard and mouse (on top of SDL3), and sound effects and
# music (on top of miniaudio). The engine runs anything that implements
# buz::playable; the game classes that do are in the game_framework module,
# on top of this one.
BUZ_MODULE(engine
           PUBLIC_DEPS buz_render_core yesno
           PRIVATE_DEPS args buz_core_internal buz_render_core_internal buz_renderer_2d buz_renderer_2d_internal buz_rhi_internal SDL3::SDL3-static miniaudio stb)
