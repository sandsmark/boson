
EXTRA_DIST = 

#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/boson/
#	$(INSTALL_DATA) $(HTML_DOCS) $(DESTDIR)$(kde_htmldir)/en/boson/
#
#uninstall-local:
#	-rm -f $(kde_htmldir)/en/boson/*.html


HTML_DOCS = index-1.html index-2.html index-3.html index-4.html index-5.html index.html

# help files
data_DATA = $(HTML_DOCS) logotp3.gif
datadir = $(kde_htmldir)/en/boson

$(HTML_DOCS) : index.sgml
	ksgml2html index.sgml en

EXTRA_DIST = $(data_DATA)

myclean:
	-rm -f $(HTML_DOCS)

doit: $(HTML_DOCS)

