link_libraries(stdc++fs)

add_library(SchedulerHelpers STATIC
    commandLineParser.cpp
    fileUpload.cpp
    pipeLogger.cpp
)

target_link_libraries(SchedulerHelpers
    PRIVATE
        curl
)

add_executable(TestProgram
    TestProgram.cpp
    pipeLogger.cpp
)

add_executable(TransferFile
    TransferFile.cpp
)
target_link_libraries(TransferFile
    PRIVATE
        curl
        SchedulerHelpers
)

add_executable(TransferDirectory
    TransferDirectory.cpp
)
target_link_libraries(TransferDirectory
    PRIVATE
        curl
        SchedulerHelpers
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_executable(BackupToLocalDirectory
    backupHelper.cpp
    backupToLocalDirectory.cpp
)
target_link_libraries(BackupToLocalDirectory
    PRIVATE
        Qt5::Core
        Precitec::precitecremovabledevices
        precitecweldmasterguigeneral
        precitecweldmasterguiconfiguration
        SchedulerHelpers
)

add_executable(BackupToRemoteDirectory
    backupHelper.cpp
    backupToRemoteDirectory.cpp
)
target_link_libraries(BackupToRemoteDirectory
    PRIVATE
        Qt5::Core
        Precitec::precitecremovabledevices
        precitecweldmasterguigeneral
        precitecweldmasterguiconfiguration
        curl
        SchedulerHelpers
)

add_executable(DeleteOldBackups
    DeleteOldBackups.cpp
)

target_link_libraries(DeleteOldBackups
    PRIVATE
        Qt5::Core
        Precitec::precitecremovabledevices
        SchedulerHelpers
)


add_executable(ResultExcelFileFromProductInstance
        ResultExcelFileFromProductInstance.cpp
        resultExcelFileFromProductInstance.cpp
)

target_link_libraries(ResultExcelFileFromProductInstance
    PRIVATE
        curl
        Qt5::Core
        Mod_Storage
        Mod_Gui
        Analyzer_Interface
        fliplib
        precitecweldmasterguigeneral
        precitecweldmasterguiconfiguration
        SchedulerHelpers
)

add_executable(ExportProduct
    exportHelper.cpp
    exportProduct.cpp
    ../../../App_Gui/src/separatelyProductExporter.cpp
)
target_link_libraries(ExportProduct
    PRIVATE
        Qt5::Core
        Mod_Storage
        Precitec::precitecremovabledevices
        precitecweldmasterguigeneral
        precitecweldmasterguiconfiguration
        curl
        SchedulerHelpers

)

install(
    TARGETS
        BackupToLocalDirectory
        BackupToRemoteDirectory
        ExportProduct
        TestProgram
        TransferFile
        TransferDirectory
        DeleteOldBackups
        ResultExcelFileFromProductInstance
    DESTINATION
        ${WM_BIN_INSTALL_DIR}
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

