#include <iostream>

// GraphVM tests
void test_basic_arithmetic();
void test_subtraction();
void test_multiplication();
void test_division();
void test_division_by_zero();
void test_comparison();
void test_conditional_jump();
void test_variables();

// ECS tests
void test_create_entity();
void test_destroy_entity();
void test_tick_callback();

// ECS Component tests
void test_add_and_get_component();
void test_has_component();
void test_remove_component();
void test_multiple_components();
void test_destroy_entity_removes_components();
void test_component_update();

// Asset tests
void test_asset_binary_roundtrip();
void test_asset_registry_scan();

// Network tests
void test_net_init();
void test_net_authority();
void test_net_shutdown();

// Network queue tests
void test_net_add_peer();
void test_net_remove_peer();
void test_net_send_receive();
void test_net_broadcast_receive();
void test_net_shutdown_clears_queues();

// Network quality monitor tests
void test_nqm_initial_rtt();
void test_nqm_rtt_ewma();
void test_nqm_stable_rtt_low_jitter();
void test_nqm_negative_rtt_ignored();
void test_nqm_no_loss();
void test_nqm_50_percent_loss();
void test_nqm_window_trims();
void test_nqm_empty_loss_rate();
void test_nqm_interp_floor();
void test_nqm_interp_ceiling();
void test_nqm_interp_increases_with_jitter();
void test_nqm_interp_increases_with_loss();
void test_nqm_reset();
void test_net_send_assigns_sequence();
void test_net_broadcast_assigns_sequence();
void test_net_sequence_resets_on_init();

// World tests
void test_cube_sphere_projection();
void test_cube_sphere_chunk_roundtrip();
void test_cube_sphere_neighbors();
void test_cube_sphere_lod();
void test_voxel_chunk_roundtrip();
void test_voxel_neighbors();

// Compiler tests
void test_compile_constants_and_add();
void test_compile_and_execute_full();
void test_compile_multiply();

// Engine tests
void test_engine_init_and_shutdown();
void test_engine_run_loop_ticks();
void test_engine_capabilities();
void test_engine_net_mode_from_config();

// Console tests
void test_console_spawn_entity();
void test_console_ecs_dump();
void test_console_set_tickrate();
void test_console_net_mode();
void test_console_help();
void test_console_unknown_command();

// HUD Panel tests
void test_station_panel_defaults();
void test_station_panel_toggle();
void test_station_panel_set_data();
void test_station_panel_callbacks();
void test_inventory_panel_defaults();
void test_inventory_panel_set_data();
void test_fitting_panel_defaults();
void test_fitting_panel_set_data();
void test_market_panel_defaults();
void test_market_panel_set_data();
void test_fleet_panel_defaults();
void test_fleet_panel_toggle();
void test_fleet_panel_set_data();
void test_existing_panel_toggles();
void test_overview_tab_filter();

// Game State & ViewMode tests
#ifndef ATLAS_NO_GLM
void test_camera_default_view_mode();
void test_camera_set_view_mode_orbit();
void test_camera_set_view_mode_fps();
void test_camera_set_view_mode_cockpit();
void test_camera_fps_position_and_forward();
void test_camera_fps_rotate();
void test_camera_orbit_position_unchanged_in_fps();
void test_camera_view_mode_kills_inertia();
void test_camera_view_matrix_differs_by_mode();
#endif

// PCG Preview Panel tests
void test_pcg_preview_defaults();
void test_pcg_preview_generate_ship();
void test_pcg_preview_generate_ship_override_hull();
void test_pcg_preview_generate_station();
void test_pcg_preview_generate_station_override_count();
void test_pcg_preview_generate_interior();
void test_pcg_preview_determinism();
void test_pcg_preview_randomize_changes_seed();
void test_pcg_preview_clear();
void test_pcg_preview_set_settings();
void test_pcg_preview_different_seeds_differ();
void test_pcg_preview_draw_does_not_crash();

// Low-Poly Character Generator tests
void test_lowpoly_char_generate_default();
void test_lowpoly_char_body_slot_count();
void test_lowpoly_char_override_archetype();
void test_lowpoly_char_override_gender();
void test_lowpoly_char_palette_regions();
void test_lowpoly_char_flat_shading();
void test_lowpoly_char_fps_arms();
void test_lowpoly_char_determinism();
void test_lowpoly_char_different_seeds_differ();
void test_lowpoly_char_clothing_always_has_basics();
void test_lowpoly_char_archetype_names();
void test_lowpoly_char_body_slot_names();
void test_lowpoly_char_clothing_slot_names();
void test_pcg_preview_generate_character();
void test_pcg_preview_character_override_archetype();
void test_pcg_preview_character_override_gender();
void test_pcg_preview_character_clear();

// Generation Style Engine tests
void test_gs_create_default_ship_style();
void test_gs_create_default_station_style();
void test_gs_create_all_default_styles();
void test_gs_validate_valid_style();
void test_gs_validate_empty_name_fails();
void test_gs_validate_out_of_range_param_fails();
void test_gs_validate_duplicate_slots_fails();
void test_gs_available_parameters();
void test_gs_find_parameter();
void test_gs_style_type_names();
void test_gs_generate_ship_style();
void test_gs_generate_station_style();
void test_gs_generate_station_with_placements();
void test_gs_generate_interior_style();
void test_gs_generate_determinism();
void test_gs_serialize_roundtrip();
void test_gs_ship_parameter_overrides();

// Generation Style Panel tests
void test_gsp_defaults();
void test_gsp_new_style();
void test_gsp_add_remove_placement();
void test_gsp_set_parameter();
void test_gsp_enable_disable_parameter();
void test_gsp_generate();
void test_gsp_generate_with_asset_style();
void test_gsp_save_load_string();
void test_gsp_draw_does_not_crash();

// Asset Style Library tests
void test_as_library_add_find();
void test_as_library_replace();
void test_as_library_remove();
void test_as_library_list_by_type();
void test_as_library_clear();
void test_as_shape_apply_to_ship();
void test_as_shape_apply_to_station();
void test_as_surface_treatment_names();
void test_as_serialize_roundtrip();

// Asset Style Panel tests
void test_asp_defaults();
void test_asp_new_style();
void test_asp_shape_control_points();
void test_asp_mirror_and_smoothing();
void test_asp_palette_colors();
void test_asp_palette_materials();
void test_asp_surface_treatment();
void test_asp_detail_level();
void test_asp_apply_and_preview_ship();
void test_asp_apply_and_preview_station();
void test_asp_library_save_load();
void test_asp_serialize_roundtrip();
void test_asp_draw_does_not_crash();

// Ship Archetype Engine tests
void test_arch_create_default_frigate();
void test_arch_create_all_hull_classes();
void test_arch_capital_bigger_than_frigate();
void test_arch_validate_valid();
void test_arch_validate_empty_name_fails();
void test_arch_validate_no_rooms_fails();
void test_arch_validate_no_hardpoints_fails();
void test_arch_validate_duplicate_hardpoint_ids_fails();
void test_arch_validate_door_bad_room_ref_fails();
void test_arch_generate_from_archetype();
void test_arch_generate_determinism();
void test_arch_apply_subsystems();
void test_arch_apply_module_visuals();
void test_arch_subsystem_type_names();
void test_arch_serialize_roundtrip();
void test_arch_different_seeds_differ();

// Ship Archetype Panel tests
void test_archp_defaults();
void test_archp_select_hull_class();
void test_archp_add_remove_hull_control_point();
void test_archp_add_remove_room();
void test_archp_add_remove_door();
void test_archp_add_remove_hardpoint();
void test_archp_subsystem_editing();
void test_archp_module_visual_rules();
void test_archp_variation_bounds();
void test_archp_generate_preview();
void test_archp_generate_preview_with_subsystems();
void test_archp_generate_preview_with_modules();
void test_archp_save_load_string();
void test_archp_draw_does_not_crash();

// Viewport Panel tests
void test_viewport_defaults();
void test_viewport_load_ship();
void test_viewport_load_station();
void test_viewport_clear_scene();
void test_viewport_select_object();
void test_viewport_deselect_all();
void test_viewport_translate_selected();
void test_viewport_rotate_selected();
void test_viewport_scale_selected();
void test_viewport_scale_clamps_positive();
void test_viewport_gizmo_mode();
void test_viewport_camera_orbit();
void test_viewport_camera_pitch_clamp();
void test_viewport_camera_distance();
void test_viewport_commit_changes();
void test_viewport_discard_changes();
void test_viewport_grid_toggle();
void test_viewport_draw_does_not_crash();
void test_viewport_log_entries();
void test_viewport_no_op_without_selection();

int main() {
    std::cout << "=== Atlas Engine Tests ===" << std::endl;

    // GraphVM
    std::cout << "\n--- Graph VM ---" << std::endl;
    test_basic_arithmetic();
    test_subtraction();
    test_multiplication();
    test_division();
    test_division_by_zero();
    test_comparison();
    test_conditional_jump();
    test_variables();

    // ECS
    std::cout << "\n--- ECS ---" << std::endl;
    test_create_entity();
    test_destroy_entity();
    test_tick_callback();

    // ECS Components
    std::cout << "\n--- ECS Components ---" << std::endl;
    test_add_and_get_component();
    test_has_component();
    test_remove_component();
    test_multiple_components();
    test_destroy_entity_removes_components();
    test_component_update();

    // Assets
    std::cout << "\n--- Asset System ---" << std::endl;
    test_asset_binary_roundtrip();
    test_asset_registry_scan();

    // Networking
    std::cout << "\n--- Networking ---" << std::endl;
    test_net_init();
    test_net_authority();
    test_net_shutdown();

    // Network Queue
    std::cout << "\n--- Network Queue ---" << std::endl;
    test_net_add_peer();
    test_net_remove_peer();
    test_net_send_receive();
    test_net_broadcast_receive();
    test_net_shutdown_clears_queues();

    // Network Quality Monitor
    std::cout << "\n--- Network Quality Monitor ---" << std::endl;
    test_nqm_initial_rtt();
    test_nqm_rtt_ewma();
    test_nqm_stable_rtt_low_jitter();
    test_nqm_negative_rtt_ignored();
    test_nqm_no_loss();
    test_nqm_50_percent_loss();
    test_nqm_window_trims();
    test_nqm_empty_loss_rate();
    test_nqm_interp_floor();
    test_nqm_interp_ceiling();
    test_nqm_interp_increases_with_jitter();
    test_nqm_interp_increases_with_loss();
    test_nqm_reset();
    test_net_send_assigns_sequence();
    test_net_broadcast_assigns_sequence();
    test_net_sequence_resets_on_init();

    // World
    std::cout << "\n--- World Layout ---" << std::endl;
    test_cube_sphere_projection();
    test_cube_sphere_chunk_roundtrip();
    test_cube_sphere_neighbors();
    test_cube_sphere_lod();
    test_voxel_chunk_roundtrip();
    test_voxel_neighbors();

    // Compiler
    std::cout << "\n--- Graph Compiler ---" << std::endl;
    test_compile_constants_and_add();
    test_compile_and_execute_full();
    test_compile_multiply();

    // Engine
    std::cout << "\n--- Engine ---" << std::endl;
    test_engine_init_and_shutdown();
    test_engine_run_loop_ticks();
    test_engine_capabilities();
    test_engine_net_mode_from_config();

    // Console
    std::cout << "\n--- Console ---" << std::endl;
    test_console_spawn_entity();
    test_console_ecs_dump();
    test_console_set_tickrate();
    test_console_net_mode();
    test_console_help();
    test_console_unknown_command();

    // HUD Panels
    std::cout << "\n--- HUD Panels ---" << std::endl;
    test_station_panel_defaults();
    test_station_panel_toggle();
    test_station_panel_set_data();
    test_station_panel_callbacks();
    test_inventory_panel_defaults();
    test_inventory_panel_set_data();
    test_fitting_panel_defaults();
    test_fitting_panel_set_data();
    test_market_panel_defaults();
    test_market_panel_set_data();
    test_fleet_panel_defaults();
    test_fleet_panel_toggle();
    test_fleet_panel_set_data();
    test_existing_panel_toggles();
    test_overview_tab_filter();

    // Game State & ViewMode
#ifndef ATLAS_NO_GLM
    std::cout << "\n--- Game State & ViewMode ---" << std::endl;
    test_camera_default_view_mode();
    test_camera_set_view_mode_orbit();
    test_camera_set_view_mode_fps();
    test_camera_set_view_mode_cockpit();
    test_camera_fps_position_and_forward();
    test_camera_fps_rotate();
    test_camera_orbit_position_unchanged_in_fps();
    test_camera_view_mode_kills_inertia();
    test_camera_view_matrix_differs_by_mode();
#else
    std::cout << "\n--- Game State & ViewMode (SKIPPED — GLM not available) ---" << std::endl;
#endif

    // PCG Preview Panel
    std::cout << "\n--- PCG Preview Panel ---" << std::endl;
    test_pcg_preview_defaults();
    test_pcg_preview_generate_ship();
    test_pcg_preview_generate_ship_override_hull();
    test_pcg_preview_generate_station();
    test_pcg_preview_generate_station_override_count();
    test_pcg_preview_generate_interior();
    test_pcg_preview_determinism();
    test_pcg_preview_randomize_changes_seed();
    test_pcg_preview_clear();
    test_pcg_preview_set_settings();
    test_pcg_preview_different_seeds_differ();
    test_pcg_preview_draw_does_not_crash();

    // Low-Poly Character Generator
    std::cout << "\n--- Low-Poly Character Generator ---" << std::endl;
    test_lowpoly_char_generate_default();
    test_lowpoly_char_body_slot_count();
    test_lowpoly_char_override_archetype();
    test_lowpoly_char_override_gender();
    test_lowpoly_char_palette_regions();
    test_lowpoly_char_flat_shading();
    test_lowpoly_char_fps_arms();
    test_lowpoly_char_determinism();
    test_lowpoly_char_different_seeds_differ();
    test_lowpoly_char_clothing_always_has_basics();
    test_lowpoly_char_archetype_names();
    test_lowpoly_char_body_slot_names();
    test_lowpoly_char_clothing_slot_names();
    test_pcg_preview_generate_character();
    test_pcg_preview_character_override_archetype();
    test_pcg_preview_character_override_gender();
    test_pcg_preview_character_clear();

    // Generation Style Engine
    std::cout << "\n--- Generation Style Engine ---" << std::endl;
    test_gs_create_default_ship_style();
    test_gs_create_default_station_style();
    test_gs_create_all_default_styles();
    test_gs_validate_valid_style();
    test_gs_validate_empty_name_fails();
    test_gs_validate_out_of_range_param_fails();
    test_gs_validate_duplicate_slots_fails();
    test_gs_available_parameters();
    test_gs_find_parameter();
    test_gs_style_type_names();
    test_gs_generate_ship_style();
    test_gs_generate_station_style();
    test_gs_generate_station_with_placements();
    test_gs_generate_interior_style();
    test_gs_generate_determinism();
    test_gs_serialize_roundtrip();
    test_gs_ship_parameter_overrides();

    // Generation Style Panel
    std::cout << "\n--- Generation Style Panel ---" << std::endl;
    test_gsp_defaults();
    test_gsp_new_style();
    test_gsp_add_remove_placement();
    test_gsp_set_parameter();
    test_gsp_enable_disable_parameter();
    test_gsp_generate();
    test_gsp_generate_with_asset_style();
    test_gsp_save_load_string();
    test_gsp_draw_does_not_crash();

    // Asset Style Library
    std::cout << "\n--- Asset Style Library ---" << std::endl;
    test_as_library_add_find();
    test_as_library_replace();
    test_as_library_remove();
    test_as_library_list_by_type();
    test_as_library_clear();
    test_as_shape_apply_to_ship();
    test_as_shape_apply_to_station();
    test_as_surface_treatment_names();
    test_as_serialize_roundtrip();

    // Asset Style Panel
    std::cout << "\n--- Asset Style Panel ---" << std::endl;
    test_asp_defaults();
    test_asp_new_style();
    test_asp_shape_control_points();
    test_asp_mirror_and_smoothing();
    test_asp_palette_colors();
    test_asp_palette_materials();
    test_asp_surface_treatment();
    test_asp_detail_level();
    test_asp_apply_and_preview_ship();
    test_asp_apply_and_preview_station();
    test_asp_library_save_load();
    test_asp_serialize_roundtrip();
    test_asp_draw_does_not_crash();

    // Ship Archetype Engine
    std::cout << "\n--- Ship Archetype Engine ---" << std::endl;
    test_arch_create_default_frigate();
    test_arch_create_all_hull_classes();
    test_arch_capital_bigger_than_frigate();
    test_arch_validate_valid();
    test_arch_validate_empty_name_fails();
    test_arch_validate_no_rooms_fails();
    test_arch_validate_no_hardpoints_fails();
    test_arch_validate_duplicate_hardpoint_ids_fails();
    test_arch_validate_door_bad_room_ref_fails();
    test_arch_generate_from_archetype();
    test_arch_generate_determinism();
    test_arch_apply_subsystems();
    test_arch_apply_module_visuals();
    test_arch_subsystem_type_names();
    test_arch_serialize_roundtrip();
    test_arch_different_seeds_differ();

    // Ship Archetype Panel
    std::cout << "\n--- Ship Archetype Panel ---" << std::endl;
    test_archp_defaults();
    test_archp_select_hull_class();
    test_archp_add_remove_hull_control_point();
    test_archp_add_remove_room();
    test_archp_add_remove_door();
    test_archp_add_remove_hardpoint();
    test_archp_subsystem_editing();
    test_archp_module_visual_rules();
    test_archp_variation_bounds();
    test_archp_generate_preview();
    test_archp_generate_preview_with_subsystems();
    test_archp_generate_preview_with_modules();
    test_archp_save_load_string();
    test_archp_draw_does_not_crash();

    // Viewport Panel
    std::cout << "\n--- Viewport Panel ---" << std::endl;
    test_viewport_defaults();
    test_viewport_load_ship();
    test_viewport_load_station();
    test_viewport_clear_scene();
    test_viewport_select_object();
    test_viewport_deselect_all();
    test_viewport_translate_selected();
    test_viewport_rotate_selected();
    test_viewport_scale_selected();
    test_viewport_scale_clamps_positive();
    test_viewport_gizmo_mode();
    test_viewport_camera_orbit();
    test_viewport_camera_pitch_clamp();
    test_viewport_camera_distance();
    test_viewport_commit_changes();
    test_viewport_discard_changes();
    test_viewport_grid_toggle();
    test_viewport_draw_does_not_crash();
    test_viewport_log_entries();
    test_viewport_no_op_without_selection();

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
