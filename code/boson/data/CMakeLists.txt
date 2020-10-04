
install_files(${KDE3_XDG_APPS_DIR} .*\\.desktop)
install_files(${KDE3_DATA_INSTALL_DIR}/boson .*ui\\.rc)
install_files(${KDE3_CONFIG_INSTALL_DIR} FILES
	bodebugrc
	bodebug.areas
)

#kde3_install_icons(hicolor)
#kde3_install_icons(locolor)

