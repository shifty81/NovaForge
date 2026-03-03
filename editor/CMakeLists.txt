# OpenGL (required by atlas_renderer.cpp when not in headless mode)
find_package(OpenGL REQUIRED)

# GLFW (required for the editor window)
find_package(glfw3 3.3 REQUIRED)

# GLEW (OpenGL extension wrangler)
find_package(GLEW QUIET)
if(GLEW_FOUND)
    message(STATUS "AtlasEditor: using system GLEW")
endif()

# GLM (header-only, required by character_mesh_system.h)
find_package(glm QUIET)
if(NOT glm_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLM QUIET glm)
    endif()
endif()

if(NOT glm_FOUND AND NOT GLM_FOUND)
    message(FATAL_ERROR "GLM not found -- required by AtlasEditor (character_mesh_system.h)")
endif()

add_executable(AtlasEditor
    main.cpp
    ui/EditorLayout.cpp
    ui/EditorMenuBar.cpp
    panels/ECSInspectorPanel.cpp
    panels/NetInspectorPanel.cpp
    panels/ConsolePanel.cpp
    tools/GamePackagerPanel.cpp
    tools/PCGPreviewPanel.cpp
    tools/GenerationStylePanel.cpp
    tools/AssetStylePanel.cpp
    tools/ShipArchetypePanel.cpp
    tools/ViewportPanel.cpp
    tools/CharacterSelectPanel.cpp
    tools/PCGOverrideStore.cpp
    tools/LiveSceneManager.cpp
    tools/GalaxyMapPanel.cpp
    tools/FleetFormationPanel.cpp
    tools/MissionEditorPanel.cpp
    tools/SceneGraphPanel.cpp
    tools/DataBrowserPanel.cpp
    tools/ModuleEditorPanel.cpp
    tools/AssetPalettePanel.cpp
    tools/NPCEditorPanel.cpp
    ai/AIAggregator.cpp
    ai/TemplateAIBackend.cpp
    ui/UndoStack.cpp
    ui/KeybindManager.cpp
    # PCG sources used by editor panels
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/pcg_manager.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/ship_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/station_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/interior_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/interior_budget.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/constraint_solver.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/pcg_trace.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/lowpoly_character_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/generation_style.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/pcg_asset_style.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/ship_archetype.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/spine_hull_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/turret_placement_system.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/turret_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/galaxy_generator.cpp
    # Character mesh system used by Character Select panel
    ${CMAKE_SOURCE_DIR}/cpp_client/src/characters/character_mesh_system.cpp
    # Window (GLFW) used by editor for the GUI window
    ${CMAKE_SOURCE_DIR}/cpp_client/src/rendering/window.cpp
    # Atlas UI sources used by editor panels
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_renderer.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_context.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_widgets.cpp
    # Input handler for GLFW mouse/keyboard → Atlas InputState
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/input_handler.cpp
)

target_link_libraries(AtlasEditor AtlasEngine OpenGL::GL glfw)

if(GLEW_FOUND)
    target_compile_definitions(AtlasEditor PRIVATE USE_GLEW)
    target_link_libraries(AtlasEditor GLEW::GLEW)
endif()

if(glm_FOUND)
    target_link_libraries(AtlasEditor glm::glm)
elseif(GLM_FOUND)
    target_include_directories(AtlasEditor PRIVATE ${GLM_INCLUDE_DIRS})
endif()

target_include_directories(AtlasEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp_server/include
    ${CMAKE_SOURCE_DIR}/cpp_client/include
)
