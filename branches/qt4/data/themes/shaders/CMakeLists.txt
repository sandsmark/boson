set(shaders_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/shaders)

FILE(GLOB shader_files "*.shader")
install(FILES ${shader_files} DESTINATION ${shaders_install_dir})

