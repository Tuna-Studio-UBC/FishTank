include(FetchContent)

cmake_policy(SET CMP0168 NEW)

if(DEFINED ENGINE_DEPENDENCIES_LOADED)
  return()
endif()
set(ENGINE_DEPENDENCIES_LOADED TRUE)

# Package Verisons
set(SPDLOG_VERSION v1.17.0)
set(FREETYPE_VERSION master)
set(RMLUI_VERSION 6.2)
set(IMGUI_VERSION v1.92.5-docking)
set(TRACY_VERSION v0.13.1)

# Generic Options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)

# ============================================================
# spdlog
# ============================================================

FetchContent_Declare(
  spdlog
  URL "https://github.com/gabime/spdlog/archive/${SPDLOG_VERSION}.zip"
)
FetchContent_MakeAvailable(spdlog)

# ============================================================
# FreeType
# ============================================================

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY "https://github.com/freetype/freetype.git"
  GIT_TAG "${FREETYPE_VERSION}"
)
FetchContent_MakeAvailable(Freetype)

set(FREETYPE_LIBRARY freetype)
add_library(Freetype::Freetype ALIAS ${FREETYPE_LIBRARY})

set(FREETYPE_INCLUDE_DIRS ${Freetype_SOURCE_DIR}/include)
target_include_directories(freetype PUBLIC ${FREETYPE_INCLUDE_DIRS})

# ============================================================
# RmlUi
# ============================================================

# FetchContent_Declare(
#   RmlUi
#   URL "https://github.com/mikke89/RmlUi/archive/${RMLUI_VERSION}.zip"
# )
# FetchContent_MakeAvailable(RmlUi)

# set(RMLUI_BACKEND_PATH ${RmlUi_SOURCE_DIR}/Backends)
# add_library(rmlui_backend_GLFW_GL3 INTERFACE)
# # target_sources(rmlui_backend_GLFW_GL3 INTERFACE
# #   ${RMLUI_BACKEND_PATH}/RmlUi_Platform_GLFW.cpp
# #   ${RMLUI_BACKEND_PATH}/RmlUi_Renderer_GL3.cpp
# # )
# target_include_directories(rmlui_backend_GLFW_GL3 INTERFACE ${RMLUI_BACKEND_PATH})
# target_compile_definitions(rmlui_backend_GLFW_GL3 INTERFACE
#   RMLUI_NUM_MSAA_SAMPLES=2
#   RMLUI_GL3_CUSTOM_LOADER=<gl3w.h>
# )

# ============================================================
# Dear ImGui
# ============================================================

FetchContent_Declare(
  imgui
  URL "https://github.com/ocornut/imgui/archive/${IMGUI_VERSION}.zip"
)

set(IMGUI_BUILD_DEMO_WINDOWS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# ============================================================
# Tracy
# ============================================================

set(TRACY_ENABLE  ON)
set(TRACY_MEMORY  ON)
set(TRACY_ON_DEMAND ON)
set(TRACY_GUI_GLEW ON)
set(TRACY_BUILD_PROFILER OFF)

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG ${TRACY_VERSION}
)

FetchContent_MakeAvailable(tracy)

add_library(tracy STATIC
  ${tracy_SOURCE_DIR}/public/TracyClient.cpp
)

target_include_directories(tracy PUBLIC
  ${tracy_SOURCE_DIR}/public
)

target_compile_definitions(tracy PUBLIC TRACY_ENABLE TRACY_MEMORY TRACY_ON_DEMAND)

# ============================================================
# Helper function to link everything
# ============================================================

function(link_engine_dependencies target)
  target_link_libraries(${target} PUBLIC
    spdlog::spdlog
    freetype
    # rmlui
    # rmlui_backend_GLFW_GL3
    imgui
    tracy
  )

  target_include_directories(imgui PUBLIC ${GLFW_INCLUDE_DIRS})
  target_link_libraries(imgui PUBLIC ${GLFW_LIBRARIES} OpenGL::GL)

endfunction()
