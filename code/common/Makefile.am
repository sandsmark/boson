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

LDFLAGS =  -s 

# libboson.a
noinst_LIBRARIES = libboson.a
libboson_a_SOURCES = boconnect.cpp log.cpp unit.cpp cell.cpp bobuffer.cpp unitType.cpp groundType.cpp boFile.cpp
libboson_a_METASOURCES = USE_AUTOMOC

# garbage
EXTRA_DIST = boconnect.cpp msgData.h log.h log.cpp unit.cpp unit.h cell.cpp cell.h ground.h bobuffer.cpp unitType.h unitType.cpp groundType.h groundType.cpp boFile.h boFile.cpp

noinst_HEADERS = bobuffer.h boconfig.h cell.h groundType.h log.h msgData.h boFile.h unit.h unitType.h


CLEANFILES = $(libboson_a_METASOURCES)

