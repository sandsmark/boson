
COMPILE_FIRST = math bobmfconverter bomemory bodebug bogl kgame bosound ufo

SUBDIRS = $(TOPSUBDIRS)

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

EXTRA_DIST = admin

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs


# Doxygen
include admin/Doxyfile.am

apidox-am-toplevel-yes: apidox-am-toplevel-boson

apidox-am-toplevel-boson: $(top_builddir)/apidocs/qt/qt.tag

$(top_builddir)/apidocs/qt/qt.tag:
	@echo "*** Creating a tag file for the Qt library:"; \
	$(mkinstalldirs) $(top_builddir)/apidocs/qt; \
	doxytag -t $(top_builddir)/apidocs/qt/qt.tag $(QTDOCDIR)

