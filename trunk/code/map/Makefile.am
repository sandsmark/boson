####### kdevelop will overwrite this part!!! (begin)##########
CXXFLAGS = -pipe -Wall -g # -O2
#MY_HOST :=  $(shell hostname | tr '[a-z]' '[A-Z]')_$(shell domainname | tr '[a-z]' '[A-Z]')
#CXXFLAGS += -D$(MY_HOST)
LDFLAGS =  -s 
#libbomap_la_SOURCES =  playField.cpp

boeditor_SOURCES = boeditor.cpp
#boeditor_LDADD   = ./libbomap.la ../server/serverCell.o ../common/libboson.la -lqt
boeditor_LDADD   = ../common/libboson.la -lqt

#boreader_SOURCES = boreader.cpp playField.cpp
#boreader_LDADD   = ./libbomap.la ../server/serverCell.o ../common/libboson.la -lqt
#boreader_LDADD   = ../common/libboson.la -lqt

SUBDIRS = 

EXTRA_DIST =  map.h boeditor.cpp boeditor.h

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
# you can add here more. This one gets installed 
#bin_PROGRAMS = libboson.la
#lib_LTLIBRARIES = libbomap.la

#libbomap_la_METASOURCES = USE_AUTOMOC


bin_PROGRAMS = boeditor #boreader

boeditor_METASOURCES = USE_AUTOMOC
# the library search path. 
#libbomap_la_LDFLAGS = $(all_libraries) #$(KDE_RPATH)

# them while "make clean", use CLEANFILES
#DISTCLEANFILES = $(libbomap_la_METASOURCES) basic.bpf
DISTCLEANFILES = basic.bpf

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
#messages:
#	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(libboson_la_SOURCES) && mv messages.po ../po/bocommon.pot

basic.bpf : boeditor
	./boeditor

