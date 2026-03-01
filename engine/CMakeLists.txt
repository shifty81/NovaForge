add_library(AtlasEngine STATIC
    core/Engine.cpp
    core/Logger.cpp
    core/EventBus.cpp
    ecs/ECS.cpp
    graphvm/GraphVM.cpp
    graphvm/GraphCompiler.cpp
    assets/AssetRegistry.cpp
    assets/AssetBinary.cpp
    net/NetContext.cpp
    net/NetworkQualityMonitor.cpp
    net/NetworkInterpolationBuffer.cpp
    net/DeltaCompression.cpp
    net/JitterBuffer.cpp
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
    world/CubeSphereLayout.cpp
    world/VoxelGridLayout.cpp
)

target_include_directories(AtlasEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
