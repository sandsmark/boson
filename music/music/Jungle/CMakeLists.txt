file(RELATIVE_PATH current_subdir ${BOSON_MUSIC_DIRECTORY} ${CMAKE_CURRENT_SOURCE_DIR})
set(music_install_dir ${BOSON_MUSIC_INSTALL_DIRECTORY}/${current_subdir})

message(STATUS ${BOSON_MUSIC_INSTALL_DIRECTORY})
message(STATUS ${BOSON_MUSIC_DIRECTORY})
message(STATUS ${current_subdir})
message(STATUS ${music_install_dir})


install_files(${music_install_dir} .*\\.mp3)
install_files(${music_install_dir} .*\\.ogg)
install_files(${music_install_dir} index.music)

