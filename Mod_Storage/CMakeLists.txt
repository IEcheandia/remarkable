set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(${XLSXWRITER_INCLUDE_PATH})

set(Lib_Storage_SRCS
    src/abstractGraphModel.cpp
    src/abstractMeasureTask.cpp
    src/attribute.cpp
    src/attributeField.cpp
    src/attributeFileInformation.cpp
    src/attributeModel.cpp
    src/checkDiskUsageCommand.cpp
    src/calibrationChangeNotifier.cpp
    src/changeTracker.cpp
    src/jsonSupport.cpp
    src/resultSetting.cpp
    src/resultSettingModel.cpp
    src/errorSettingModel.cpp
    src/errorTemplate.cpp
    src/metaDataWriterCommand.cpp
    src/nioSettingModel.cpp
    src/filterAttributeModel.cpp
    src/filterGroupsModel.cpp
    src/filterInstanceModel.cpp
    src/graphModel.cpp
    src/moveProductInstanceCommand.cpp
    src/parameter.cpp
    src/parameterSet.cpp
    src/product.cpp
    src/productSeamModel.cpp
    src/productInstanceModel.cpp
    src/productInstanceTableModel.cpp
    src/productInstanceSeriesModel.cpp
    src/productInstanceSortModel.cpp
    src/productInstanceSeamModel.cpp
    src/productInstanceSeamSortModel.cpp
    src/productMetaData.cpp
    src/productModel.cpp
    src/resultsExporter.cpp
    src/resultsLoader.cpp
    src/resultsSeriesLoader.cpp
    src/resultsSerializer.cpp
    src/resultsServer.cpp
    src/resultsStorageService.cpp
    src/resultsStatisticsController.cpp
    src/resultsStatisticsAbstractErrorModel.cpp
    src/resultsStatisticsSeamSeriesErrorModel.cpp
    src/resultsStatisticsSeamSeriesModel.cpp
    src/resultsStatisticsSeamsErrorModel.cpp
    src/resultsStatisticsSeamsModel.cpp
    src/resultsStatisticsSeamErrorModel.cpp
    src/resultsStatisticsSeamModel.cpp
    src/resultsWriterCommand.cpp
    src/resultChangeEntry.cpp
    src/subGraphCategoryFilterModel.cpp
    src/subGraphCheckedFilterModel.cpp
    src/subGraphModel.cpp
    src/seam.cpp
    src/seamMetaData.cpp
    src/seamInterval.cpp
    src/seamSeries.cpp
    src/seamSeriesMetaData.cpp
    src/overlyingError.cpp
    src/seamSeriesError.cpp
    src/simpleError.cpp
    src/seamError.cpp
    src/intervalError.cpp
    src/productError.cpp
    src/graphExporter.cpp
    src/laserControlPreset.cpp
    src/levelConfig.cpp
    src/resultHelper.cpp
    src/sensorSettingsModel.cpp
    src/referenceSerializer.cpp
    src/qualityNorm.cpp
    src/qualityNormModel.cpp
    src/qualityNormResult.cpp
    src/qualityNormLevel.cpp
    src/gaugeRange.cpp
    src/videoDataLoader.cpp
    src/linkedSeam.cpp
    src/referenceCurve.cpp
    src/referenceCurveData.cpp
    src/productStatistics.cpp
    src/seamSeriesStatistics.cpp
    src/seamStatistics.cpp
    src/attributeGroupItem.cpp
    src/attributeGroup.cpp
    src/abstractFilterAttributeModel.cpp
    src/extendedProductInfoHelper.cpp
    src/graphSortModel.cpp
    src/parameterFilterGroup.cpp
    src/resultTemplate.cpp
    src/graphFunctions.cpp
    src/productInstanceDownloadHelper.cpp
)
add_library(Mod_Storage SHARED ${Lib_Storage_SRCS})
set_target_properties(Mod_Storage PROPERTIES EXPORT_NAME Storage)
add_library(WeldMaster::Storage ALIAS Mod_Storage)
target_link_libraries(Mod_Storage Mod_VideoRecorder Qt5::Core Qt5::Concurrent
    Precitec::precitecusermanagement
    Precitec::precitecplotter
    Precitec::precitecremovabledevices
    fliplib
    ${XLSXWRITER}
)

install(TARGETS Mod_Storage EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

file(GLOB MOD_STORAGE_INCLUDES "src/*.h")

installWeldmasterHeaders(
    FILES
        ${MOD_STORAGE_INCLUDES}
    DESTINATION
        storage
)


if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

add_subdirectory(tools)
