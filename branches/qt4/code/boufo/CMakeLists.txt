project(boufo)

add_subdirectory(bouic)
#add_subdirectory(designer)
#add_subdirectory(test)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${CMAKE_SOURCE_DIR}/bogl
  ${CMAKE_SOURCE_DIR}/ufo/include
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


set(ufoext_SRCS
	ufoext/boufofontrenderer.cpp
	ufoext/ubosonstyle.cpp
	ufoext/ubolabel.cpp
	ufoext/ubolabelui.cpp
	ufoext/ubogridlayout.cpp
	ufoext/ubodrawableicon.cpp
	ufoext/uboboxlayout.cpp
)

set(bosonfont_SRCS
	bosonfont/fnt.cpp
	bosonfont/fntTXF.cpp
	bosonfont/bosonglfont.cpp
)

set(boufo_SRCS
	boufofontinfo.cpp
	boufodrawable.cpp
	boufoimage.cpp
	boufowidget.cpp
	boufolabel.cpp
	boufolineedit.cpp
	boufotextedit.cpp
	boufopushbutton.cpp
	boufolistbox.cpp
	boufocombobox.cpp
	boufocheckbox.cpp
	bouforadiobutton.cpp
	boufomatrix.cpp
	boufoslider.cpp
	boufoprogress.cpp
	boufonuminput.cpp
	boufowidgetstack.cpp
	boufotabwidget.cpp
	boufolayeredpane.cpp
	boufogroupbox.cpp
	boufocustomwidget.cpp
	boufointernalframe.cpp
	boufoinputdialog.cpp
	boufomanager.cpp
	boufofactory.cpp
	boufoaction.cpp
	#boufodebugwidget.cpp
	boufostandalonefont.cpp
	boufoprofiling.cpp
	boufozoomscrollviewporthandler.cpp
	${ufoext_SRCS}
	${bosonfont_SRCS}
)

boson_add_library(boufo STATIC ${boufo_SRCS})

target_link_libraries(boufo ufo)


