project(realtime)
cmake_minimum_required(VERSION 3.5)

# find ECM, needed for writing version information to header
find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(Git)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/branchName.txt)
    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/branchName.txt WELDMASTER_BRANCH_NAME)
else()
    set(GIT_RESULT 1)
    if (${GIT_FOUND})
        execute_process(
            COMMAND
                ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY
                ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE
                GIT_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE
                WELDMASTER_BRANCH_NAME
        )
    endif()
    if (NOT (${GIT_RESULT} EQUAL 0))
        find_package(Hg REQUIRED)
        HG_WC_INFO(${PROJECT_SOURCE_DIR} Project)
        execute_process(
                COMMAND ${HG_EXECUTABLE} branch
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE WELDMASTER_BRANCH_NAME)
    endif()
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/changesetId.txt)
    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/changesetId.txt WELDMASTER_CHANGESET_ID)
else()
    set(GIT_RESULT 1)
    if (${GIT_FOUND})
        execute_process(
            COMMAND
                ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY
                ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE
                GIT_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE
                WELDMASTER_CHANGESET_ID
        )
    endif()
    if (NOT (${GIT_RESULT} EQUAL 0))
        find_package(Hg REQUIRED)
        HG_WC_INFO(${PROJECT_SOURCE_DIR} Project)
        set(WELDMASTER_CHANGESET_ID ${Project_WC_CHANGESET})
    endif()
endif()
if (WELDMASTER_CHANGESET_ID)
    configure_file(changesetId.h.in ${CMAKE_CURRENT_BINARY_DIR}/changesetId.h)
endif()

file(STRINGS ${CMAKE_SOURCE_DIR}/../deploy/version.ini WELDMASTER_VERSION REGEX "OutputFileVersion=(.*)")
string(REGEX REPLACE "OutputFileVersion=(.*)" "\\1" WELDMASTER_VERSION "${WELDMASTER_VERSION}")

if (NOT(${WELDMASTER_BRANCH_NAME} MATCHES "release"))
    set(WELDMASTER_VERSION "${WELDMASTER_VERSION}-${WELDMASTER_CHANGESET_ID}")
endif()

string(REPLACE "_" "." PROJECT_VERSION "${WELDMASTER_VERSION}")
include(ECMSetupVersion)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX WELDMASTER VERSION_HEADER weldmasterVersion.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# set corporate id version
set(CORPIDELEMENTS_VERSION 2.21.0)

option(BUILD_WELDMASTER "Build WeldMaster applications" ON)

option(BUILD_SISO_GRABBER "Build support for silicon software grabber" ${BUILD_WELDMASTER})
option(MOD_GRABBER_DEBUG_DISPLAY "Build Mod_Grabber with debug display support" OFF)

option(BUILD_GIGE_GRABBER "Build support for GigE grabber" ${BUILD_WELDMASTER})
option(BUILD_NOHW_GRABBER "Build support for NoHw grabber" ${BUILD_WELDMASTER})

option(BUILD_GRAPHEDITOR "Build support for Grapheditor" ${BUILD_WELDMASTER})
option(BUILD_FILTERS "Build filters" ${BUILD_WELDMASTER})
option(BUILD_FILTERTEST_STANDALONE "Build filtertest only" OFF)
option(BUILD_SCANLAB_RTC6 "Build support for Scanlab RTC6 card" ${BUILD_WELDMASTER})
option(BUILD_HILSCHER_FIELDBUS "Build support for fieldbus based on Hilscher libcifx" ${BUILD_WELDMASTER})
option(BUILD_ETHERCAT_MASTER "Build support for EtherCAT Master" ${BUILD_WELDMASTER})
option(BUILD_CHR "Build support for CHR communication" ${BUILD_WELDMASTER})
option(BUILD_TRIGGER "Build support for PreciEncoder based trigger" ${BUILD_WELDMASTER})
option(BUILD_WELDHEAD "Build support for WeldHead" ${BUILD_WELDMASTER})
option(BUILD_SRING_TCPCOMMUNICATION "Build support for Sring TCP communication" ${BUILD_WELDMASTER})
option(BUILD_WERROR "Enable building with -Werror" FALSE)
option(BUILD_TESTING "Enable building unit tests" TRUE)
option(BUILD_COVERAGE "Build the project with gcov support" OFF)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-result -fsigned-char -g")

if (BUILD_WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

if (BUILD_TESTING)
    enable_testing()
endif()

if(BUILD_COVERAGE)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
    else()
        set(BUILD_COVERAGE FALSE)
    endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

include_directories(
    thirdParty/photonfocus/include/
    Analyzer_Interface/include/
    Analyzer_Interface/include/math/
    fliplib/include/
    Framework_Module/include/
    GlobalDefs/
    Interfaces/include/
    Mod_Analyzer/include/
    Mod_Calibration/include/
    Mod_CutBoxBackend/src/
    Mod_CutBoxBackend/src/hardware_interfaces
    Mod_Grabber/include/
    Mod_GrabberNoHw/include/
    Mod_CHRCommunication/include/
    Mod_EtherCATMaster/include/
    Mod_Fieldbus/include/
    Mod_InspectionControl/include/
    Mod_ModuleManager/include/
    Mod_Service/include/
    Mod_VideoRecorder/include/
    Mod_WeldHeadControl/include/
    Mod_Workflow/include/
    Mod_Trigger/include/
    Mod_TCPCommunication/include/
    Mod_Scheduler/include/
    Rhothor_Lib/
    System/include/
    VImapping/
    wmAtomics/
    wmLogger/
    wmMPSCQueue/
    dxfReader/
)

find_package(OpenCV REQUIRED core imgproc)
include_directories(${OpenCV_INCLUDE_DIRS})

find_library(POCO_FOUNDATION NAMES libPocoFoundation.so)
find_library(POCO_NET NAMES libPocoNet.so)
find_library(POCO_UTIL NAMES libPocoUtil.so)
find_library(POCO_XML NAMES libPocoXML.so)
set(POCO_LIBS
    ${POCO_FOUNDATION}
    ${POCO_NET}
    ${POCO_UTIL}
    ${POCO_XML}
)

find_library(XLSXWRITER NAMES libxlsxwriter.so)
find_path(
    XLSXWRITER_INCLUDE_PATH
    NAMES
        xmlwriter.h
    HINTS
        /usr/include/xlsxwriter/
)

# This is required to make feature_summary() work with find_path()/find_library()
# calls. Should be dropped as soon as XlxsWriter is updated to at least 1.0.1 (VY)
set_property(GLOBAL APPEND PROPERTY _CMAKE_XlsxWriter_TYPE REQUIRED)
if (XLSXWRITER AND XLSXWRITER_INCLUDE_PATH)
    set_property(GLOBAL APPEND PROPERTY PACKAGES_FOUND XlsxWriter)
else()
    set_property(GLOBAL APPEND PROPERTY PACKAGES_NOT_FOUND XlsxWriter)
endif()

find_package(PkgConfig)
if (${BUILD_GIGE_GRABBER})
    pkg_check_modules(ARAVIS REQUIRED aravis-0.8)
endif()

if (${BUILD_SISO_GRABBER})
    file(TO_CMAKE_PATH "$ENV{SISODIR5}" SISODIR5)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SISODIR5}/cmake/modules")
    include(SisoPlatform)
    find_package(FgLib5 REQUIRED)
    find_package(ClSerSis REQUIRED)
endif()

if (${BUILD_SCANLAB_RTC6})
    find_path(
        SCANLAB_INCLUDE_PATH
        NAMES
            rtc6.h
        HINTS
            /usr/include/scanlab
    )
    find_library(
        SCANLAB_LIB
            NAMES
            librtc6.so
        HINTS
            /usr/lib/x86_64-linux-gnu
    )
endif()

if (${BUILD_ETHERCAT_MASTER})
    find_path(
        ETHERCAT_INCLUDE_PATH
        NAMES
            ecrt.h ectty.h
        HINTS
            /opt/etherlab/include
    )
    find_library(
        ETHERCAT_LIB
        NAMES
            libethercat.so
        HINTS
            /opt/etherlab/lib
    )
endif()

if (${BUILD_HILSCHER_FIELDBUS})
    find_path(
        HILSCHER_INCLUDE_PATH
        NAMES
            cifxlinux.h rcX_Public.h
        HINTS
            /usr/include/cifx
    )
    find_library(
        HILSCHER_LIB
        NAMES
            libcifx.so
        HINTS
            /usr/lib/x86_64-linux-gnu
    )
endif()

if (${BUILD_WELDHEAD})
    find_path(
        PRECIRS422_INCLUDE_PATH
        NAMES
            PreciRS422.h
        HINTS
            /opt/Precitec/PreciRS422Driver
    )
endif()

if (${BUILD_TRIGGER})
    find_path(
        PRECIENCODER_INCLUDE_PATH
        NAMES
            PreciEncoder.h
        HINTS
            /opt/Precitec/PreciEncoderDriver
    )
endif()

if (${MOD_GRABBER_DEBUG_DISPLAY})
    find_package(XCB 1.10
        REQUIRED COMPONENTS
            XCB
            ICCCM
    )
endif()

if (${BUILD_CHR})
    find_path(
        CHROCODILE_INCLUDE_PATH
        NAMES
            CHRocodileLib.h CHRocodileLibSpecialFunc.h
        HINTS
            /usr/include/CHRocodileLibCore
    )
    find_library(
        CHROCODILE_LIB
        NAMES
            libCHRocodile.so
        HINTS
            /usr/lib/x86_64-linux-gnu
    )
endif()

if(${BUILD_CUTBOX})
    find_package(CanOpenNode REQUIRED)
endif()

if (${BUILD_TESTING})
    find_package(CppUnit)
else ()
    set(CPPUNIT_FOUND, FALSE)
endif ()

if (${CPPUNIT_FOUND})

    function(testCase)
        set(oneValueArgs NAME)
        set(multiValueArgs SRCS LIBS)
        cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        include_directories(${CPPUNIT_INCLUDE_DIRS})

        add_executable(${ARGS_NAME} ${ARGS_SRCS})

        target_link_libraries(${ARGS_NAME} ${CPPUNIT_LIBRARIES} ${POCO_LIBS} pthread rt ${ARGS_LIBS})
        add_test(Weldmaster-${ARGS_NAME} ${ARGS_NAME})
        set_property(TEST Weldmaster-${ARGS_NAME} PROPERTY ENVIRONMENT "WM_LOG_STDOUT=1")
    endfunction()
endif()

include(CheckIncludeFiles)
check_include_files(smmintrin.h HAVE_SSE4)

set(WM_INSTALL_DIR opt/wm_inst)
set(WM_BIN_INSTALL_DIR ${WM_INSTALL_DIR}/bin)
set(WM_LIB_INSTALL_DIR ${WM_INSTALL_DIR}/lib)
set(WM_INCLUDE_INSTALL_DIR ${WM_INSTALL_DIR}/include)

# set RPATH
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/opt/wm_inst/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(PrecitecPlotter_DIR "/usr/local/lib/cmake/PrecitecPlotter")

find_package(PrecitecUserManagement ${CORPIDELEMENTS_VERSION})
find_package(PrecitecPlotter ${CORPIDELEMENTS_VERSION})
find_package(PrecitecNotifications ${CORPIDELEMENTS_VERSION})
find_package(PrecitecNetworkManagement ${CORPIDELEMENTS_VERSION})
find_package(PrecitecRemovableDevices ${CORPIDELEMENTS_VERSION})

set(CMAKE_PREFIX_PATH "/opt/qt5/5.15.4/gcc_64/lib/cmake")
set(Qt5Quick_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Quick")
#set (QML2_IMPORT_PATH "/usr/lib/x86_64-linux-gnu/qt5/")
set(Qt5Concurrent_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Concurrent")
#set(Qt5Core_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Core")
#set(Qt5DBus_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5DBus")
#set(Qt5Gui_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Gui")
#set(Qt5LinguistTools_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5LinguistTools")
#set(Qt5Network_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Network")
set(Qt5Pdf_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Pdf")
#set(Qt5QmlModels_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5QmlModels")
#set(Qt5Qml_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Qml")
set(Qt5QuickControls2_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5QuickControls2")
set(Qt5Quick_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Quick")
#set(Qt5Test_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Test")
set(Qt5Widgets_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5Widgets")
set(Qt5_DIR "/opt/qt5/5.15.4/gcc_64/lib/cmake/Qt5")

set(QtModules
    Concurrent
    Gui
    Quick
    QuickControls2
    Widgets
    Pdf
)
if (BUILD_TESTING)
    set(QtModules ${QtModules} Test)
endif ()
find_package(Qt5 COMPONENTS ${QtModules})
find_package(Qt5LinguistTools)
#add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

if (Qt5Test_FOUND)
    function(qtTestCase)
        set(oneValueArgs NAME)
        set(multiValueArgs SRCS LIBS)
        cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        include_directories(${CPPUNIT_INCLUDE_DIRS})

        add_executable(${ARGS_NAME} ${ARGS_SRCS})

        target_link_libraries(${ARGS_NAME} Qt5::Test Qt5::Core ${ARGS_LIBS})
        add_test(NAME Weldmaster-Gui-${ARGS_NAME} COMMAND xvfb-run -a --server-args=-screen\ 0\ 1024x768x24 ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_NAME})
        set_property(TEST Weldmaster-Gui-${ARGS_NAME} PROPERTY ENVIRONMENT "QT_QPA_PLATFORM=offscreen;WM_LOG_STDOUT=1")
    endfunction()

    function(qtBenchmarkCase)
        set(oneValueArgs NAME)
        set(multiValueArgs SRCS LIBS)
        cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        include_directories(${CPPUNIT_INCLUDE_DIRS})

        add_executable(${ARGS_NAME} ${ARGS_SRCS})

        target_link_libraries(${ARGS_NAME} Qt5::Test Qt5::Core ${ARGS_LIBS})
    endfunction()

endif()

function(installWeldmasterHeaders)
    set(oneValueArgs DESTINATION)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    install(
        FILES
            ${ARGS_FILES}
        DESTINATION
            ${WM_INCLUDE_INSTALL_DIR}/precitec/weldmaster/${ARGS_DESTINATION}/
        COMPONENT
            devel
    )
    foreach(HEADER ${ARGS_FILES})
        get_filename_component(HEADERNAME ${HEADER} NAME)
        configure_file(${HEADER} "${CMAKE_BINARY_DIR}/bin/include/${ARGS_DESTINATION}/${HEADERNAME}" COPYONLY)
    endforeach()
endfunction()

string(TIMESTAMP WELDMASTER_BUILD_TIMESTAMP %Y-%m-%dT%H:%M:%S)


configure_file(config-weldmaster.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-weldmaster.h)

include(FeatureSummary)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (${BUILD_WELDMASTER})
    add_subdirectory(Analyzer_Interface)
    add_subdirectory(App_Calibration)
    add_subdirectory(App_InspectionControl)
    add_subdirectory(App_LoggerServer)
    add_subdirectory(App_MockAxis)
    add_subdirectory(App_ModuleManager)
    add_subdirectory(App_Service)
    add_subdirectory(App_Simulation)
    add_subdirectory(App_VideoRecorder)
    add_subdirectory(ConnectServer)
    add_subdirectory(fliplib)
    add_subdirectory(Framework_Module)
    add_subdirectory(App_WobbelFigureEditor)
    add_subdirectory(App_Workflow)
    add_subdirectory(App_Scheduler)
    add_subdirectory(GlobalDefs)
    add_subdirectory(Interfaces)
    add_subdirectory(Mod_Analyzer)
    add_subdirectory(Mod_Calibration)
    add_subdirectory(Mod_InspectionControl)
    add_subdirectory(Mod_ModuleManager)
    add_subdirectory(Mod_Service)
    add_subdirectory(Mod_VideoRecorder)
    add_subdirectory(Mod_Workflow)
    add_subdirectory(Mod_Scheduler)
    add_subdirectory(System)
    add_subdirectory(wmAtomics)
    add_subdirectory(wmLogger)
    add_subdirectory(wm_inst)
    add_subdirectory(thirdParty)
    add_subdirectory(dxfReader)
    add_subdirectory(dxf2precitec)
    if (Qt5Gui_FOUND)
        add_subdirectory(Filtertest)
        add_subdirectory(Mod_Storage)
        add_subdirectory(App_Storage)
        add_subdirectory(App_Gui)
    endif()
elseif (${BUILD_CUTBOX})
    add_subdirectory(ConnectServer)
    add_subdirectory(Analyzer_Interface)
    add_subdirectory(thirdParty)
    add_subdirectory(Mod_Analyzer)
    add_subdirectory(Mod_ModuleManager)
    add_subdirectory(App_ModuleManager)
    add_subdirectory(wmLogger)
    add_subdirectory(Mod_CutBoxBackend)
    add_subdirectory(cb_inst)
    if (Qt5Gui_FOUND)
        add_subdirectory(Framework_Module)
        add_subdirectory(Filtertest)
        add_subdirectory(Mod_Storage)
        add_subdirectory(App_Storage)
        add_subdirectory(App_Gui/plugins)
        add_subdirectory(fliplib)
        add_subdirectory(System)
        add_subdirectory(Interfaces)
        add_subdirectory(Mod_VideoRecorder)
        add_subdirectory(App_CutBoxGui)
        add_subdirectory(App_CutBoxBackend)
    endif()
elseif(${BUILD_FILTERTEST_STANDALONE})
    set(CMAKE_DISABLE_FIND_PACKAGE_PrecitecUserManagement TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_PrecitecPlotter TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_PrecitecNotifications TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_PrecitecNetworkManagement TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_PrecitecRemovableDevices TRUE)
    add_subdirectory(Analyzer_Interface)
    add_subdirectory(Mod_Analyzer)
    add_subdirectory(fliplib)
    add_subdirectory(Framework_Module)
    add_subdirectory(System)
    add_subdirectory(Interfaces)
    add_subdirectory(thirdParty)
    add_subdirectory(Filtertest)
endif()

if(${BUILD_GRAPHEDITOR})
    add_subdirectory(App_Grapheditor)
endif()

if (${BUILD_ETHERCAT_MASTER})
    add_subdirectory(App_EtherCATMaster)
endif()

if (${BUILD_HILSCHER_FIELDBUS})
    add_subdirectory(App_Fieldbus)
endif()

if (${BUILD_GIGE_GRABBER})
   add_subdirectory(App_GrabberGigE)
endif()

if (${BUILD_NOHW_GRABBER})
    add_subdirectory(App_GrabberNoHw)
    add_subdirectory(Mod_GrabberNoHw)
endif()

if (${BUILD_CHR})
    add_subdirectory(App_CHRCommunication)
endif()

if (${BUILD_WELDHEAD})
    add_subdirectory(App_WeldHeadControl)
endif()

if (${BUILD_TRIGGER})
    add_subdirectory(App_Trigger)
endif()

if (${BUILD_SRING_TCPCOMMUNICATION})
    add_subdirectory(App_TCPCommunication)
endif()

if (${BUILD_FILTERS} OR ${BUILD_FILTERTEST_STANDALONE})
    add_subdirectory(Filter_Bridges)
    add_subdirectory(Filter_Calibration)
    add_subdirectory(Filter_GapTracking)
    add_subdirectory(Filter_ImageProcessing)
    add_subdirectory(Filter_ImageSource)
    add_subdirectory(Filter_LineGeometry)
    add_subdirectory(Filter_LineTracking)
    add_subdirectory(Filter_PoreAnalysis)
    add_subdirectory(Filter_Results)
    add_subdirectory(Filter_SampleSource)
    add_subdirectory(Filter_SeamSearch)
    add_subdirectory(Filter_Utility)
endif()

if (${BUILD_ETHERCAT_MASTER})
    add_subdirectory(Mod_EtherCATMaster)
endif()

if (${BUILD_HILSCHER_FIELDBUS})
    add_subdirectory(Mod_Fieldbus)
endif()

if (${BUILD_CHR})
    add_subdirectory(Mod_CHRCommunication)
endif()

if (${BUILD_WELDHEAD})
    add_subdirectory(Mod_SimulatedRTCBoard)
    add_subdirectory(Mod_WeldHeadControl)
endif()

if (${BUILD_TRIGGER})
    add_subdirectory(Mod_Trigger)
endif()

if (${BUILD_SRING_TCPCOMMUNICATION})
    add_subdirectory(Mod_TCPCommunication)
endif()

if (${BUILD_CHR})
    add_subdirectory(Rhothor_Lib)
endif()

if (${BUILD_SISO_GRABBER})
    add_subdirectory(App_Grabber)
    add_subdirectory(Mod_Grabber)
    add_subdirectory(me4TestLinux)
    add_subdirectory(Pf_Lib)
    add_subdirectory(Pf)
endif()

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR "${WM_INCLUDE_INSTALL_DIR}/precitec/weldmaster/")
configure_package_config_file(
        WeldMaster.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/WeldMasterConfig.cmake
    INSTALL_DESTINATION
        ${WM_LIB_INSTALL_DIR}/cmake/WeldMaster/
    PATH_VARS INCLUDE_INSTALL_DIR
)
set(INCLUDE_INSTALL_DIR "${CMAKE_BINARY_DIR}/bin/include/")
configure_package_config_file(
        WeldMaster.cmake.in ${CMAKE_BINARY_DIR}/bin/cmake/WeldMasterConfig.cmake
    INSTALL_DESTINATION
        ${WM_LIB_INSTALL_DIR}/cmake/WeldMaster/
    PATH_VARS INCLUDE_INSTALL_DIR
)
write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/bin/cmake/WeldMasterConfigVersion.cmake"
    VERSION
        ${WELDMASTER_VERSION}
    COMPATIBILITY
        AnyNewerVersion
)
export(
    TARGETS
        Mod_GrabberNoHw
        Mod_Storage
        Mod_VideoRecorder
        fliplib
        simplIpc
        Analyzer_Interface
        Framework_Module
        Interfaces
        System
    FILE
        ${CMAKE_BINARY_DIR}/bin/cmake/WeldMasterTargets.cmake
    NAMESPACE
        WeldMaster::
)
install(
    FILES
        "${CMAKE_BINARY_DIR}/bin/cmake/WeldMasterConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/WeldMasterConfig.cmake"
    DESTINATION
        ${WM_LIB_INSTALL_DIR}/cmake/WeldMaster/
)
install(
    EXPORT
        WeldMasterTargets
    DESTINATION
        ${WM_LIB_INSTALL_DIR}/cmake/WeldMaster/
    FILE
        WeldMasterTargets.cmake
    NAMESPACE WeldMaster::
)
