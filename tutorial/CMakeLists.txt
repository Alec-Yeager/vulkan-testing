cmake_minimum_required (VERSION 4.2)

project (VulkanTutorialSandbox)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

set(CMAKE_PREFIX_PATH ${VULKAN_SDK})

find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
#target_link_libraries(main PRIVATE glm::glm)
find_package(Vulkan 1.4.335 REQUIRED) # Require Vulkan SDK version 1.4.335 or higher

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(spdlog)

add_executable(VulkanPhysics
    src/main.cpp
    src/VpeWindow.cpp
    src/VpePipeline.cpp
    src/VpeDevice.cpp
    src/VpeSwapChain.cpp
    src/VpeModel.cpp
    src/BasicApp.cpp
)

target_link_libraries(VulkanPhysics PRIVATE glm::glm glfw Vulkan::Vulkan spdlog::spdlog)

target_compile_definitions(VulkanPhysics PRIVATE
    SPDLOG_ACTIVE_LEVEL=$<IF:$<CONFIG:Debug>,SPDLOG_LEVEL_DEBUG,SPDLOG_LEVEL_INFO>
)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_program(GLSLC glslc REQUIRED)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)
file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag")

set(SPIRV_SHADERS)

foreach(shader ${SHADERS})
    get_filename_component(FILENAME ${shader} NAME)
    set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${FILENAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC} ${shader} -o ${SPIRV}
        DEPENDS ${shader}
        COMMENT "Compiling shader ${shader}"
    )

    list(APPEND SPIRV_SHADERS ${SPIRV})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})
add_dependencies(VulkanPhysics Shaders)