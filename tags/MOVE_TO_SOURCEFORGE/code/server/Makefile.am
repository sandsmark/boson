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
boson_appsdir = $(kde_appsdir)/Games/Boson

#LDFLAGS =  -s 


bin_PROGRAMS = boserver

#boserver
METASOURCES = AUTO
boserver_LDFLAGS = $(all_libraries) $(KDE_RPATH)
boserver_SOURCES = boserver.cpp main.cpp connect.cpp serverUnit.cpp serverMap.cpp player.cpp game.cpp serverCell.cpp
boserver_LDADD   = ../common/libboson.a $(LIB_KDEUI)

# garbage
CLEANFILES = boson-*.log

EXTRA_DIST = main.cpp boserver.cpp boserver.h connect.h connect.cpp serverUnit.cpp serverUnit.h serverMap.cpp player.cpp serverCell.h game.h game.cpp

appsdir = $(kde_appsdir)/Games/Boson

apps_DATA = boserver.desktop
EXTRA_DIST = $(apps_DATA)

KDE_ICON = boserver

