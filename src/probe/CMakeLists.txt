add_library(
    logstore_probe
    OBJECT
    gc_lifespan.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_probe>
        PARENT_SCOPE)