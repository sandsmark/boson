set(BOSON_MUSIC_INSTALL_DIRECTORY ${KDE3_DATA_INSTALL_DIR}/boson/music)

# the <path_to_data>/music directory. this will be used in CMakeLists.txt files
# of the subdirectories
set(BOSON_MUSIC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory( Jungle )
add_subdirectory( Progressive )

