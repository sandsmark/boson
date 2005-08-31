INCLUDES = $(all_includes) -I$(top_srcdir) -I$(top_srcdir)/bodebug

METASOURCES = AUTO

bin_PROGRAMS = boserver

boserver_SOURCES = \
	main.cpp \
	server.cpp \
	webinterface.cpp \
	game.cpp \
	player.cpp

boserver_LDFLAGS = $(all_libraries)
boserver_LDADD = \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		$(LIB_KGAME)
