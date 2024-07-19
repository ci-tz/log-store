add_library(
    logstore_frame
    OBJECT
    segment.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_frame>
        PARENT_SCOPE)