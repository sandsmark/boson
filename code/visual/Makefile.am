############################################################################
#                         Makefile.am  -  description                    
#                            -------------------                                         
#
#   version              : $Id$
#   begin                : Wed Sep  8 00:21:24 CET 1999
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
#LDFLAGS =  -s 

# libbovisual.a
noinst_LIBRARIES = libbovisual.a
libbovisual_a_SOURCES = visualBigDisplay.cpp visualMiniDisplay.cpp visualCanvas.cpp selectPart.cpp speciesTheme.cpp visualTopLevel.cpp visualUnit.cpp 
METASOURCES = AUTO

#garbage
CLEANFILES = boson-*.log

EXTRA_DIST = visualBigDisplay.cpp visualMiniDisplay.cpp visualCanvas.cpp selectPart.cpp speciesTheme.cpp visualTopLevel.cpp visualUnit.cpp visualBigDisplay.h visualMiniDisplay.h visualCanvas.h selectPart.h speciesTheme.h sprites.h visualTopLevel.h visual.h visualUnit.h

