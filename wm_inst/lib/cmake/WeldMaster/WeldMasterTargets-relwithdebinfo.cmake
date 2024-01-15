#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "WeldMaster::AnalyzerInterface" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::AnalyzerInterface APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::AnalyzerInterface PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libAnalyzer_Interface.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libAnalyzer_Interface.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::AnalyzerInterface )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::AnalyzerInterface "${_IMPORT_PREFIX}/opt/wm_inst/lib/libAnalyzer_Interface.so" )

# Import target "WeldMaster::fliplib" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::fliplib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::fliplib PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libfliplib.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libfliplib.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::fliplib )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::fliplib "${_IMPORT_PREFIX}/opt/wm_inst/lib/libfliplib.so" )

# Import target "WeldMaster::FrameworkModule" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::FrameworkModule APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::FrameworkModule PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libFramework_Module.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libFramework_Module.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::FrameworkModule )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::FrameworkModule "${_IMPORT_PREFIX}/opt/wm_inst/lib/libFramework_Module.so" )

# Import target "WeldMaster::Interfaces" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::Interfaces APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::Interfaces PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libInterfaces.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libInterfaces.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::Interfaces )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::Interfaces "${_IMPORT_PREFIX}/opt/wm_inst/lib/libInterfaces.so" )

# Import target "WeldMaster::VideoRecorder" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::VideoRecorder APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::VideoRecorder PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_VideoRecorder.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libMod_VideoRecorder.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::VideoRecorder )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::VideoRecorder "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_VideoRecorder.so" )

# Import target "WeldMaster::System" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::System APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::System PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libSystem.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libSystem.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::System )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::System "${_IMPORT_PREFIX}/opt/wm_inst/lib/libSystem.so" )

# Import target "WeldMaster::simplIpc" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::simplIpc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::simplIpc PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libsimplIpc.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libsimplIpc.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::simplIpc )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::simplIpc "${_IMPORT_PREFIX}/opt/wm_inst/lib/libsimplIpc.so" )

# Import target "WeldMaster::Storage" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::Storage APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::Storage PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_Storage.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libMod_Storage.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::Storage )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::Storage "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_Storage.so" )

# Import target "WeldMaster::GrabberNoHw" for configuration "RelWithDebInfo"
set_property(TARGET WeldMaster::GrabberNoHw APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(WeldMaster::GrabberNoHw PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_GrabberNoHw.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libMod_GrabberNoHw.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS WeldMaster::GrabberNoHw )
list(APPEND _IMPORT_CHECK_FILES_FOR_WeldMaster::GrabberNoHw "${_IMPORT_PREFIX}/opt/wm_inst/lib/libMod_GrabberNoHw.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
