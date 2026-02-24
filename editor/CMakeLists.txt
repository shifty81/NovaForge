add_executable(AtlasEditor
    main.cpp
    ui/EditorLayout.cpp
    panels/ECSInspectorPanel.cpp
    panels/NetInspectorPanel.cpp
    panels/ConsolePanel.cpp
    tools/GamePackagerPanel.cpp
    tools/PCGPreviewPanel.cpp
    tools/GenerationStylePanel.cpp
    tools/AssetStylePanel.cpp
    tools/ShipArchetypePanel.cpp
    tools/ViewportPanel.cpp
    tools/PCGOverrideStore.cpp
    tools/LiveSceneManager.cpp
    ai/AIAggregator.cpp
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
    # Atlas UI sources used by editor panels
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_renderer.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_context.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_widgets.cpp
)

target_link_libraries(AtlasEditor AtlasEngine)
target_include_directories(AtlasEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp_server/include
    ${CMAKE_SOURCE_DIR}/cpp_client/include
)
