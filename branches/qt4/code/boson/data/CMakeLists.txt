
install_files(${XDG_APPS_INSTALL_DIR} .*\\.desktop)
install_files(${DATA_INSTALL_DIR}/boson .*ui\\.rc)
install_files(${CONFIG_INSTALL_DIR} FILES
	bodebugrc
	bodebug.areas
)

kde4_install_icons(hicolor)
#kde3_install_icons(locolor)

