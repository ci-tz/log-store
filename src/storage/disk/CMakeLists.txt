add_library(
    logstore_disk
    OBJECT
    disk_manager_memory.cpp
    disk_manager.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_disk>
        PARENT_SCOPE)