############################################################################
#                         Makefile.am  -  description                    
#                            -------------------                                         
#
#   version              : $Id$
#   begin                : before CVS was used :-)
#                                          
#   copyright            : (C) 1999 by Thomas Capricelli                         
#   email                : capricel@enst.fr                                     
############################################################################
#
############################################################################
##                                                                         #
##   This program is free software; you can redistribute it and/or modify  #
##   it under the terms of the GNU General Public License as published by  #
##   the Free Software Foundation; either version 2 of the License, or     #
##   (at your option) any later version.                                   # 
##                                                                         #
############################################################################

# include files
INCLUDES= -I../visual $(all_includes)
boson_appsdir = $(kde_appsdir)/Games/Boson

# compilation
CXXFLAGS = -pipe -Wall -g # -O2
#	MY_HOST :=  $(shell hostname | tr '[a-z]' '[A-Z]')_$(shell domainname | tr '[a-z]' '[A-Z]')
#	CXXFLAGS += -D$(MY_HOST)
LDFLAGS =  -s 


bin_PROGRAMS = boeditor 
#SUBDIRS = docs 


#boeditor 
boeditor_METASOURCES = USE_AUTOMOC
boeditor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boeditor_SOURCES = boeditor.cpp editorView.cpp main.cpp mainView.cpp miniDisplay.cpp editorField.cpp editorBigDisplay.cpp
boeditor_LDADD   =  ../visual/libbovisual.a ../common/libboson.a -lkfile -lkfm -lkdeui -lkdecore -lQwSpriteField -lqt -lXext -lX11 #$(LIBSOCKET)


#(un)install stuff
install-data-local:
	$(mkinstalldirs) $(boson_appsdir)/
	$(INSTALL_DATA) boeditor.kdelnk $(boson_appsdir)/boeditor.kdelnk
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) boeditor.xpm $(kde_icondir)/boeditor.xpm
	$(mkinstalldirs) $(kde_minidir)/
	$(INSTALL_DATA) mini-boeditor.xpm $(kde_minidir)/mini-boeditor.xpm

uninstall-local:
	-rm -f $(boson_appsdir)/boeditor.kdelnk
	-rm -f $(kde_icondir)/boeditor.xpm
	-rm -f $(kde_minidir)/mini-boeditor.xpm


#garbage
CLEANFILES = $(boeditor_METASOURCES) boson-*.log

EXTRA_DIST = boeditor.cpp editorView.cpp main.cpp mainView.cpp miniDisplay.cpp editorField.cpp editorField.h ressource.h boeditor.h editorView.h mainView.h editorBigDisplay.cpp editorBigDisplay.h

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boeditor_SOURCES) && mv messages.po ../po/boeditor.pot


