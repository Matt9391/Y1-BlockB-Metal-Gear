# 2D drawing: sprites, rectangles, lines and text. The world_renderer draws
# the game world through a camera, the screen_renderer draws the hud on top
# in plain window pixels. Textures and shaders come from the rhi, which the
# 3D renderer shares.
BUZ_MODULE(renderer_2d TIER tmpl_2d
           PUBLIC_DEPS buz_core buz_rhi
           PRIVATE_DEPS buz_core_internal
           INTERNAL_DEPS buz_rhi_internal)
