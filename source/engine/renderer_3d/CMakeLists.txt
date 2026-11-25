# 3D drawing, next to renderer_2d on the rhi: the game's
# world_renderer_3d draws meshes with a simple material, primitive shapes,
# debug views and debug lines into the world pass; a game can swap the
# shaders it draws meshes with (surface_shading.h). The model types
# live here too (nodes, materials, animations sampled into poses); reading
# them from glTF files is asset_management's job, and playing the poses back
# is up to the game.
BUZ_MODULE(renderer_3d
           PUBLIC_DEPS buz_core buz_render_core yesno
           PRIVATE_DEPS buz_core_internal buz_render_core_internal buz_rhi_internal)
