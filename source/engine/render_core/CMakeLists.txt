# What both renderers share, on top of the rhi: buz::texture (the game's
# handle to an image in gpu memory, which owns the rhi's gpu_texture),
# loading textures from image files, the compiled shaders, and the
# render_context that hands the renderers the device, the shaders and the
# texture loader. Games see buz::texture and texture_filter; the rhi stays
# private.
BUZ_MODULE(render_core
           PUBLIC_DEPS buz_core
           PRIVATE_DEPS buz_core_internal stb
           INTERNAL_DEPS buz_rhi_internal)
