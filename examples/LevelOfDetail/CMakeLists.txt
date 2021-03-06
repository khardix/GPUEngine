cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(LevelOfDetail LANGUAGES CXX)

# Find dependencies

# GPUEngine dependency
if (NOT TARGET geGL)
    find_package(GPUEngine COMPONENTS geGL geSG QUIET)
endif (NOT TARGET geGL)
list(APPEND external_deps geGL geSG)

if (NOT TARGET AssimpModelLoader)
    find_package(AssimpModelLoader CONFIG QUIET HINTS "${CMAKE_CURRENT_LIST_DIR}/../../geAd/AssimpModelLoader/cmake")
endif (NOT TARGET AssimpModelLoader)
list(APPEND external_deps AssimpModelLoader)

# Requires building with -DGPUENGINE_BUILD_geAD=ON
list(APPEND external_deps LoDGenerator)

# Qt5
find_package(Qt5 COMPONENTS Qml Quick QUIET)
list(APPEND external_deps Qt5::Quick Qt5::Qml)

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

add_executable(${PROJECT_NAME}
    main.cpp gl_view.cpp scene_util.cpp simplified_scene.cpp visualization.cpp
    resources.qrc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
target_link_libraries(${PROJECT_NAME} ${external_deps})
