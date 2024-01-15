set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
link_libraries(stdc++fs)
qtTestCase(
    NAME
        TriggerTest
    SRCS
        triggerTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        CronTriggerTest
    SRCS
        cronTriggerTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        EventTriggerTest
    SRCS
        eventTriggerTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        TaskTest
    SRCS
        taskTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        TransferFileTaskTest
    SRCS
        transferFileTaskTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        TransferDirectoryTaskTest
    SRCS
        transferDirectoryTaskTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        signalManagerTest
    SRCS
        signalManagerTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        signalManagerWriterTest
    SRCS
        signalManagerWriterTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        signalManagerLoaderTest
    SRCS
        signalManagerLoaderTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        taskSchedulerTest
    SRCS
        taskSchedulerTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        taskProcessTest
    SRCS
        taskProcessTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        configurationMonitorTest
    SRCS
        configurationMonitorTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
        Mod_Scheduler
)


qtTestCase(
    NAME
        deleteOldBackupsTaskTest
   SRCS
        deleteOldBackupsTaskTest.cpp
        ../../Filtertest/dummyLogger.cpp
   LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        resultExcelFileFromProductInstanceTaskTest
    SRCS
        resultExcelFileFromProductInstanceTaskTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)

qtTestCase(
    NAME
        exportProductTaskTest
    SRCS
        exportProductTaskTest.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Mod_Scheduler
)
