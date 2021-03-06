set(LIB_NAME geRG)
set(LIB_NAME_UPPER)
string(TOUPPER ${LIB_NAME} LIB_NAME_UPPER)

if(GPUENGINE_DYNAMIC_LINKAGE)
   set(GPUENGINE_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
   add_definitions(-D"${LIB_NAME_UPPER}"_LIBRARY)
else()
   set(GPUENGINE_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
   add_definitions(-DGE_LIBRARY_STATIC)
endif()

set(HEADER_PATH ${GPUEngine_SOURCE_DIR}/include/${LIB_NAME})
set(RG_INCLUDES
    ${HEADER_PATH}/Export.h
    ${HEADER_PATH}/Basics.h
    ${HEADER_PATH}/ParentChildList.h
    ${HEADER_PATH}/AllocationManagers.h
    ${HEADER_PATH}/BufferStorage.h
    ${HEADER_PATH}/FlexibleArrayList.h
    ${HEADER_PATH}/AttribType.h
    ${HEADER_PATH}/AttribConfig.h
    ${HEADER_PATH}/AttribStorage.h
    ${HEADER_PATH}/Mesh.h
    ${HEADER_PATH}/Primitive.h
    ${HEADER_PATH}/Drawable.h
    ${HEADER_PATH}/DrawCommand.h
    ${HEADER_PATH}/RenderingContext.h
    ${HEADER_PATH}/StateSet.h
    ${HEADER_PATH}/StateSetManager.h
    ${HEADER_PATH}/Transformation.h
    ${HEADER_PATH}/MatrixList.h
    ${HEADER_PATH}/MatrixGpuData.h
    ${HEADER_PATH}/FlexibleUniform.h
    ${HEADER_PATH}/Model.h
    ${HEADER_PATH}/ProgressStamp.h
  )

set(RG_SOURCES
    Basics.cpp
    AllocationManagers.cpp
    AttribType.cpp
    AttribConfig.cpp
    AttribStorage.cpp
    Primitive.cpp
    DrawCommand.cpp
    RenderingContext.cpp
    StateSet.cpp
    StateSetManager.cpp
    Transformation.cpp
    MatrixList.cpp
    FlexibleUniform.cpp
    ProgressStamp.cpp
  )

################################################
# PACKAGES

find_package(glm    REQUIRED)


add_library(${LIB_NAME}
    ${GPUENGINE_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${RG_INCLUDES}
    ${RG_SOURCES}
  )

################################################
# Internal_deps - only 'ge' targets goes here (e.g. geCore), it configures this package intra project dependencies and also configures the config file
# External_deps_Export - other than 'ge' packages goes here (e.g. glew), it configures this package extra project dependencies and also configures the config file
# External_libs - external libs or targets to link with
# Internal_inc - additional include directories
# Includes_to_export - includes to be exported from this target to application


set(Internal_deps geCore geGL)
set(External_deps_Export glm)
set(External_libs glm)
set(Internal_inc ${GPUEngine_SOURCE_DIR}/include)
set(Includes_to_export ${GPUEngine_SOURCE_DIR}/include)

target_link_libraries(${LIB_NAME} ${Internal_deps} ${External_libs})

set_target_properties(${LIB_NAME} PROPERTIES 
    INCLUDE_DIRECTORIES "${Internal_inc}"
    INTERFACE_INCLUDE_DIRECTORIES "${Includes_to_export}"
  )


install(FILES
    ${RG_INCLUDES}
    DESTINATION include/${LIB_NAME}
  )

install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

#########################################
#Export

configure_file(
   geRGConfig.cmake.in
   geRGConfig.cmake
   @ONLY
)

configure_file(
   ../CMake/geExport.h.in
   "${GPUEngine_SOURCE_DIR}/include/${LIB_NAME}/Export.h"
   @ONLY
)

# INSTALL_CM_DIR is defined in master CMakeLists file

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/geRGConfig.cmake"
        DESTINATION "${INSTALL_CM_DIR}/geRG"
        )
