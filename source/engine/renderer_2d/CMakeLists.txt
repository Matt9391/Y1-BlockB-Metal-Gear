# 2D drawing: sprites, rectangles, lines and text. The world_renderer_2d draws
# the game world through a camera, the screen_renderer draws the hud on top
# in plain window pixels. It draws with the rhi, through render_core, which
# the 3D renderer shares; games only see render_core's public types.
BUZ_MODULE(renderer_2d
           PUBLIC_DEPS buz_core buz_render_core
           PRIVATE_DEPS buz_core_internal
           INTERNAL_DEPS buz_render_core_internal buz_rhi_internal)
