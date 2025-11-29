cmake_minimum_required(VERSION 3.10)
project(Windows VERSION 0.1.0)

#==============================================================================
# VULKAN
#
find_package(Vulkan REQUIRED)
set(VULKAN_INCLUDE_DIR  Z:/VulkanSDK/1.3.239.0/Include)
set(VULKAN_LIB_DIR      Z:/VulkanSDK/1.3.239.0/Lib)  

#SET ( CMAKE_CXX_FLAGS "-municode" )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)  #lib destination
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)     #executable destination

add_executable(${PROJECT_NAME} main.cpp Vulkanus_Circuit_sim_gradient.cpp) 

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${VULKAN_INCLUDE_DIR}
  )

target_link_directories(${PROJECT_NAME} PRIVATE ${VULKAN_LIB_DIR})
target_link_libraries(${PROJECT_NAME} 
:vulkan-1.dll )