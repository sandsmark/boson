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
CXXFLAGS = -pipe -Wall -g # -O2
LDFLAGS =  -s 

# libbovisual.a
noinst_LIBRARIES = libbovisual.a
libbovisual_a_SOURCES = fieldDisplay.cpp fieldMap.cpp groundTheme.cpp miniDisplay.cpp miniEvent.cpp miniMap.cpp physMap.cpp selectPart.cpp speciesTheme.cpp viewMap.cpp visualCell.cpp visualUnit.cpp viewSelect.cpp
libbovisual_a_METASOURCES = USE_AUTOMOC

#garbage
CLEANFILES = $(boson_METASOURCES) boson-*.log

EXTRA_DIST = fieldDisplay.cpp fieldMap.cpp groundTheme.cpp miniDisplay.cpp miniEvent.cpp miniMap.cpp physMap.cpp selectPart.cpp speciesTheme.cpp viewMap.cpp visualCell.cpp visualUnit.cpp fieldMap.h groundTheme.h miniMap.h physMap.h selectPart.h speciesTheme.h sprites.h viewMap.h visual.h visualCell.h visualUnit.h

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boson_SOURCES) && mv messages.po ../po/boson.pot


