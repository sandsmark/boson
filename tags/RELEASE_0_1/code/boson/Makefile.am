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
INCLUDES= $(all_includes)

# compilation
CXXFLAGS = -pipe -Wall -g # -O2
#	MY_HOST :=  $(shell hostname | tr '[a-z]' '[A-Z]')_$(shell domainname | tr '[a-z]' '[A-Z]')
#	CXXFLAGS += -D$(MY_HOST)
LDFLAGS =  -s 


bin_PROGRAMS = boson 
SUBDIRS = docs 


#boson
boson_METASOURCES = USE_AUTOMOC
boson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boson_SOURCES = mainView.cpp boson.cpp main.cpp connect.cpp groundTheme.cpp speciesTheme.cpp playerUnit.cpp playerCell.cpp physMap.cpp viewMap.cpp fieldMap.cpp miniMap.cpp fieldDisplay.cpp miniDisplay.cpp fieldEvent.cpp miniEvent.cpp orderWin.cpp game.cpp selectPart.cpp
boson_LDADD   =  ../common/libboson.a -lkfile -lkfm -lkdeui -lkdecore -lQwSpriteField -lqt -lXext -lX11 $(LIBSOCKET)


#(un)install stuff
install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/
	$(INSTALL_DATA) boson.kdelnk $(kde_appsdir)/Games/boson.kdelnk
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) boson.xpm $(kde_icondir)/boson.xpm
	$(mkinstalldirs) $(kde_minidir)/
	$(INSTALL_DATA) mini-boson.xpm $(kde_minidir)/mini-boson.xpm

uninstall-local:
	-rm -f $(kde_appsdir)/boson.kdelnk
	-rm -f $(kde_icondir)/boson.xpm
	-rm -f $(kde_minidir)/mini-boson.xpm


#garbage
CLEANFILES = $(boson_METASOURCES) boson-*.log

EXTRA_DIST = main.cpp boson.cpp boson.h ressource.h boson.kdelnk boson.xpm mini-boson.xpm connect.h connect.cpp groundTheme.cpp groundTheme.h speciesTheme.cpp speciesTheme.h playerUnit.cpp playerUnit.h playerCell.cpp playerCell.h physMap.cpp physMap.h viewMap.cpp viewMap.h fieldMap.cpp fieldMap.h miniMap.cpp miniMap.h fieldDisplay.cpp miniDisplay.cpp fieldEvent.cpp miniEvent.cpp orderWin.cpp orderWin.h game.cpp game.h mainView.cpp mainView.h selectPart.h selectPart.cpp



# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boson_SOURCES) && mv messages.po ../po/boson.pot

