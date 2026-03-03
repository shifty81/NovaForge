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
    test_hud_control_ring.cpp
    test_pcg_preview.cpp
    test_lowpoly_character.cpp
    test_character_select.cpp
    test_generation_style.cpp
    test_viewport.cpp
    test_editor_panels.cpp
    test_live_scene.cpp
    test_sim_systems.cpp
    test_galaxy_map.cpp
    test_eventbus.cpp
    test_fleet_formation.cpp
    test_mission_editor.cpp
    test_net_interpolation.cpp
    test_delta_compression.cpp
    test_jitter_buffer.cpp
    test_lag_compensation.cpp
    test_scene_graph.cpp
    test_keybind_manager.cpp
    test_alert_stack.cpp
    test_ai_miner.cpp
    test_ai_hauler.cpp
    test_ai_pirate.cpp
    test_ai_security.cpp
    test_ai_wallet.cpp
    test_ai_trader.cpp
    test_ai_industrialist.cpp
    test_advanced_mission.cpp
    test_universe_map.cpp
    test_ai_profile.cpp
    test_economy_rules.cpp
    test_pirate_security_coordinator.cpp
    test_engine_frame_callback.cpp
    test_editor_menubar.cpp
    test_data_editors.cpp
    test_warp_performance_budget.cpp
    test_fleet_positional_audio.cpp
    test_editor_tool_layer.cpp
    test_system_registry.cpp
    test_entity_commands.cpp
    test_game_state_manager.cpp
    test_runtime_bootstrap.cpp
    test_remove_component_cmd.cpp
    test_undoable_command_bus.cpp
    test_editor_event_bus.cpp
    test_integration.cpp
    test_delta_edit_store.cpp
    test_map_editor_tool.cpp
    test_ship_module_editor_tool.cpp
    test_asset_palette_panel.cpp
    test_physics_tuner_panel.cpp
    test_environment_control_tool.cpp
    test_npc_spawner_tool.cpp
    test_resource_balancer_tool.cpp
    test_live_scene_snapshot.cpp
    test_event_timeline_tool.cpp
    test_batch_operations_tool.cpp
    test_scene_bookmark_manager.cpp
    test_layer_tag_system.cpp
    test_snap_align_tool.cpp
    test_asset_stats_panel.cpp
    test_prefab_library.cpp
    test_simulation_step_controller.cpp
    test_multi_selection_manager.cpp
    test_function_assignment_tool.cpp
    test_edit_propagation_tool.cpp
    test_visual_diff_tool.cpp
    test_animation_editor_tool.cpp
    test_lighting_control_tool.cpp
    test_script_console.cpp
    test_camera_view_tool.cpp
    test_material_shader_tool.cpp
    test_hotkey_action_manager.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/editor/editor_command_bus.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/editor/undoable_command_bus.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/editor/editor_event_bus.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ConsolePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/PCGPreviewPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GenerationStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/AssetStylePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ShipArchetypePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GamePackagerPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ViewportPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/PCGOverrideStore.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/LiveSceneManager.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/GalaxyMapPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/FleetFormationPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/MissionEditorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/SceneGraphPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/DataBrowserPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/ModuleEditorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/NPCEditorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/AssetPalettePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/PhysicsTunerPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/ai/AIAggregator.cpp
    ${CMAKE_SOURCE_DIR}/editor/ai/TemplateAIBackend.cpp
    ${CMAKE_SOURCE_DIR}/editor/ui/UndoStack.cpp
    ${CMAKE_SOURCE_DIR}/editor/ui/KeybindManager.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ECSInspectorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/NetInspectorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/ui/EditorLayout.cpp
    ${CMAKE_SOURCE_DIR}/editor/ui/EditorMenuBar.cpp
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
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/spine_hull_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/turret_placement_system.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/turret_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/galaxy_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/economy_driven_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/npc_encounter_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/damage_state_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/procedural_texture_generator.cpp
    ${CMAKE_SOURCE_DIR}/cpp_server/src/pcg/shield_effect_generator.cpp
)

# Camera tests require GLM; include conditionally
if(glm_FOUND OR GLM_FOUND)
    list(APPEND ATLAS_TEST_SOURCES
        test_game_state.cpp
        ${CMAKE_SOURCE_DIR}/cpp_client/src/rendering/camera.cpp
        ${CMAKE_SOURCE_DIR}/editor/tools/CharacterSelectPanel.cpp
        ${CMAKE_SOURCE_DIR}/cpp_client/src/characters/character_mesh_system.cpp
    )
    message(STATUS "GLM found -- camera & character select tests enabled")
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
