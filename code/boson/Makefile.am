####### kdevelop will overwrite this part!!! (begin)##########
CXXFLAGS = -pipe -Wall -g # -O2
#MY_HOST :=  $(shell hostname | tr '[a-z]' '[A-Z]')_$(shell domainname | tr '[a-z]' '[A-Z]')
#CXXFLAGS += -D$(MY_HOST)
LDFLAGS =  -s 
boson_SOURCES = mainView.cpp boson.cpp main.cpp connect.cpp groundTheme.cpp speciesTheme.cpp playerUnit.cpp playerCell.cpp physMap.cpp viewMap.cpp fieldMap.cpp miniMap.cpp fieldDisplay.cpp miniDisplay.cpp fieldEvent.cpp miniEvent.cpp orderWin.cpp game.cpp selectPart.cpp
boson_LDADD   =  ../common/libboson.la -lkfile -lkfm -lkdeui -lkdecore -lQwSpriteField -lqt -lXext -lX11 $(LIBSOCKET)

SUBDIRS = docs 

EXTRA_DIST = main.cpp boson.cpp boson.h ressource.h boson.kdelnk boson.xpm mini-boson.xpm connect.h connect.cpp groundTheme.cpp groundTheme.h speciesTheme.cpp speciesTheme.h playerUnit.cpp playerUnit.h playerCell.cpp playerCell.h physMap.cpp physMap.h viewMap.cpp viewMap.h fieldMap.cpp fieldMap.h miniMap.cpp miniMap.h fieldDisplay.cpp miniDisplay.cpp fieldEvent.cpp miniEvent.cpp orderWin.cpp orderWin.h game.cpp game.h mainView.cpp mainView.h selectPart.h selectPart.cpp

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/
	$(INSTALL_DATA) boson.kdelnk $(kde_appsdir)/boson.kdelnk
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) boson.xpm $(kde_icondir)/boson.xpm
	$(mkinstalldirs) $(kde_minidir)/
	$(INSTALL_DATA) mini-boson.xpm $(kde_minidir)/mini-boson.xpm

uninstall-local:
	-rm -f $(kde_appsdir)/boson.kdelnk
	-rm -f $(kde_icondir)/boson.xpm
	-rm -f $(kde_minidir)/mini-boson.xpm

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
bin_PROGRAMS = boson 

boson_METASOURCES = USE_AUTOMOC

# the library search path. 
boson_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(boson_METASOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(boson_SOURCES) && mv messages.po ../po/boson.pot






