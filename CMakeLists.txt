cmake_minimum_required(VERSION 3.15)
project(word_count)
set(CMAKE_CXX_STANDARD 17)
find_package(LibArchive REQUIRED)
#find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS
        locale filesystem system)

set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
set(Boost_USE_MULTITHREAD ON)  # enable multithreading
# filesystem -- на майбутнє
include_directories(${ICU_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LibArchive_INCLUDE_DIRS})
add_executable(word_count src/main.cpp inc/t_queue.h inc/time.hpp)
target_link_libraries(word_count Threads::Threads ${Boost_LIBRARIES} ${ICU_LIBRARIES} ${LibArchive_LIBRARIES})