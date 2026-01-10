cmake_minimum_required(VERSION 3.10)
project(Windows VERSION 0.1.0)

#SET ( CMAKE_CXX_FLAGS "-municode" )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)  #lib destination
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)     #executable destination

#==============================================================================
# DRLIBS
#
set(DRLIBS_INCLUDE_DIR  ${PROJECT_SOURCE_DIR}/Deps/dr_libs) 

#==============================================================================
# OPEN AL
#
set(OPENAL_INCLUDE_DIR  Z:/OpenALSDK/include) 
set(OPENAL_LIB_DIR      Z:/OpenALSDK/libs/Win64) 

#==============================================================================
# ONDE
#
set(ONDE_DIR  ${PROJECT_SOURCE_DIR}/Deps/onde) 

#==============================================================================
# OGG
#
set(OGG_DIR ${PROJECT_SOURCE_DIR}/Deps/ogg)
set(OGG_SOURCES
    ${OGG_DIR}/src/bitwise.c
    ${OGG_DIR}/src/framing.c
    ${OGG_DIR}/src/crctable.h)
add_library(ogg ${OGG_SOURCES})
target_include_directories(ogg PRIVATE ${OGG_DIR}/include)

#==============================================================================
# VORBIS
#
set(VORBIS_PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/Deps/vorbis/include/vorbis/codec.h
    ${PROJECT_SOURCE_DIR}/Deps/vorbis/include/vorbis/vorbisenc.h
    ${PROJECT_SOURCE_DIR}/Deps/vorbis/include/vorbis/vorbisfile.h
)
set(VORBIS_DIR ${PROJECT_SOURCE_DIR}/Deps/vorbis/lib)
set(VORBIS_HEADERS
    ${VORBIS_DIR}/envelope.h
    ${VORBIS_DIR}/lpc.h
    ${VORBIS_DIR}/lsp.h
    ${VORBIS_DIR}/codebook.h
    ${VORBIS_DIR}/misc.h
    ${VORBIS_DIR}/psy.h
    ${VORBIS_DIR}/masking.h
    ${VORBIS_DIR}/os.h
    ${VORBIS_DIR}/mdct.h
    ${VORBIS_DIR}/smallft.h
    ${VORBIS_DIR}/highlevel.h
    ${VORBIS_DIR}/registry.h
    ${VORBIS_DIR}/scales.h
    ${VORBIS_DIR}/window.h
    ${VORBIS_DIR}/lookup.h
    ${VORBIS_DIR}/lookup_data.h
    ${VORBIS_DIR}/codec_internal.h
    ${VORBIS_DIR}/backends.h
    ${VORBIS_DIR}/bitrate.h
)

set(VORBIS_SOURCES
    ${VORBIS_DIR}/mdct.c
    ${VORBIS_DIR}/smallft.c
    ${VORBIS_DIR}/block.c
    ${VORBIS_DIR}/envelope.c
    ${VORBIS_DIR}/window.c
    ${VORBIS_DIR}/lsp.c
    ${VORBIS_DIR}/lpc.c
    ${VORBIS_DIR}/analysis.c
    ${VORBIS_DIR}/synthesis.c
    ${VORBIS_DIR}/psy.c
    ${VORBIS_DIR}/info.c
    ${VORBIS_DIR}/floor1.c
    ${VORBIS_DIR}/floor0.c
    ${VORBIS_DIR}/res0.c
    ${VORBIS_DIR}/mapping0.c
    ${VORBIS_DIR}/registry.c
    ${VORBIS_DIR}/codebook.c
    ${VORBIS_DIR}/sharedbook.c
    ${VORBIS_DIR}/lookup.c
    ${VORBIS_DIR}/bitrate.c)

set(VORBISFILE_SOURCES
	${VORBIS_DIR}/vorbisfile.c)

set(VORBISENC_SOURCES
	${VORBIS_DIR}/vorbisenc.c)
    
if(WIN32)
    list(APPEND VORBIS_SOURCES ${VORBIS_DIR}/vorbisenc.c)
    list(APPEND VORBIS_SOURCES ${PROJECT_SOURCE_DIR}/Deps/vorbis/win32/vorbis.def)
    list(APPEND VORBISENC_SOURCES ${PROJECT_SOURCE_DIR}/Deps/vorbis/win32/vorbisenc.def)
    list(APPEND VORBISFILE_SOURCES ${PROJECT_SOURCE_DIR}/Deps/vorbis/win32/vorbisfile.def)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

    add_library(vorbis ${VORBIS_HEADERS} ${VORBIS_SOURCES})
    target_include_directories(vorbis
        PUBLIC
            ${VORBIS_DIR}
            ${OGG_DIR}/include
            ${PROJECT_SOURCE_DIR}/Deps/vorbis/include
            ${PROJECT_SOURCE_DIR}/Deps/vorbis/include
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
       PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_library(vorbisenc ${VORBISENC_SOURCES})
    target_include_directories(vorbisenc
        PUBLIC
            ${PROJECT_SOURCE_DIR}/Deps/vorbis/include
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_library(vorbisfile ${VORBISFILE_SOURCES})
    target_include_directories(vorbisfile
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(vorbis PUBLIC ogg)
    target_link_libraries(vorbisenc PUBLIC vorbis)
    target_link_libraries(vorbisfile PUBLIC vorbis)
    
#==============================================================================
# VULKAN
#
#find_package(Slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found in: ${SHADERC_LIB}")

set(VULKAN_INCLUDE_DIR  Z:/VulkanSDK/1.3.239.0/Include)
set(VULKAN_LIB_DIR      Z:/VulkanSDK/1.3.239.0/Lib)  
set(IMGUI_INCLUDE_DIR  ${PROJECT_SOURCE_DIR}/deps/imgui)

#==============================================================================
# IMGUI
#file(GLOB_RECURSE IMGUIS ${PROJECT_SOURCE_DIR}/Deps/imgui/*.cpp) 

file(GLOB IMGUIS ${PROJECT_SOURCE_DIR}/Deps/imgui/*.cpp) 
add_library(imgui ${IMGUIS}
            ${PROJECT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_vulkan.cpp
            ${PROJECT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_win32.cpp)
target_include_directories(imgui PRIVATE ${PROJECT_SOURCE_DIR}/Deps/imgui
    ${VULKAN_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/deps/imgui/backends/
    ${PROJECT_SOURCE_DIR}/deps/imgui/
  )
target_link_directories(imgui PRIVATE
    ${PROJECT_SOURCE_DIR}/Lib
    ${VULKAN_LIB_DIR}
  )
#==============================================================================
# IMGUIzMO

add_library(imguizmo ${PROJECT_SOURCE_DIR}/deps/imGuIZMO.quat/imguizmo_quat/imGuIZMOquat.cpp)
target_include_directories(imguizmo PRIVATE ${PROJECT_SOURCE_DIR}/deps/imGuIZMO.quat/imguizmo_quat/
                                            ${PROJECT_SOURCE_DIR}/deps/)

#==============================================================================
# APPLICATION

add_executable(${PROJECT_NAME} cpp/main.cpp 
			cpp/Vulkanus_Circuit_sim_gradient.cpp 
			cpp/Vulkan.cpp 
      cpp/sn_onde.cpp) 

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${ONDE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${MF64_INCLUDE_DIR}
    ${VORBIS_DIR}
    ${OGG_DIR}/include
    ${VORBIS_DIR}/include
    ${DRLIBS_INCLUDE_DIR}
    ${VULKAN_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/deps/imGuIZMO.quat/imguizmo_quat/
    ${PROJECT_SOURCE_DIR}/deps/imgui/backends/
    ${PROJECT_SOURCE_DIR}/deps/imgui/
    ${PROJECT_SOURCE_DIR}/deps/
  )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_directories(${PROJECT_NAME} PRIVATE ${VULKAN_LIB_DIR} ${OPENAL_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
imgui
imguizmo
Dwmapi.lib
:vulkan-1.dll
ogg
vorbisfile
:OpenAL32.dll)
#==============================================================================
# AUTO COMPILE SHADERS
#

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
find_program(GLSLANG_EXECUTABLE glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin")
find_program(SPIRV_OPT_EXECUTABLE spirv-opt
        HINTS "$ENV{VULKAN_SDK}/bin")


set(SHADER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${PROJECT_SOURCE_DIR}/shaders)

file(GLOB SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})