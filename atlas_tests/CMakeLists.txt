add_executable(AtlasTests
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
    test_game_state.cpp
    test_pcg_preview.cpp
    test_lowpoly_character.cpp
    test_generation_style.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ConsolePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/PCGPreviewPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GenerationStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/AssetStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ShipArchetypePanel.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_renderer.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_context.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_widgets.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_hud.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/rendering/camera.cpp
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

target_compile_definitions(AtlasTests PRIVATE ATLAS_HEADLESS=1)
target_link_libraries(AtlasTests AtlasEngine)
target_include_directories(AtlasTests PRIVATE
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/cpp_client/include
    ${CMAKE_SOURCE_DIR}/cpp_server/include
)

add_test(NAME AtlasTests COMMAND AtlasTests)
