set(particles_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/effects/particles)

FILE(GLOB png_files "*.png")
install(FILES ${png_files} DESTINATION ${particles_install_dir})

