# add all grounds directories here
GROUNDS = earth

# This is where it will all be installed
groundsdir = $(kde_datadir)/boson/themes/grounds

uninstall-local:
	rm -rf $(groundsdir)
install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(groundsdir)
	@for i in $(GROUNDS) ; do \
		if test -d $(DESTDIR)$(groundsdir)/$$i; then \
			echo "refreshing and removing $$i" ;\
			rm -rf $(DESTDIR)$(groundsdir)/$$i;\
		fi ;\
		echo "installing $$i" ;\
		dirs=`cd $(srcdir)/$$i && find . -type d -not -regex ".*CVS.*"`;\
		mkdir -p $(DESTDIR)$(groundsdir)/$$i ;\
		for j in $$dirs; do \
			mkdir -p $(DESTDIR)$(groundsdir)/$$i/$$j ;\
		done ;\
		files=`cd $(srcdir)/$$i && find . -type f -not -regex ".*CVS.*"` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(groundsdir)/$$i/$$f ;\
			fi \
		done \
	done

