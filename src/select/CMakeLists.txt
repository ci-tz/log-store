add_library(
    logstore_select
    OBJECT
    costbenefit_selection.cpp
    greedy_selection.cpp
)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:logstore_select>
        PARENT_SCOPE)