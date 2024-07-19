add_library(
    logstore_index
    OBJECT
    array_indexmap.cpp
    hash_indexmap.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_index>
        PARENT_SCOPE)