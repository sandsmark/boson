CURSORS = default

cursordir = $(kde_datadir)/boson/themes/cursors

uninstall-local:
	rm -rf $(cursordir)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(cursordir)
	@for i in $(CURSORS); do \
		if test -d $(DESTDIR)$(cursordir)/$$i; then \
			echo "refreshing and removing $$i" ;\
			rm -rf $(DESTDIR)$(cursordir)/$$i;\
		fi ;\
		echo "installing $$i" ;\
		mkdir -p $(DESTDIR)$(cursordir)/$$i ;\
		dirs=`cd $(srcdir)/$$i && find -type d`; \
		for d in $$dirs; do \
			mkdir -p $(DESTDIR)$(cursordir)/$$i/$$d; \
		done; \
		files=`cd $(srcdir)/$$i && find -type f`; \
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(cursordir)/$$i/$$f ;\
			fi \
		done\
	done
