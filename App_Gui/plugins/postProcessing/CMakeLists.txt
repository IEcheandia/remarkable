set(plugin_SRCS
    main.cpp       
    postProcessingController.cpp  
    postProcessingSequenceDataReady.cpp
    postProcessingTriggerServer.cpp
    postProcessing.cpp
  #  postProcessingModule.cpp
  #  abstractModule.cpp
  
   
  
)
#qt5_add_resources(plugin_SRCS precitecpostprocessingcomponentplugin.qrc)
include_directories(../../Mod_Storage/src)
include_directories(../../Framework_Module/include)
include_directories(../../System/include)

add_library(precitecpostprocessingcomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecpostprocessingcomponentplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/postProcessing/")
target_link_libraries(precitecpostprocessingcomponentplugin
    Qt5::Core
    Qt5::Concurrent
    Qt5::Quick
    Precitec::precitecplotter
    Mod_Analyzer
    Mod_GrabberNoHw
    Interfaces
    Mod_InspectionControl
    Framework_Module
)

# copy qml files to build dir
# required for executing from build directory
#configure_file(qml/qmldir "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/postProcessing/qmldir" COPYONLY)

install(TARGETS precitecpostprocessingcomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/postprocessing)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/postprocessing)


#if (Qt5Test_FOUND)
#    add_subdirectory(autotests)
#endif()
