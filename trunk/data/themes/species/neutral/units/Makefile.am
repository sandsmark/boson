MOBILE_UNITS =
FACILITIES = \
	barn1 \
	barrel-big \
	barrel-small \
	broken_tree \
	crate-cross \
	crate-cross-double \
	crate-plain \
	crate-plain-double \
	crate-wooden-1 \
	crate-wooden-1-double \
	haystack1 \
	house-1 \
	rock1 \
	rubbishpile-1 \
	rubbishpile-1-big \
	rubbishpile-1-small \
	toilet \
	tree1 \
	tree2 \
	tree3 \
	tree4 \
	tree4-big \
	tree4-small \
	tree5 \
	tree5-big \
	tree5-small \
	tree6 \
	tree6-big \
	tree6-small \
	oiltower \
	mineralmine

unitsdir = $(kde_datadir)/boson/themes/species/neutral/units

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

