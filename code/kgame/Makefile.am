
noinst_LTLIBRARIES = libkgame.la

KGAME = kgame.cpp kplayer.cpp kgamenetwork.cpp kgameproperty.cpp \
		kgamemessage.cpp kgameio.cpp kgameprocess.cpp kgamechat.cpp \
		kgamepropertyhandler.cpp kgameerror.cpp kgamesequence.cpp
KGAME_H = kgame.h kplayer.h kgamenetwork.h kgameproperty.h kgamemessage.h \
		kgameio.h kgameprocess.h kgamepropertyarray.h \
		kgamepropertylist.h kgamechat.h kgamepropertyhandler.h \
		kgameerror.h kgamesequence.h kgameversion.h

KMESSAGE = kmessageio.cpp kmessageserver.cpp kmessageclient.cpp
KMESSAGE_H = kmessageio.h kmessageserver.h kmessageclient.h

DIALOGS = kgamedebugdialog.cpp
DIALOGS_H = kgamedebugdialog.h

# AB: these files were not part of KGame, but of libkdegames.
# they are here because we need them in boson as well.
LIBKDEGAMES = kchatbase.cpp kgameprogress.cpp kstdgameaction.cpp
LIBKDEGAMES_H = kchatbase.h kgameprogress.h kstdgameaction.h

libkgameincludedir=$(includedir)/kgame
libkgame_la_SOURCES = $(KMESSAGE) $(KGAME) $(DIALOGS) $(LIBKDEGAMES)
libkgame_la_LIBADD = $(top_builddir)/bodebug/libbodebug.la
libkgame_la_LDFLAGS = $(all_libraries)

noinst_HEADERS = $(KMESSAGE_H) $(KGAME_H) $(DIALOGS_H) $(LIBKDEGAMES_H)

INCLUDES = -I$(top_srcdir)/bodebug $(all_includes)

METASOURCES = AUTO

