
SUBDIRS = common map server boson

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

AUTOMAKE_OPTIONS = foreign 
