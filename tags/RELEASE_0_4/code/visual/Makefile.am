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
libbovisual_a_SOURCES = visualBigDisplay.cpp groundTheme.cpp visualMiniDisplay.cpp visualField.cpp selectPart.cpp speciesTheme.cpp visualView.cpp visualCell.cpp visualUnit.cpp 
libbovisual_a_METASOURCES = USE_AUTOMOC

#garbage
CLEANFILES = $(boson_METASOURCES) boson-*.log

EXTRA_DIST = visualBigDisplay.cpp groundTheme.cpp visualMiniDisplay.cpp visualField.cpp selectPart.cpp speciesTheme.cpp visualView.cpp visualCell.cpp visualUnit.cpp visualBigDisplay.h groundTheme.h visualMiniDisplay.h visualField.h selectPart.h speciesTheme.h sprites.h visualView.h visual.h visualCell.h visualUnit.h

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boson_SOURCES) && mv messages.po ../po/boson.pot


