set(scripts_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/scripts)

FILE(GLOB py_files "*.py")
install(FILES ${py_files} DESTINATION ${scripts_install_dir})

