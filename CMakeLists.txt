cmake_minimum_required(VERSION 3.0)
project(engine)
set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(GLSLANGVALIDATOR_BINARY "glslangValidator" CACHE PATH "Path to glslang validator binary to compile shaders by \"make shader\"")


set(SOURCES 
src/assets.cpp
src/main.cpp
src/gpuVulkan.cpp
src/windowGlfw.cpp
src/simulation.cpp
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

add_custom_target(run 	COMMAND ${PROJECT_NAME}
						DEPENDS ${PROJECT_NAME}
						WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/precompiled/fragment.spv)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/precompiled/vertex.spv)
add_custom_target(shader    COMMAND ${GLSLANGVALIDATOR_BINARY} -V ${CMAKE_SOURCE_DIR}/src/shader/vertex.vert -o ${CMAKE_SOURCE_DIR}/precompiled/vertex.spv
                            COMMAND ${GLSLANGVALIDATOR_BINARY} -V ${CMAKE_SOURCE_DIR}/src/shader/fragment.frag -o ${CMAKE_SOURCE_DIR}/precompiled/fragment.spv
						   # DEPENDS ${GLSLANGVALIDATOR_BINARY}
    						WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
target_link_libraries(${PROJECT_NAME} vulkan glfw glm)
