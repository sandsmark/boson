# cmake <= 2.4.2 can catch wrong versions of moc and uic - e.g. if "moc" or
# "uic" in /usr/bin are from Qt 4.x. Even if QTDIR is set correctly.
# This macro checks which version is catched and reports errors.

MACRO ( CHECK_QT3_EXECUTABLES )
  IF (NOT QT_MOC_EXECUTABLE)
    MESSAGE( FATAL_ERROR "QT_MOC_EXECUTABLE is not set" )
  ENDIF (NOT QT_MOC_EXECUTABLE)
  IF (NOT QT_UIC_EXECUTABLE)
    MESSAGE( FATAL_ERROR "QT_UIC_EXECUTABLE is not set" )
  ENDIF (NOT QT_UIC_EXECUTABLE)

  EXEC_PROGRAM(${QT_MOC_EXECUTABLE}
     ARGS -v
     OUTPUT_VARIABLE _moc_version
  )
  STRING(REGEX MATCH "Qt\\ 3\\." _moc_version_2 ${_moc_version})
  IF (NOT _moc_version_2)
    MESSAGE( FATAL_ERROR "moc seems not to be from Qt 3.x\n  version reported: ${_moc_version}\n  executable: ${QT_MOC_EXECUTABLE}")
  ENDIF (NOT _moc_version_2)

  EXEC_PROGRAM(${QT_UIC_EXECUTABLE}
     ARGS -version
     OUTPUT_VARIABLE _uic_version
  )
  STRING(REGEX MATCH "for Qt\\ version\\ 3\\." _uic_version_2 ${_uic_version})
  IF (NOT _uic_version_2)
    MESSAGE( FATAL_ERROR "uic seems not to be from Qt 3.x\n  version reported: ${_uic_version}\n  executable: ${QT_UIC_EXECUTABLE}")
  ENDIF (NOT _uic_version_2)



  # AB: cmake <= 2.4.2 (at least!) have a bug in kde3uic.cmake: it uses "uic" as
  #     executable, not QT_UIC_EXECUTABLE. thus if /usr/bin/uic is from Qt 4.x,
  #     we're in trouble.
  #     report that to the user!
  EXEC_PROGRAM(uic
     ARGS -version
     OUTPUT_VARIABLE _uic_version
  )

  # AB: this sucks - we can test for cmake 2.4 and 2.5, but not for 2.4.2
  # -> therefore we need to use a STATUS message here instead of a FATAL_ERROR:
  #    the user might be using 2.4.x where kde3uic.cmake is fixed.
  # AB: UPDATE: we don't need this test anymore: we provide a fixed version of
  #             kde3uic.cmake
#  IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.5)
#    STRING(REGEX MATCH "for Qt\\ version\\ 3\\." _uic_version_2 ${_uic_version})
#    IF (NOT _uic_version_2)
#      MESSAGE( STATUS "WARNING: QT_UIC_EXECUTABLE is set correctly, however cmake version <= 2.4.2 also requires that the \"uic\" executable in path is a Qt 3.x uic. This seems not to be the case on your system. Version string: ${_uic_version}")
#    ENDIF (NOT _uic_version_2)
#  ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.5)

ENDMACRO ( CHECK_QT3_EXECUTABLES )

