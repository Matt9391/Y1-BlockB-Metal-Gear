# The Block B starter: a small 3D scene to build a game on.
BUZ_GAME(metal_gear TIER tmpl_3d
    LINKS buz_engine buz_renderer_3d)

# allow to include "camera/camera.h" without the need of going back to one level "../camera/camera.h"
TARGET_INCLUDE_DIRECTORIES(metal_gear PRIVATE ${CMAKE_CURRENT_LIST_DIR}/private)