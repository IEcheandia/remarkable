include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

set(STORAGE_LIBS
    Mod_Storage
    Precitec::precitecusermanagement
    Precitec::precitecplotter
    Interfaces
    fliplib
    ${POCO_LIBS}
    pthread
    ${XLSXWRITER}
)

qtTestCase(
    NAME
        testProductLoading
    SRCS
        testProductLoading.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProduct
    SRCS
        testProduct.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductSaving
    SRCS
        testProductSaving.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductModel
    SRCS
        testProductModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsSerializer
    SRCS
        testResultsSerializer.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsStatisticsController
    SRCS
        testResultsStatisticsController.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsStatisticsSeamSeriesModel
    SRCS
        testResultsStatisticsSeamSeriesModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsStatisticsSeamsModel
    SRCS
        testResultsStatisticsSeamsModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsStatisticsSeamModel
    SRCS
        testResultsStatisticsSeamModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsLoader
    SRCS
        testResultsLoader.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductInstanceModel
    SRCS
        testProductInstanceModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductInstanceTableModel
    SRCS
        testProductInstanceTableModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductInstanceSeamModel
    SRCS
        testProductInstanceSeamModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultsStorageService
    SRCS
        testResultsStorageService.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testLaserControlPreset
    SRCS
        testLaserControlPreset.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testFilterInstanceModel
    SRCS
        testFilterInstanceModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testAttributeModel
    SRCS
        testAttributeModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testGraphModel
    SRCS
        testGraphModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testFilterGroupsModel
    SRCS
        testFilterGroupsModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testFilterAttributesModel
    SRCS
        testFilterAttributesModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testResultConfig
    SRCS
        testResultConfig.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testSeamError
    SRCS
        testSeamError.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testIntervalError
    SRCS
        testIntervalError.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testReferenceCurve
    SRCS
        testReferenceCurve.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testSubGraphModel
    SRCS
        testSubGraphModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testErrorSettingModel
    SRCS
        testErrorSettingModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testSensorSettingsModel
    SRCS
        testSensorSettingsModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testGraphExporter
    SRCS
        testGraphExporter.cpp
        ../src/graphExporter.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        System
)

qtTestCase(
    NAME
        testGraphMacroExtender
    SRCS
        testGraphMacroExtender.cpp
        ../src/graphExporter.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
)

qtTestCase(
    NAME
        testLevelConfig
    SRCS
        testLevelConfig.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testSimpleError
    SRCS
        testSimpleError.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testProductError
    SRCS
        testProductError.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testSeamSeriesError
    SRCS
        testSeamSeriesError.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testGaugeRange
    SRCS
        testGaugeRange.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testQualityNormLevel
    SRCS
        testQualityNormLevel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testQualityNormResult
    SRCS
        testQualityNormResult.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testQualityNorm
    SRCS
        testQualityNorm.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testQualityNormModel
    SRCS
        testQualityNormModel.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testLinkedSeam
    SRCS
        testLinkedSeam.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testExtendedProductInfoHelper
    SRCS
        testExtendedProductInfoHelper.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testGraphFunctions
    SRCS
        testGraphFunctions.cpp
    LIBS
        ${STORAGE_LIBS}
)

qtTestCase(
    NAME
        testGraphReference
    SRCS
        testGraphReference.cpp
    LIBS
        ${STORAGE_LIBS}
)
