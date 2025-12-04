cmake_minimum_required(VERSION 3.10)
project(Windows VERSION 0.1.0)

#==============================================================================
# VULKAN
#
#find_package(Vulkan REQUIRED)
set(VULKAN_INCLUDE_DIR  Z:/VulkanSDK/1.3.239.0/Include)
set(IMGUI_INCLUDE_DIR  ${PROJECT_SOURCE_DIR}/deps/imgui)
set(VULKAN_LIB_DIR      Z:/VulkanSDK/1.3.239.0/Lib)  

#==============================================================================
# IMGUI
#file(GLOB_RECURSE IMGUIS ${PROJECT_SOURCE_DIR}/Deps/imgui/*.cpp) 

file(GLOB IMGUIS ${PROJECT_SOURCE_DIR}/Deps/imgui/*.cpp) 
add_library(imgui ${IMGUIS}
            ${PROJECT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_vulkan.cpp)
target_include_directories(imgui PRIVATE ${PROJECT_SOURCE_DIR}/Deps/imgui
    ${VULKAN_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/deps/imgui/backends/
    ${PROJECT_SOURCE_DIR}/deps/imgui/
  )
target_link_directories(imgui PRIVATE
    ${PROJECT_SOURCE_DIR}/Lib
    ${VULKAN_LIB_DIR}
  )

#SET ( CMAKE_CXX_FLAGS "-municode" )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)  #lib destination
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)     #executable destination

add_executable(${PROJECT_NAME} main.cpp 
			Vulkanus_Circuit_sim_gradient.cpp 
			Vulkan.cpp ) 

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${VULKAN_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/deps/imgui/backends/
    ${PROJECT_SOURCE_DIR}/deps/imgui/
  )

target_link_directories(${PROJECT_NAME} PRIVATE ${VULKAN_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
imgui
:vulkan-1.dll )