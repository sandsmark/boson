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
boson_appsdir = $(kde_appsdir)/Games/Boson

LDFLAGS =  -s 


bin_PROGRAMS = boserver

#boserver
boserver_METASOURCES = USE_AUTOMOC
boserver_LDFLAGS = $(all_libraries) $(KDE_RPATH)

boserver_SOURCES = boserver.cpp main.cpp connect.cpp serverUnit.cpp serverMap.cpp player.cpp serverCell.cpp game.cpp
boserver_LDADD   = ../common/libboson.a  -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

#(un)install
install-data-local:
	$(mkinstalldirs) $(boson_appsdir)/
	$(INSTALL_DATA) boserver.kdelnk $(boson_appsdir)/boserver.kdelnk
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) boserver.xpm $(kde_icondir)/boserver.xpm
	$(mkinstalldirs) $(kde_minidir)/
	$(INSTALL_DATA) mini-boserver.xpm $(kde_minidir)/mini-boserver.xpm

uninstall-local:
	-rm -f $(boson_appsdir)/boserver.kdelnk
	-rm -f $(kde_icondir)/boserver.xpm
	-rm -f $(kde_minidir)/mini-boserver.xpm


# garbage
CLEANFILES = $(boserver_METASOURCES) boson-*.log

EXTRA_DIST = main.cpp boserver.cpp boserver.h connect.h connect.cpp serverUnit.cpp serverUnit.h serverMap.cpp player.cpp serverCell.h serverCell.cpp game.h game.cpp

