testCase(
    NAME
        imageDataHolderTest
    SRCS
        imageDataHolderTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        System
        Framework_Module
        Interfaces
        Mod_GrabberNoHw
)

testCase(
    NAME
        imageLoaderTest
    SRCS
        imageLoaderTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Mod_GrabberNoHw
        System
        Interfaces
)

testCase(
    NAME sequenceInformationTest
    SRCS 
        sequenceInformationTest.cpp
    LIBS
        Mod_GrabberNoHw
)
