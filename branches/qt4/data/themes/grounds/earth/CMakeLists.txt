file(RELATIVE_PATH current_subdir ${BOSON_GROUNDS_DIRECTORY} ${CMAKE_CURRENT_SOURCE_DIR})
set(ground_install_dir ${BOSON_GROUNDS_INSTALL_DIR}/${current_subdir})

FILE(GLOB jpg_files "*.jpg")
FILE(GLOB png_files "*.png")
install(FILES
	index.ground
	${jpg_files}
	${png_files}
	DESTINATION ${ground_install_dir}
)

add_subdirectory(water)

