# - Find python libraries
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This module also uses FindPythonInterp and thus defines the
# variables defined there.
# This code sets the following variables:
#
#  PYTHON_LIBRARIES     = contains the python library and all libraries it
#                         depends on
#  PYTHON_INCLUDE_DIR   = path to where Python.h is found
#                         (this variable did not exist in cmake <= 2.4.x)
#  PYTHON_LIBS_FOUND    = set to TRUE if the libs were found, FALSE otherwise
#
# Deprecated variables:
#  PYTHON_INCLUDE_PATH  = same as PYTHON_INCLUDE_DIR. this variable exists for
#                         historic reasons, use PYTHON_INCLUDE_DIR instead (see
#                         readme.txt)

INCLUDE(CMakeFindFrameworks)

FIND_PACKAGE(PythonInterp)

# internal macro
MACRO(_GET_PYTHON_CONFIG_VARIABLE _name)
	# see also http://docs.python.org/dist/module-distutils.sysconfig.html
	# note: spaces are important in the python parameters! (even on
	# newlines!)
	EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE}
	-c "import sys
from distutils.sysconfig import get_config_var
sys.stdout.write(get_config_var(\"${_name}\")),"
	OUTPUT_VARIABLE _python_config_variable
	)
ENDMACRO(_GET_PYTHON_CONFIG_VARIABLE)

IF (PYTHON_EXECUTABLE)
	_GET_PYTHON_CONFIG_VARIABLE("LIBS")
	set(_python_LIBS "${_python_config_variable}")
	_GET_PYTHON_CONFIG_VARIABLE("SYSLIBS")
	set(_python_SYSLIBS "${_python_config_variable}")
	set(_python_dependency_libs "${_python_LIBS} ${_python_SYSLIBS}")
	_GET_PYTHON_CONFIG_VARIABLE("VERSION")
	set(_python_version "${_python_config_variable}")

	FIND_LIBRARY(PYTHON_LIBRARY
		NAMES "python${_python_version}"
		PATH_SUFFIXES "python${_python_version}/config"
	)

	_GET_PYTHON_CONFIG_VARIABLE("INCLUDEPY")
	set(PYTHON_INCLUDE_DIR "${_python_config_variable}")
ELSE (PYTHON_EXECUTABLE)
	MESSAGE(STATUS "No python executable found")
ENDIF (PYTHON_EXECUTABLE)


# AB: readme.txt says we should use PYTHON_INCLUDE_DIR. _PATH exists for
#     historic reasons, too, so we should provide it as well
SET(PYTHON_INCLUDE_PATH ${PYTHON_INCLUDE_DIR})

IF (NOT PYTHON_LIBRARY)
	SET(PYTHON_LIBS_FOUND FALSE)
	IF(BosonPythonLibs_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Python libraries could not be found")
	ELSE(BosonPythonLibs_FIND_REQUIRED)
		IF(NOT BosonPythonLibs_FIND_QUIETLY)
			MESSAGE(STATUS "Python libraries could not be found")
		ENDIF(NOT BosonPythonLibs_FIND_QUIETLY)
	ENDIF(BosonPythonLibs_FIND_REQUIRED)
ELSE (NOT PYTHON_LIBRARY)
	SET(PYTHON_LIBS_FOUND TRUE)

	# PYTHON_LIBRARY is used for the cache entry only
	# PYTHON_LIBRARIES is meant to be public
	set(PYTHON_LIBRARIES "${PYTHON_LIBRARY} ${_python_dependency_libs}")
ENDIF (NOT PYTHON_LIBRARY)

