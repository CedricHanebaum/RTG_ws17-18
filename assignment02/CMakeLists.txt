cmake_minimum_required(VERSION 3.0)
project(Assignment02)

# Add aion lib
add_subdirectory(../libs/aion ${CMAKE_BINARY_DIR}/libs/aion)

# Add Assimp lib
add_subdirectory(../libs/assimp ${CMAKE_BINARY_DIR}/libs/assimp)

# Add AntTweakBar
add_subdirectory(../libs/AntTweakBar ${CMAKE_BINARY_DIR}/libs/AntTweakBar)

# Add GLFW lib (with disabled spam)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
if(MSVC)
  option(BUILD_SHARED_LIBS "" OFF)
else()
  option(BUILD_SHARED_LIBS "" ON)
endif()
add_subdirectory(../libs/glfw ${CMAKE_BINARY_DIR}/libs/glfw)

# Add GLOW lib
add_subdirectory(../libs/glow ${CMAKE_BINARY_DIR}/libs/glow)
# Add GLOW Extras lib
add_subdirectory(../libs/glow-extras ${CMAKE_BINARY_DIR}/libs/glow-extras)

# Create target
add_executable(Assignment02
    main.cc
    Assignment02.cc
    Assignment02.hh
    Entity.hh
    Components.hh
    Messages.cc
    Messages.hh
    Player.hh
)

# Link libs
target_link_libraries(Assignment02 PUBLIC 
    glow 
    glow-extras 
    glfw
    AntTweakBar
)

# Compile flags
if(MSVC)
    target_compile_options(Assignment02 PUBLIC 
        /MP
    )
else()
    target_compile_options(Assignment02 PUBLIC 
        -Wall
        -std=c++11
    )
endif()
