# The render hardware interface: the small graphics api the renderers are
# written against (internal/rhi/), and one backend per graphics api that
# implements it:
#
#   private/d3d12/   DirectX 12, Windows only
#   private/vulkan/  Vulkan, every platform
#   private/opengl/  OpenGL 3.3 (and OpenGL ES 3.0), every platform
#   private/common/  what the backends share
#
# A build compiles only the backend BUZ_RHI names (see the root
# CMakeLists.txt); the other folders are left out. Every backend implements
# the same internal/rhi/ headers, so no source file asks which backend it is.
#
# The rhi also holds what every renderer shares: textures (and loading them
# from image files) and the compiled shaders. Game code only sees
# buz::texture (public/rhi/texture.h); the rest is for the renderers.
IF(BUZ_RHI STREQUAL "d3d12")
    SET(rhi_backend_libraries d3d12 dxgi)
    SET(rhi_excluded_backends "/private/vulkan/" "/private/opengl/")
ELSEIF(BUZ_RHI STREQUAL "vulkan")
    SET(rhi_backend_libraries volk vma)
    SET(rhi_excluded_backends "/private/d3d12/" "/private/opengl/")
ELSE()
    SET(rhi_backend_libraries opengl_headers)
    SET(rhi_excluded_backends "/private/d3d12/" "/private/vulkan/")
ENDIF()

BUZ_MODULE(rhi TIER tmpl_2d
           PUBLIC_DEPS buz_core yesno
           PRIVATE_DEPS SDL3::SDL3-static stb ${rhi_backend_libraries}
           EXCLUDE ${rhi_excluded_backends})
