set(plugin_SRCS
    main.cpp
    intensityProfileModel.cpp
    imageHistogramModel.cpp
    imageItem.cpp
    imageMeasurementController.cpp
    imageNode.cpp
    nodeBasedOverlayCanvas.cpp
    overlayGroupFilterModel.cpp
    overlayGroupModel.cpp
    sourceImageItem.cpp
    lineModel.cpp
    lineLaserFilterModel.cpp
)

add_library(precitecimagecomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecimagecomponentplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/")
target_link_libraries(precitecimagecomponentplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Concurrent
    ${POCO_LIBS}
    Framework_Module
    Interfaces
    Precitec::precitecplotter
    Precitec::precitecnotifications
)

# copy qml files to build dir
# required for executing from build directory
configure_file(qml/qmldir "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/qmldir" COPYONLY)
configure_file(qml/ImageComponent.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/ImageComponent.qml" COPYONLY)
configure_file(qml/SourceImageComponent.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/SourceImageComponent.qml" COPYONLY)
configure_file(qml/ImageMeasuringDialog.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/ImageMeasuringDialog.qml" COPYONLY)
configure_file(qml/PointSelector.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/PointSelector.qml" COPYONLY)
configure_file(qml/InfoBoxDialog.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/InfoBoxDialog.qml" COPYONLY)
configure_file(qml/InfoBoxTab.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/image/InfoBoxTab.qml" COPYONLY)

install(TARGETS precitecimagecomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/image)
install(DIRECTORY qml/ DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/image)

if (Qt5Test_FOUND)
    add_subdirectory(autotests)
endif()
