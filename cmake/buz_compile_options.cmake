# -------------------------------
# BUZ_COMPILE_OPTIONS: the compiler settings every target of buz's own code
# gets (modules, games, tests, tools); third-party code keeps its own.
#
# - No run-time type information (docs/conventions.md, Classes): no typeid,
#   dynamic_cast or std::any, on every compiler.
# - Warnings are errors (docs/conventions.md, Builds and platforms): a
#   warning is fixed where it starts, so the build stays clean. GCC's
#   missing-field-initializers is off: it warns about every designated
#   initializer that leaves a member out ({ .window_title = "my game" }),
#   although C++ gives every member left out its default.
# -------------------------------
FUNCTION(BUZ_COMPILE_OPTIONS target)
    IF(MSVC)
        TARGET_COMPILE_OPTIONS(${target} PRIVATE /GR- /W4 /WX)
    ELSE()
        TARGET_COMPILE_OPTIONS(${target} PRIVATE -fno-rtti -Wall -Wextra -Werror -Wno-missing-field-initializers)
    ENDIF()
ENDFUNCTION()
