#include <iostream>
#include <string>
#include <cstring>
#include "test_log.h"

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

// Editor Panel tests — ECS Inspector
void test_ecsi_defaults();
void test_ecsi_select_entity();
void test_ecsi_select_dead_entity_ignored();
void test_ecsi_destroy_selected();
void test_ecsi_clear_selection();
void test_ecsi_search_filter_by_id();
void test_ecsi_search_filter_empty_shows_all();
void test_ecsi_draw_clears_dead_selection();
void test_ecsi_name();
void test_ecsi_visibility();

// Editor Panel tests — Network Inspector
void test_neti_defaults();
void test_neti_mode_to_string();
void test_neti_select_peer();
void test_neti_select_nonexistent_peer_ignored();
void test_neti_clear_peer_selection();
void test_neti_stats_with_peers();
void test_neti_draw_clears_disconnected_peer();
void test_neti_name();

// Editor Panel tests — Game Packager
void test_pkg_defaults();
void test_pkg_set_settings();
void test_pkg_start_package();
void test_pkg_advance_full_pipeline();
void test_pkg_cancel_package();
void test_pkg_empty_output_path_fails();
void test_pkg_is_packaging();
void test_pkg_step_to_string();
void test_pkg_target_to_string();
void test_pkg_mode_to_string();
void test_pkg_log_messages();

// Editor Panel tests — AI Aggregator
void test_ai_no_backends();
void test_ai_register_null_ignored();
void test_ai_single_backend();
void test_ai_best_confidence_wins();

// Editor Panel tests — Dock Layout
void test_dock_tab_draw();
void test_dock_tab_switch();
void test_dock_register_panels();

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

int main(int argc, char* argv[]) {
    std::string logPath;
    for (int i = 1; i < argc; ++i) {
        if ((std::strcmp(argv[i], "--log") == 0) && i + 1 < argc) {
            logPath = argv[++i];
        }
    }

    auto& log = atlas::test::TestLog::Instance();

    std::cout << "=== Atlas Engine Tests ===" << std::endl;

    // GraphVM
    log.BeginSection("Graph VM");
    RUN_TEST(test_basic_arithmetic);
    RUN_TEST(test_subtraction);
    RUN_TEST(test_multiplication);
    RUN_TEST(test_division);
    RUN_TEST(test_division_by_zero);
    RUN_TEST(test_comparison);
    RUN_TEST(test_conditional_jump);
    RUN_TEST(test_variables);

    // ECS
    log.BeginSection("ECS");
    RUN_TEST(test_create_entity);
    RUN_TEST(test_destroy_entity);
    RUN_TEST(test_tick_callback);

    // ECS Components
    log.BeginSection("ECS Components");
    RUN_TEST(test_add_and_get_component);
    RUN_TEST(test_has_component);
    RUN_TEST(test_remove_component);
    RUN_TEST(test_multiple_components);
    RUN_TEST(test_destroy_entity_removes_components);
    RUN_TEST(test_component_update);

    // Assets
    log.BeginSection("Asset System");
    RUN_TEST(test_asset_binary_roundtrip);
    RUN_TEST(test_asset_registry_scan);

    // Networking
    log.BeginSection("Networking");
    RUN_TEST(test_net_init);
    RUN_TEST(test_net_authority);
    RUN_TEST(test_net_shutdown);

    // Network Queue
    log.BeginSection("Network Queue");
    RUN_TEST(test_net_add_peer);
    RUN_TEST(test_net_remove_peer);
    RUN_TEST(test_net_send_receive);
    RUN_TEST(test_net_broadcast_receive);
    RUN_TEST(test_net_shutdown_clears_queues);

    // Network Quality Monitor
    log.BeginSection("Network Quality Monitor");
    RUN_TEST(test_nqm_initial_rtt);
    RUN_TEST(test_nqm_rtt_ewma);
    RUN_TEST(test_nqm_stable_rtt_low_jitter);
    RUN_TEST(test_nqm_negative_rtt_ignored);
    RUN_TEST(test_nqm_no_loss);
    RUN_TEST(test_nqm_50_percent_loss);
    RUN_TEST(test_nqm_window_trims);
    RUN_TEST(test_nqm_empty_loss_rate);
    RUN_TEST(test_nqm_interp_floor);
    RUN_TEST(test_nqm_interp_ceiling);
    RUN_TEST(test_nqm_interp_increases_with_jitter);
    RUN_TEST(test_nqm_interp_increases_with_loss);
    RUN_TEST(test_nqm_reset);
    RUN_TEST(test_net_send_assigns_sequence);
    RUN_TEST(test_net_broadcast_assigns_sequence);
    RUN_TEST(test_net_sequence_resets_on_init);

    // World
    log.BeginSection("World Layout");
    RUN_TEST(test_cube_sphere_projection);
    RUN_TEST(test_cube_sphere_chunk_roundtrip);
    RUN_TEST(test_cube_sphere_neighbors);
    RUN_TEST(test_cube_sphere_lod);
    RUN_TEST(test_voxel_chunk_roundtrip);
    RUN_TEST(test_voxel_neighbors);

    // Compiler
    log.BeginSection("Graph Compiler");
    RUN_TEST(test_compile_constants_and_add);
    RUN_TEST(test_compile_and_execute_full);
    RUN_TEST(test_compile_multiply);

    // Engine
    log.BeginSection("Engine");
    RUN_TEST(test_engine_init_and_shutdown);
    RUN_TEST(test_engine_run_loop_ticks);
    RUN_TEST(test_engine_capabilities);
    RUN_TEST(test_engine_net_mode_from_config);

    // Console
    log.BeginSection("Console");
    RUN_TEST(test_console_spawn_entity);
    RUN_TEST(test_console_ecs_dump);
    RUN_TEST(test_console_set_tickrate);
    RUN_TEST(test_console_net_mode);
    RUN_TEST(test_console_help);
    RUN_TEST(test_console_unknown_command);

    // HUD Panels
    log.BeginSection("HUD Panels");
    RUN_TEST(test_station_panel_defaults);
    RUN_TEST(test_station_panel_toggle);
    RUN_TEST(test_station_panel_set_data);
    RUN_TEST(test_station_panel_callbacks);
    RUN_TEST(test_inventory_panel_defaults);
    RUN_TEST(test_inventory_panel_set_data);
    RUN_TEST(test_fitting_panel_defaults);
    RUN_TEST(test_fitting_panel_set_data);
    RUN_TEST(test_market_panel_defaults);
    RUN_TEST(test_market_panel_set_data);
    RUN_TEST(test_fleet_panel_defaults);
    RUN_TEST(test_fleet_panel_toggle);
    RUN_TEST(test_fleet_panel_set_data);
    RUN_TEST(test_existing_panel_toggles);
    RUN_TEST(test_overview_tab_filter);

    // Game State & ViewMode
#ifndef ATLAS_NO_GLM
    log.BeginSection("Game State & ViewMode");
    RUN_TEST(test_camera_default_view_mode);
    RUN_TEST(test_camera_set_view_mode_orbit);
    RUN_TEST(test_camera_set_view_mode_fps);
    RUN_TEST(test_camera_set_view_mode_cockpit);
    RUN_TEST(test_camera_fps_position_and_forward);
    RUN_TEST(test_camera_fps_rotate);
    RUN_TEST(test_camera_orbit_position_unchanged_in_fps);
    RUN_TEST(test_camera_view_mode_kills_inertia);
    RUN_TEST(test_camera_view_matrix_differs_by_mode);
#else
    log.BeginSection("Game State & ViewMode (SKIPPED — GLM not available)");
#endif

    // PCG Preview Panel
    log.BeginSection("PCG Preview Panel");
    RUN_TEST(test_pcg_preview_defaults);
    RUN_TEST(test_pcg_preview_generate_ship);
    RUN_TEST(test_pcg_preview_generate_ship_override_hull);
    RUN_TEST(test_pcg_preview_generate_station);
    RUN_TEST(test_pcg_preview_generate_station_override_count);
    RUN_TEST(test_pcg_preview_generate_interior);
    RUN_TEST(test_pcg_preview_determinism);
    RUN_TEST(test_pcg_preview_randomize_changes_seed);
    RUN_TEST(test_pcg_preview_clear);
    RUN_TEST(test_pcg_preview_set_settings);
    RUN_TEST(test_pcg_preview_different_seeds_differ);
    RUN_TEST(test_pcg_preview_draw_does_not_crash);

    // Low-Poly Character Generator
    log.BeginSection("Low-Poly Character Generator");
    RUN_TEST(test_lowpoly_char_generate_default);
    RUN_TEST(test_lowpoly_char_body_slot_count);
    RUN_TEST(test_lowpoly_char_override_archetype);
    RUN_TEST(test_lowpoly_char_override_gender);
    RUN_TEST(test_lowpoly_char_palette_regions);
    RUN_TEST(test_lowpoly_char_flat_shading);
    RUN_TEST(test_lowpoly_char_fps_arms);
    RUN_TEST(test_lowpoly_char_determinism);
    RUN_TEST(test_lowpoly_char_different_seeds_differ);
    RUN_TEST(test_lowpoly_char_clothing_always_has_basics);
    RUN_TEST(test_lowpoly_char_archetype_names);
    RUN_TEST(test_lowpoly_char_body_slot_names);
    RUN_TEST(test_lowpoly_char_clothing_slot_names);
    RUN_TEST(test_pcg_preview_generate_character);
    RUN_TEST(test_pcg_preview_character_override_archetype);
    RUN_TEST(test_pcg_preview_character_override_gender);
    RUN_TEST(test_pcg_preview_character_clear);

    // Generation Style Engine
    log.BeginSection("Generation Style Engine");
    RUN_TEST(test_gs_create_default_ship_style);
    RUN_TEST(test_gs_create_default_station_style);
    RUN_TEST(test_gs_create_all_default_styles);
    RUN_TEST(test_gs_validate_valid_style);
    RUN_TEST(test_gs_validate_empty_name_fails);
    RUN_TEST(test_gs_validate_out_of_range_param_fails);
    RUN_TEST(test_gs_validate_duplicate_slots_fails);
    RUN_TEST(test_gs_available_parameters);
    RUN_TEST(test_gs_find_parameter);
    RUN_TEST(test_gs_style_type_names);
    RUN_TEST(test_gs_generate_ship_style);
    RUN_TEST(test_gs_generate_station_style);
    RUN_TEST(test_gs_generate_station_with_placements);
    RUN_TEST(test_gs_generate_interior_style);
    RUN_TEST(test_gs_generate_determinism);
    RUN_TEST(test_gs_serialize_roundtrip);
    RUN_TEST(test_gs_ship_parameter_overrides);

    // Generation Style Panel
    log.BeginSection("Generation Style Panel");
    RUN_TEST(test_gsp_defaults);
    RUN_TEST(test_gsp_new_style);
    RUN_TEST(test_gsp_add_remove_placement);
    RUN_TEST(test_gsp_set_parameter);
    RUN_TEST(test_gsp_enable_disable_parameter);
    RUN_TEST(test_gsp_generate);
    RUN_TEST(test_gsp_generate_with_asset_style);
    RUN_TEST(test_gsp_save_load_string);
    RUN_TEST(test_gsp_draw_does_not_crash);

    // Asset Style Library
    log.BeginSection("Asset Style Library");
    RUN_TEST(test_as_library_add_find);
    RUN_TEST(test_as_library_replace);
    RUN_TEST(test_as_library_remove);
    RUN_TEST(test_as_library_list_by_type);
    RUN_TEST(test_as_library_clear);
    RUN_TEST(test_as_shape_apply_to_ship);
    RUN_TEST(test_as_shape_apply_to_station);
    RUN_TEST(test_as_surface_treatment_names);
    RUN_TEST(test_as_serialize_roundtrip);

    // Asset Style Panel
    log.BeginSection("Asset Style Panel");
    RUN_TEST(test_asp_defaults);
    RUN_TEST(test_asp_new_style);
    RUN_TEST(test_asp_shape_control_points);
    RUN_TEST(test_asp_mirror_and_smoothing);
    RUN_TEST(test_asp_palette_colors);
    RUN_TEST(test_asp_palette_materials);
    RUN_TEST(test_asp_surface_treatment);
    RUN_TEST(test_asp_detail_level);
    RUN_TEST(test_asp_apply_and_preview_ship);
    RUN_TEST(test_asp_apply_and_preview_station);
    RUN_TEST(test_asp_library_save_load);
    RUN_TEST(test_asp_serialize_roundtrip);
    RUN_TEST(test_asp_draw_does_not_crash);

    // Ship Archetype Engine
    log.BeginSection("Ship Archetype Engine");
    RUN_TEST(test_arch_create_default_frigate);
    RUN_TEST(test_arch_create_all_hull_classes);
    RUN_TEST(test_arch_capital_bigger_than_frigate);
    RUN_TEST(test_arch_validate_valid);
    RUN_TEST(test_arch_validate_empty_name_fails);
    RUN_TEST(test_arch_validate_no_rooms_fails);
    RUN_TEST(test_arch_validate_no_hardpoints_fails);
    RUN_TEST(test_arch_validate_duplicate_hardpoint_ids_fails);
    RUN_TEST(test_arch_validate_door_bad_room_ref_fails);
    RUN_TEST(test_arch_generate_from_archetype);
    RUN_TEST(test_arch_generate_determinism);
    RUN_TEST(test_arch_apply_subsystems);
    RUN_TEST(test_arch_apply_module_visuals);
    RUN_TEST(test_arch_subsystem_type_names);
    RUN_TEST(test_arch_serialize_roundtrip);
    RUN_TEST(test_arch_different_seeds_differ);

    // Ship Archetype Panel
    log.BeginSection("Ship Archetype Panel");
    RUN_TEST(test_archp_defaults);
    RUN_TEST(test_archp_select_hull_class);
    RUN_TEST(test_archp_add_remove_hull_control_point);
    RUN_TEST(test_archp_add_remove_room);
    RUN_TEST(test_archp_add_remove_door);
    RUN_TEST(test_archp_add_remove_hardpoint);
    RUN_TEST(test_archp_subsystem_editing);
    RUN_TEST(test_archp_module_visual_rules);
    RUN_TEST(test_archp_variation_bounds);
    RUN_TEST(test_archp_generate_preview);
    RUN_TEST(test_archp_generate_preview_with_subsystems);
    RUN_TEST(test_archp_generate_preview_with_modules);
    RUN_TEST(test_archp_save_load_string);
    RUN_TEST(test_archp_draw_does_not_crash);

    // Viewport Panel
    log.BeginSection("Viewport Panel");
    RUN_TEST(test_viewport_defaults);
    RUN_TEST(test_viewport_load_ship);
    RUN_TEST(test_viewport_load_station);
    RUN_TEST(test_viewport_clear_scene);
    RUN_TEST(test_viewport_select_object);
    RUN_TEST(test_viewport_deselect_all);
    RUN_TEST(test_viewport_translate_selected);
    RUN_TEST(test_viewport_rotate_selected);
    RUN_TEST(test_viewport_scale_selected);
    RUN_TEST(test_viewport_scale_clamps_positive);
    RUN_TEST(test_viewport_gizmo_mode);
    RUN_TEST(test_viewport_camera_orbit);
    RUN_TEST(test_viewport_camera_pitch_clamp);
    RUN_TEST(test_viewport_camera_distance);
    RUN_TEST(test_viewport_commit_changes);
    RUN_TEST(test_viewport_discard_changes);
    RUN_TEST(test_viewport_grid_toggle);
    RUN_TEST(test_viewport_draw_does_not_crash);
    RUN_TEST(test_viewport_log_entries);
    RUN_TEST(test_viewport_no_op_without_selection);

    // ECS Inspector
    log.BeginSection("ECS Inspector");
    RUN_TEST(test_ecsi_defaults);
    RUN_TEST(test_ecsi_select_entity);
    RUN_TEST(test_ecsi_select_dead_entity_ignored);
    RUN_TEST(test_ecsi_destroy_selected);
    RUN_TEST(test_ecsi_clear_selection);
    RUN_TEST(test_ecsi_search_filter_by_id);
    RUN_TEST(test_ecsi_search_filter_empty_shows_all);
    RUN_TEST(test_ecsi_draw_clears_dead_selection);
    RUN_TEST(test_ecsi_name);
    RUN_TEST(test_ecsi_visibility);

    // Network Inspector
    log.BeginSection("Network Inspector");
    RUN_TEST(test_neti_defaults);
    RUN_TEST(test_neti_mode_to_string);
    RUN_TEST(test_neti_select_peer);
    RUN_TEST(test_neti_select_nonexistent_peer_ignored);
    RUN_TEST(test_neti_clear_peer_selection);
    RUN_TEST(test_neti_stats_with_peers);
    RUN_TEST(test_neti_draw_clears_disconnected_peer);
    RUN_TEST(test_neti_name);

    // Game Packager
    log.BeginSection("Game Packager");
    RUN_TEST(test_pkg_defaults);
    RUN_TEST(test_pkg_set_settings);
    RUN_TEST(test_pkg_start_package);
    RUN_TEST(test_pkg_advance_full_pipeline);
    RUN_TEST(test_pkg_cancel_package);
    RUN_TEST(test_pkg_empty_output_path_fails);
    RUN_TEST(test_pkg_is_packaging);
    RUN_TEST(test_pkg_step_to_string);
    RUN_TEST(test_pkg_target_to_string);
    RUN_TEST(test_pkg_mode_to_string);
    RUN_TEST(test_pkg_log_messages);

    // AI Aggregator
    log.BeginSection("AI Aggregator");
    RUN_TEST(test_ai_no_backends);
    RUN_TEST(test_ai_register_null_ignored);
    RUN_TEST(test_ai_single_backend);
    RUN_TEST(test_ai_best_confidence_wins);

    // Dock Layout
    log.BeginSection("Dock Layout");
    RUN_TEST(test_dock_tab_draw);
    RUN_TEST(test_dock_tab_switch);
    RUN_TEST(test_dock_register_panels);

    if (!logPath.empty()) {
        log.WriteLogFile(logPath);
    }

    int exitCode = log.PrintSummary();
    atlas::test::TestLog::WaitOnWindows();
    return exitCode;
}
