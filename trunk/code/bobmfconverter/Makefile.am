INCLUDES = $(all_includes)

SUBDIRS = libgfx mixkit  loaders processors

bin_PROGRAMS = bobmfconverter

bobmfconverter_SOURCES = \
	main.cpp \
	bo3dtools.cpp \
	frame.cpp \
	loader.cpp \
	lod.cpp \
	material.cpp \
	mesh.cpp \
	model.cpp \
	processor.cpp \
	saver.cpp \
	texture.cpp

bobmfconverter_LDFLAGS = $(all_libraries)
bobmfconverter_LDADD = \
		$(top_builddir)/bobmfconverter/loaders/libloaders.la \
		$(top_builddir)/bobmfconverter/processors/libprocessors.la \
		$(LIB_QT) \
		$(LIB_KDECORE) \
		-l3ds \
		$(GLLIB) \
		$(top_srcdir)/bobmfconverter/libgfx/libgfx.la \
		$(top_srcdir)/bobmfconverter/mixkit/libmixkit.la
