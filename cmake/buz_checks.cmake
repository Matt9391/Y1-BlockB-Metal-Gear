# -------------------------------
# Checks on the targets, run once at the end of the root CMakeLists.txt.
# -------------------------------

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
# Internal headers (the internal/ folder of a module, see
# cmake/buz_module.cmake) are for engine modules only: configuring fails
# when a game links a buz_<module>_internal target.
# -------------------------------
FUNCTION(BUZ_CHECK_INTERNAL_LINKS)
    BUZ_COLLECT_TARGETS(${CMAKE_SOURCE_DIR} all_targets)
    FOREACH(target ${all_targets})
        GET_TARGET_PROPERTY(is_module ${target} BUZ_MODULE)
        IF(is_module OR target MATCHES "_internal$")
            CONTINUE()
        ENDIF()
        GET_TARGET_PROPERTY(links ${target} LINK_LIBRARIES)
        GET_TARGET_PROPERTY(interface_links ${target} INTERFACE_LINK_LIBRARIES)
        FOREACH(dependency ${links} ${interface_links})
            IF(NOT TARGET ${dependency})
                CONTINUE()
            ENDIF()
            GET_TARGET_PROPERTY(dependency_internal ${dependency} BUZ_INTERNAL)
            IF(dependency_internal)
                MESSAGE(FATAL_ERROR "'${target}' links '${dependency}': internal headers are for engine "
                                    "modules only. A game uses the public headers of a module.")
            ENDIF()
        ENDFOREACH()
    ENDFOREACH()
ENDFUNCTION()
