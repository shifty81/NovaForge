# Check for GLM (required by camera tests)
find_package(glm QUIET)
if(NOT glm_FOUND)
    # Try pkg-config as fallback
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLM QUIET glm)
    endif()
endif()

set(ATLAS_TEST_SOURCES
    main.cpp
    test_graphvm.cpp
    test_ecs.cpp
    test_components.cpp
    test_asset.cpp
    test_net.cpp
    test_net_queue.cpp
    test_net_quality.cpp
    test_world.cpp
    test_compiler.cpp
    test_engine.cpp
    test_console.cpp
    test_hud_panels.cpp
    test_pcg_preview.cpp
    test_lowpoly_character.cpp
    test_generation_style.cpp
    test_viewport.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ConsolePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/PCGPreviewPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GenerationStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/AssetStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ShipArchetypePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GamePackagerPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ViewportPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/ai/AIAggregator.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ECSInspectorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/NetInspectorPanel.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_renderer.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_context.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_widgets.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_hud.cpp
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
)

# Camera tests require GLM; include conditionally
if(glm_FOUND OR GLM_FOUND)
    list(APPEND ATLAS_TEST_SOURCES
        test_game_state.cpp
        ${CMAKE_SOURCE_DIR}/cpp_client/src/rendering/camera.cpp
    )
    message(STATUS "GLM found -- camera tests enabled")
else()
    message(STATUS "GLM not found -- camera tests disabled (test_game_state skipped)")
endif()

add_executable(AtlasTests ${ATLAS_TEST_SOURCES})

target_compile_definitions(AtlasTests PRIVATE ATLAS_HEADLESS=1)
if(NOT glm_FOUND AND NOT GLM_FOUND)
    target_compile_definitions(AtlasTests PRIVATE ATLAS_NO_GLM=1)
endif()

target_link_libraries(AtlasTests AtlasEngine)
target_include_directories(AtlasTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/cpp_client/include
    ${CMAKE_SOURCE_DIR}/cpp_server/include
)

if(glm_FOUND)
    target_link_libraries(AtlasTests glm::glm)
elseif(GLM_FOUND)
    target_include_directories(AtlasTests PRIVATE ${GLM_INCLUDE_DIRS})
endif()

add_test(NAME AtlasTests COMMAND AtlasTests)
