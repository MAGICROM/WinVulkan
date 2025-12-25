cmake_minimum_required(VERSION 3.10)
project(Windows VERSION 0.1.0)

#SET ( CMAKE_CXX_FLAGS "-municode" )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)  #lib destination
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)     #executable destination

#==============================================================================
# VULKAN
#
find_package(Slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")

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

add_executable(${PROJECT_NAME} main.cpp 
			Vulkanus_Circuit_sim_gradient.cpp 
			Vulkan.cpp ) 

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${VULKAN_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/deps/imGuIZMO.quat/imguizmo_quat/
    ${PROJECT_SOURCE_DIR}/deps/imgui/backends/
    ${PROJECT_SOURCE_DIR}/deps/imgui/
    ${PROJECT_SOURCE_DIR}/deps/
  )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_directories(${PROJECT_NAME} PRIVATE ${VULKAN_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
imgui
imguizmo
Dwmapi.lib
:vulkan-1.dll )
#==============================================================================
# COMPILE SHADERS
#

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

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