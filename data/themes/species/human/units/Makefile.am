MOBILE_UNITS = \
	mob_ferry \
	mob_grizzly \
	mob_leopard \
	mob_mineralharvester \
	mob_oilharvester \
	mob_panther \
	mob_piranha \
	mob_ship \
	mob_tiger \
	mob_tornado
FACILITIES = \
	fix_commandbunker \
	fix_comsat \
	fix_helipad \
	fix_mineralrefinery \
	fix_oilrefinery \
	fix_oiltower \
	fix_powerplant \
	fix_repairpad \
	fix_samsite \
	fix_techcenter \
	fix_turret \
	fix_weaponsfactory

unitsdir = $(kde_datadir)/boson/themes/species/human/units

uninstall-local:
	rm -rf $(unitsdir)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(unitsdir)
	@for i in $(MOBILE_UNITS) $(FACILITIES); do \
		if test -d $(DESTDIR)$(unitsdir)/$$i; then \
			echo "refreshing and removing $$i" ;\
			rm -rf $(DESTDIR)$(unitsdir)/$$i;\
		fi ;\
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

