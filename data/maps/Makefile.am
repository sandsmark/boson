
# This is where the maps will all be installed
mapsdir = $(kde_datadir)/boson/maps

uninstall-local:
	rm -rf $(mapsdir)

install-data-local:
	@if test -d $(DESTDIR)$(mapsdir); then \
		echo "Removing old maps directory" ;\
		rm -rf $(DESTDIR)$(mapsdir) ;\
	fi
	@$(mkinstalldirs) $(DESTDIR)$(mapsdir)
	@echo "Creating campaign dirs" ;\
	dirs=`cd $(srcdir) && find . -maxdepth 1 -type d | grep -v CVS` ;\
	for d in $$dirs; do \
		if test -d $(srcdir)/$$d; then \
			$(mkinstalldirs) $(DESTDIR)$(mapsdir)/$$d ;\
		fi \
	done

	@echo "Installing maps"
	@mkdir -p $(DESTDIR)$(mapsdir)
	@files=`cd $(srcdir) && find . -maxdepth 2 -type f -name "*.bpf"`;\
	for f in $$files; do \
		if test -f $(srcdir)/$$f; then \
			$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(mapsdir)/$$f ;\
		fi \
	done

