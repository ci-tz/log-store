add_library(
    logstore_framework
    OBJECT
    segment.cpp
    phy_segment.cpp
    segment_lists.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_framework>
        PARENT_SCOPE)