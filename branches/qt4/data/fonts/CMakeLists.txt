set(fonts_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/fonts)

FILE(GLOB font_files "*.txf")
install(FILES ${font_files} DESTINATION ${fonts_install_dir})

