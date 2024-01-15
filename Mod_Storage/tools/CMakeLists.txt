set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(  RESULT_SRCS
    ../src/resultsSerializer.cpp
)


set (LIBS
    Qt5::Core
    Interfaces
    ${POCO_LIBS}
)

add_executable(compareResults compareResults.cpp ../src/resultsSerializer.cpp)
target_link_libraries(compareResults   ${LIBS})


add_executable(convertResultsToCSV convertResultsToCSV.cpp ../src/resultsSerializer.cpp )
target_link_libraries(convertResultsToCSV  ${LIBS})

add_executable(readProductJson 
    readProductJson.cpp
    ../src/attribute.cpp
    ../src/attributeField.cpp
    ../src/attributeFileInformation.cpp
    ../src/attributeModel.cpp
    ../src/abstractMeasureTask.cpp
    ../src/changeTracker.cpp
    ../src/jsonSupport.cpp
    ../src/resultSetting.cpp
    ../src/resultSettingModel.cpp
    ../src/parameter.cpp
    ../src/parameterSet.cpp
    ../src/product.cpp
    ../src/resultChangeEntry.cpp
    ../src/seam.cpp
    ../src/seamInterval.cpp
    ../src/seamSeries.cpp
    ../src/overlyingError.cpp
    ../src/seamSeriesError.cpp
    ../src/simpleError.cpp
    ../src/seamError.cpp
    ../src/intervalError.cpp
    ../src/productError.cpp
    ../src/resultsSerializer.cpp
    ../src/levelConfig.cpp
    ../src/referenceSerializer.cpp
    ../src/qualityNorm.cpp
    ../src/qualityNormResult.cpp
    ../src/qualityNormLevel.cpp
    ../src/gaugeRange.cpp
    ../src/linkedSeam.cpp
    ../src/referenceCurve.cpp
    ../src/referenceCurveData.cpp
    ../src/parameterFilterGroup.cpp
)

target_link_libraries(readProductJson 
    ${LIBS}
    Qt5::Concurrent
    Precitec::precitecusermanagement
    Precitec::precitecplotter
)
