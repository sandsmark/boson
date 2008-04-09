
FILE(GLOB desktop_files "*.desktop")
FILE(GLOB ui_files "*ui.rc")
install(FILES ${desktop_files} DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ${ui_files} DESTINATION ${DATA_INSTALL_DIR}/boson)
install(FILES bodebugrc bodebug.areas DESTINATION ${CONFIG_INSTALL_DIR})

kde4_install_icons(hicolor)
#kde4_install_icons(locolor)

