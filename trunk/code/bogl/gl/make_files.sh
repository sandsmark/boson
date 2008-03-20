#!/bin/bash

# we are assuming that boglc has been built in the "boglc/build" subdir

RESOLVE_FILES=""
# GL + GL extensions + GLX
for i in specs/bogl_*.boglc; do
	echo "$i -> `basename $i .boglc`.h"
	./boglc/build/boglc --input "$i" --outputheader `basename $i .boglc`.h || exit 1

	RESOLVE_FILES="$RESOLVE_FILES --input $i"
done
for i in specs/extensions/*.boglc; do
	echo "$i -> extensions/`basename $i .boglc`.h"
	./boglc/build/boglc --input "$i" --outputheader extensions/`basename $i .boglc`.h || exit 1

	RESOLVE_FILES="$RESOLVE_FILES --input $i"
done
for i in specs/boglx_*.boglc; do
	echo "$i -> `basename $i .boglc`.h"
	./boglc/build/boglc --input "$i" --outputheader `basename $i .boglc`.h || exit 1

	RESOLVE_FILES="$RESOLVE_FILES --input $i"
done
RESOLVE_CPP="bogl_resolve_symbols.cpp"
echo "generating $RESOLVE_CPP"
./boglc/build/boglc $RESOLVE_FILES --outputresolve $RESOLVE_CPP

# GLU
glu_file=specs/boglu_decl_p.boglc
echo "$glu_file -> `basename $glu_file .boglc`.h"
./boglc/build/boglc --input "$glu_file" --outputheader `basename $glu_file .boglc`.h || exit 1

echo "Success."

