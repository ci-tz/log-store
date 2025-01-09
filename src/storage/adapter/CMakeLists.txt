add_library(
    logstore_adapter
    OBJECT
    memory_adapter.cpp
    local_adapter.cpp
    no_adapter.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_adapter>
        PARENT_SCOPE)