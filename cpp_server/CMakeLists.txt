cmake_minimum_required(VERSION 3.15)
project(AtlasDedicatedServer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(USE_STEAM_SDK "Enable Steam integration" ON)
option(BUILD_TESTS "Build test suite" OFF)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DNOMINMAX)
elseif(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# Core source files shared between server and test binaries
set(CORE_SOURCES
    src/server.cpp
    src/game_session.cpp
    src/game_session_network.cpp
    src/game_session_entities.cpp
    src/game_session_utils.cpp
    src/handlers/combat_handler.cpp
    src/handlers/station_handler.cpp
    src/handlers/movement_handler.cpp
    src/handlers/scanner_handler.cpp
    src/handlers/mission_handler.cpp
    src/network/tcp_server.cpp
    src/network/protocol_handler.cpp
    src/config/server_config.cpp
    src/auth/steam_auth.cpp
    src/auth/whitelist.cpp
    src/utils/name_generator.cpp
    src/utils/logger.cpp
    src/utils/server_metrics.cpp
    src/ui/server_console.cpp
    src/ecs/entity.cpp
    src/ecs/world.cpp
    src/systems/movement_system.cpp
    src/systems/combat_system.cpp
    src/systems/ai_system.cpp
    src/systems/targeting_system.cpp
    src/systems/capacitor_system.cpp
    src/systems/shield_recharge_system.cpp
    src/systems/weapon_system.cpp
    src/systems/wormhole_system.cpp
    src/systems/fleet_system.cpp
    src/systems/mission_system.cpp
    src/systems/skill_system.cpp
    src/systems/module_system.cpp
    src/systems/inventory_system.cpp
    src/systems/loot_system.cpp
    src/systems/insurance_system.cpp
    src/systems/captain_personality_system.cpp
    src/systems/corporation_system.cpp
    src/systems/bounty_system.cpp
    src/systems/market_system.cpp
    src/systems/drone_system.cpp
    src/systems/contract_system.cpp
    src/systems/pi_system.cpp
    src/systems/manufacturing_system.cpp
    src/systems/research_system.cpp
    src/systems/chat_system.cpp
    src/systems/character_creation_system.cpp
    src/systems/tournament_system.cpp
    src/systems/leaderboard_system.cpp
    src/systems/station_system.cpp
    src/systems/wreck_salvage_system.cpp
    src/systems/fleet_morale_system.cpp
    src/systems/fleet_chatter_system.cpp
    src/systems/warp_anomaly_system.cpp
    src/systems/captain_relationship_system.cpp
    src/systems/emotional_arc_system.cpp
    src/systems/fleet_cargo_system.cpp
    src/systems/tactical_overlay_system.cpp
    src/systems/mining_system.cpp
    src/systems/fleet_formation_system.cpp
    src/systems/captain_memory_system.cpp
    src/systems/ship_fitting_system.cpp
    src/systems/warp_cinematic_system.cpp
    src/systems/warp_hud_travel_mode_system.cpp
    src/systems/warp_auto_comfort_system.cpp
    src/systems/refining_system.cpp
    src/systems/anomaly_system.cpp
    src/systems/reputation_system.cpp
    src/systems/scanner_system.cpp
    src/systems/difficulty_scaling_system.cpp
    src/systems/mission_template_system.cpp
    src/systems/mission_generator_system.cpp
    src/systems/lod_system.cpp
    src/systems/spatial_hash_system.cpp
    src/systems/background_simulation_system.cpp
    src/systems/npc_intent_system.cpp
    src/systems/npc_behavior_tree_system.cpp
    src/systems/combat_threat_system.cpp
    src/systems/security_response_system.cpp
    src/systems/ambient_traffic_system.cpp
    src/systems/snapshot_replication_system.cpp
    src/systems/interest_management_system.cpp
    src/systems/fleet_progression_system.cpp
    src/systems/station_deployment_system.cpp
    src/systems/fleet_warp_formation_system.cpp
    src/systems/warp_meditation_system.cpp
    src/systems/rumor_questline_system.cpp
    src/systems/captain_departure_system.cpp
    src/systems/captain_transfer_system.cpp
    src/systems/npc_rerouting_system.cpp
    src/systems/local_reputation_system.cpp
    src/systems/station_news_system.cpp
    src/systems/wreck_persistence_system.cpp
    src/systems/fleet_history_system.cpp
    src/systems/rig_system.cpp
    src/systems/legend_system.cpp
    src/systems/ancient_tech_system.cpp
    src/systems/docking_system.cpp
    src/systems/survival_system.cpp
    src/systems/menu_system.cpp
    src/systems/crew_system.cpp
    src/systems/salvage_exploration_system.cpp
    src/systems/market_order_system.cpp
    src/systems/ai_economic_actor_system.cpp
    src/systems/turret_ai_system.cpp
    src/systems/titan_assembly_system.cpp
    src/systems/alliance_system.cpp
    src/systems/sovereignty_system.cpp
    src/systems/war_declaration_system.cpp
    src/systems/convoy_ambush_system.cpp
    src/systems/npc_dialogue_system.cpp
    src/systems/station_monument_system.cpp
    src/systems/information_propagation_system.cpp
    src/systems/crew_activity_system.cpp
    src/systems/visual_cue_system.cpp
    src/systems/supply_demand_system.cpp
    src/systems/black_market_system.cpp
    src/systems/price_history_system.cpp
    src/systems/propaganda_system.cpp
    src/systems/rest_station_system.cpp
    src/systems/myth_boss_system.cpp
    src/systems/ancient_tech_upgrade_system.cpp
    src/systems/mod_manifest_system.cpp
    src/systems/ancient_ai_remnant_system.cpp
    src/systems/character_creation_screen_system.cpp
    src/systems/view_mode_transition_system.cpp
    src/systems/station_hangar_system.cpp
    src/systems/tether_docking_system.cpp
    src/systems/fps_spawn_system.cpp
    src/systems/interior_door_system.cpp
    src/systems/fps_character_controller_system.cpp
    src/systems/eva_airlock_system.cpp
    src/systems/fps_interaction_system.cpp
    src/systems/fps_combat_system.cpp
    src/systems/fps_inventory_system.cpp
    src/systems/ship_interior_layout_system.cpp
    src/systems/environmental_hazard_system.cpp
    src/systems/fps_objective_system.cpp
    src/systems/wing_management_system.cpp
    src/systems/economic_flow_system.cpp
    src/systems/resource_production_chain_system.cpp
    src/systems/fleet_doctrine_system.cpp
    src/systems/player_progression_system.cpp
    src/systems/terraforming_system.cpp
    src/systems/cloaking_system.cpp
    src/systems/food_processor_system.cpp
    src/systems/fleet_squad_system.cpp
    src/systems/jump_drive_system.cpp
    src/systems/cargo_scan_system.cpp
    src/systems/commander_disagreement_system.cpp
    src/systems/imperfect_information_system.cpp
    src/systems/captain_background_system.cpp
    src/systems/rover_interior_system.cpp
    src/systems/bike_garage_system.cpp
    src/systems/visual_rig_system.cpp
    src/systems/planetary_traversal_system.cpp
    src/systems/solar_panel_system.cpp
    src/systems/farming_deck_system.cpp
    src/systems/docking_ring_extension_system.cpp
    src/systems/rover_bay_ramp_system.cpp
    src/systems/grid_construction_system.cpp
    src/systems/space_planet_transition_system.cpp
    src/systems/fleet_hangar_system.cpp
    src/systems/hangar_environment_system.cpp
    src/systems/dock_node_layout_system.cpp
    src/systems/mission_consequence_system.cpp
    src/systems/server_performance_monitor_system.cpp
    src/systems/atlas_ui_panel_system.cpp
    src/systems/keyboard_navigation_system.cpp
    src/systems/data_binding_system.cpp
    src/systems/entity_stress_test_system.cpp
    src/systems/rig_locker_system.cpp
    src/systems/visual_coupling_system.cpp
    src/systems/fps_salvage_path_system.cpp
    src/systems/lavatory_interaction_system.cpp
    src/systems/eva_airlock_exit_system.cpp
    src/systems/ancient_module_discovery_system.cpp
    src/systems/client_prediction_system.cpp
    src/systems/ship_template_mod_system.cpp
    src/systems/database_persistence_system.cpp
    src/systems/task_scheduler_system.cpp
    src/systems/mod_manager_system.cpp
    src/systems/ship_designer_system.cpp
    src/systems/warp_performance_budget_system.cpp
    src/systems/fleet_positional_audio_system.cpp
    src/systems/mission_editor_system.cpp
    src/systems/content_validation_system.cpp
    src/systems/cloud_deployment_config_system.cpp
    src/systems/interest_management_priority_system.cpp
    src/systems/visual_feedback_queue_system.cpp
    src/systems/mod_doc_generator_system.cpp
    src/systems/community_content_repository_system.cpp
    src/systems/pvp_toggle_system.cpp
    src/systems/dynamic_event_system.cpp
    src/systems/fleet_morale_resolution_system.cpp
    src/systems/persistence_delta_system.cpp
    src/systems/outer_rim_logistics_distortion_system.cpp
    src/systems/rumor_propagation_system.cpp
    src/systems/galactic_response_curve_system.cpp
    src/systems/ship_capability_rating_system.cpp
    src/systems/module_cascading_failure_system.cpp
    src/systems/navigation_bookmark_system.cpp
    src/systems/fleet_supply_line_system.cpp
    src/systems/crew_training_system.cpp
    src/systems/asteroid_belt_system.cpp
    src/systems/scan_probe_system.cpp
    src/systems/autopilot_system.cpp
    src/systems/jump_gate_system.cpp
    src/systems/snapshot_replication_system2.cpp
    src/systems/procedural_mission_generator_system.cpp
    src/systems/incursion_system.cpp
    src/systems/clone_bay_system.cpp
    src/systems/loyalty_point_store_system.cpp
    src/systems/contract_auction_system.cpp
    src/systems/trade_route_analytics_system.cpp
    src/systems/combat_log_system.cpp
    src/systems/fleet_coordination_system.cpp
    src/systems/npc_patrol_route_system.cpp
    src/systems/mining_belt_populator_system.cpp
    src/systems/station_service_registry_system.cpp
    src/systems/npc_schedule_system.cpp
    src/systems/jitter_buffer_system.cpp
    src/systems/entity_lifecycle_system.cpp
    src/systems/player_wallet_system.cpp
    src/systems/cargo_manifest_system.cpp
    src/systems/loadout_persistence_system.cpp
    src/systems/mission_reward_system.cpp
    src/systems/player_standing_system.cpp
    src/systems/undock_sequence_system.cpp
    src/systems/damage_notification_system.cpp
    src/systems/loot_container_system.cpp
    src/systems/system_map_system.cpp
    src/systems/overview_filter_system.cpp
    src/systems/fps_enemy_ai_system.cpp
    src/systems/fps_stealth_system.cpp
    src/systems/fps_companion_system.cpp
    src/systems/fps_terminal_hack_system.cpp
    src/systems/fps_cover_system.cpp
    src/systems/star_system_manager_system.cpp
    src/systems/wallet_transaction_system.cpp
    src/systems/spatial_partition_system.cpp
    src/systems/player_session_system.cpp
    src/systems/save_game_system.cpp
    src/systems/onboarding_system.cpp
    src/systems/star_system_populator_system.cpp
    src/systems/session_progression_system.cpp
    src/systems/encounter_spawner_system.cpp
    src/systems/objective_tracker_system.cpp
    src/systems/event_notification_feed_system.cpp
    src/systems/session_summary_system.cpp
    src/systems/market_browser_system.cpp
    src/systems/skill_queue_system.cpp
    src/systems/ship_fitting_validation_system.cpp
    src/systems/combat_after_action_system.cpp
    src/systems/transit_planner_system.cpp
    src/systems/fleet_readiness_system.cpp
    src/systems/damage_application_system.cpp
    src/systems/combat_engagement_system.cpp
    src/systems/warp_disruption_system.cpp
    src/systems/invulnerability_timer_system.cpp
    src/systems/mining_laser_cycle_system.cpp
    src/systems/thermal_management_system.cpp
    src/systems/aggro_table_system.cpp
    src/systems/cargo_transfer_system.cpp
    src/systems/bounty_collection_system.cpp
    src/systems/module_power_grid_system.cpp
    src/data/ship_database.cpp
    src/data/npc_database.cpp
    src/data/wormhole_database.cpp
    src/data/world_persistence.cpp
    src/data/world_serializer.cpp
    src/data/world_deserializer.cpp
    src/data/persistence_json_utils.cpp
    src/data/world_persistence_compressed.cpp
    src/systems/ore_processing_system.cpp
    src/systems/station_repair_service_system.cpp
    src/systems/combat_reward_system.cpp
    src/systems/anomaly_escalation_system.cpp
    src/systems/ship_repair_cost_system.cpp
    src/systems/system_traffic_monitor_system.cpp
    src/systems/fleet_command_terminal_system.cpp
    src/systems/insurance_claim_system.cpp
    src/systems/safe_zone_system.cpp
    src/systems/ore_compression_system.cpp
    src/systems/crew_management_system.cpp
    src/systems/ship_maintenance_system.cpp
    src/systems/fuel_consumption_system.cpp
    src/systems/beacon_navigation_system.cpp
    src/systems/formation_flight_system.cpp
    src/systems/asteroid_respawn_system.cpp
    src/systems/asteroid_depletion_tracker_system.cpp
    src/systems/bounty_payout_system.cpp
    src/systems/npc_trader_scheduler_system.cpp
    src/systems/npc_fleet_composition_system.cpp
    src/systems/docking_request_system.cpp
    src/systems/tractor_beam_system.cpp
    src/systems/contract_board_system.cpp
    src/systems/module_overheat_system.cpp
    src/systems/asteroid_mining_laser_system.cpp
    src/systems/system_security_response_system.cpp
    src/systems/power_grid_management_system.cpp
    src/systems/salvage_drone_system.cpp
    src/systems/cargo_hold_management_system.cpp
    src/systems/market_trade_execution_system.cpp
    src/systems/ship_approach_orbit_system.cpp
    src/systems/combat_loot_drop_system.cpp
    src/systems/damage_resistance_profile_system.cpp
    src/systems/npc_spawn_scheduler_system.cpp
    src/systems/skill_training_system.cpp
    src/systems/warp_charge_sequence_system.cpp
    src/systems/player_progression_tracking_system.cpp
    src/systems/sector_map_discovery_system.cpp
    src/systems/ship_loadout_preset_system.cpp
    src/systems/mining_yield_optimizer_system.cpp
    src/systems/fleet_coordination_broadcast_system.cpp
    src/systems/station_service_broker_system.cpp
    src/systems/jump_gate_activation_system.cpp
    src/systems/asteroid_scanner_system.cpp
    src/systems/ship_insurance_payout_system.cpp
    src/systems/turret_tracking_system.cpp
    src/systems/colony_management_system.cpp
    src/systems/fps_input_context_system.cpp
    src/systems/encounter_balance_system.cpp
    src/systems/network_quality_monitor_system.cpp
    src/systems/loot_table_system.cpp
    src/systems/resource_respawn_system.cpp
    src/systems/hyperspace_routing_system.cpp
    src/systems/player_hangar_inventory_system.cpp
    src/systems/server_tick_metrics_system.cpp
    src/systems/shield_harmonics_system.cpp
    src/systems/trade_route_optimizer_system.cpp
    src/systems/scan_probe_deployment_system.cpp
    src/systems/docking_bay_allocation_system.cpp
    src/systems/fleet_warp_coordinator_system.cpp
    src/systems/mining_laser_system.cpp
    src/systems/npc_behavior_scheduler_system.cpp
    src/systems/capacitor_warfare_system.cpp
    src/systems/signature_analysis_system.cpp
    src/systems/overheat_management_system.cpp
    src/systems/salvage_processing_system.cpp
    src/systems/jump_fatigue_tracker_system.cpp
    src/systems/session_persistence_system.cpp
    src/systems/star_system_population_system.cpp
    src/systems/dynamic_difficulty_system.cpp
    src/systems/gameplay_loop_tracker_system.cpp
    src/systems/chat_router_system.cpp
    src/systems/editor_overlay_system.cpp
    src/systems/hangar_transition_system.cpp
    src/systems/control_mode_context_system.cpp
    src/systems/construction_placement_system.cpp
    src/systems/hull_class_capability_system.cpp
    src/systems/shield_arc_hud_system.cpp
    src/systems/capacitor_hud_bar_system.cpp
    src/systems/velocity_arc_hud_system.cpp
    src/systems/alert_stack_hud_system.cpp
    src/systems/damage_feedback_hud_system.cpp
    src/systems/gate_gun_system.cpp
    src/systems/aggression_switching_system.cpp
    src/systems/officer_spawn_system.cpp
    src/systems/anomaly_spawning_system.cpp
    src/systems/relay_clone_system.cpp
    src/systems/aegis_spawn_system.cpp
    src/systems/sleeper_ai_system.cpp
    src/systems/sleeper_cache_system.cpp
    src/systems/order_matching_system.cpp
    src/systems/blueprint_research_system.cpp
    src/systems/drifter_ai_system.cpp
    src/systems/warp_bubble_system.cpp
    src/systems/invention_system.cpp
    src/systems/regional_market_system.cpp
    src/systems/abyssal_filament_system.cpp
    src/systems/abyssal_weather_system.cpp
    src/systems/abyssal_escalation_system.cpp
    src/systems/mutaplasmid_system.cpp
    src/systems/planet_scan_system.cpp
    src/systems/pi_customs_system.cpp
    src/systems/tech2_module_system.cpp
    src/systems/capital_component_system.cpp
    src/systems/market_api_system.cpp
    src/systems/session_manager_system.cpp
    src/systems/relay_clone_install_ui_system.cpp
    src/systems/player_spawn_system.cpp
    src/systems/d_scan_system.cpp
    src/systems/kill_report_system.cpp
    src/systems/respawn_selection_system.cpp
    src/systems/tutorial_system.cpp
    src/systems/trade_window_system.cpp
    src/systems/damage_log_system.cpp
    src/systems/loot_distribution_system.cpp
    src/systems/combat_timer_system.cpp
    src/systems/player_notification_queue_system.cpp
    src/systems/fleet_after_action_report_system.cpp
    src/systems/ecm_jamming_system.cpp
    src/systems/sensor_dampening_system.cpp
    src/systems/tracking_disruption_system.cpp
    src/systems/ship_skin_system.cpp
    src/systems/propulsion_module_system.cpp
    src/systems/citadel_management_system.cpp
    src/systems/character_portrait_system.cpp
    src/systems/inertia_modifier_system.cpp
    src/systems/alliance_management_system.cpp
    src/systems/corporation_logo_system.cpp
    src/systems/structure_skin_system.cpp
    src/systems/skill_injector_system.cpp
    src/systems/stasis_web_system.cpp
    src/systems/asset_safety_system.cpp
    src/systems/command_burst_system.cpp
    src/systems/target_painter_system.cpp
    src/systems/warp_scrambler_system.cpp
    src/systems/remote_repair_system.cpp
    src/systems/incursion_system.cpp
    src/systems/meta_level_system.cpp
    src/systems/reaction_formula_system.cpp
    src/pcg/pcg_manager.cpp
    src/pcg/ship_generator.cpp
    src/pcg/fleet_doctrine.cpp
    src/pcg/room_graph.cpp
    src/pcg/deck_graph.cpp
    src/pcg/elevator_system.cpp
    src/pcg/hull_mesher.cpp
    src/pcg/capital_ship_generator.cpp
    src/pcg/ship_designer.cpp
    src/pcg/snappable_grid.cpp
    src/pcg/station_generator.cpp
    src/pcg/salvage_system.cpp
    src/pcg/rover_system.cpp
    src/pcg/collision_manager.cpp
    src/pcg/asteroid_field_generator.cpp
    src/pcg/anomaly_generator.cpp
    src/pcg/npc_encounter_generator.cpp
    src/pcg/pcg_trace.cpp
    src/pcg/constraint_solver.cpp
    src/pcg/weapon_socket.cpp
    src/pcg/pcg_verify.cpp
    src/pcg/interior_budget.cpp
    src/pcg/interior_generator.cpp
    src/pcg/character_mesh_system.cpp
    src/pcg/lowpoly_character_generator.cpp
    src/pcg/turret_generator.cpp
    src/pcg/planet_generator.cpp
    src/pcg/habitat_generator.cpp
    src/pcg/grav_bike_generator.cpp
    src/pcg/spine_hull_generator.cpp
    src/pcg/terrain_generator.cpp
    src/pcg/turret_placement_system.cpp
    src/pcg/damage_state_generator.cpp
    src/pcg/procedural_texture_generator.cpp
    src/pcg/shield_effect_generator.cpp
    src/pcg/economy_driven_generator.cpp
    src/pcg/star_system_generator.cpp
    src/pcg/galaxy_generator.cpp
    src/pcg/generation_style.cpp
    src/pcg/pcg_asset_style.cpp
    src/pcg/ship_archetype.cpp
)

# Static library: compile core sources once, link from both server and test binaries
add_library(novaforge_core STATIC ${CORE_SOURCES})
target_include_directories(novaforge_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)
target_link_libraries(novaforge_core PUBLIC Threads::Threads ZLIB::ZLIB)
if(WIN32)
    target_link_libraries(novaforge_core PUBLIC ws2_32)
elseif(UNIX)
    target_link_libraries(novaforge_core PUBLIC dl)
endif()
if(TARGET AtlasEngine)
    target_link_libraries(novaforge_core PUBLIC AtlasEngine)
endif()

set(SERVER_HEADERS
    include/server.h
    include/game_session.h
    include/handlers/message_handler.h
    include/handlers/handler_utils.h
    include/handlers/combat_handler.h
    include/handlers/station_handler.h
    include/handlers/movement_handler.h
    include/handlers/scanner_handler.h
    include/handlers/mission_handler.h
    include/network/tcp_server.h
    include/network/protocol_handler.h
    include/config/server_config.h
    include/auth/steam_auth.h
    include/auth/whitelist.h
    include/utils/name_generator.h
    include/utils/logger.h
    include/utils/server_metrics.h
    include/ui/server_console.h
    include/ecs/component.h
    include/ecs/entity.h
    include/ecs/system.h
    include/ecs/world.h
    include/components/game_components.h
    include/systems/movement_system.h
    include/systems/combat_system.h
    include/systems/ai_system.h
    include/systems/targeting_system.h
    include/systems/capacitor_system.h
    include/systems/shield_recharge_system.h
    include/systems/weapon_system.h
    include/systems/wormhole_system.h
    include/systems/fleet_system.h
    include/systems/mission_system.h
    include/systems/skill_system.h
    include/systems/module_system.h
    include/systems/inventory_system.h
    include/systems/loot_system.h
    include/systems/insurance_system.h
    include/systems/captain_personality_system.h
    include/systems/corporation_system.h
    include/systems/bounty_system.h
    include/systems/market_system.h
    include/systems/drone_system.h
    include/systems/contract_system.h
    include/systems/pi_system.h
    include/systems/manufacturing_system.h
    include/systems/research_system.h
    include/systems/chat_system.h
    include/systems/character_creation_system.h
    include/systems/tournament_system.h
    include/systems/leaderboard_system.h
    include/systems/station_system.h
    include/systems/wreck_salvage_system.h
    include/systems/fleet_morale_system.h
    include/systems/fleet_chatter_system.h
    include/systems/warp_anomaly_system.h
    include/systems/captain_relationship_system.h
    include/systems/emotional_arc_system.h
    include/systems/fleet_cargo_system.h
    include/systems/tactical_overlay_system.h
    include/systems/mining_system.h
    include/systems/fleet_formation_system.h
    include/systems/captain_memory_system.h
    include/systems/ship_fitting_system.h
    include/systems/warp_cinematic_system.h
    include/systems/warp_hud_travel_mode_system.h
    include/systems/warp_auto_comfort_system.h
    include/systems/refining_system.h
    include/systems/anomaly_system.h
    include/systems/reputation_system.h
    include/systems/scanner_system.h
    include/systems/difficulty_scaling_system.h
    include/systems/mission_template_system.h
    include/systems/mission_generator_system.h
    include/systems/lod_system.h
    include/systems/spatial_hash_system.h
    include/systems/background_simulation_system.h
    include/systems/npc_intent_system.h
    include/systems/npc_behavior_tree_system.h
    include/systems/combat_threat_system.h
    include/systems/security_response_system.h
    include/systems/ambient_traffic_system.h
    include/systems/snapshot_replication_system.h
    include/systems/interest_management_system.h
    include/systems/fleet_progression_system.h
    include/systems/station_deployment_system.h
    include/systems/fleet_warp_formation_system.h
    include/systems/warp_meditation_system.h
    include/systems/rumor_questline_system.h
    include/systems/captain_departure_system.h
    include/systems/captain_transfer_system.h
    include/systems/npc_rerouting_system.h
    include/systems/local_reputation_system.h
    include/systems/station_news_system.h
    include/systems/wreck_persistence_system.h
    include/systems/fleet_history_system.h
    include/systems/rig_system.h
    include/systems/legend_system.h
    include/systems/ancient_tech_system.h
    include/systems/docking_system.h
    include/systems/survival_system.h
    include/systems/menu_system.h
    include/systems/crew_system.h
    include/systems/salvage_exploration_system.h
    include/systems/market_order_system.h
    include/systems/ai_economic_actor_system.h
    include/systems/turret_ai_system.h
    include/systems/titan_assembly_system.h
    include/systems/alliance_system.h
    include/systems/sovereignty_system.h
    include/systems/war_declaration_system.h
    include/systems/information_propagation_system.h
    include/systems/crew_activity_system.h
    include/systems/visual_cue_system.h
    include/systems/supply_demand_system.h
    include/systems/black_market_system.h
    include/systems/price_history_system.h
    include/systems/propaganda_system.h
    include/systems/rest_station_system.h
    include/systems/myth_boss_system.h
    include/systems/ancient_tech_upgrade_system.h
    include/systems/mod_manifest_system.h
    include/systems/ancient_ai_remnant_system.h
    include/systems/character_creation_screen_system.h
    include/systems/view_mode_transition_system.h
    include/systems/station_hangar_system.h
    include/systems/tether_docking_system.h
    include/systems/fps_spawn_system.h
    include/systems/interior_door_system.h
    include/systems/fps_character_controller_system.h
    include/systems/eva_airlock_system.h
    include/systems/fps_interaction_system.h
    include/systems/fps_combat_system.h
    include/systems/fps_inventory_system.h
    include/systems/ship_interior_layout_system.h
    include/systems/environmental_hazard_system.h
    include/systems/fps_objective_system.h
    include/systems/economic_flow_system.h
    include/systems/resource_production_chain_system.h
    include/systems/fleet_doctrine_system.h
    include/systems/player_progression_system.h
    include/systems/terraforming_system.h
    include/systems/cloaking_system.h
    include/systems/food_processor_system.h
    include/systems/fleet_squad_system.h
    include/systems/jump_drive_system.h
    include/systems/cargo_scan_system.h
    include/systems/commander_disagreement_system.h
    include/systems/imperfect_information_system.h
    include/systems/captain_background_system.h
    include/systems/warp_performance_budget_system.h
    include/systems/fleet_positional_audio_system.h
    include/systems/space_planet_transition_system.h
    include/systems/fleet_hangar_system.h
    include/systems/hangar_environment_system.h
    include/systems/dock_node_layout_system.h
    include/systems/mission_consequence_system.h
    include/systems/server_performance_monitor_system.h
    include/systems/atlas_ui_panel_system.h
    include/systems/keyboard_navigation_system.h
    include/systems/data_binding_system.h
    include/systems/entity_stress_test_system.h
    include/systems/rig_locker_system.h
    include/systems/visual_coupling_system.h
    include/systems/fps_salvage_path_system.h
    include/systems/lavatory_interaction_system.h
    include/systems/eva_airlock_exit_system.h
    include/systems/ancient_module_discovery_system.h
    include/systems/client_prediction_system.h
    include/systems/ship_template_mod_system.h
    include/systems/database_persistence_system.h
    include/systems/task_scheduler_system.h
    include/systems/mod_manager_system.h
    include/systems/ship_designer_system.h
    include/data/ship_database.h
    include/data/npc_database.h
    include/data/wormhole_database.h
    include/data/world_persistence.h
    include/pcg/deterministic_rng.h
    include/pcg/hash_utils.h
    include/pcg/pcg_context.h
    include/pcg/pcg_manager.h
    include/pcg/ship_generator.h
    include/pcg/fleet_doctrine.h
    include/pcg/room_graph.h
    include/pcg/deck_graph.h
    include/pcg/elevator_system.h
    include/pcg/hull_mesher.h
    include/pcg/capital_ship_generator.h
    include/pcg/ship_designer.h
    include/pcg/snappable_grid.h
    include/pcg/station_generator.h
    include/pcg/salvage_system.h
    include/pcg/rover_system.h
    include/pcg/collision_manager.h
    include/pcg/asteroid_field_generator.h
    include/pcg/anomaly_generator.h
    include/pcg/npc_encounter_generator.h
    include/pcg/pcg_trace.h
    include/pcg/constraint_solver.h
    include/pcg/weapon_socket.h
    include/pcg/pcg_version.h
    include/pcg/pcg_verify.h
    include/pcg/interior_budget.h
    include/pcg/interior_generator.h
    include/pcg/character_mesh_system.h
    include/pcg/lowpoly_character_generator.h
    include/pcg/turret_generator.h
    include/pcg/planet_generator.h
    include/pcg/habitat_generator.h
    include/pcg/grav_bike_generator.h
    include/pcg/spine_hull_generator.h
    include/pcg/terrain_generator.h
    include/pcg/turret_placement_system.h
    include/pcg/damage_state_generator.h
    include/pcg/procedural_texture_generator.h
    include/pcg/shield_effect_generator.h
    include/pcg/economy_driven_generator.h
    include/pcg/star_system_generator.h
    include/pcg/galaxy_generator.h
    include/pcg/generation_style.h
    include/pcg/pcg_asset_style.h
    include/pcg/ship_archetype.h
    include/systems/mission_editor_system.h
    include/systems/content_validation_system.h
    include/systems/cloud_deployment_config_system.h
    include/systems/interest_management_priority_system.h
    include/systems/visual_feedback_queue_system.h
    include/systems/mod_doc_generator_system.h
    include/systems/community_content_repository_system.h
    include/systems/pvp_toggle_system.h
    include/systems/dynamic_event_system.h
    include/systems/convoy_ambush_system.h
    include/systems/npc_dialogue_system.h
    include/systems/station_monument_system.h
    include/systems/fleet_morale_resolution_system.h
    include/systems/persistence_delta_system.h
    include/systems/outer_rim_logistics_distortion_system.h
    include/systems/rumor_propagation_system.h
    include/systems/galactic_response_curve_system.h
    include/systems/ship_capability_rating_system.h
    include/systems/module_cascading_failure_system.h
    include/systems/navigation_bookmark_system.h
    include/systems/fleet_supply_line_system.h
    include/systems/crew_training_system.h
    include/systems/asteroid_belt_system.h
    include/systems/scan_probe_system.h
    include/systems/autopilot_system.h
    include/systems/jump_gate_system.h
    include/systems/snapshot_replication_system2.h
    include/systems/procedural_mission_generator_system.h
    include/systems/incursion_system.h
    include/systems/clone_bay_system.h
    include/systems/loyalty_point_store_system.h
    include/systems/contract_auction_system.h
    include/systems/trade_route_analytics_system.h
    include/systems/combat_log_system.h
    include/systems/fleet_coordination_system.h
    include/systems/npc_patrol_route_system.h
    include/systems/mining_belt_populator_system.h
    include/systems/station_service_registry_system.h
    include/systems/npc_schedule_system.h
    include/systems/jitter_buffer_system.h
    include/systems/entity_lifecycle_system.h
    include/systems/player_wallet_system.h
    include/systems/cargo_manifest_system.h
    include/systems/loadout_persistence_system.h
    include/systems/mission_reward_system.h
    include/systems/player_standing_system.h
    include/systems/undock_sequence_system.h
    include/systems/damage_notification_system.h
    include/systems/loot_container_system.h
    include/systems/system_map_system.h
    include/systems/overview_filter_system.h
    include/systems/fps_enemy_ai_system.h
    include/systems/fps_stealth_system.h
    include/systems/fps_companion_system.h
    include/systems/fps_terminal_hack_system.h
    include/systems/fps_cover_system.h
    include/systems/star_system_manager_system.h
    include/systems/wallet_transaction_system.h
    include/systems/spatial_partition_system.h
    include/systems/player_session_system.h
    include/systems/save_game_system.h
    include/systems/onboarding_system.h
    include/systems/star_system_populator_system.h
    include/systems/session_progression_system.h
    include/systems/encounter_spawner_system.h
    include/systems/objective_tracker_system.h
    include/systems/event_notification_feed_system.h
    include/systems/session_summary_system.h
    include/systems/market_browser_system.h
    include/systems/skill_queue_system.h
    include/systems/ship_fitting_validation_system.h
    include/systems/combat_after_action_system.h
    include/systems/transit_planner_system.h
    include/systems/fleet_readiness_system.h
    include/utils/entity_builder.h
    include/systems/ore_processing_system.h
    include/systems/station_repair_service_system.h
    include/systems/combat_reward_system.h
    include/systems/anomaly_escalation_system.h
    include/systems/ship_repair_cost_system.h
    include/systems/system_traffic_monitor_system.h
    include/systems/fleet_command_terminal_system.h
    include/systems/insurance_claim_system.h
    include/systems/safe_zone_system.h
    include/systems/ore_compression_system.h
    include/systems/crew_management_system.h
    include/systems/ship_maintenance_system.h
    include/systems/fuel_consumption_system.h
    include/systems/beacon_navigation_system.h
    include/systems/formation_flight_system.h
    include/systems/asteroid_respawn_system.h
    include/systems/asteroid_depletion_tracker_system.h
    include/systems/bounty_payout_system.h
    include/systems/npc_trader_scheduler_system.h
    include/systems/npc_fleet_composition_system.h
    include/systems/docking_request_system.h
    include/systems/tractor_beam_system.h
    include/systems/contract_board_system.h
    include/systems/module_overheat_system.h
    include/systems/asteroid_mining_laser_system.h
    include/systems/system_security_response_system.h
    include/systems/power_grid_management_system.h
    include/systems/salvage_drone_system.h
    include/systems/cargo_hold_management_system.h
    include/systems/market_trade_execution_system.h
    include/systems/ship_approach_orbit_system.h
    include/systems/combat_loot_drop_system.h
    include/systems/damage_resistance_profile_system.h
    include/systems/npc_spawn_scheduler_system.h
    include/systems/skill_training_system.h
    include/systems/warp_charge_sequence_system.h
    include/systems/player_progression_tracking_system.h
    include/systems/sector_map_discovery_system.h
    include/systems/ship_loadout_preset_system.h
    include/systems/mining_yield_optimizer_system.h
    include/systems/fleet_coordination_broadcast_system.h
    include/systems/station_service_broker_system.h
    include/systems/jump_gate_activation_system.h
    include/systems/asteroid_scanner_system.h
    include/systems/ship_insurance_payout_system.h
    include/systems/turret_tracking_system.h
    include/systems/colony_management_system.h
    include/systems/fps_input_context_system.h
    include/systems/encounter_balance_system.h
    include/systems/network_quality_monitor_system.h
    include/systems/loot_table_system.h
    include/systems/resource_respawn_system.h
    include/systems/hyperspace_routing_system.h
    include/systems/player_hangar_inventory_system.h
    include/systems/server_tick_metrics_system.h
    include/systems/shield_harmonics_system.h
    include/systems/trade_route_optimizer_system.h
    include/systems/scan_probe_deployment_system.h
    include/systems/docking_bay_allocation_system.h
    include/systems/fleet_warp_coordinator_system.h
    include/systems/mining_laser_system.h
    include/systems/npc_behavior_scheduler_system.h
    include/systems/capacitor_warfare_system.h
    include/systems/signature_analysis_system.h
    include/systems/overheat_management_system.h
    include/systems/salvage_processing_system.h
    include/systems/jump_fatigue_tracker_system.h
    include/systems/session_persistence_system.h
    include/systems/star_system_population_system.h
    include/systems/dynamic_difficulty_system.h
    include/systems/gameplay_loop_tracker_system.h
    include/systems/chat_router_system.h
    include/systems/editor_overlay_system.h
    include/systems/hangar_transition_system.h
    include/systems/control_mode_context_system.h
    include/systems/construction_placement_system.h
    include/systems/hull_class_capability_system.h
    include/systems/respawn_selection_system.h
    include/systems/tutorial_system.h
    include/systems/trade_window_system.h
    include/systems/damage_log_system.h
    include/systems/loot_distribution_system.h
    include/systems/combat_timer_system.h
    include/systems/player_notification_queue_system.h
    include/systems/fleet_after_action_report_system.h
    include/systems/ecm_jamming_system.h
    include/systems/sensor_dampening_system.h
    include/systems/tracking_disruption_system.h
    include/systems/stasis_web_system.h
    include/systems/asset_safety_system.h
    include/systems/command_burst_system.h
    include/systems/target_painter_system.h
    include/systems/warp_scrambler_system.h
    include/systems/remote_repair_system.h
)

# Steam SDK configuration
if(USE_STEAM_SDK)
    # Path to Steamworks SDK (users need to download this separately)
    set(STEAM_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/steamworks_sdk" CACHE PATH "Path to Steamworks SDK")
    
    if(EXISTS "${STEAM_SDK_PATH}/public/steam/steam_api.h")
        message(STATUS "Found Steamworks SDK at: ${STEAM_SDK_PATH}")
        add_definitions(-DUSE_STEAM)
        include_directories(${STEAM_SDK_PATH}/public)
        
        # Link Steam library
        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                link_directories(${STEAM_SDK_PATH}/redistributable_bin/win64)
            else()
                link_directories(${STEAM_SDK_PATH}/redistributable_bin)
            endif()
            set(STEAM_LIBRARIES steam_api64)
        elseif(LINUX)
            link_directories(${STEAM_SDK_PATH}/redistributable_bin/linux64)
            set(STEAM_LIBRARIES steam_api)
        endif()
    else()
        message(WARNING "Steamworks SDK not found. Steam integration will be disabled.")
        message(WARNING "Download SDK from: https://partner.steamgames.com/")
        set(USE_STEAM_SDK OFF)
    endif()
endif()

# Main executable
add_executable(atlas_dedicated_server src/main.cpp ${SERVER_HEADERS})
target_link_libraries(atlas_dedicated_server novaforge_core)

if(USE_STEAM_SDK AND STEAM_LIBRARIES)
    target_link_libraries(atlas_dedicated_server ${STEAM_LIBRARIES})
endif()

# Installation
install(TARGETS atlas_dedicated_server
    RUNTIME DESTINATION bin
)

install(DIRECTORY config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.json" PATTERN "*.cfg"
)

# Copy configuration files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

message(STATUS "")
message(STATUS "Atlas Dedicated Server Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Steam Integration: ${USE_STEAM_SDK}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")

# Test executables
if(BUILD_TESTS)
    file(GLOB TEST_SOURCES tests/test_*.cpp)
    list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_single_main.cpp")
    list(APPEND TEST_SOURCES tests/test_main.cpp)
    list(REMOVE_DUPLICATES TEST_SOURCES)

    # ── Monolithic test binary (all suites in one executable) ────
    add_executable(test_systems ${TEST_SOURCES})
    target_include_directories(test_systems PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(test_systems novaforge_core)
    if(MSVC)
        target_compile_options(test_systems PRIVATE /bigobj)
    endif()

    # ── Per-system test binaries ─────────────────────────────────
    # Each test_*.cpp file becomes its own executable that compiles
    # only that one test plus a tiny main (test_single_main.cpp).
    # Build with: cmake --build . --target test_capacitor_system
    # This avoids recompiling 300+ other test files on every change.
    enable_testing()
    foreach(TEST_SRC ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
        if(NOT TEST_NAME STREQUAL "test_main" AND NOT TEST_NAME STREQUAL "test_single_main")
            # Derive the run_*_tests() function name from the file name.
            # Convention: test_foo.cpp  →  run_foo_tests()
            string(REGEX REPLACE "^test_" "" SUITE_NAME "${TEST_NAME}")
            set(FUNC_NAME "run_${SUITE_NAME}_tests")

            add_executable(${TEST_NAME}
                tests/test_single_main.cpp
                ${TEST_SRC}
            )
            target_include_directories(${TEST_NAME} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/tests
            )
            target_link_libraries(${TEST_NAME} novaforge_core)
            target_compile_definitions(${TEST_NAME} PRIVATE
                "NOVAFORGE_TEST_FUNC=${FUNC_NAME}"
            )
            if(MSVC)
                target_compile_options(${TEST_NAME} PRIVATE /bigobj)
            endif()
            set_target_properties(${TEST_NAME} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            )

            # CTest: run the per-system binary (faster than monolithic)
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
    endforeach()
endif()
