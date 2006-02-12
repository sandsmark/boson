project(bosound)

find_package(OpenAL)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bosonsoundcommon_sources
	boaudiocommand.cpp
)

set(bosonsound_sources
	boaudiothread.cpp
	bosonaudio.cpp
)
kde3_automoc(${bosonsoundcommon_sources})
kde3_automoc(${bosonsound_sources})


add_library(bosonsoundcommon STATIC ${bosonsoundcommon_sources})
add_library(bosonsound STATIC ${bosonsound_sources})

# TODO: only if LIB_OPENAL is defined
if (OPENAL_FOUND)
	add_subdirectory(openal)
	target_link_libraries(bosonsound bosonsoundopenal)
	target_link_libraries(bosonsound ${OPENAL_LIBRARY})
	set(BOSON_USE_OPENAL 1)
endif (OPENAL_FOUND)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-bosound.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-bosound.h )

