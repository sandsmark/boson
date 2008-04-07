set(pics_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/pics)

FILE(GLOB pics "*.png")
install(FILES ${pics} DESTINATION ${pics_install_dir})

