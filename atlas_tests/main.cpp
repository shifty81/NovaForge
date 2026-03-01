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
void test_console_ecs_count();
void test_console_ecs_destroy();
void test_console_ecs_destroy_invalid();
void test_console_net_stats();
void test_console_net_peers();
void test_console_clear();
void test_console_ai_query();
void test_console_ai_query_empty();
void test_console_list_command();
void test_console_status_command();

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
void test_pcg_preview_generate_spine_hull();
void test_pcg_preview_spine_hull_override_hull_class();
void test_pcg_preview_spine_hull_override_faction();
void test_pcg_preview_spine_hull_determinism();
void test_pcg_preview_spine_hull_aspect_ratio_clamped();
void test_pcg_preview_generate_turret_placement();
void test_pcg_preview_turret_placement_override_slots();
void test_pcg_preview_turret_placement_with_faction();
void test_pcg_preview_turret_placement_overlap_validation();
void test_pcg_preview_clear_includes_new_modes();

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

// Character Select Panel tests
#ifndef ATLAS_NO_GLM
void test_charsel_defaults();
void test_charsel_generate();
void test_charsel_set_archetype();
void test_charsel_set_gender();
void test_charsel_body_sliders();
void test_charsel_randomize();
void test_charsel_clear_preview();
void test_charsel_orbit_camera();
void test_charsel_orbit_camera_pitch_clamp();
void test_charsel_camera_distance();
void test_charsel_reference_mesh_archive();
void test_charsel_determinism();
void test_charsel_draw_does_not_crash();
void test_charsel_panel_name();
void test_charsel_log_entries();
#endif

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

// Star System generation tests
void test_gs_generate_star_system();
void test_gs_star_system_planet_count_override();
void test_gs_star_system_gate_count_override();
void test_gs_star_system_security_level();
void test_gs_star_system_with_placements();
void test_gs_star_system_determinism();

// Asteroid Field generation tests
void test_gs_generate_asteroid_field();
void test_gs_asteroid_field_density_affects_count();
void test_gs_asteroid_field_with_placements();
void test_gs_asteroid_field_determinism();

// Fleet Composition generation tests
void test_gs_generate_fleet_composition();
void test_gs_fleet_size_override();
void test_gs_fleet_capital_ratio();
void test_gs_fleet_support_ratio();
void test_gs_fleet_with_placements();
void test_gs_fleet_determinism();

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
void test_gsp_save_load_file();
void test_gsp_load_nonexistent_file();

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
void test_archp_save_load_file();
void test_archp_load_nonexistent_file();

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
void test_pkg_settings_target_toggle();
void test_pkg_settings_mode_cycle();
void test_pkg_settings_options();
void test_keybind_viewport_gizmo_wiring();
void test_keybind_panel_toggle_wiring();
void test_keybind_save_wiring();
void test_keybind_delete_wiring();
void test_ecsi_component_types_visible();
void test_undo_viewport_transform();

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
void test_viewport_load_spine_hull();
void test_viewport_load_spine_hull_with_turrets();
void test_viewport_spine_hull_zone_names();
void test_viewport_spine_hull_select_and_transform();
void test_viewport_load_character();
void test_viewport_load_character_body_part_type();
void test_viewport_load_character_clothing_type();
void test_viewport_load_character_select_and_transform();
void test_viewport_load_character_log_entry();
void test_viewport_load_character_female();
void test_viewport_load_character_clear_scene();

// PCG Override Store tests
void test_override_store_empty();
void test_override_store_add_and_clear();
void test_override_store_remove_by_object();
void test_override_store_serialize_roundtrip();
void test_override_store_file_roundtrip();
void test_override_store_load_nonexistent();
void test_override_store_import_from_viewport();

// Live Scene Manager tests
void test_live_scene_defaults();
void test_live_scene_populate();
void test_live_scene_capture_changes();
void test_live_scene_save_and_load();
void test_live_scene_regenerate();
void test_live_scene_on_asset_reload_triggers_regen();
void test_live_scene_seed_and_version();

// Galaxy Generator tests
void test_galaxy_generate_default();
void test_galaxy_system_count_clamped();
void test_galaxy_security_zone_counts();
void test_galaxy_all_nodes_connected();
void test_galaxy_chokepoints_exist();
void test_galaxy_determinism();
void test_galaxy_different_seeds_differ();
void test_galaxy_security_level_range();
void test_galaxy_security_zone_name();
void test_galaxy_route_distances_positive();

// Economy-Driven Generator tests
void test_economy_generate_default();
void test_economy_resource_rich_has_miners();
void test_economy_lawless_has_pirates();
void test_economy_quality_range();
void test_economy_determinism();
void test_economy_state_names();
void test_economy_role_names();
void test_economy_war_torn_has_armed_ships();

// NPC Encounter Generator tests
void test_encounter_generate_default();
void test_encounter_wave_count_explicit();
void test_encounter_high_sec_easier();
void test_encounter_null_sec_higher_bounty();
void test_encounter_determinism();
void test_encounter_calculate_bounty();
void test_encounter_wave_trigger_delays();

// Damage State Generator tests
void test_damage_generate_pristine();
void test_damage_generate_critical();
void test_damage_level_progression();
void test_damage_decals_increase_with_severity();
void test_damage_determinism();
void test_damage_structural_integrity_range();
void test_damage_level_names();
void test_damage_decal_type_names();

// Galaxy Map Panel tests
void test_galaxy_panel_defaults();
void test_galaxy_panel_generate();
void test_galaxy_panel_chokepoints();
void test_galaxy_panel_system_count_clamped_low();
void test_galaxy_panel_system_count_clamped_high();
void test_galaxy_panel_log_after_generate();
void test_galaxy_panel_filter_all();
void test_galaxy_panel_filter_highsec();
void test_galaxy_panel_filter_lowsec();
void test_galaxy_panel_filter_nullsec();
void test_galaxy_panel_filter_sum_equals_total();
void test_galaxy_panel_select_system();
void test_galaxy_panel_select_nonexistent_ignored();
void test_galaxy_panel_clear_selection();
void test_galaxy_panel_generate_clears_selection();
void test_galaxy_panel_determinism();
void test_galaxy_panel_different_seeds_differ();
void test_galaxy_panel_draw_does_not_crash();

// EventBus tests
void test_eventbus_initial_state();
void test_eventbus_subscribe_and_count();
void test_eventbus_subscribe_null_callback();
void test_eventbus_unsubscribe();
void test_eventbus_unsubscribe_invalid_noop();
void test_eventbus_publish_invokes_callback();
void test_eventbus_publish_only_matching_type();
void test_eventbus_multiple_subscribers_same_type();
void test_eventbus_wildcard_receives_all();
void test_eventbus_enqueue_and_flush();
void test_eventbus_flush_fifo_order();
void test_eventbus_enqueue_during_flush_deferred();
void test_eventbus_total_published();
void test_eventbus_reset();
void test_eventbus_sender_id_and_str_param();
void test_eventbus_no_subscribers_no_crash();
void test_eventbus_unsubscribe_wildcard();

// Fleet Formation Panel tests
void test_fmtn_defaults();
void test_fmtn_set_formation_type();
void test_fmtn_formation_type_names();
void test_fmtn_set_fleet_size();
void test_fmtn_fleet_size_clamped_low();
void test_fmtn_fleet_size_clamped_high();
void test_fmtn_set_spacing();
void test_fmtn_spacing_clamped();
void test_fmtn_commander_at_origin();
void test_fmtn_arrow_offsets_nonzero();
void test_fmtn_line_offsets_lateral();
void test_fmtn_spread_circular();
void test_fmtn_bounding_radius();
void test_fmtn_centre_of_mass();
void test_fmtn_select_slot();
void test_fmtn_select_invalid_ignored();
void test_fmtn_import_fleet();
void test_fmtn_none_formation_no_offsets();
void test_fmtn_spacing_affects_offsets();
void test_fmtn_draw_does_not_crash();
void test_fmtn_log_after_actions();

// Mission Editor Panel tests
void test_mission_editor_defaults();
void test_mission_editor_add_template();
void test_mission_editor_add_multiple();
void test_mission_editor_remove_template();
void test_mission_editor_remove_out_of_range();
void test_mission_editor_update_template();
void test_mission_editor_update_out_of_range();
void test_mission_editor_select_template();
void test_mission_editor_select_invalid_ignored();
void test_mission_editor_clear_selection();
void test_mission_editor_remove_fixes_selection();
void test_mission_editor_type_filter();
void test_mission_editor_level_filter();
void test_mission_editor_combined_filter();
void test_mission_editor_validate_valid();
void test_mission_editor_validate_empty_id();
void test_mission_editor_validate_empty_type();
void test_mission_editor_validate_bad_level();
void test_mission_editor_validate_no_objectives();
void test_mission_editor_validate_bad_objective();
void test_mission_editor_validate_negative_isk();
void test_mission_editor_validate_all();
void test_mission_editor_export_json();
void test_mission_editor_import_json();
void test_mission_editor_draw_does_not_crash();
void test_mission_editor_log_after_actions();

// Undo Stack tests
void test_undo_stack_defaults();
void test_undo_stack_push_and_undo();
void test_undo_stack_redo();
void test_undo_stack_multiple_actions();
void test_undo_stack_push_clears_redo();
void test_undo_stack_clear();
void test_undo_stack_max_depth();
void test_undo_stack_empty_undo_returns_false();
void test_undo_stack_empty_redo_returns_false();

// Template AI Backend tests
void test_template_ai_defaults_installed();
void test_template_ai_query_match();
void test_template_ai_query_no_match();
void test_template_ai_case_insensitive();
void test_template_ai_best_match_wins();
void test_template_ai_multiple_matches_boost();
void test_template_ai_add_and_remove();
void test_template_ai_remove_nonexistent();
void test_template_ai_add_empty_keyword_ignored();
void test_template_ai_clear_templates();
void test_template_ai_with_aggregator();

// NetworkInterpolationBuffer tests
void test_interp_defaults();
void test_interp_custom_config();
void test_interp_push_snapshot();
void test_interp_push_multiple_entities();
void test_interp_buffer_depth_limit();
void test_interp_lerp_midpoint();
void test_interp_lerp_quarter();
void test_interp_lerp_at_exact_tick();
void test_interp_clamp_before_first();
void test_interp_extrapolation();
void test_interp_extrapolation_max_exceeded();
void test_interp_unknown_entity();
void test_interp_remove_entity();
void test_interp_clear();
void test_interp_duplicate_tick_replaces();
void test_interp_out_of_order_ticks();
void test_interp_multiple_segments();
void test_interp_snapshot_count_nonexistent();

// DeltaCompression tests
void test_dc_defaults();
void test_dc_custom_interval();
void test_dc_zero_interval_clamped();
void test_dc_quantize_position();
void test_dc_quantize_negative();
void test_dc_quantize_rotation();
void test_dc_quantize_zero();
void test_dc_keyframe_roundtrip();
void test_dc_first_encode_is_keyframe();
void test_dc_delta_after_keyframe();
void test_dc_delta_values_small();
void test_dc_delta_stationary_entity();
void test_dc_multiple_entities();
void test_dc_periodic_keyframe();
void test_dc_force_keyframe();
void test_dc_force_all_keyframes();
void test_dc_remove_entity();
void test_dc_clear();
void test_dc_decode_delta_without_baseline();
void test_dc_empty_encode();
void test_dc_encode_decode_full_sequence();

// JitterBuffer tests
void test_jb_defaults();
void test_jb_custom_config();
void test_jb_min_delay_clamped();
void test_jb_push_single();
void test_jb_flush_after_delay();
void test_jb_flush_releases_in_order();
void test_jb_flush_partial();
void test_jb_late_packet_dropped();
void test_jb_overflow_trims();
void test_jb_out_of_order_sorted();
void test_jb_duplicate_tick_replaces();
void test_jb_adaptive_adjusts_delay();
void test_jb_non_adaptive_fixed_delay();
void test_jb_reset();
void test_jb_flush_empty();

// Lag Compensation tests
void test_lc_defaults();
void test_lc_custom_config();
void test_lc_zero_history_clamped();
void test_lc_store_single();
void test_lc_store_multiple_ticks();
void test_lc_store_multiple_entities();
void test_lc_store_prunes_old();
void test_lc_get_exact_tick();
void test_lc_get_interpolated();
void test_lc_get_at_latest_tick();
void test_lc_get_beyond_latest_returns_latest();
void test_lc_get_before_earliest_invalid();
void test_lc_get_unknown_entity();
void test_lc_interpolation_3d();
void test_lc_hit_exact_position();
void test_lc_hit_within_radius();
void test_lc_miss_outside_radius();
void test_lc_hit_at_interpolated_position();
void test_lc_hit_unknown_target();
void test_lc_hit_3d_distance();
void test_lc_clamp_rewind_too_far();
void test_lc_clamp_rewind_within_range();
void test_lc_clamp_rewind_future();
void test_lc_remove_entity();
void test_lc_remove_nonexistent();
void test_lc_clear();
void test_lc_realistic_combat_scenario();
void test_lc_high_latency_scenario();
void test_lc_boundary_hit_radius();
void test_lc_snapshot_count_nonexistent();
void test_lc_zero_max_rewind();

// SceneGraphPanel tests
void test_sg_defaults();
void test_sg_add_node();
void test_sg_add_child_node();
void test_sg_add_child_invalid_parent();
void test_sg_remove_node();
void test_sg_remove_cascades_children();
void test_sg_remove_nonexistent();
void test_sg_rename_node();
void test_sg_rename_nonexistent();
void test_sg_reparent();
void test_sg_reparent_to_root();
void test_sg_reparent_self_rejected();
void test_sg_reparent_cycle_prevented();
void test_sg_reparent_nonexistent_parent();
void test_sg_root_nodes();
void test_sg_children();
void test_sg_depth();
void test_sg_select_node();
void test_sg_select_nonexistent_ignored();
void test_sg_clear_selection();
void test_sg_remove_clears_selection();
void test_sg_select_locked_ignored();
void test_sg_set_visible();
void test_sg_set_locked();
void test_sg_lock_deselects();
void test_sg_toggle_expanded();
void test_sg_expand_all();
void test_sg_collapse_all();
void test_sg_search_filter();
void test_sg_search_by_type();
void test_sg_draw_does_not_crash();
void test_sg_log_after_actions();

// KeybindManager tests
void test_kb_defaults_installed();
void test_kb_clear();
void test_kb_add_binding();
void test_kb_add_duplicate_action_rejected();
void test_kb_add_empty_action_rejected();
void test_kb_remove_binding();
void test_kb_remove_nonexistent();
void test_kb_rebind();
void test_kb_rebind_nonexistent();
void test_kb_set_enabled();
void test_kb_find_by_key();
void test_kb_bindings_in_category();
void test_kb_conflict_detection();
void test_kb_conflict_with_exclude();
void test_kb_handle_key_press();
void test_kb_handle_key_press_no_match();
void test_kb_disabled_binding_not_dispatched();
void test_kb_callback_replacement();
void test_kb_describe_binding();
void test_kb_describe_action();
void test_kb_default_bindings();
void test_kb_serialize_roundtrip();
void test_kb_deserialize_empty_returns_false();
void test_kb_file_roundtrip();
void test_kb_load_nonexistent_returns_false();

// Alert Stack tests
void test_alert_defaults();
void test_alert_custom_max();
void test_alert_push();
void test_alert_push_multiple();
void test_alert_active_sorted_by_priority();
void test_alert_category_filter();
void test_alert_tick_expiry();
void test_alert_tick_ages_alerts();
void test_alert_dismiss();
void test_alert_dismiss_nonexistent();
void test_alert_dismiss_category();
void test_alert_eviction();
void test_alert_clear();
void test_alert_ids_monotonic();
void test_alert_same_priority_ordered_by_age();

// AI Miner State Machine tests
void test_miner_defaults();
void test_miner_add();
void test_miner_add_duplicate_rejected();
void test_miner_remove();
void test_miner_remove_nonexistent();
void test_miner_idle_without_deposits();
void test_miner_full_cycle();
void test_miner_travel_progress();
void test_miner_cargo_accumulation();
void test_miner_kill();
void test_miner_kill_nonexistent();
void test_miner_respawn();
void test_miner_count_in_state();
void test_miner_field_distance();
void test_miner_multiple_miners();

// AI Hauler State Machine tests
void test_hauler_defaults();
void test_hauler_add();
void test_hauler_add_duplicate_rejected();
void test_hauler_remove();
void test_hauler_remove_nonexistent();
void test_hauler_idle_without_cargo();
void test_hauler_full_cycle();
void test_hauler_travel_progress();
void test_hauler_picks_best_cargo();
void test_hauler_kill();
void test_hauler_kill_nonexistent();
void test_hauler_respawn();
void test_hauler_count_in_state();
void test_hauler_station_distance();
void test_hauler_multiple_haulers();

// AI Pirate State Machine tests
void test_pirate_defaults();
void test_pirate_add();
void test_pirate_add_duplicate_rejected();
void test_pirate_remove();
void test_pirate_remove_nonexistent();
void test_pirate_patrol_without_targets();
void test_pirate_full_attack_cycle();
void test_pirate_travel_progress();
void test_pirate_damage();
void test_pirate_damage_kills();
void test_pirate_damage_nonexistent();
void test_pirate_kill();
void test_pirate_kill_nonexistent();
void test_pirate_respawn();
void test_pirate_count_in_state();
void test_pirate_patrol_distance();
void test_pirate_multiple_pirates();

// AI Security State Machine tests
void test_security_defaults();
void test_security_add();
void test_security_add_duplicate_rejected();
void test_security_remove();
void test_security_remove_nonexistent();
void test_security_standby_without_threats();
void test_security_full_engagement_cycle();
void test_security_warp_progress();
void test_security_damage();
void test_security_damage_kills();
void test_security_damage_nonexistent();
void test_security_kill();
void test_security_kill_nonexistent();
void test_security_respawn();
void test_security_count_in_state();
void test_security_post_distance();
void test_security_multiple_guards();

// AI Wallet System tests
void test_wallet_defaults();
void test_wallet_create();
void test_wallet_create_duplicate_rejected();
void test_wallet_create_negative_balance_rejected();
void test_wallet_remove();
void test_wallet_remove_nonexistent();
void test_wallet_deposit();
void test_wallet_deposit_nonexistent();
void test_wallet_deposit_zero_rejected();
void test_wallet_withdraw();
void test_wallet_withdraw_insufficient();
void test_wallet_withdraw_zero_rejected();
void test_wallet_transfer();
void test_wallet_transfer_insufficient();
void test_wallet_transfer_self_rejected();
void test_wallet_total_circulation();
void test_wallet_wealthiest_entity();
void test_wallet_clear();
void test_wallet_get_balance_nonexistent();

// AI Trader State Machine tests
void test_trader_defaults();
void test_trader_add();
void test_trader_add_duplicate_rejected();
void test_trader_remove();
void test_trader_remove_nonexistent();
void test_trader_idle_without_listings();
void test_trader_full_cycle();
void test_trader_travel_progress();
void test_trader_finds_best_trade();
void test_trader_damage();
void test_trader_damage_kills();
void test_trader_damage_nonexistent();
void test_trader_kill();
void test_trader_kill_nonexistent();
void test_trader_respawn();
void test_trader_count_in_state();
void test_trader_default_distance();
void test_trader_multiple_traders();

// AI Industrialist State Machine tests
void test_industrialist_defaults();
void test_industrialist_add();
void test_industrialist_add_duplicate_rejected();
void test_industrialist_remove();
void test_industrialist_remove_nonexistent();
void test_industrialist_idle_without_blueprints();
void test_industrialist_full_cycle();
void test_industrialist_build_progress();
void test_industrialist_picks_best_blueprint();
void test_industrialist_cant_afford();
void test_industrialist_kill();
void test_industrialist_kill_nonexistent();
void test_industrialist_respawn();
void test_industrialist_count_in_state();
void test_industrialist_market_distance();
void test_industrialist_multiple();

// Advanced Mission Generator tests
void test_mission_gen_defaults();
void test_mission_gen_install_defaults();
void test_mission_gen_add_template();
void test_mission_gen_add_duplicate_rejected();
void test_mission_gen_remove_template();
void test_mission_gen_remove_nonexistent();
void test_mission_gen_get_by_category();
void test_mission_gen_generate_for_system();
void test_mission_gen_generate_empty_templates();
void test_mission_gen_generate_zero_count();
void test_mission_gen_from_template();
void test_mission_gen_from_nonexistent_template();
void test_mission_gen_difficulty_scaling();
void test_mission_gen_reward_scaling();
void test_mission_gen_branching();
void test_mission_gen_no_branching();
void test_mission_gen_level_filtering();
void test_mission_gen_deterministic();
void test_mission_gen_unique_ids();
void test_mission_gen_time_limit();
void test_mission_gen_standing_reward();

// Universe Map System tests
void test_universe_defaults();
void test_universe_add_system();
void test_universe_add_duplicate_rejected();
void test_universe_remove_system();
void test_universe_remove_nonexistent();
void test_universe_add_gate();
void test_universe_add_gate_duplicate_rejected();
void test_universe_add_gate_self_rejected();
void test_universe_add_gate_missing_system();
void test_universe_remove_gate();
void test_universe_remove_gate_nonexistent();
void test_universe_route_direct();
void test_universe_route_multi_hop();
void test_universe_route_no_path();
void test_universe_route_same_system();
void test_universe_route_by_distance();
void test_universe_safe_route();
void test_universe_safe_route_no_path();
void test_universe_security_classification();
void test_universe_travel_time();
void test_universe_travel_time_empty();
void test_universe_systems_by_security();
void test_universe_neighbours_nonexistent();

// AIProfileLoader tests
void test_ai_profile_defaults();
void test_ai_profile_install_defaults();
void test_ai_profile_add();
void test_ai_profile_add_overwrites();
void test_ai_profile_remove();
void test_ai_profile_remove_nonexistent();
void test_ai_profile_get_nonexistent();
void test_ai_profile_get_by_archetype();
void test_ai_profile_get_by_archetype_multiple();
void test_ai_profile_ids();
void test_ai_profile_load_empty();
void test_ai_profile_load_single();
void test_ai_profile_load_multiple();
void test_ai_profile_load_overwrites_defaults();
void test_ai_profile_all_archetypes();

// EconomyRulesLoader tests
void test_economy_rules_defaults();
void test_economy_rules_install_defaults();
void test_economy_rules_add();
void test_economy_rules_add_overwrites();
void test_economy_rules_remove();
void test_economy_rules_remove_nonexistent();
void test_economy_rules_get_nonexistent();
void test_economy_rules_security_highsec();
void test_economy_rules_security_lowsec();
void test_economy_rules_security_nullsec();
void test_economy_rules_security_boundary();
void test_economy_rules_ids();
void test_economy_rules_load_empty();
void test_economy_rules_load_single();
void test_economy_rules_load_multiple();
void test_economy_rules_load_overwrites_defaults();
void test_economy_rules_all_bands();

// Engine frame callback tests
void test_engine_frame_callback_invoked();
void test_engine_frame_callback_receives_dt();
void test_engine_frame_callback_server_mode();
void test_engine_no_callback_safe();
void test_engine_tick_count();
void test_engine_callback_replace();

// EditorLayout serialisation tests
void test_layout_serialize_empty();
void test_layout_serialize_single_panel();
void test_layout_serialize_split();
void test_layout_serialize_tabs();
void test_layout_roundtrip_single_panel();
void test_layout_roundtrip_horizontal_split();
void test_layout_roundtrip_tabs();
void test_layout_deserialize_invalid();
void test_layout_deserialize_empty_string();
void test_layout_file_roundtrip();
void test_layout_load_nonexistent_file();
void test_layout_roundtrip_nested();

// Theme loading tests
void test_theme_load_from_file();
void test_theme_load_nonexistent_file();
void test_theme_dpi_scale();
void test_theme_dpi_scale_identity();
void test_theme_font_scale_default();

// EditorMenuBar tests
void test_menubar_empty();
void test_menubar_file_menu_items();
void test_menubar_view_menu_lists_panels();
void test_menubar_pcg_menu_filters_panels();
void test_menubar_no_pcg_menu_if_no_pcg_panels();
void test_menubar_view_checked_state();
void test_menubar_draw_returns_height();
void test_menubar_initial_state();
void test_menu_item_defaults();
void test_menu_bar_state_defaults();
void test_layout_menubar_panels_registered();
void test_layout_draw_with_menubar();

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
    RUN_TEST(test_console_ecs_count);
    RUN_TEST(test_console_ecs_destroy);
    RUN_TEST(test_console_ecs_destroy_invalid);
    RUN_TEST(test_console_net_stats);
    RUN_TEST(test_console_net_peers);
    RUN_TEST(test_console_clear);
    RUN_TEST(test_console_ai_query);
    RUN_TEST(test_console_ai_query_empty);
    RUN_TEST(test_console_list_command);
    RUN_TEST(test_console_status_command);

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

    // SpineHull & TurretPlacement Preview
    log.BeginSection("SpineHull & TurretPlacement Preview");
    RUN_TEST(test_pcg_preview_generate_spine_hull);
    RUN_TEST(test_pcg_preview_spine_hull_override_hull_class);
    RUN_TEST(test_pcg_preview_spine_hull_override_faction);
    RUN_TEST(test_pcg_preview_spine_hull_determinism);
    RUN_TEST(test_pcg_preview_spine_hull_aspect_ratio_clamped);
    RUN_TEST(test_pcg_preview_generate_turret_placement);
    RUN_TEST(test_pcg_preview_turret_placement_override_slots);
    RUN_TEST(test_pcg_preview_turret_placement_with_faction);
    RUN_TEST(test_pcg_preview_turret_placement_overlap_validation);
    RUN_TEST(test_pcg_preview_clear_includes_new_modes);

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

    // Character Select Panel
#ifndef ATLAS_NO_GLM
    log.BeginSection("Character Select Panel");
    RUN_TEST(test_charsel_defaults);
    RUN_TEST(test_charsel_generate);
    RUN_TEST(test_charsel_set_archetype);
    RUN_TEST(test_charsel_set_gender);
    RUN_TEST(test_charsel_body_sliders);
    RUN_TEST(test_charsel_randomize);
    RUN_TEST(test_charsel_clear_preview);
    RUN_TEST(test_charsel_orbit_camera);
    RUN_TEST(test_charsel_orbit_camera_pitch_clamp);
    RUN_TEST(test_charsel_camera_distance);
    RUN_TEST(test_charsel_reference_mesh_archive);
    RUN_TEST(test_charsel_determinism);
    RUN_TEST(test_charsel_draw_does_not_crash);
    RUN_TEST(test_charsel_panel_name);
    RUN_TEST(test_charsel_log_entries);
#else
    log.BeginSection("Character Select Panel (SKIPPED — GLM not available)");
#endif

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

    // Star System Generation
    log.BeginSection("Star System Generation");
    RUN_TEST(test_gs_generate_star_system);
    RUN_TEST(test_gs_star_system_planet_count_override);
    RUN_TEST(test_gs_star_system_gate_count_override);
    RUN_TEST(test_gs_star_system_security_level);
    RUN_TEST(test_gs_star_system_with_placements);
    RUN_TEST(test_gs_star_system_determinism);

    // Asteroid Field Generation
    log.BeginSection("Asteroid Field Generation");
    RUN_TEST(test_gs_generate_asteroid_field);
    RUN_TEST(test_gs_asteroid_field_density_affects_count);
    RUN_TEST(test_gs_asteroid_field_with_placements);
    RUN_TEST(test_gs_asteroid_field_determinism);

    // Fleet Composition Generation
    log.BeginSection("Fleet Composition Generation");
    RUN_TEST(test_gs_generate_fleet_composition);
    RUN_TEST(test_gs_fleet_size_override);
    RUN_TEST(test_gs_fleet_capital_ratio);
    RUN_TEST(test_gs_fleet_support_ratio);
    RUN_TEST(test_gs_fleet_with_placements);
    RUN_TEST(test_gs_fleet_determinism);

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
    RUN_TEST(test_gsp_save_load_file);
    RUN_TEST(test_gsp_load_nonexistent_file);

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
    RUN_TEST(test_archp_save_load_file);
    RUN_TEST(test_archp_load_nonexistent_file);

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
    RUN_TEST(test_viewport_load_spine_hull);
    RUN_TEST(test_viewport_load_spine_hull_with_turrets);
    RUN_TEST(test_viewport_spine_hull_zone_names);
    RUN_TEST(test_viewport_spine_hull_select_and_transform);
    RUN_TEST(test_viewport_load_character);
    RUN_TEST(test_viewport_load_character_body_part_type);
    RUN_TEST(test_viewport_load_character_clothing_type);
    RUN_TEST(test_viewport_load_character_select_and_transform);
    RUN_TEST(test_viewport_load_character_log_entry);
    RUN_TEST(test_viewport_load_character_female);
    RUN_TEST(test_viewport_load_character_clear_scene);

    // PCG Override Store
    log.BeginSection("PCG Override Store");
    RUN_TEST(test_override_store_empty);
    RUN_TEST(test_override_store_add_and_clear);
    RUN_TEST(test_override_store_remove_by_object);
    RUN_TEST(test_override_store_serialize_roundtrip);
    RUN_TEST(test_override_store_file_roundtrip);
    RUN_TEST(test_override_store_load_nonexistent);
    RUN_TEST(test_override_store_import_from_viewport);

    // Galaxy Generator
    log.BeginSection("Galaxy Generator");
    RUN_TEST(test_galaxy_generate_default);
    RUN_TEST(test_galaxy_system_count_clamped);
    RUN_TEST(test_galaxy_security_zone_counts);
    RUN_TEST(test_galaxy_all_nodes_connected);
    RUN_TEST(test_galaxy_chokepoints_exist);
    RUN_TEST(test_galaxy_determinism);
    RUN_TEST(test_galaxy_different_seeds_differ);
    RUN_TEST(test_galaxy_security_level_range);
    RUN_TEST(test_galaxy_security_zone_name);
    RUN_TEST(test_galaxy_route_distances_positive);

    // Economy-Driven Generator
    log.BeginSection("Economy-Driven Generator");
    RUN_TEST(test_economy_generate_default);
    RUN_TEST(test_economy_resource_rich_has_miners);
    RUN_TEST(test_economy_lawless_has_pirates);
    RUN_TEST(test_economy_quality_range);
    RUN_TEST(test_economy_determinism);
    RUN_TEST(test_economy_state_names);
    RUN_TEST(test_economy_role_names);
    RUN_TEST(test_economy_war_torn_has_armed_ships);

    // NPC Encounter Generator
    log.BeginSection("NPC Encounter Generator");
    RUN_TEST(test_encounter_generate_default);
    RUN_TEST(test_encounter_wave_count_explicit);
    RUN_TEST(test_encounter_high_sec_easier);
    RUN_TEST(test_encounter_null_sec_higher_bounty);
    RUN_TEST(test_encounter_determinism);
    RUN_TEST(test_encounter_calculate_bounty);
    RUN_TEST(test_encounter_wave_trigger_delays);

    // Damage State Generator
    log.BeginSection("Damage State Generator");
    RUN_TEST(test_damage_generate_pristine);
    RUN_TEST(test_damage_generate_critical);
    RUN_TEST(test_damage_level_progression);
    RUN_TEST(test_damage_decals_increase_with_severity);
    RUN_TEST(test_damage_determinism);
    RUN_TEST(test_damage_structural_integrity_range);
    RUN_TEST(test_damage_level_names);
    RUN_TEST(test_damage_decal_type_names);

    // Galaxy Map Panel
    log.BeginSection("Galaxy Map Panel");
    RUN_TEST(test_galaxy_panel_defaults);
    RUN_TEST(test_galaxy_panel_generate);
    RUN_TEST(test_galaxy_panel_chokepoints);
    RUN_TEST(test_galaxy_panel_system_count_clamped_low);
    RUN_TEST(test_galaxy_panel_system_count_clamped_high);
    RUN_TEST(test_galaxy_panel_log_after_generate);
    RUN_TEST(test_galaxy_panel_filter_all);
    RUN_TEST(test_galaxy_panel_filter_highsec);
    RUN_TEST(test_galaxy_panel_filter_lowsec);
    RUN_TEST(test_galaxy_panel_filter_nullsec);
    RUN_TEST(test_galaxy_panel_filter_sum_equals_total);
    RUN_TEST(test_galaxy_panel_select_system);
    RUN_TEST(test_galaxy_panel_select_nonexistent_ignored);
    RUN_TEST(test_galaxy_panel_clear_selection);
    RUN_TEST(test_galaxy_panel_generate_clears_selection);
    RUN_TEST(test_galaxy_panel_determinism);
    RUN_TEST(test_galaxy_panel_different_seeds_differ);
    RUN_TEST(test_galaxy_panel_draw_does_not_crash);

    // EventBus
    log.BeginSection("EventBus");
    RUN_TEST(test_eventbus_initial_state);
    RUN_TEST(test_eventbus_subscribe_and_count);
    RUN_TEST(test_eventbus_subscribe_null_callback);
    RUN_TEST(test_eventbus_unsubscribe);
    RUN_TEST(test_eventbus_unsubscribe_invalid_noop);
    RUN_TEST(test_eventbus_publish_invokes_callback);
    RUN_TEST(test_eventbus_publish_only_matching_type);
    RUN_TEST(test_eventbus_multiple_subscribers_same_type);
    RUN_TEST(test_eventbus_wildcard_receives_all);
    RUN_TEST(test_eventbus_enqueue_and_flush);
    RUN_TEST(test_eventbus_flush_fifo_order);
    RUN_TEST(test_eventbus_enqueue_during_flush_deferred);
    RUN_TEST(test_eventbus_total_published);
    RUN_TEST(test_eventbus_reset);
    RUN_TEST(test_eventbus_sender_id_and_str_param);
    RUN_TEST(test_eventbus_no_subscribers_no_crash);
    RUN_TEST(test_eventbus_unsubscribe_wildcard);

    // Fleet Formation Panel
    log.BeginSection("Fleet Formation Panel");
    RUN_TEST(test_fmtn_defaults);
    RUN_TEST(test_fmtn_set_formation_type);
    RUN_TEST(test_fmtn_formation_type_names);
    RUN_TEST(test_fmtn_set_fleet_size);
    RUN_TEST(test_fmtn_fleet_size_clamped_low);
    RUN_TEST(test_fmtn_fleet_size_clamped_high);
    RUN_TEST(test_fmtn_set_spacing);
    RUN_TEST(test_fmtn_spacing_clamped);
    RUN_TEST(test_fmtn_commander_at_origin);
    RUN_TEST(test_fmtn_arrow_offsets_nonzero);
    RUN_TEST(test_fmtn_line_offsets_lateral);
    RUN_TEST(test_fmtn_spread_circular);
    RUN_TEST(test_fmtn_bounding_radius);
    RUN_TEST(test_fmtn_centre_of_mass);
    RUN_TEST(test_fmtn_select_slot);
    RUN_TEST(test_fmtn_select_invalid_ignored);
    RUN_TEST(test_fmtn_import_fleet);
    RUN_TEST(test_fmtn_none_formation_no_offsets);
    RUN_TEST(test_fmtn_spacing_affects_offsets);
    RUN_TEST(test_fmtn_draw_does_not_crash);
    RUN_TEST(test_fmtn_log_after_actions);

    // Live Scene Manager
    log.BeginSection("Live Scene Manager");
    RUN_TEST(test_live_scene_defaults);
    RUN_TEST(test_live_scene_populate);
    RUN_TEST(test_live_scene_capture_changes);
    RUN_TEST(test_live_scene_save_and_load);
    RUN_TEST(test_live_scene_regenerate);
    RUN_TEST(test_live_scene_on_asset_reload_triggers_regen);
    RUN_TEST(test_live_scene_seed_and_version);

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

    // GamePackager Settings
    log.BeginSection("GamePackager Settings");
    RUN_TEST(test_pkg_settings_target_toggle);
    RUN_TEST(test_pkg_settings_mode_cycle);
    RUN_TEST(test_pkg_settings_options);

    // Keybind Wiring
    log.BeginSection("Keybind Wiring");
    RUN_TEST(test_keybind_viewport_gizmo_wiring);
    RUN_TEST(test_keybind_panel_toggle_wiring);
    RUN_TEST(test_keybind_save_wiring);
    RUN_TEST(test_keybind_delete_wiring);

    // ECS Inspector Component Display
    log.BeginSection("ECS Inspector Component Display");
    RUN_TEST(test_ecsi_component_types_visible);

    // Undo Integration
    log.BeginSection("Undo Integration");
    RUN_TEST(test_undo_viewport_transform);

    // Mission Editor Panel
    log.BeginSection("Mission Editor Panel");
    RUN_TEST(test_mission_editor_defaults);
    RUN_TEST(test_mission_editor_add_template);
    RUN_TEST(test_mission_editor_add_multiple);
    RUN_TEST(test_mission_editor_remove_template);
    RUN_TEST(test_mission_editor_remove_out_of_range);
    RUN_TEST(test_mission_editor_update_template);
    RUN_TEST(test_mission_editor_update_out_of_range);
    RUN_TEST(test_mission_editor_select_template);
    RUN_TEST(test_mission_editor_select_invalid_ignored);
    RUN_TEST(test_mission_editor_clear_selection);
    RUN_TEST(test_mission_editor_remove_fixes_selection);
    RUN_TEST(test_mission_editor_type_filter);
    RUN_TEST(test_mission_editor_level_filter);
    RUN_TEST(test_mission_editor_combined_filter);
    RUN_TEST(test_mission_editor_validate_valid);
    RUN_TEST(test_mission_editor_validate_empty_id);
    RUN_TEST(test_mission_editor_validate_empty_type);
    RUN_TEST(test_mission_editor_validate_bad_level);
    RUN_TEST(test_mission_editor_validate_no_objectives);
    RUN_TEST(test_mission_editor_validate_bad_objective);
    RUN_TEST(test_mission_editor_validate_negative_isk);
    RUN_TEST(test_mission_editor_validate_all);
    RUN_TEST(test_mission_editor_export_json);
    RUN_TEST(test_mission_editor_import_json);
    RUN_TEST(test_mission_editor_draw_does_not_crash);
    RUN_TEST(test_mission_editor_log_after_actions);

    // Undo Stack
    log.BeginSection("Undo Stack");
    RUN_TEST(test_undo_stack_defaults);
    RUN_TEST(test_undo_stack_push_and_undo);
    RUN_TEST(test_undo_stack_redo);
    RUN_TEST(test_undo_stack_multiple_actions);
    RUN_TEST(test_undo_stack_push_clears_redo);
    RUN_TEST(test_undo_stack_clear);
    RUN_TEST(test_undo_stack_max_depth);
    RUN_TEST(test_undo_stack_empty_undo_returns_false);
    RUN_TEST(test_undo_stack_empty_redo_returns_false);

    // Template AI Backend
    log.BeginSection("Template AI Backend");
    RUN_TEST(test_template_ai_defaults_installed);
    RUN_TEST(test_template_ai_query_match);
    RUN_TEST(test_template_ai_query_no_match);
    RUN_TEST(test_template_ai_case_insensitive);
    RUN_TEST(test_template_ai_best_match_wins);
    RUN_TEST(test_template_ai_multiple_matches_boost);
    RUN_TEST(test_template_ai_add_and_remove);
    RUN_TEST(test_template_ai_remove_nonexistent);
    RUN_TEST(test_template_ai_add_empty_keyword_ignored);
    RUN_TEST(test_template_ai_clear_templates);
    RUN_TEST(test_template_ai_with_aggregator);

    // Network Interpolation Buffer
    log.BeginSection("Network Interpolation Buffer");
    RUN_TEST(test_interp_defaults);
    RUN_TEST(test_interp_custom_config);
    RUN_TEST(test_interp_push_snapshot);
    RUN_TEST(test_interp_push_multiple_entities);
    RUN_TEST(test_interp_buffer_depth_limit);
    RUN_TEST(test_interp_lerp_midpoint);
    RUN_TEST(test_interp_lerp_quarter);
    RUN_TEST(test_interp_lerp_at_exact_tick);
    RUN_TEST(test_interp_clamp_before_first);
    RUN_TEST(test_interp_extrapolation);
    RUN_TEST(test_interp_extrapolation_max_exceeded);
    RUN_TEST(test_interp_unknown_entity);
    RUN_TEST(test_interp_remove_entity);
    RUN_TEST(test_interp_clear);
    RUN_TEST(test_interp_duplicate_tick_replaces);
    RUN_TEST(test_interp_out_of_order_ticks);
    RUN_TEST(test_interp_multiple_segments);
    RUN_TEST(test_interp_snapshot_count_nonexistent);

    // Delta Compression
    log.BeginSection("Delta Compression");
    RUN_TEST(test_dc_defaults);
    RUN_TEST(test_dc_custom_interval);
    RUN_TEST(test_dc_zero_interval_clamped);
    RUN_TEST(test_dc_quantize_position);
    RUN_TEST(test_dc_quantize_negative);
    RUN_TEST(test_dc_quantize_rotation);
    RUN_TEST(test_dc_quantize_zero);
    RUN_TEST(test_dc_keyframe_roundtrip);
    RUN_TEST(test_dc_first_encode_is_keyframe);
    RUN_TEST(test_dc_delta_after_keyframe);
    RUN_TEST(test_dc_delta_values_small);
    RUN_TEST(test_dc_delta_stationary_entity);
    RUN_TEST(test_dc_multiple_entities);
    RUN_TEST(test_dc_periodic_keyframe);
    RUN_TEST(test_dc_force_keyframe);
    RUN_TEST(test_dc_force_all_keyframes);
    RUN_TEST(test_dc_remove_entity);
    RUN_TEST(test_dc_clear);
    RUN_TEST(test_dc_decode_delta_without_baseline);
    RUN_TEST(test_dc_empty_encode);
    RUN_TEST(test_dc_encode_decode_full_sequence);

    // Jitter Buffer
    log.BeginSection("Jitter Buffer");
    RUN_TEST(test_jb_defaults);
    RUN_TEST(test_jb_custom_config);
    RUN_TEST(test_jb_min_delay_clamped);
    RUN_TEST(test_jb_push_single);
    RUN_TEST(test_jb_flush_after_delay);
    RUN_TEST(test_jb_flush_releases_in_order);
    RUN_TEST(test_jb_flush_partial);
    RUN_TEST(test_jb_late_packet_dropped);
    RUN_TEST(test_jb_overflow_trims);
    RUN_TEST(test_jb_out_of_order_sorted);
    RUN_TEST(test_jb_duplicate_tick_replaces);
    RUN_TEST(test_jb_adaptive_adjusts_delay);
    RUN_TEST(test_jb_non_adaptive_fixed_delay);
    RUN_TEST(test_jb_reset);
    RUN_TEST(test_jb_flush_empty);

    // Lag Compensation
    log.BeginSection("Lag Compensation");
    RUN_TEST(test_lc_defaults);
    RUN_TEST(test_lc_custom_config);
    RUN_TEST(test_lc_zero_history_clamped);
    RUN_TEST(test_lc_store_single);
    RUN_TEST(test_lc_store_multiple_ticks);
    RUN_TEST(test_lc_store_multiple_entities);
    RUN_TEST(test_lc_store_prunes_old);
    RUN_TEST(test_lc_get_exact_tick);
    RUN_TEST(test_lc_get_interpolated);
    RUN_TEST(test_lc_get_at_latest_tick);
    RUN_TEST(test_lc_get_beyond_latest_returns_latest);
    RUN_TEST(test_lc_get_before_earliest_invalid);
    RUN_TEST(test_lc_get_unknown_entity);
    RUN_TEST(test_lc_interpolation_3d);
    RUN_TEST(test_lc_hit_exact_position);
    RUN_TEST(test_lc_hit_within_radius);
    RUN_TEST(test_lc_miss_outside_radius);
    RUN_TEST(test_lc_hit_at_interpolated_position);
    RUN_TEST(test_lc_hit_unknown_target);
    RUN_TEST(test_lc_hit_3d_distance);
    RUN_TEST(test_lc_clamp_rewind_too_far);
    RUN_TEST(test_lc_clamp_rewind_within_range);
    RUN_TEST(test_lc_clamp_rewind_future);
    RUN_TEST(test_lc_remove_entity);
    RUN_TEST(test_lc_remove_nonexistent);
    RUN_TEST(test_lc_clear);
    RUN_TEST(test_lc_realistic_combat_scenario);
    RUN_TEST(test_lc_high_latency_scenario);
    RUN_TEST(test_lc_boundary_hit_radius);
    RUN_TEST(test_lc_snapshot_count_nonexistent);
    RUN_TEST(test_lc_zero_max_rewind);

    // Scene Graph Panel
    log.BeginSection("Scene Graph Panel");
    RUN_TEST(test_sg_defaults);
    RUN_TEST(test_sg_add_node);
    RUN_TEST(test_sg_add_child_node);
    RUN_TEST(test_sg_add_child_invalid_parent);
    RUN_TEST(test_sg_remove_node);
    RUN_TEST(test_sg_remove_cascades_children);
    RUN_TEST(test_sg_remove_nonexistent);
    RUN_TEST(test_sg_rename_node);
    RUN_TEST(test_sg_rename_nonexistent);
    RUN_TEST(test_sg_reparent);
    RUN_TEST(test_sg_reparent_to_root);
    RUN_TEST(test_sg_reparent_self_rejected);
    RUN_TEST(test_sg_reparent_cycle_prevented);
    RUN_TEST(test_sg_reparent_nonexistent_parent);
    RUN_TEST(test_sg_root_nodes);
    RUN_TEST(test_sg_children);
    RUN_TEST(test_sg_depth);
    RUN_TEST(test_sg_select_node);
    RUN_TEST(test_sg_select_nonexistent_ignored);
    RUN_TEST(test_sg_clear_selection);
    RUN_TEST(test_sg_remove_clears_selection);
    RUN_TEST(test_sg_select_locked_ignored);
    RUN_TEST(test_sg_set_visible);
    RUN_TEST(test_sg_set_locked);
    RUN_TEST(test_sg_lock_deselects);
    RUN_TEST(test_sg_toggle_expanded);
    RUN_TEST(test_sg_expand_all);
    RUN_TEST(test_sg_collapse_all);
    RUN_TEST(test_sg_search_filter);
    RUN_TEST(test_sg_search_by_type);
    RUN_TEST(test_sg_draw_does_not_crash);
    RUN_TEST(test_sg_log_after_actions);

    // Keybind Manager
    log.BeginSection("Keybind Manager");
    RUN_TEST(test_kb_defaults_installed);
    RUN_TEST(test_kb_clear);
    RUN_TEST(test_kb_add_binding);
    RUN_TEST(test_kb_add_duplicate_action_rejected);
    RUN_TEST(test_kb_add_empty_action_rejected);
    RUN_TEST(test_kb_remove_binding);
    RUN_TEST(test_kb_remove_nonexistent);
    RUN_TEST(test_kb_rebind);
    RUN_TEST(test_kb_rebind_nonexistent);
    RUN_TEST(test_kb_set_enabled);
    RUN_TEST(test_kb_find_by_key);
    RUN_TEST(test_kb_bindings_in_category);
    RUN_TEST(test_kb_conflict_detection);
    RUN_TEST(test_kb_conflict_with_exclude);
    RUN_TEST(test_kb_handle_key_press);
    RUN_TEST(test_kb_handle_key_press_no_match);
    RUN_TEST(test_kb_disabled_binding_not_dispatched);
    RUN_TEST(test_kb_callback_replacement);
    RUN_TEST(test_kb_describe_binding);
    RUN_TEST(test_kb_describe_action);
    RUN_TEST(test_kb_default_bindings);
    RUN_TEST(test_kb_serialize_roundtrip);
    RUN_TEST(test_kb_deserialize_empty_returns_false);
    RUN_TEST(test_kb_file_roundtrip);
    RUN_TEST(test_kb_load_nonexistent_returns_false);

    // Alert Stack
    log.BeginSection("Alert Stack");
    RUN_TEST(test_alert_defaults);
    RUN_TEST(test_alert_custom_max);
    RUN_TEST(test_alert_push);
    RUN_TEST(test_alert_push_multiple);
    RUN_TEST(test_alert_active_sorted_by_priority);
    RUN_TEST(test_alert_category_filter);
    RUN_TEST(test_alert_tick_expiry);
    RUN_TEST(test_alert_tick_ages_alerts);
    RUN_TEST(test_alert_dismiss);
    RUN_TEST(test_alert_dismiss_nonexistent);
    RUN_TEST(test_alert_dismiss_category);
    RUN_TEST(test_alert_eviction);
    RUN_TEST(test_alert_clear);
    RUN_TEST(test_alert_ids_monotonic);
    RUN_TEST(test_alert_same_priority_ordered_by_age);

    // AI Miner State Machine
    log.BeginSection("AI Miner State Machine");
    RUN_TEST(test_miner_defaults);
    RUN_TEST(test_miner_add);
    RUN_TEST(test_miner_add_duplicate_rejected);
    RUN_TEST(test_miner_remove);
    RUN_TEST(test_miner_remove_nonexistent);
    RUN_TEST(test_miner_idle_without_deposits);
    RUN_TEST(test_miner_full_cycle);
    RUN_TEST(test_miner_travel_progress);
    RUN_TEST(test_miner_cargo_accumulation);
    RUN_TEST(test_miner_kill);
    RUN_TEST(test_miner_kill_nonexistent);
    RUN_TEST(test_miner_respawn);
    RUN_TEST(test_miner_count_in_state);
    RUN_TEST(test_miner_field_distance);
    RUN_TEST(test_miner_multiple_miners);

    // AI Hauler State Machine
    log.BeginSection("AI Hauler State Machine");
    RUN_TEST(test_hauler_defaults);
    RUN_TEST(test_hauler_add);
    RUN_TEST(test_hauler_add_duplicate_rejected);
    RUN_TEST(test_hauler_remove);
    RUN_TEST(test_hauler_remove_nonexistent);
    RUN_TEST(test_hauler_idle_without_cargo);
    RUN_TEST(test_hauler_full_cycle);
    RUN_TEST(test_hauler_travel_progress);
    RUN_TEST(test_hauler_picks_best_cargo);
    RUN_TEST(test_hauler_kill);
    RUN_TEST(test_hauler_kill_nonexistent);
    RUN_TEST(test_hauler_respawn);
    RUN_TEST(test_hauler_count_in_state);
    RUN_TEST(test_hauler_station_distance);
    RUN_TEST(test_hauler_multiple_haulers);

    // AI Pirate State Machine
    log.BeginSection("AI Pirate State Machine");
    RUN_TEST(test_pirate_defaults);
    RUN_TEST(test_pirate_add);
    RUN_TEST(test_pirate_add_duplicate_rejected);
    RUN_TEST(test_pirate_remove);
    RUN_TEST(test_pirate_remove_nonexistent);
    RUN_TEST(test_pirate_patrol_without_targets);
    RUN_TEST(test_pirate_full_attack_cycle);
    RUN_TEST(test_pirate_travel_progress);
    RUN_TEST(test_pirate_damage);
    RUN_TEST(test_pirate_damage_kills);
    RUN_TEST(test_pirate_damage_nonexistent);
    RUN_TEST(test_pirate_kill);
    RUN_TEST(test_pirate_kill_nonexistent);
    RUN_TEST(test_pirate_respawn);
    RUN_TEST(test_pirate_count_in_state);
    RUN_TEST(test_pirate_patrol_distance);
    RUN_TEST(test_pirate_multiple_pirates);

    // AI Security State Machine
    log.BeginSection("AI Security State Machine");
    RUN_TEST(test_security_defaults);
    RUN_TEST(test_security_add);
    RUN_TEST(test_security_add_duplicate_rejected);
    RUN_TEST(test_security_remove);
    RUN_TEST(test_security_remove_nonexistent);
    RUN_TEST(test_security_standby_without_threats);
    RUN_TEST(test_security_full_engagement_cycle);
    RUN_TEST(test_security_warp_progress);
    RUN_TEST(test_security_damage);
    RUN_TEST(test_security_damage_kills);
    RUN_TEST(test_security_damage_nonexistent);
    RUN_TEST(test_security_kill);
    RUN_TEST(test_security_kill_nonexistent);
    RUN_TEST(test_security_respawn);
    RUN_TEST(test_security_count_in_state);
    RUN_TEST(test_security_post_distance);
    RUN_TEST(test_security_multiple_guards);

    // AI Wallet System
    log.BeginSection("AI Wallet System");
    RUN_TEST(test_wallet_defaults);
    RUN_TEST(test_wallet_create);
    RUN_TEST(test_wallet_create_duplicate_rejected);
    RUN_TEST(test_wallet_create_negative_balance_rejected);
    RUN_TEST(test_wallet_remove);
    RUN_TEST(test_wallet_remove_nonexistent);
    RUN_TEST(test_wallet_deposit);
    RUN_TEST(test_wallet_deposit_nonexistent);
    RUN_TEST(test_wallet_deposit_zero_rejected);
    RUN_TEST(test_wallet_withdraw);
    RUN_TEST(test_wallet_withdraw_insufficient);
    RUN_TEST(test_wallet_withdraw_zero_rejected);
    RUN_TEST(test_wallet_transfer);
    RUN_TEST(test_wallet_transfer_insufficient);
    RUN_TEST(test_wallet_transfer_self_rejected);
    RUN_TEST(test_wallet_total_circulation);
    RUN_TEST(test_wallet_wealthiest_entity);
    RUN_TEST(test_wallet_clear);
    RUN_TEST(test_wallet_get_balance_nonexistent);

    // AI Trader State Machine
    log.BeginSection("AI Trader State Machine");
    RUN_TEST(test_trader_defaults);
    RUN_TEST(test_trader_add);
    RUN_TEST(test_trader_add_duplicate_rejected);
    RUN_TEST(test_trader_remove);
    RUN_TEST(test_trader_remove_nonexistent);
    RUN_TEST(test_trader_idle_without_listings);
    RUN_TEST(test_trader_full_cycle);
    RUN_TEST(test_trader_travel_progress);
    RUN_TEST(test_trader_finds_best_trade);
    RUN_TEST(test_trader_damage);
    RUN_TEST(test_trader_damage_kills);
    RUN_TEST(test_trader_damage_nonexistent);
    RUN_TEST(test_trader_kill);
    RUN_TEST(test_trader_kill_nonexistent);
    RUN_TEST(test_trader_respawn);
    RUN_TEST(test_trader_count_in_state);
    RUN_TEST(test_trader_default_distance);
    RUN_TEST(test_trader_multiple_traders);

    // AI Industrialist State Machine
    log.BeginSection("AI Industrialist State Machine");
    RUN_TEST(test_industrialist_defaults);
    RUN_TEST(test_industrialist_add);
    RUN_TEST(test_industrialist_add_duplicate_rejected);
    RUN_TEST(test_industrialist_remove);
    RUN_TEST(test_industrialist_remove_nonexistent);
    RUN_TEST(test_industrialist_idle_without_blueprints);
    RUN_TEST(test_industrialist_full_cycle);
    RUN_TEST(test_industrialist_build_progress);
    RUN_TEST(test_industrialist_picks_best_blueprint);
    RUN_TEST(test_industrialist_cant_afford);
    RUN_TEST(test_industrialist_kill);
    RUN_TEST(test_industrialist_kill_nonexistent);
    RUN_TEST(test_industrialist_respawn);
    RUN_TEST(test_industrialist_count_in_state);
    RUN_TEST(test_industrialist_market_distance);
    RUN_TEST(test_industrialist_multiple);

    // Advanced Mission Generator
    log.BeginSection("Advanced Mission Generator");
    RUN_TEST(test_mission_gen_defaults);
    RUN_TEST(test_mission_gen_install_defaults);
    RUN_TEST(test_mission_gen_add_template);
    RUN_TEST(test_mission_gen_add_duplicate_rejected);
    RUN_TEST(test_mission_gen_remove_template);
    RUN_TEST(test_mission_gen_remove_nonexistent);
    RUN_TEST(test_mission_gen_get_by_category);
    RUN_TEST(test_mission_gen_generate_for_system);
    RUN_TEST(test_mission_gen_generate_empty_templates);
    RUN_TEST(test_mission_gen_generate_zero_count);
    RUN_TEST(test_mission_gen_from_template);
    RUN_TEST(test_mission_gen_from_nonexistent_template);
    RUN_TEST(test_mission_gen_difficulty_scaling);
    RUN_TEST(test_mission_gen_reward_scaling);
    RUN_TEST(test_mission_gen_branching);
    RUN_TEST(test_mission_gen_no_branching);
    RUN_TEST(test_mission_gen_level_filtering);
    RUN_TEST(test_mission_gen_deterministic);
    RUN_TEST(test_mission_gen_unique_ids);
    RUN_TEST(test_mission_gen_time_limit);
    RUN_TEST(test_mission_gen_standing_reward);

    // Universe Map System
    log.BeginSection("Universe Map System");
    RUN_TEST(test_universe_defaults);
    RUN_TEST(test_universe_add_system);
    RUN_TEST(test_universe_add_duplicate_rejected);
    RUN_TEST(test_universe_remove_system);
    RUN_TEST(test_universe_remove_nonexistent);
    RUN_TEST(test_universe_add_gate);
    RUN_TEST(test_universe_add_gate_duplicate_rejected);
    RUN_TEST(test_universe_add_gate_self_rejected);
    RUN_TEST(test_universe_add_gate_missing_system);
    RUN_TEST(test_universe_remove_gate);
    RUN_TEST(test_universe_remove_gate_nonexistent);
    RUN_TEST(test_universe_route_direct);
    RUN_TEST(test_universe_route_multi_hop);
    RUN_TEST(test_universe_route_no_path);
    RUN_TEST(test_universe_route_same_system);
    RUN_TEST(test_universe_route_by_distance);
    RUN_TEST(test_universe_safe_route);
    RUN_TEST(test_universe_safe_route_no_path);
    RUN_TEST(test_universe_security_classification);
    RUN_TEST(test_universe_travel_time);
    RUN_TEST(test_universe_travel_time_empty);
    RUN_TEST(test_universe_systems_by_security);
    RUN_TEST(test_universe_neighbours_nonexistent);

    // AIProfileLoader tests
    RUN_TEST(test_ai_profile_defaults);
    RUN_TEST(test_ai_profile_install_defaults);
    RUN_TEST(test_ai_profile_add);
    RUN_TEST(test_ai_profile_add_overwrites);
    RUN_TEST(test_ai_profile_remove);
    RUN_TEST(test_ai_profile_remove_nonexistent);
    RUN_TEST(test_ai_profile_get_nonexistent);
    RUN_TEST(test_ai_profile_get_by_archetype);
    RUN_TEST(test_ai_profile_get_by_archetype_multiple);
    RUN_TEST(test_ai_profile_ids);
    RUN_TEST(test_ai_profile_load_empty);
    RUN_TEST(test_ai_profile_load_single);
    RUN_TEST(test_ai_profile_load_multiple);
    RUN_TEST(test_ai_profile_load_overwrites_defaults);
    RUN_TEST(test_ai_profile_all_archetypes);

    // EconomyRulesLoader tests
    RUN_TEST(test_economy_rules_defaults);
    RUN_TEST(test_economy_rules_install_defaults);
    RUN_TEST(test_economy_rules_add);
    RUN_TEST(test_economy_rules_add_overwrites);
    RUN_TEST(test_economy_rules_remove);
    RUN_TEST(test_economy_rules_remove_nonexistent);
    RUN_TEST(test_economy_rules_get_nonexistent);
    RUN_TEST(test_economy_rules_security_highsec);
    RUN_TEST(test_economy_rules_security_lowsec);
    RUN_TEST(test_economy_rules_security_nullsec);
    RUN_TEST(test_economy_rules_security_boundary);
    RUN_TEST(test_economy_rules_ids);
    RUN_TEST(test_economy_rules_load_empty);
    RUN_TEST(test_economy_rules_load_single);
    RUN_TEST(test_economy_rules_load_multiple);
    RUN_TEST(test_economy_rules_load_overwrites_defaults);
    RUN_TEST(test_economy_rules_all_bands);

    // Engine Frame Callback
    log.BeginSection("Engine Frame Callback");
    RUN_TEST(test_engine_frame_callback_invoked);
    RUN_TEST(test_engine_frame_callback_receives_dt);
    RUN_TEST(test_engine_frame_callback_server_mode);
    RUN_TEST(test_engine_no_callback_safe);
    RUN_TEST(test_engine_tick_count);
    RUN_TEST(test_engine_callback_replace);

    // EditorLayout Serialisation
    log.BeginSection("EditorLayout Serialisation");
    RUN_TEST(test_layout_serialize_empty);
    RUN_TEST(test_layout_serialize_single_panel);
    RUN_TEST(test_layout_serialize_split);
    RUN_TEST(test_layout_serialize_tabs);
    RUN_TEST(test_layout_roundtrip_single_panel);
    RUN_TEST(test_layout_roundtrip_horizontal_split);
    RUN_TEST(test_layout_roundtrip_tabs);
    RUN_TEST(test_layout_deserialize_invalid);
    RUN_TEST(test_layout_deserialize_empty_string);
    RUN_TEST(test_layout_file_roundtrip);
    RUN_TEST(test_layout_load_nonexistent_file);
    RUN_TEST(test_layout_roundtrip_nested);

    // Theme loading
    log.BeginSection("Theme Loading");
    RUN_TEST(test_theme_load_from_file);
    RUN_TEST(test_theme_load_nonexistent_file);
    RUN_TEST(test_theme_dpi_scale);
    RUN_TEST(test_theme_dpi_scale_identity);
    RUN_TEST(test_theme_font_scale_default);

    // EditorMenuBar
    log.BeginSection("EditorMenuBar");
    RUN_TEST(test_menubar_empty);
    RUN_TEST(test_menubar_file_menu_items);
    RUN_TEST(test_menubar_view_menu_lists_panels);
    RUN_TEST(test_menubar_pcg_menu_filters_panels);
    RUN_TEST(test_menubar_no_pcg_menu_if_no_pcg_panels);
    RUN_TEST(test_menubar_view_checked_state);
    RUN_TEST(test_menubar_draw_returns_height);
    RUN_TEST(test_menubar_initial_state);
    RUN_TEST(test_menu_item_defaults);
    RUN_TEST(test_menu_bar_state_defaults);
    RUN_TEST(test_layout_menubar_panels_registered);
    RUN_TEST(test_layout_draw_with_menubar);

    if (!logPath.empty()) {
        log.WriteLogFile(logPath);
    }

    int exitCode = log.PrintSummary();
    atlas::test::TestLog::WaitOnWindows();
    return exitCode;
}
