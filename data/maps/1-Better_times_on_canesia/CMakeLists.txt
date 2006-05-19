file(RELATIVE_PATH current_subdir ${BOSON_MAPS_DIRECTORY} ${CMAKE_CURRENT_SOURCE_DIR})
set(maps_install_dir ${BOSON_MAPS_INSTALL_DIR}/${current_subdir})

install_files(${maps_install_dir} .*\\.bpf)

