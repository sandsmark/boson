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


bin_PROGRAMS = boeditor 
#SUBDIRS = docs 


#boeditor 
METASOURCES = AUTO
boeditor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boeditor_SOURCES = boeditor.cpp main.cpp mainWidget.cpp editorTopLevel.cpp miniDisplay.cpp editorCanvas.cpp editorBigDisplay.cpp newDlg.cpp
boeditor_LDADD   =  ../visual/libbovisual.a ../common/libboson.a $(LIB_KFILE)
#$(LIB_KDEUI)

#garbage
CLEANFILES = boson-*.log

EXTRA_DIST = boeditor.cpp boeditor.h main.cpp mainWidget.cpp editorTopLevel.cpp miniDisplay.cpp editorCanvas.cpp editorCanvas.h ressource.h mainWidget.h editorTopLevel.h editorBigDisplay.cpp editorBigDisplay.h newDlg.h


appsdir = $(kde_appsdir)/Games/Boson
rcdir = $(kde_datadir)/boeditor

apps_DATA = boeditor.desktop
EXTRA_DIST = $(apps_DATA)

KDE_ICON = boeditor

rc_DATA = boeditorui.rc


