SUBDIRS = ufoext bouic . designer test

INCLUDES = $(UFO_INCLUDES) -I$(top_srcdir)/bodebug -I$(top_srcdir)/bogl $(all_includes) -I$(top_srcdir)/boson

noinst_LTLIBRARIES = libboufo.la

METASOURCES = AUTO

KDE_CXXFLAGS = $(UFO_CFLAGS)

libboufo_la_SOURCES = \
	boufofontinfo.cpp \
	boufodrawable.cpp \
	boufoimage.cpp \
	boufowidget.cpp \
	boufolabel.cpp \
	boufolineedit.cpp \
	boufotextedit.cpp \
	boufopushbutton.cpp \
	boufolistbox.cpp \
	boufocombobox.cpp \
	boufocheckbox.cpp \
	bouforadiobutton.cpp \
	boufomatrix.cpp \
	boufoslider.cpp \
	boufoprogress.cpp \
	boufonuminput.cpp \
	boufowidgetstack.cpp \
	boufotabwidget.cpp \
	boufolayeredpane.cpp \
	boufowidgetstack.cpp \
	boufogroupbox.cpp \
	boufocustomwidget.cpp \
	boufointernalframe.cpp \
	boufoinputdialog.cpp \
	boufomanager.cpp \
	boufofactory.cpp \
	boufoaction.cpp \
	boufodebugwidget.cpp \
	boufostandalonefont.cpp

libboufo_la_LIBADD = \
		$(top_builddir)/boson/boufo/ufoext/libufoext.la \
		$(top_builddir)/boson/bosonfont/libbosonfont.la \
		$(LIB_UFO)

