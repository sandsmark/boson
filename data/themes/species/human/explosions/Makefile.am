EXPLOSIONS = shots facilities units

explosionsdir = $(kde_datadir)/boson/themes/species/human/explosions

uninstall-local:
	rm -rf $(explosionsdir)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(explosionssdir)
	@for i in $(EXPLOSIONS); do \
		if test -d $(DESTDIR)$(explosionsdir)/$$i; then \
			echo "refreshing and removing $$i" ;\
			rm -rf $(DESTDIR)$(explosionsdir)/$$i;\
		fi ;\
		echo "installing $$i" ;\
		mkdir $(DESTDIR)$(explosionsdir)/$$i ;\
		files=`cd $(srcdir)/$$i && ls -1d *` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(explosionsdir)/$$i/$$f ;\
			fi \
		done \
	done


