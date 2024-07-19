add_library(
    logstore_select
    OBJECT
    costbenefit_select_segment.cpp
    greedy_select_segment.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_select>
        PARENT_SCOPE)