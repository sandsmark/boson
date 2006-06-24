#find_package(PythonLibs REQUIRED)
find_package(BosonPythonLibs REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${QT_INCLUDE_DIR}
	${PYTHON_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(script_SRCS
	bosonscript.cpp
	pythonscript.cpp
	bosonscriptinterface.cpp
)

kde3_automoc(${script_SRCS})

add_library(bosonscript STATIC ${script_SRCS})
boson_target_link_libraries(bosonscript ${PYTHON_LIBRARIES})

