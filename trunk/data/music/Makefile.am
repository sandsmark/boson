musicdir = $(kde_datadir)/boson/music

uninstall-local:
	rm -rf $(musicdir)

# this causes an error for orzel on make install if music_DATA is empty.
# Doesn't happen to me - disable this stuff until a solution is found

# enabled again for testing...
install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(musicdir)
	@dirs=`find -name index.desktop -or -name index.music ` ;\
	if test -n "$$dirs" ; then \
	for i in $$dirs ; do \
		i=`dirname $$i`; \
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
			fi; \
		done; \
	done;\
	fi

