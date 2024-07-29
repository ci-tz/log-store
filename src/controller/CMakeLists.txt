add_library(
    logstore_controller
    OBJECT
    controller.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_controller>
        PARENT_SCOPE)