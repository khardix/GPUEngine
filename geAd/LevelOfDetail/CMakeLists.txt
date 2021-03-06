cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(LoDGenerator LANGUAGES CXX)

include(GNUInstallDirs)  # FIXME: Unify with rest of GPUE

# Ensure dependencies are available
if (NOT TARGET geSG)
    find_package(GPUEngine COMPONENTS geSG QUIET)
endif (NOT TARGET geSG)
list(APPEND external_deps geSG)

foreach (dep IN LISTS external_deps)
    if (NOT TARGET ${dep})
        return()
    endif (NOT TARGET ${dep})
endforeach (dep IN LISTS external_deps)

# Add the library
add_library(${PROJECT_NAME}
    src/graph/Triangle.cpp src/graph/Mesh.cpp src/graph/algorithm.cpp
    src/metric/qem.cpp
    src/operator/edge_collapse.cpp)
target_include_directories(${PROJECT_NAME}
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/geAd/${PROJECT_NAME}>
    PRIVATE src)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
# FIXME: Workaround geSG installation not using export sets.
# The BUILD_INTERFACE means that the library links with geSG, but not propagate the dependency to its users!
# The install process should be unified with other components.
target_link_libraries(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${external_deps}>)

# Add installation instructions
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ vendor/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/geAd/${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}Config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Add tests
add_executable(${PROJECT_NAME}TestSuite
    tests/main.cpp
    tests/test_utility_set_operation.cpp
    tests/test_graph_node.cpp tests/test_graph_edge.cpp tests/test_graph_mesh.cpp tests/test_graph_algorithm.cpp
    tests/test_metric_qem.cpp
    tests/test_operator_edge_collapse.cpp
    tests/test_algorithm_lazy_selection.cpp)
target_compile_features(${PROJECT_NAME}TestSuite PRIVATE cxx_std_14)
target_link_libraries(${PROJECT_NAME}TestSuite ${PROJECT_NAME})
add_test(test_lod_addon ${PROJECT_NAME}TestSuite)

# Benchmark
if (NOT TARGET AssimpModelLoader)
    find_package(AssimpModelLoader CONFIG QUIET HINTS "${CMAKE_CURRENT_LIST_DIR}/../AssimpModelLoader/cmake")
endif (NOT TARGET AssimpModelLoader)
if (NOT TARGET ste)
    find_package(ste)
endif (NOT TARGET ste)
if (NOT (TARGET AssimpModelLoader AND TARGET ste))
    return()
endif (NOT (TARGET AssimpModelLoader AND TARGET ste))

set(BENCHMARK_NAME ${PROJECT_NAME}Benchmark)
add_executable(${BENCHMARK_NAME} benchmark.cpp)
target_compile_features(${BENCHMARK_NAME} PRIVATE cxx_std_14)
target_link_libraries(${BENCHMARK_NAME} ${PROJECT_NAME} AssimpModelLoader ste)
