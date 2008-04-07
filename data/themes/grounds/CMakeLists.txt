set(BOSON_GROUNDS_INSTALL_DIR ${KDE4_DATA_INSTALL_DIR}/boson/themes/grounds)

# the <path_to_data>/maps directory. this will be used in CMakeLists.txt files
# of the subdirectories
set(BOSON_GROUNDS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(earth)

