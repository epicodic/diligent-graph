project(diligent-graph)
cmake_minimum_required(VERSION 3.8)


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release")
endif()

# add_definitions(-fdiagnostics-color=always)

find_package(Eigen3 REQUIRED)


set(DILIGENT_ENGINE_ROOT "/opt/diligent-engine/")
set(DILIGENT_ENGINE_INCLUDE_DIRS "${DILIGENT_ENGINE_ROOT}/include;${DILIGENT_ENGINE_ROOT}/") 
set(DILIGENT_ENGINE_LIBRARY_DIR "${DILIGENT_ENGINE_ROOT}/lib" )

add_library(diligent-graph SHARED
  
  src/core/frustum.cpp
  src/core/type_id.cpp
  
  src/geometry/sphere_geometry.cpp
  src/geometry/box_geometry.cpp

  src/internal/pso_manager.cpp
  
  src/input/keys.cpp
  src/input/key_sequence.cpp
  src/input/shortcuts.cpp
  
  src/material/shader_program.cpp
  src/material/diffuse_material.cpp
  src/material/dynamic_texture.cpp
  src/material/unlit_material.cpp

  src/objects/assimp_mesh.cpp
  src/objects/canvas_drawing_layer.cpp
  src/objects/canvas_image_layer.cpp
  src/objects/canvas_manual_layer.cpp
  src/objects/canvas_object.cpp
  src/objects/geometry_object.cpp  
  src/objects/gltf_mesh.cpp
  src/objects/manual_object.cpp

  src/platform/render_window.cpp
    
  src/scene/camera.cpp
  src/scene/node.cpp
  src/scene/object.cpp
  src/scene/scene_manager.cpp
  
  
  # GUI
  external/imgui/imgui.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui/imgui_demo.cpp
  
  src/gui/ImGuiImplDiligent.cpp
  src/gui/imgui_integration.cpp
  src/gui/imgui_impl_dg.cpp
  
)

target_compile_features(diligent-graph PUBLIC cxx_std_11)
set_target_properties(diligent-graph PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(diligent-graph
  PUBLIC
    PLATFORM_LINUX
    GL_SUPPORTED
    VULKAN_SUPPORTED
)
target_include_directories(diligent-graph
  PUBLIC
    ${EIGEN3_INCLUDE_DIR}
    ${DILIGENT_ENGINE_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/>
    $<INSTALL_INTERFACE:external/>  
)
  
target_link_directories(diligent-graph 
  PUBLIC
    ${DILIGENT_ENGINE_LIBRARY_DIR}
)
target_link_libraries(diligent-graph
  PRIVATE
    ${CMAKE_DL_LIBS}
    diligent-engine
  PUBLIC
    assimp
    -Wl,-rpath,/opt/diligent-graph/lib/
    -Wl,-rpath,/opt/diligent-engine/lib/
)



add_library(diligent-graph-xcb SHARED
  src/platform/xcb/xcb_keyboard.cpp
  src/platform/xcb/xcb_render_window.cpp
)
target_link_libraries(diligent-graph-xcb
  PUBLIC
    diligent-graph
    xkbcommon
    xkbcommon-x11
    X11
)

add_library(diligent-graph-opengl SHARED
  src/platform/xcb/xcb_opengl_render_window.cpp
)
target_link_libraries(diligent-graph-opengl
  PRIVATE
    diligent-graph-xcb
    diligent-engine-opengl
    X11-xcb
    GL X11
)


add_library(diligent-graph-vulkan SHARED
  src/platform/xcb/xcb_vulkan_render_window.cpp
)
target_link_libraries(diligent-graph-vulkan
  PRIVATE
    diligent-graph-xcb
    diligent-engine-vulkan
)

#add_subdirectory(tools)

# Install
install(TARGETS diligent-graph diligent-graph-xcb diligent-graph-opengl diligent-graph-vulkan
        EXPORT diligent-graph
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)
install(DIRECTORY include DESTINATION .)
install(FILES 
          external/imgui/imconfig.h 
          external/imgui/imgui.h 
          external/imgui/imgui_internal.h
          external/imgui/imstb_rectpack.h
          external/imgui/imstb_textedit.h 
          external/imgui/imstb_truetype.h
        DESTINATION 
          external/imgui)

install(EXPORT diligent-graph DESTINATION share/cmake)
install(FILES cmake/diligent-graph-config.cmake DESTINATION share/cmake)

# for exporting to local package registry during development
export(TARGETS diligent-graph FILE diligent-graphConfig.cmake)
export(PACKAGE diligent-graph)

###############################################################################
# Packaging
###############################################################################
include(CMakePackageConfigHelpers)
set(CPACK_PACKAGE_NAME "diligent-graph")
set(CPACK_PACKAGE_DESCRIPTION "Scene graph and material system for Diligent Engine")
set(CPACK_PACKAGE_CONTACT "epicodic@web.de")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH_NO_CODENAME "$ENV{GITHUB_RUN_NUMBER}")

set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_VERSION_PATCH "${CPACK_PACKAGE_VERSION_PATCH_NO_CODENAME}-$ENV{DISTRO_CODENAME}")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "diligent-engine")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.deb")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(PACKAGE_VERSION_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CPACK_PACKAGE_NAME}-config-version.cmake")
write_basic_package_version_file(
  ${PACKAGE_VERSION_CONFIG_FILE}
  VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH_NO_CODENAME}>"
  COMPATIBILITY SameMajorVersion
)
install(FILES ${PACKAGE_VERSION_CONFIG_FILE} DESTINATION share/cmake)

include(CPack)
