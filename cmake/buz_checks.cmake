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

# -------------------------------
# An ENGINE_ONLY module (the rhi, see cmake/buz_module.cmake) is for other
# engine modules only: configuring fails when a game can include its
# headers. That is the case when the game links it, or links a module
# whose public links lead to it. A private link of a static library still
# puts it on the linker's command line ($<LINK_ONLY:...>), but no headers
# come along, so the walk stops there.
# -------------------------------
FUNCTION(BUZ_CHECK_ENGINE_ONLY)
    BUZ_COLLECT_TARGETS(${CMAKE_SOURCE_DIR} all_targets)
    FOREACH(target ${all_targets})
        GET_TARGET_PROPERTY(is_module ${target} BUZ_MODULE)
        GET_TARGET_PROPERTY(is_internal ${target} BUZ_INTERNAL)
        IF(is_module OR is_internal)
            CONTINUE()
        ENDIF()

        # Each entry is "<dependency>|<the target that links it>", for the message
        GET_TARGET_PROPERTY(links ${target} LINK_LIBRARIES)
        SET(to_visit "")
        FOREACH(dependency ${links})
            LIST(APPEND to_visit "${dependency}|${target}")
        ENDFOREACH()
        SET(visited "")
        WHILE(to_visit)
            LIST(POP_FRONT to_visit entry)
            STRING(REPLACE "|" ";" entry "${entry}")
            LIST(GET entry 0 dependency)
            LIST(GET entry 1 linked_by)
            IF(NOT TARGET ${dependency} OR dependency IN_LIST visited)
                CONTINUE()
            ENDIF()
            LIST(APPEND visited ${dependency})

            GET_TARGET_PROPERTY(engine_only ${dependency} BUZ_ENGINE_ONLY)
            IF(engine_only)
                IF(linked_by STREQUAL target)
                    SET(route "links '${dependency}'")
                ELSE()
                    SET(route "reaches '${dependency}' through '${linked_by}', which links it publicly")
                ENDIF()
                MESSAGE(FATAL_ERROR "'${target}' ${route}, but that module is for the engine only. Link it "
                                    "privately in the engine modules, and give games the types they need in a "
                                    "public module.")
            ENDIF()

            GET_TARGET_PROPERTY(interface_links ${dependency} INTERFACE_LINK_LIBRARIES)
            FOREACH(next ${interface_links})
                IF(NOT next MATCHES "^\\$<LINK_ONLY:")
                    LIST(APPEND to_visit "${next}|${dependency}")
                ENDIF()
            ENDFOREACH()
        ENDWHILE()
    ENDFOREACH()
ENDFUNCTION()
