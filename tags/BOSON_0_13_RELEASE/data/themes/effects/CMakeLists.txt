set(effects_install_dir ${KDE3_DATA_INSTALL_DIR}/boson/themes/effects)

install_files(${effects_install_dir} FILES
	effects.boson
)

add_subdirectory(particles)

