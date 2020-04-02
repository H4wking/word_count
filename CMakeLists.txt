cmake_minimum_required(VERSION 3.10)
project(word_count)
set(CMAKE_CXX_STANDARD 14)
set(LibZip_LIBRARIES "")
include(CheckLibraryExists)
SET(LibZip_LIBRARY_PATH "")
check_library_exists(zip zip_open_from_source
        "${LibZip_LIBRARY_PATH}" LibZip_FOUND)
if (LibZip_FOUND)
    message("Found LibZip")
    list(APPEND LibZip_LIBRARIES "zip")
    link_directories(${LibZip_LIBRARY_PATH})
endif ()
find_package(LibArchive REQUIRED)
#find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Threads REQUIRED)
#add_subdirectory("path-to-boost-nowide-repo")
#find_package(boost_nowide)
find_package(Boost REQUIRED COMPONENTS
        locale filesystem system)

set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
set(Boost_USE_MULTITHREAD ON)  # enable multithreading
# filesystem -- на майбутнє
include_directories(${Boost_INCLUDE_DIRS})
add_executable(word_count main.cpp)
target_link_libraries(word_count ${Boost_LIBRARIES} ${ICU_LIBRARIES} ${LibArchive_LIBRARIES})