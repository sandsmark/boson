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

# compilation
CXXFLAGS = -pipe -Wall -g # -O2
#MY_HOST :=  $(shell hostname | tr '[a-z]' '[A-Z]')_$(shell domainname | tr '[a-z]' '[A-Z]')
#CXXFLAGS += -D$(MY_HOST)
LDFLAGS =  -s 

noinst_PROGRAMS = boeditor
#boeditor
#boeditor_METASOURCES = USE_AUTOMOC
boeditor_SOURCES = boeditor.cpp
boeditor_LDADD   = ../common/libboson.a -lkdecore -lqt # -lXext -lX11 $(LIBSOCKET)
boeditor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#basic.bpf
data_DATA = basic.bpf
datadir = $(kde_datadir)/boson/map
basic.bpf : boeditor
	./boeditor


#garbage
EXTRA_DIST =  map.h boeditor.cpp boeditor.h
CLEANFILES = basic.bpf

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
#messages:
#	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(libboson_la_SOURCES) && mv messages.po ../po/bocommon.pot
