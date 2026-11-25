# Loading assets from the content folder: textures, and for the 3D template
# models and meshes. Loading depends on the engine because loaded assets are
# uploaded to the graphics card the engine owns; reading a glTF file onto the
# cpu (import_model) does not. Every call loads the file again: keeping what
# was loaded and reusing it is up to the game.
BUZ_MODULE(asset_management
           PUBLIC_DEPS buz_engine buz_renderer_3d
           PRIVATE_DEPS buz_core_internal buz_render_core_internal buz_rhi_internal buz_renderer_3d_internal cgltf)
