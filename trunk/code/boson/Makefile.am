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


bin_PROGRAMS = boson 
SUBDIRS = docs 


#boson
boson_METASOURCES = USE_AUTOMOC
boson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boson_SOURCES = boson.cpp connect.cpp fieldEvent.cpp game.cpp main.cpp mainView.cpp miniDisplay.cpp infoWin.cpp playerMap.cpp playerUnit.cpp bosonViewMap.cpp
boson_LDADD   =  ../visual/libbovisual.a ../common/libboson.a -lkfile -lkfm -lkdeui -lkdecore -lQwSpriteField -lqt -lXext -lX11 $(LIBSOCKET)


#(un)install stuff
install-data-local:
	$(mkinstalldirs) $(boson_appsdir)/
	$(INSTALL_DATA) .directory $(boson_appsdir)/.directory
	$(INSTALL_DATA) boson.kdelnk $(boson_appsdir)/boson.kdelnk
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) boson.xpm $(kde_icondir)/boson.xpm
	$(mkinstalldirs) $(kde_minidir)/
	$(INSTALL_DATA) mini-boson.xpm $(kde_minidir)/mini-boson.xpm

uninstall-local:
	-rm -f $(boson_appsdir)/boson.kdelnk
	-rm -f $(kde_icondir)/boson.xpm
	-rm -f $(kde_minidir)/mini-boson.xpm


#garbage
CLEANFILES = $(boson_METASOURCES) boson-*.log

EXTRA_DIST = boson.cpp connect.cpp fieldEvent.cpp game.cpp main.cpp mainView.cpp miniDisplay.cpp infoWin.cpp playerMap.cpp playerUnit.cpp boson.h connect.h game.h mainView.h orderType.h infoWin.h playerMap.h playerUnit.h ressource.h bosonViewMap.cpp bosonViewMap.h


# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boson_SOURCES) && mv messages.po ../po/boson.pot


