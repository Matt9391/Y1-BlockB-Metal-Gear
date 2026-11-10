# 3D drawing (Block B), next to renderer_2d on the rhi: the game's
# world_renderer_3d draws meshes with a simple material, primitive shapes,
# the PS1 look, debug views and debug lines into the world pass. Also glTF
# import: meshes, whole models with their node hierarchy (levels, and
# characters made of rigid parts) and their animations, sampled into poses.
# Playing those poses back is the students' job.
BUZ_MODULE(renderer_3d TIER tmpl_3d
           PUBLIC_DEPS buz_core buz_rhi yesno
           PRIVATE_DEPS buz_core_internal buz_engine buz_rhi_internal cgltf)
