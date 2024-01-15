set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(App_GRAPHEDITOR_SRCS
    src/main.cpp
)

qt5_add_resources(App_GRAPHEDITOR_SRCS resources.qrc)

set(plugin_SRCS
    src/main_plugin.cpp
    ../Filtertest/dummyLogger.cpp
    src/GraphFilterModel.cpp
    src/FilterSortModel.cpp
    src/GraphModelVisualizer.cpp
    src/GraphLoader.cpp
    src/FilterGraph.cpp
    src/FilterNode.cpp
    src/filterMacro.cpp
    src/filterMacroConnector.cpp
    src/FilterGroup.cpp
    src/FilterConnector.cpp
    src/FilterPort.cpp
    src/FilterComment.cpp
    src/FilterGraphImageSaver.cpp
    src/GraphEditor.cpp
    src/FilterConnectorSortModel.cpp
    src/FilterNodeModel.cpp
    src/FilterNodeSortModel.cpp
    src/FilterPortModel.cpp
    src/FilterPortSortModel.cpp
    src/FilterPortPartnerModel.cpp
    src/NodeModel.cpp
    src/NodeSortModel.cpp
    src/FilterGroupModel.cpp
    src/FilterGraphFilterModel.cpp
    src/DnDConnector.cpp
    src/InvalidNodeModel.cpp
    src/CopyPasteHandler.cpp
    src/DirectoryLoader.cpp
    src/DirectoryModel.cpp
    src/invalidIDModel.cpp
    src/pipeController.cpp
    src/selectedFilterNodesFilterModel.cpp
    src/groupController.cpp
    src/graphHelper.cpp
    src/abstractNodeController.cpp
    src/macroController.cpp
    src/plausibilityController.cpp
    src/filterAttributeGroupModel.cpp
    src/ExistingFileValidator.cpp
)

qt5_add_resources(plugin_SRCS resources_plugin.qrc)

include_directories(../Mod_Storage/src)
include_directories(/usr/include/quickqanava)
include_directories(/usr/include/quickcontainers)
include_directories(../App_Gui/plugins)
include_directories(../App_Gui/src) #For filterAttributeSortFilterModel.h FIXME change location?

add_executable(App_Grapheditor ${App_GRAPHEDITOR_SRCS})
#set_target_properties(App_Grapheditor PROPERTIES COMPILE_FLAGS "-fsanitize=address")
add_library(precitecgrapheditorcomponentplugin MODULE ${plugin_SRCS})
#set_target_properties(precitecgrapheditorcomponentplugin PROPERTIES COMPILE_FLAGS "-fsanitize=address")
target_link_libraries(precitecgrapheditorcomponentplugin
    #asan
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Pdf
    ${POCO_LIBS}
    fliplib
    Interfaces
    Analyzer_Interface
    Mod_Storage
    Mod_Gui
    /usr/lib/x86_64-linux-gnu/libQuickQanava.so
    /usr/lib/x86_64-linux-gnu/libQuickContainers.so
    precitecweldmasterguigeneral
)

target_link_libraries(App_Grapheditor
    #asan
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    ${POCO_LIBS}
    fliplib
    Interfaces
    Analyzer_Interface
    precitecweldmasterguigeneral
    dummyLogger
)

install(TARGETS App_Grapheditor DESTINATION ${WM_BIN_INSTALL_DIR})
install(TARGETS precitecgrapheditorcomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/grapheditor/components)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/grapheditor/components)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
