MOBILE_UNITS = \
	air_owl \
	air_shark \
	mob_bulldog \
	mob_frog \
	mob_gecko \
	mob_leo \
	mob_mineralharvester \
	mob_oilharvester \
	mob_wolf

FACILITIES = \
	fix_commandcenter \
	fix_factory \
	fix_mineralrefinery \
	fix_oilrefinery \
	fix_powerplant \
	fix_samsite \
	fix_turret

unitsdir = $(kde_datadir)/boson/themes/species/smackware1/units

uninstall-local:
	rm -rf $(unitsdir)

install-data-local:
	@if test -d $(DESTDIR)$(unitsdir); then \
		echo "removing old unit directory" ;\
		rm -rf $(DESTDIR)$(unitsdir);\
	fi
	@$(mkinstalldirs) $(DESTDIR)$(unitsdir)
	@for i in $(MOBILE_UNITS) $(FACILITIES); do \
		echo "installing $$i" ;\
		mkdir -p $(DESTDIR)$(unitsdir)/$$i ;\
		files=`cd $(srcdir)/$$i && find . -type f | grep -v CVS` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(unitsdir)/$$i/$$f ;\
			fi \
		done \
	done

# we install the entire directory - so we need to add the entire directory to
# the make dist target. remember to remove the CVS dirs here after doing a make
# dist!
EXTRA_DIST = $(FACILITIES) $(MOBILE_UNITS)

