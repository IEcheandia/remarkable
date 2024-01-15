add_library(Filter_Bridges MODULE
    manifest.cpp
    blobSource.cpp
    doubleSource.cpp
    houghPPCandidateSource.cpp
    imageSource.cpp
    pointListSource.cpp
    poorPenetrationCandidateSource.cpp
    sampleSource.cpp
    seamFindingSource.cpp
    startEndInfoSource.cpp
    surfaceInfoSource.cpp
    blobSink.cpp
    doubleSink.cpp
    houghPPCandidateSink.cpp
    imageSink.cpp
    pointListSink.cpp
    poorPenetrationCandidateSink.cpp
    sampleSink.cpp
    seamFindingSink.cpp
    startEndInfoSink.cpp
    surfaceInfoSink.cpp
    lineSink.cpp
    lineSource.cpp
)

target_link_libraries(Filter_Bridges PRIVATE fliplib)

install(TARGETS Filter_Bridges DESTINATION ${WM_LIB_INSTALL_DIR})
