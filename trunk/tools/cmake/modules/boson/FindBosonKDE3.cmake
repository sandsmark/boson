# AB: this file adds some extra features to the default FindKDE3.cmake file of
#     official cmake.


# if KDEDIR is not set, try using kde-config to set it.
# this is particular helpful on gentoo, as cmake seems to never find kde there,
# unless KDEDIR is set.
if (NOT EXISTS "$ENV{KDEDIR}")
	message(STATUS "KDEDIR not set or does not exist. trying kde-config.")
	execute_process(COMMAND kde-config --version
		OUTPUT_VARIABLE kde_config_version
	)
	string(REGEX MATCH "KDE: .\\." kde_version ${kde_config_version})
	if (${kde_version} MATCHES "KDE: 3\\.")
		execute_process(COMMAND kde-config --prefix
			OUTPUT_VARIABLE kdedir
		)
		string(REGEX REPLACE "\n" "" kdedir ${kdedir})
		set(ENV{KDEDIR} ${kdedir})
		message(STATUS "set KDEDIR to: $ENV{KDEDIR}")
	else (${kde_version} MATCHES "KDE: 3\\.")
		message(STATUS "kde-config from KDE 3.x not found. Not using it to find KDEDIR.")
	endif (${kde_version} MATCHES "KDE: 3\\.")
endif (NOT EXISTS "$ENV{KDEDIR}")

find_package(KDE3)

