set(BOSON_MAPS_INSTALL_DIR ${KDE4_DATA_INSTALL_DIR}/boson/maps)

# the <path_to_data>/maps directory. this will be used in CMakeLists.txt files
# of the subdirectories
set(BOSON_MAPS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB map_files "*.bpf")
install(FILES ${map_files} DESTINATION ${BOSON_MAPS_INSTALL_DIR})

add_subdirectory(testing)
add_subdirectory(1-Better_times_on_canesia)

