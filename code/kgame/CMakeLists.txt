project(libkdegames)

include_directories(
	${CMAKE_SOURCE_DIR}/bodebug
	${CMAKE_SOURCE_DIR}/bogl
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(kmessage_sources
	kmessageio.cpp
	kmessageserver.cpp
	kmessageclient.cpp
)

set(dialogs_sources
	kgamedebugdialog.cpp
)

set(kgame_sources
	kgame.cpp
	kplayer.cpp
	kgamenetwork.cpp
	kgameproperty.cpp
	kgamemessage.cpp
	kgameio.cpp
	kgameprocess.cpp
	kgamechat.cpp
	kgamepropertyhandler.cpp
	kgameerror.cpp
	kgamesequence.cpp
	${kmessage_sources}
	${dialogs_sources}
)

set(libkdegames_sources
	kchatbase.cpp
	kchatbasemodel.cpp
	kchatbaseitemdelegate.cpp
	kgameprogress.cpp
	kstandardgameaction.cpp
	${kgame_sources}
)

boson_add_library(kgame STATIC ${libkdegames_sources})

