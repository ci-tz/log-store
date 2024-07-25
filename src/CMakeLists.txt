add_subdirectory(index)
add_subdirectory(segment)
# add_subdirectory(select)
add_subdirectory(storage)


add_library(logstore STATIC ${ALL_OBJECT_FILES})

set(LOGSTORE_LIBS
        logstore_frame
        logstore_index
        logstore_select
        logstore_disk_manager)

find_package(Threads REQUIRED)

set(LEARN_CMAKE_THIRDPARTY_LIBS
        Threads::Threads)

target_link_libraries(
        logstore 
        ${LEARN_CMAKE_LIBS} 
        ${LEARN_CMAKE_THIRDPARTY_LIBS})

target_include_directories(
        logstore PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)