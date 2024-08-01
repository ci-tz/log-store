add_library(
    logstore_framework
    OBJECT
    segment.cpp
    segment_manager.cpp
    controller.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_framework>
        PARENT_SCOPE)