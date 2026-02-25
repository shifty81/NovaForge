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
option(BUILD_TESTS "Build test suite" ON)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WIN32)
elseif(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/server.cpp
    src/game_session.cpp
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
    src/systems/information_propagation_system.cpp
    src/systems/crew_activity_system.cpp
    src/systems/visual_cue_system.cpp
    src/data/ship_database.cpp
    src/data/npc_database.cpp
    src/data/wormhole_database.cpp
    src/data/world_persistence.cpp
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
    src/pcg/economy_driven_generator.cpp
    src/pcg/star_system_generator.cpp
    src/pcg/galaxy_generator.cpp
    src/pcg/generation_style.cpp
    src/pcg/pcg_asset_style.cpp
    src/pcg/ship_archetype.cpp
)

set(SERVER_HEADERS
    include/server.h
    include/game_session.h
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
    include/pcg/economy_driven_generator.h
    include/pcg/star_system_generator.h
    include/pcg/galaxy_generator.h
    include/pcg/generation_style.h
    include/pcg/pcg_asset_style.h
    include/pcg/ship_archetype.h
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
add_executable(atlas_dedicated_server ${SERVER_SOURCES} ${SERVER_HEADERS})

# Link libraries
target_link_libraries(atlas_dedicated_server 
    Threads::Threads
    ZLIB::ZLIB
)

if(USE_STEAM_SDK AND STEAM_LIBRARIES)
    target_link_libraries(atlas_dedicated_server ${STEAM_LIBRARIES})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atlas_dedicated_server ws2_32)
elseif(UNIX)
    target_link_libraries(atlas_dedicated_server dl)
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

# Test executable
if(BUILD_TESTS)
    # Sources needed by test_systems (exclude main.cpp, server.cpp, game_session.cpp)
    set(TEST_SUPPORT_SOURCES
        src/ecs/entity.cpp
        src/ecs/world.cpp
        src/systems/capacitor_system.cpp
        src/systems/shield_recharge_system.cpp
        src/systems/weapon_system.cpp
        src/systems/combat_system.cpp
        src/systems/targeting_system.cpp
        src/systems/movement_system.cpp
        src/systems/ai_system.cpp
        src/data/ship_database.cpp
        src/data/wormhole_database.cpp
        src/data/npc_database.cpp
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
        src/data/world_persistence.cpp
        src/utils/logger.cpp
        src/utils/server_metrics.cpp
        src/ui/server_console.cpp
        src/server.cpp
        src/game_session.cpp
        src/network/tcp_server.cpp
        src/network/protocol_handler.cpp
        src/config/server_config.cpp
        src/auth/steam_auth.cpp
        src/auth/whitelist.cpp
        src/utils/name_generator.cpp
        src/data/ship_database.cpp
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
        src/pcg/economy_driven_generator.cpp
        src/pcg/star_system_generator.cpp
        src/pcg/galaxy_generator.cpp
        src/pcg/generation_style.cpp
        src/pcg/pcg_asset_style.cpp
        src/pcg/ship_archetype.cpp
    )

    add_executable(test_systems
        test_systems.cpp
        ${TEST_SUPPORT_SOURCES}
    )
    target_link_libraries(test_systems Threads::Threads ZLIB::ZLIB)
    if(WIN32)
        target_link_libraries(test_systems ws2_32)
    endif()
endif()
