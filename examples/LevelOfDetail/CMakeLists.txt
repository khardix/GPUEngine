cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(LevelOfDetail LANGUAGES CXX)

# Find dependencies

# GPUEngine dependency
if (NOT TARGET geGL)
    find_package(GPUEngine COMPONENTS geGL QUIET)
endif (NOT TARGET geGL)
list(APPEND external_deps geGL)

# Qt5
find_package(Qt5 COMPONENTS Quick QUIET)
list(APPEND external_deps Qt5::Quick)

# Proceed only if all dependencies were found
foreach (dep IN LISTS external_deps)
    if (NOT TARGET ${dep})
        return()
    endif (NOT TARGET ${dep})
endforeach (dep IN LISTS external_deps)

# Declare targets

# Automatically process Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME} main.cc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
target_link_libraries(${PROJECT_NAME} ${external_deps})
