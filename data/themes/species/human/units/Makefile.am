MOBILE_UNITS = \
	mob_aircraft \
	mob_mineralharvester \
	mob_oilharvester \
	mob_quad \
	mob_ship \
	mob_tank
FACILITIES = \
	fix_barracks \
	fix_comsat \
	fix_commandbunker \
	fix_helipad \
	fix_oiltower \
	fix_powerplant \
	fix_prison \
	fix_refinery \
	fix_repairpad \
	fix_samsite \
	fix_techcenter \
	fix_turret \
	fix_warfactory

unitsdir = $(kde_datadir)/boson/themes/species/human/units

uninstall-local:
	rm -rf $(unitsdir)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(unitsdir)
	@for i in $(MOBILE_UNITS) $(FACILITIES); do \
		if test -d $(DESTDIR)$(unitsdir)/$$i; then \
			echo "refreshing and removing $$i" ;\
			rm -rf $(DESTDIR)$(cursordir)/$$i;\
		fi ;\
		echo "installing $$i" ;\
		mkdir -p $(DESTDIR)$(unitsdir)/$$i ;\
		files=`cd $(srcdir)/$$i && ls -1d *` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(unitsdir)/$$i/$$f ;\
			fi \
		done \
	done


