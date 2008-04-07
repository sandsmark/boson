set(textures_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/textures)

FILE(GLOB png_files "*.png")
FILE(GLOB jpg_files "*.jpg")
install(
	FILES
	${png_files}
	${jpg_files}
	DESTINATION ${textures_install_dir})

