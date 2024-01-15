set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(App_WobbelFigureEditor_SRCS
    src/main.cpp
)

qt5_add_resources(App_WobbelFigureEditor_SRCS resources.qrc)

set(plugin_SRCS
    src/main_plugin.cpp
    src/WobbleFigure.cpp
    src/LaserPoint.cpp
    src/commandManager.cpp
    src/WobbleFigureEditor.cpp
    src/FileModel.cpp
    src/FigureCreator.cpp
    src/LaserTrajectory.cpp
    src/AttributeController.cpp
    src/FigureAnalyzer.cpp
    src/PlotHandler.cpp
    src/fileSortModel.cpp
    src/fileSaveHandler.cpp
    src/laserPowerController.cpp
    src/selectionHandler.cpp
    src/powerRampModel.cpp
    src/figureEditorSettings.cpp
    src/gridController.cpp
    src/wobbleFigureModel.cpp
    src/abstractDataExchangeController.cpp
    src/importSeamDataController.cpp
    src/laserPointController.cpp
    src/simulationController.cpp
    src/wobbleFigureDataModel.cpp
    src/plausibilityChecker.cpp
    src/previewController.cpp
    src/dxfImportController.cpp
    src/actionPointPositionSeamFigure.cpp
    src/actionPointPositionWobbleFigure.cpp
    src/actionPointPositionOverlay.cpp
    src/actionPointPropertieSeamLaserPower.cpp
    src/actionPointPropertieWobbleLaserPower.cpp
    src/actionPointPropertieSeamRingPower.cpp
    src/actionPointPropertieWobbleRingPower.cpp
    src/actionPointPropertieSeamVelocity.cpp
    src/actionPointRampSeamFigure.cpp
    src/actionPointCreateSeamFigure.cpp
    src/rampModel.cpp
    src/rampValidator.cpp
    src/valueConverter.cpp
    src/requestChangesManager.cpp
    src/trajectoryColorsValidator.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

qt5_add_resources(plugin_SRCS resources_plugin.qrc)

include_directories(../Mod_Storage/src)
include_directories(/usr/include/quickqanava)
include_directories(/usr/include/quickcontainers)

add_executable(App_WobbelFigureEditor ${App_WobbelFigureEditor_SRCS})

add_library(precitecfigureeditorcomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecfigureeditorcomponentplugin
    PROPERTIES COMPILE_FLAGS
    "-Wno-error=catch-value"
)

target_link_libraries(precitecfigureeditorcomponentplugin
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Concurrent
    Precitec::precitecplotter
    Mod_Storage
    /usr/lib/x86_64-linux-gnu/libQuickQanava.so
    /usr/lib/x86_64-linux-gnu/libQuickContainers.so
    Interfaces
    dxfReader
)

target_link_libraries(App_WobbelFigureEditor
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Concurrent
    Qt5::Pdf
    Precitec::precitecplotter
    precitecweldmasterguigeneral
    Interfaces
)

install(TARGETS App_WobbelFigureEditor DESTINATION ${WM_BIN_INSTALL_DIR})
install(TARGETS precitecfigureeditorcomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/wobbleFigureEditor/components)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/wobbleFigureEditor/components)

if (BUILD_TESTING)
    add_subdirectory(autotests/)
endif ()
