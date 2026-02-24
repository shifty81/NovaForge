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
)

target_link_libraries(AtlasEditor AtlasEngine)
target_include_directories(AtlasEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp_server/include
)
