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

#LDFLAGS =  -s 

noinst_PROGRAMS = createGround

METASOURCES = AUTO

createGround_SOURCES = createGround.cpp
createGround_LDADD   = ../common/libboson.a -lqt
createGround_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#basic.bpf
data_DATA = earth.png
datadir = $(kde_datadir)/boson/themes/grounds

earth.png : createGround
	./createGround

#garbage
EXTRA_DIST =  createGround.cpp
CLEANFILES = basic.bpf earth.png

