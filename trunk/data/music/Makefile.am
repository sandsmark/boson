music_DATA = 
musicdir = $(kde_datadir)/boson/music

uninstall-local:
	rm -rf $(musicdir)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(musicdir)
	@for i in $(music_DATA); do \
		if test -d $(DESTDIR)$(musicdir)/$$i; then \
			echo "refreshing and removing $$i" ;\
				rm -rf $(DESTDIR)$(musicdir)/$$i;\
		fi ;\
		echo "installing $$i" ;\
		mkdir -p $(DESTDIR)$(musicdir)/$$i ;\
		files=`cd $(srcdir)/$$i && ls -1d *` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(musicdir)/$$i/$$f ;\
			fi \
		done \
	done 

