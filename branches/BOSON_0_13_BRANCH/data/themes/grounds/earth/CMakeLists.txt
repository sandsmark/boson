file(RELATIVE_PATH current_subdir ${BOSON_GROUNDS_DIRECTORY} ${CMAKE_CURRENT_SOURCE_DIR})
set(ground_install_dir ${BOSON_GROUNDS_INSTALL_DIR}/${current_subdir})

install_files(${ground_install_dir} FILES index.ground)
install_files(${ground_install_dir} .*\\.jpg)
install_files(${ground_install_dir} .*\\.png)

add_subdirectory(water)

