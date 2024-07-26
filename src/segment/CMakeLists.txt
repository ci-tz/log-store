add_library(
    logstore_segment
    OBJECT
    segment.cpp
    segment_manager.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_segment>
        PARENT_SCOPE)