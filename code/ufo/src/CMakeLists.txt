project(ufo)


include_directories(
  ${CMAKE_SOURCE_DIR}/bogl
  ${CMAKE_SOURCE_DIR}/ufo/include
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)


set(events_SRCS
	events/uactionevent.cpp
	events/uevent.cpp
	events/ufocusevent.cpp
	events/uinputevent.cpp
	events/ukeyevent.cpp
	events/umouseevent.cpp
	events/umousewheelevent.cpp
	events/upropertychangeevent.cpp
	events/utimerevent.cpp
	events/uwidgetevent.cpp
)

set(font_SRCS
	font/ufont.cpp
	font/utexturefont.cpp
)

set(gl_SRCS
	gl/ugl_builtinfontrenderer.cpp
	gl/ugl_driver.cpp
	gl/ugl_font_data.cpp
	gl/ugl_graphics.cpp
	gl/ugl_image.cpp
	gl/ugl_texturefontrenderer.cpp
)

set(image_SRCS
	image/pnm.cpp
	image/pnm.h
	image/tga.cpp
	image/tga.h
	image/uimage.cpp
	image/uimagefilter.cpp
	image/uimageicon.cpp
	image/uimageio.cpp
	image/uxbmicon.cpp
)

set(layouts_SRCS
	layouts/uborderlayout.cpp
	layouts/uboxlayout.cpp
	layouts/uflowlayout.cpp
)

set(text_SRCS
	text/ubasicdocument.cpp
	text/udocumentfactory.cpp
	text/utextlayout.cpp
)

set(ui_SRCS
	ui/ubasicstyle.cpp
	ui/ucss.cpp
	ui/ustylehints.cpp
	ui/ustylemanager.cpp
)

set(util_SRCS
	util/ucolor.cpp
	util/ufilearchive.cpp
	util/uinteger.cpp
	util/upalette.cpp
	util/uproperties.cpp
	util/ustring.cpp
)

set(ux_SRCS
	ux/uxcontext.cpp
	ux/uxdisplay.cpp
	ux/uxframe.cpp
	ux/uxglxdriver.cpp
	ux/uxsdldriver.cpp
	ux/uxtoolkit.cpp
	ux/uxwgldriver.cpp
)

set(widgets_SRCS
	widgets/uabstractslider.cpp
	widgets/ubutton.cpp
	widgets/ucheckbox.cpp
	widgets/ucheckboxmenuitem.cpp
	widgets/ucombobox.cpp
	widgets/ucompound.cpp
	widgets/udesktoppane.cpp
	widgets/udockwidget.cpp
	widgets/ugroupbox.cpp
	widgets/uinternalframe.cpp
	widgets/uitem.cpp
	widgets/ulabel.cpp
	widgets/ulayeredpane.cpp
	widgets/ulineedit.cpp
	widgets/ulistbox.cpp
	widgets/umenu.cpp
	widgets/umenubar.cpp
	widgets/umenuitem.cpp
	widgets/upopupmenu.cpp
	widgets/uprogressbar.cpp
	widgets/uradiobutton.cpp
	widgets/urootpane.cpp
	widgets/uscrollablewidget.cpp
	widgets/uscrollbar.cpp
	widgets/uscrollpane.cpp
	widgets/useparator.cpp
	widgets/uslider.cpp
	widgets/uspinbox.cpp
	widgets/utextedit.cpp
	widgets/utextwidget.cpp
	widgets/uviewport.cpp
	widgets/uwidget.cpp
)

set(xml_SRCS
	xml/tinyxml.cpp
	xml/tinyxmlerror.cpp
	xml/tinyxmlparser.cpp
	xml/uxul.cpp
)

set(ufo_SRCS
	uabstractcontext.cpp
	uabstractdisplay.cpp
	uabstracttoolkit.cpp
	ubuttongroup.cpp
	ucollectable.cpp
	ucontextgroup.cpp
	ufo_debug.cpp
	ufocusmanager.cpp
	ukeystroke.cpp
	umenumanager.cpp
	uobject.cpp
	uplugin.cpp
	upopupmanager.cpp
	urepaintmanager.cpp
	usharedlib.cpp
	uvalidator.cpp
	uvertexarray.cpp
	uvolatiledata.cpp
	${events_SRCS}
	${font_SRCS}
	${gl_SRCS}
	${image_SRCS}
	${layouts_SRCS}
	${text_SRCS}
	${ui_SRCS}
	${util_SRCS}
	${ux_SRCS}
	${widgets_SRCS}
	${xml_SRCS}
)

boson_add_library(ufo STATIC ${ufo_SRCS})


