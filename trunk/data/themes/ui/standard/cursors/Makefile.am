#cursors_DATA = move.png move.desktop attack.png attack.desktop default.png \
#		default.desktop
#EXTRA_DIST = $(cursors_DATA)

cursorsdir = $(kde_datadir)/boson/themes/cursors

CURSORSDIRS = move attack default


uninstall-local:
	rm -rf $(cursorsdir)
install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(cursorsdir)
	@for i in $(CURSORSDIRS) ; do \
                if test -d $(DESTDIR)$(cursorsdir)/$$i; then \
                        echo "refreshing and removing $$i" ;\
                        rm -rf $(DESTDIR)$(cursorsdir)/$$i;\
                fi ;\
                echo "installing $$i" ;\
                mkdir -p $(DESTDIR)$(cursorsdir)/$$i ;\
                files=`cd $(srcdir)/$$i && ls -1d *` ;\
                for f in $$files; do \
                        if test -f $(srcdir)/$$i/$$f; then \
                                $(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(cursorsdir)/$$i/$$f ;\
                        fi \
                done \
        done
