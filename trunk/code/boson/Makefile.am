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

LDFLAGS =  -s 


bin_PROGRAMS = boson 
SUBDIRS = docs 


#boson
boson_METASOURCES = USE_AUTOMOC
boson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boson_SOURCES = boson.cpp connect.cpp bosonBigDisplay.cpp game.cpp main.cpp mainWidget.cpp miniDisplay.cpp bosonCanvas.cpp playerUnit.cpp connectDlg.cpp serverDlg.cpp boshot.cpp bosonTopLevel.cpp
boson_LDADD   =  ../visual/libbovisual.a ../common/libboson.a $(LIB_KDEUI)


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

EXTRA_DIST = boson.cpp connect.cpp bosonBigDisplay.cpp game.cpp main.cpp mainWidget.cpp miniDisplay.cpp bosonCanvas.cpp playerUnit.cpp boson.h connect.h game.h mainWidget.h bosonCanvas.h playerUnit.h ressource.h connectDlg.cpp connectDlg.h serverDlg.cpp serverDlg.h boshot.cpp boshot.h bosonTopLevel.h bosonTopLevel.cpp

