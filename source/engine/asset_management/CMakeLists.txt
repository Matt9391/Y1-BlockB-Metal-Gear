# Loading assets from the content folder: textures, and for the 3D template
# models and meshes. Built on the engine, whose graphics card gets what is
# loaded. The asset cache students build sits on top of it.
BUZ_MODULE(asset_management
           PUBLIC_DEPS buz_engine buz_renderer_3d
           PRIVATE_DEPS buz_core_internal buz_rhi_internal buz_renderer_3d_internal)
