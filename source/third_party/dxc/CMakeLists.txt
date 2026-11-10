# dxc is vendored as the prebuilt release from
# https://github.com/microsoft/DirectXShaderCompiler/releases (v1.9.2609) so
# every machine (and CI) compiles shaders with the exact same compiler,
# instead of whichever version happens to be installed.
#
# One folder per platform, from the same release:
#   windows/  inc/, lib/x64/dxcompiler.lib (import library), bin/x64/*.dll
#   linux/    inc/ (plus WinAdapter.h, the COM stand-in dxcapi.h needs there),
#             lib/*.so
#
# At runtime the shader_compiler tool needs dxcompiler (the actual compiler)
# and dxil (signs DXIL bytecode so DirectX accepts it) next to its
# executable; targets linking against dxc copy them there, see
# DXC_RUNTIME_LIBRARIES.

IF(WIN32)
    SET(DXC_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/windows)
    SET(DXC_IMPORT_LIBRARY ${DXC_DIRECTORY}/lib/x64/dxcompiler.lib)
    SET(DXC_COMPILER_LIBRARY ${DXC_DIRECTORY}/bin/x64/dxcompiler.dll)
    SET(DXC_DXIL_LIBRARY ${DXC_DIRECTORY}/bin/x64/dxil.dll)
ELSE()
    SET(DXC_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/linux)
    SET(DXC_COMPILER_LIBRARY ${DXC_DIRECTORY}/lib/libdxcompiler.so)
    SET(DXC_DXIL_LIBRARY ${DXC_DIRECTORY}/lib/libdxil.so)
ENDIF()

SET(DXC_INCLUDE_DIRECTORY ${DXC_DIRECTORY}/inc)

FOREACH(DXC_FILE ${DXC_INCLUDE_DIRECTORY}/dxcapi.h ${DXC_IMPORT_LIBRARY} ${DXC_COMPILER_LIBRARY} ${DXC_DXIL_LIBRARY})
    IF(NOT EXISTS ${DXC_FILE})
        MESSAGE(FATAL_ERROR "Missing ${DXC_FILE}. Extract the dxc release from https://github.com/microsoft/DirectXShaderCompiler/releases into ${DXC_DIRECTORY}.")
    ENDIF()
ENDFOREACH()

ADD_LIBRARY(dxc SHARED IMPORTED GLOBAL)
SET_TARGET_PROPERTIES(dxc PROPERTIES
    IMPORTED_LOCATION ${DXC_COMPILER_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${DXC_INCLUDE_DIRECTORY})

IF(WIN32)
    SET_TARGET_PROPERTIES(dxc PROPERTIES IMPORTED_IMPLIB ${DXC_IMPORT_LIBRARY})
ENDIF()

SET(DXC_RUNTIME_LIBRARIES ${DXC_COMPILER_LIBRARY} ${DXC_DXIL_LIBRARY} CACHE INTERNAL "dxc libraries to copy next to executables using dxc")
