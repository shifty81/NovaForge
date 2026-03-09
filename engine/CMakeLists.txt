add_library(AtlasEngine STATIC
    core/Engine.cpp
    core/Logger.cpp
    core/EventBus.cpp
    core/RuntimeBootstrap.cpp
    core/GameStateManager.cpp
    ecs/ECS.cpp
    ecs/System.cpp
    ecs/DeltaEditStore.cpp
    graphvm/GraphVM.cpp
    graphvm/GraphCompiler.cpp
    assets/AssetRegistry.cpp
    assets/AssetBinary.cpp
    net/NetContext.cpp
    net/NetworkQualityMonitor.cpp
    net/NetworkInterpolationBuffer.cpp
    net/DeltaCompression.cpp
    net/JitterBuffer.cpp
    net/LagCompensation.cpp
    sim/TickScheduler.cpp
    sim/AlertStack.cpp
    sim/AIMinerStateMachine.cpp
    sim/AIHaulerStateMachine.cpp
    sim/AIPirateStateMachine.cpp
    sim/AISecurityStateMachine.cpp
    sim/AIWalletSystem.cpp
    sim/AITraderStateMachine.cpp
    sim/AIIndustrialistStateMachine.cpp
    sim/AdvancedMissionGenerator.cpp
    sim/UniverseMapSystem.cpp
    sim/AIProfileLoader.cpp
    sim/EconomyRulesLoader.cpp
    sim/PirateSecurityCoordinator.cpp
    world/CubeSphereLayout.cpp
    world/VoxelGridLayout.cpp
    physics/PhysicsWorld.cpp
    input/InputManager.cpp
    camera/Camera.cpp
    audio/AudioEngine.cpp
    animation/AnimationGraph.cpp
    animation/AnimationNodes.cpp
    plugin/PluginSystem.cpp
)

target_include_directories(AtlasEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Prevent Windows.h min/max macros from interfering with std::min/std::max
if(WIN32)
    target_compile_definitions(AtlasEngine PUBLIC NOMINMAX)
endif()
