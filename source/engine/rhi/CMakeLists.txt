# The render hardware interface: the small graphics api the renderers are
# written against (internal/rhi/), and one backend per graphics api that
# implements it:
#
#   private/d3d12/   DirectX 12, Windows only
#   private/vulkan/  Vulkan, Windows and Linux
#   private/opengl/  OpenGL 3.3 (and OpenGL ES 3.0), Windows, Linux and the browser
#   private/common/  what the backends share
#
# A build compiles only the backend BUZ_RHI names (see the root
# CMakeLists.txt); the other folders are left out. Every backend implements
# the same internal/rhi/ headers, so the code that uses them compiles
# unchanged against any backend.
#
# The rhi depends on nothing above it: settings come in as its own types
# (rhi/settings.h), and a gpu_texture does not know the buz::texture that
# owns it (render_core).
#
# Games never see any of it: the engine modules link the rhi privately, and
# ENGINE_ONLY makes configuring fail when a game reaches it anyway, through
# its own links or a module's public ones.
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

BUZ_MODULE(rhi ENGINE_ONLY
           PUBLIC_DEPS buz_core yesno
           PRIVATE_DEPS SDL3::SDL3-static ${rhi_backend_libraries}
           EXCLUDE ${rhi_excluded_backends})
