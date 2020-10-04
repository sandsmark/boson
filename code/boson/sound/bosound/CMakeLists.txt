project(bosound)

find_package(OpenAL)

# AB: FindOpenAL.cmake also defines OPENAL_FOUND if the headers aren't found.
if (OPENAL_FOUND AND NOT OPENAL_INCLUDE_DIR)
	set(OPENAL_FOUND FALSE)
endif (OPENAL_FOUND AND NOT OPENAL_INCLUDE_DIR)

# give a proper message about a missing OpenAL (FindOpenAL.cmake doesn't do so)
if (NOT OPENAL_FOUND)
	if (NOT OPENAL_INCLUDE_DIR)
		message(STATUS "OpenAL headers have not been found")
	endif (NOT OPENAL_INCLUDE_DIR)
	if (NOT OPENAL_LIBRARY)
		message(STATUS "OpenAL library has not been found")
	endif (NOT OPENAL_LIBRARY)
	message(STATUS "Did not find a usable OpenAL installation. Sound will be disabled in Boson.")
endif (NOT OPENAL_FOUND)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bosonsoundcommon_sources
	boaudiocommand.cpp
)

set(bosonsound_sources
	boaudiothread.cpp
	bosonaudio.cpp
)


boson_add_library(bosonsoundcommon STATIC ${bosonsoundcommon_sources})
boson_add_library(bosonsound STATIC ${bosonsound_sources})

if (OPENAL_FOUND)
	add_subdirectory(openal)
	boson_target_link_libraries(bosonsound bosonsoundopenal)
	boson_target_link_libraries(bosonsound ${OPENAL_LIBRARY})
	set(BOSON_USE_OPENAL 1)
endif (OPENAL_FOUND)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-bosound.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-bosound.h )

