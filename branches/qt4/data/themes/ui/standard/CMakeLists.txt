set(ui_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/ui/standard)

FILE(GLOB png_files "*.png")
install(FILES ${png_files} DESTINATION ${ui_install_dir})

