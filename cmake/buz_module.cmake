# -------------------------------
# BUZ_MODULE: one engine module, one static library.
#
# Every module lives in its own folder under source/engine:
#
#   source/engine/<name>/public/<name>/*.h   what other modules and games include,
#                                            as #include "<name>/file.h"
#   source/engine/<name>/internal/<name>/*.h what only other engine modules include:
#                                            the engine's side of a class games use
#                                            (optional; see "internal" below)
#   source/engine/<name>/private/...         the implementation
#
# and its CMakeLists.txt is a single call:
#
#   BUZ_MODULE(renderer_2d TIER tmpl_2d
#              PUBLIC_DEPS buz_core buz_rhi
#              PRIVATE_DEPS stb)
#
# which creates the target buz_<name>, picks up every file in the folder,
# and declares its tier (see cmake/buz_tiers.cmake). PUBLIC_DEPS are the
# libraries whose headers the module's public headers include; everything
# else is a PRIVATE_DEP, so it stays out of the code that uses the module.
#
# internal: a module with an internal/ folder also gets the target
# buz_<name>_internal. Engine modules that need those headers list it in
# their PRIVATE_DEPS; games never may (BUZ_CHECK_TIERS fails if one does).
# INTERNAL_DEPS are what the internal headers themselves include (other
# modules' _internal targets, usually); they come along with the target.
# This keeps engine plumbing out of reach of game code, like Unreal's
# Internal folders: a header is a promise, and games get the public one.
# -------------------------------
FUNCTION(BUZ_MODULE name)
    CMAKE_PARSE_ARGUMENTS(MODULE "" "TIER" "PUBLIC_DEPS;PRIVATE_DEPS;INTERNAL_DEPS;EXCLUDE" ${ARGN})
    IF(NOT MODULE_TIER)
        MESSAGE(FATAL_ERROR "BUZ_MODULE(${name}) needs a TIER")
    ENDIF()

    SET(target buz_${name})
    MESSAGE(STATUS "Adding module - ${target}")
    ADD_LIBRARY(${target} STATIC)

    FILE(GLOB_RECURSE module_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_LIST_DIR}/public/*.h"
         "${CMAKE_CURRENT_LIST_DIR}/internal/*.h"
         "${CMAKE_CURRENT_LIST_DIR}/private/*.h"
         "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp")

    # Files that only build on some platforms (a graphics backend, say) are
    # left out with EXCLUDE regular expressions.
    FOREACH(pattern ${MODULE_EXCLUDE})
        LIST(FILTER module_sources EXCLUDE REGEX "${pattern}")
    ENDFOREACH()

    TARGET_SOURCES(${target} PRIVATE ${module_sources})
    GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/public public)
    IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/private)
        GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)
    ENDIF()

    TARGET_INCLUDE_DIRECTORIES(${target}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/public
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/private)

    IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/internal)
        GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/internal internal)
        TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/internal)

        ADD_LIBRARY(${target}_internal INTERFACE)
        TARGET_INCLUDE_DIRECTORIES(${target}_internal INTERFACE ${CMAKE_CURRENT_LIST_DIR}/internal)
        TARGET_LINK_LIBRARIES(${target}_internal INTERFACE ${target} ${MODULE_INTERNAL_DEPS})
        SET_PROPERTY(TARGET ${target}_internal PROPERTY BUZ_INTERNAL TRUE)
        BUZ_TIER(${target}_internal ${MODULE_TIER})
    ENDIF()

    BUZ_COMPILE_OPTIONS(${target})

    TARGET_LINK_LIBRARIES(${target}
        PUBLIC ${MODULE_PUBLIC_DEPS}
        PRIVATE ${MODULE_PRIVATE_DEPS} ${MODULE_INTERNAL_DEPS})

    # Staff modules sit in a folder of their own in the IDE, so the engine
    # folder shows exactly what a student template has.
    IF(MODULE_TIER STREQUAL "staff")
        SET_TARGET_PROPERTIES(${target} PROPERTIES FOLDER "engine/staff")
    ELSE()
        SET_TARGET_PROPERTIES(${target} PROPERTIES FOLDER "engine")
    ENDIF()
    SET_PROPERTY(TARGET ${target} PROPERTY BUZ_MODULE TRUE)
    BUZ_TIER(${target} ${MODULE_TIER})
ENDFUNCTION()
