set(BOSON_MAPS_INSTALL_DIR ${KDE3_DATA_INSTALL_DIR}/boson/maps)

# the <path_to_data>/maps directory. this will be used in CMakeLists.txt files
# of the subdirectories
set(BOSON_MAPS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install_files(${BOSON_MAPS_INSTALL_DIR} .*\\.bpf)

add_subdirectory(testing)
add_subdirectory(1-Better_times_on_canesia)

