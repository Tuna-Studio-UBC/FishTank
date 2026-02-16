include(FetchContent)

cmake_policy(SET CMP0168 NEW)

if(DEFINED ENGINE_DEPENDENCIES_LOADED)
  return()
endif()
set(ENGINE_DEPENDENCIES_LOADED TRUE)

# Package Verisons
set(SPDLOG_VERSION v1.17.0)
set(IMGUI_VERSION v1.92.5)

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
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# ============================================================
# Helper function to link everything
# ============================================================

function(link_engine_dependencies target)

  # Tileset
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    TILESET_JSON="${CMAKE_CURRENT_SOURCE_DIR}/res/tilesets/tileset.json"
    TILEMAP_JSON="${CMAKE_CURRENT_SOURCE_DIR}/res/maps/map.json"
  )

  # Remaining
  target_link_libraries(${target} PUBLIC
    spdlog::spdlog
    imgui
  )

endfunction()
