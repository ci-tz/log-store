add_subdirectory(index)
add_subdirectory(framework)
add_subdirectory(select)
add_subdirectory(storage)


add_library(logstore STATIC ${ALL_OBJECT_FILES})

set(LOGSTORE_LIBS
        logstore_index
        logstore_framework
        logstore_select
        logstore_adapter
        logstore_disk)

find_package(Threads REQUIRED)

set(LOGSTORE_THIRDPARTY_LIBS
        Threads::Threads)

target_link_libraries(
        logstore 
        ${LOGSTORE_LIBS} 
        ${LOGSTORE_THIRDPARTY_LIBS})

target_include_directories(
        logstore PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)