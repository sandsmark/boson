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

LDFLAGS =  -s 


bin_PROGRAMS = boson 
SUBDIRS = docs 

METASOURCES = AUTO
boson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boson_SOURCES = boson.cpp connect.cpp bosonBigDisplay.cpp game.cpp main.cpp mainWidget.cpp miniDisplay.cpp bosonCanvas.cpp playerUnit.cpp connectDlg.cpp serverDlg.cpp boshot.cpp bosonTopLevel.cpp
boson_LDADD   =  ../visual/libbovisual.a ../common/libboson.a $(LIB_KDEUI)


appsdir = $(kde_appsdir)/Games/Boson
rcdir = $(kde_datadir)/boson

apps_DATA = boson.desktop .directory
EXTRA_DIST = $(apps_DATA)

KDE_ICON = boson

#rc_DATA = kwrite_browser.rc kwriteui.rc kwrite_shell.rc

#garbage
CLEANFILES = boson-*.log

EXTRA_DIST = boson.cpp connect.cpp bosonBigDisplay.cpp game.cpp main.cpp mainWidget.cpp miniDisplay.cpp bosonCanvas.cpp playerUnit.cpp boson.h connect.h game.h mainWidget.h bosonCanvas.h playerUnit.h ressource.h connectDlg.cpp connectDlg.h serverDlg.cpp serverDlg.h boshot.cpp boshot.h bosonTopLevel.h bosonTopLevel.cpp


