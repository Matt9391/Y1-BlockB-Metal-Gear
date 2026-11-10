# -------------------------------
# Tiers: which student template a target ships in.
#
# This repository is the complete engine used by staff. The student templates
# are exported from it by tools/export_tmpl.py, and every first-party target
# says which templates it belongs to:
#
#   tmpl_2d  - Block A template (and everything above it)
#   tmpl_3d  - Block B template (and staff)
#   staff    - never exported: solved exercises, reference games, demos
#
# The order matters: a target may only link targets of its own tier or a
# lower one. A tmpl_2d engine module that links a staff module would drag the
# solution into the student template, so BUZ_CHECK_TIERS refuses to configure
# when that happens. The export tool reads the same BUZ_TIER lines to decide
# which folders a template gets.
#
# Usage, right after creating the target:
#   BUZ_TIER(sample_2d tmpl_2d)
# -------------------------------
SET(BUZ_TIERS tmpl_2d tmpl_3d staff)

FUNCTION(BUZ_TIER target tier)
    LIST(FIND BUZ_TIERS ${tier} tier_index)
    IF(tier_index EQUAL -1)
        MESSAGE(FATAL_ERROR "BUZ_TIER(${target} ${tier}): unknown tier, use one of: ${BUZ_TIERS}")
    ENDIF()

    SET_PROPERTY(TARGET ${target} PROPERTY BUZ_TIER ${tier})
    SET_PROPERTY(GLOBAL APPEND PROPERTY BUZ_TIERED_TARGETS ${target})
ENDFUNCTION()

# -------------------------------
# Collects every target defined in a directory and its subdirectories.
# -------------------------------
FUNCTION(BUZ_COLLECT_TARGETS directory out_var)
    GET_PROPERTY(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    GET_PROPERTY(subdirectories DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    FOREACH(subdirectory ${subdirectories})
        BUZ_COLLECT_TARGETS(${subdirectory} sub_targets)
        LIST(APPEND targets ${sub_targets})
    ENDFOREACH()
    SET(${out_var} ${targets} PARENT_SCOPE)
ENDFUNCTION()

# -------------------------------
# Called once at the end of the root CMakeLists.txt.
#
# 1. Every first-party target (anything under source/, except third_party)
#    must declare a tier, so nothing ends up in a template by accident.
# 2. No target may link a target of a higher tier.
# -------------------------------
FUNCTION(BUZ_CHECK_TIERS)
    BUZ_COLLECT_TARGETS(${CMAKE_SOURCE_DIR} all_targets)

    FOREACH(target ${all_targets})
        GET_TARGET_PROPERTY(source_dir ${target} SOURCE_DIR)
        GET_TARGET_PROPERTY(target_type ${target} TYPE)
        IF(target_type STREQUAL "UTILITY")
            CONTINUE()
        ENDIF()
        IF(NOT source_dir MATCHES "^${CMAKE_SOURCE_DIR}/source/" OR source_dir MATCHES "/third_party(/|$)")
            CONTINUE()
        ENDIF()

        GET_TARGET_PROPERTY(tier ${target} BUZ_TIER)
        IF(NOT tier)
            MESSAGE(FATAL_ERROR "Target '${target}' (${source_dir}) has no tier. Add BUZ_TIER(${target} <tier>) with one of: ${BUZ_TIERS}")
        ENDIF()
    ENDFOREACH()

    GET_PROPERTY(tiered_targets GLOBAL PROPERTY BUZ_TIERED_TARGETS)
    FOREACH(target ${tiered_targets})
        GET_TARGET_PROPERTY(tier ${target} BUZ_TIER)
        LIST(FIND BUZ_TIERS ${tier} tier_index)

        GET_TARGET_PROPERTY(links ${target} LINK_LIBRARIES)
        GET_TARGET_PROPERTY(interface_links ${target} INTERFACE_LINK_LIBRARIES)
        FOREACH(dependency ${links} ${interface_links})
            IF(NOT TARGET ${dependency})
                CONTINUE()
            ENDIF()
            GET_TARGET_PROPERTY(dependency_tier ${dependency} BUZ_TIER)
            IF(NOT dependency_tier)
                CONTINUE()
            ENDIF()
            # Internal headers are for engine modules only
            GET_TARGET_PROPERTY(dependency_internal ${dependency} BUZ_INTERNAL)
            GET_TARGET_PROPERTY(is_module ${target} BUZ_MODULE)
            IF(dependency_internal AND NOT is_module AND NOT target MATCHES "_internal$")
                MESSAGE(FATAL_ERROR "'${target}' links '${dependency}': internal headers are for engine "
                                    "modules only. A game uses the public headers of a module.")
            ENDIF()

            LIST(FIND BUZ_TIERS ${dependency_tier} dependency_index)
            IF(dependency_index GREATER tier_index)
                MESSAGE(FATAL_ERROR "'${target}' (tier ${tier}) links '${dependency}' (tier ${dependency_tier}). "
                                    "A target may only depend on its own tier or a lower one, otherwise "
                                    "${dependency_tier} code would end up in the ${tier} template.")
            ENDIF()
        ENDFOREACH()
    ENDFOREACH()
ENDFUNCTION()
