
SUBDIRS = common server visual boson editor 

EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO 

#MAKE = nice -20 make -j 3

api :
	kdoc -p -dapi/ -ufile:/home/orzel/c/boson/api/ boson */*h

orzel.clean : maintainer-clean
	rm -f configure Makefile{,.in}
	rm -Rf */.deps
	rm -f */Makefile{,.in}
	rm -f `find -name core`
	rm -f `find -name boson-client.log`
	rm -f `find -name boson-server.log`
#	cd .. && tar cvf /work/save/boson/

AUTOMAKE_OPTIONS = foreign  1.4

tags:
	ctags `find * -name '*.h' -o -name '*.cpp' | grep -v moc | grep /`

_common:
	cd common && make

_server: _common
	cd server && make

_visual: _common
	cd visual && make

_boson: _visual
	cd boson && make

_editor: _visual
	cd editor && make

