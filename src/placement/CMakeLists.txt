add_library(
    logstore_placement
    OBJECT
    sepgc.cpp
    no_placement.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_placement>
        PARENT_SCOPE)