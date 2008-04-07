set(effects_install_dir ${KDE4_DATA_INSTALL_DIR}/boson/themes/effects)

install(
	FILES effects.boson
	DESTINATION ${effects_install_dir}
)

add_subdirectory(particles)

