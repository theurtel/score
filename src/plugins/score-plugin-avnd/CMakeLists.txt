cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(score_plugin_avnd LANGUAGES CXX)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
  # Basically what does not work is clang 13 + libstdc++ due to coroutines
  if(NOT APPLE AND NOT WIN32 AND NOT DEPLOYMENT_BUILD)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
      return()
    endif()
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
    return()
  endif()
endif()

# Check for coroutines support
include(CheckCXXSourceCompiles)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    set(CMAKE_REQUIRED_FLAGS "-std=c++20 -fcoroutines-ts")
  else()
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_REQUIRED_FLAGS "-std=c++20 -fcoroutines")
endif()

check_cxx_source_compiles("#include <coroutine>\nint main() {}" COROUTINES_WORKING)
if(NOT COROUTINES_WORKING)
  check_cxx_source_compiles("#include <experimental/coroutine>\nint main() {}" EXPERIMENTAL_COROUTINES_WORKING)
  if(NOT EXPERIMENTAL_COROUTINES_WORKING)
    message("score_plugin_avnd: coroutines not supported")
    set(AVND_DISABLE_COROUTINES 1)
  endif()
endif()

unset(CMAKE_REQUIRED_FLAGS)
include_directories("${3RDPARTY_FOLDER}/avendish/include")
add_library(
  score_plugin_avnd

  Crousti/Attributes.hpp
  Crousti/Concepts.hpp
  Crousti/Executor.hpp
  Crousti/GfxNode.hpp
  Crousti/GpuNode.hpp
  Crousti/GpuComputeNode.hpp
  Crousti/GpuUtils.hpp
  Crousti/GpuUtils.cpp
  Crousti/Layer.hpp
  Crousti/Layer.cpp
  Crousti/Layout.hpp
  Crousti/Metadatas.hpp
  Crousti/Painter.hpp
  Crousti/ProcessModel.hpp
  Crousti/Widgets.hpp

  score_plugin_avnd.hpp
  score_plugin_avnd.cpp
)

target_include_directories(score_plugin_avnd
    PUBLIC
    "${3RDPARTY_FOLDER}/avendish/include"
    /home/scrime/Documents/SAF/Spatial_Audio_Framework/framework/include
    /home/scrime/Documents/SAF/Spatial_Audio_Framework/examples/include
)
target_link_libraries(
    score_plugin_avnd
    PRIVATE
    score_plugin_engine
    /home/scrime/Documents/SAF/Spatial_Audio_Framework/build/framework/libsaf.a
    /home/scrime/Documents/SAF/Spatial_Audio_Framework/build/examples/libsaf_example_rotator.a
    -lopenblas
    -llapack
    -llapacke
)

if(TARGET score_plugin_gfx)
  target_link_libraries(score_plugin_avnd PRIVATE score_plugin_gfx)
endif()

if(NOT AVND_DISABLE_COROUTINES)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    target_compile_options(
        score_plugin_avnd
        PUBLIC
          -fcoroutines
    )
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
    target_compile_options(
        score_plugin_avnd
        PUBLIC
          -fcoroutines-ts
    )
  endif()
else()
  target_compile_definitions(score_plugin_avnd PUBLIC AVND_DISABLE_COROUTINES)
endif()

setup_score_plugin(score_plugin_avnd)

target_precompile_headers(score_plugin_avnd PRIVATE
  Crousti/Attributes.hpp
  Crousti/Concepts.hpp
  Crousti/Executor.hpp
  Crousti/GfxNode.hpp
  Crousti/GpuNode.hpp
  Crousti/GpuComputeNode.hpp
  Crousti/GpuUtils.hpp
  Crousti/Layer.hpp
  Crousti/Layout.hpp
  Crousti/Metadatas.hpp
  Crousti/Painter.hpp
  Crousti/ProcessModel.hpp
  Crousti/Widgets.hpp
)
