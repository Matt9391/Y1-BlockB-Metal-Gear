# -------------------------------
# Macro that will define our source groups
#
# Visual studio shows every project as one flat list of files unless the build
# system assigns each file to a "filter". This macro walks a folder tree and
# mirrors it as filters, so the project layout in the IDE matches the layout
# on disk.
# -------------------------------
MACRO(GROUPSOURCES curdir folder_name)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    FOREACH(child ${children})
           IF(IS_DIRECTORY ${curdir}/${child})
              GROUPSOURCES(${curdir}/${child} ${folder_name}/${child})
           ELSE()
              SOURCE_GROUP(${folder_name} FILES ${curdir}/${child})
           ENDIF()
    ENDFOREACH()
ENDMACRO()
