#!/bin/bash

# we are assuming that boglc has been built in the "boglc/build" subdir

for i in specs/*.boglc; do
	echo "$i -> `basename $i .boglc`.h"
	./boglc/build/boglc --input "$i" --output `basename $i .boglc`.h || exit 1
done
for i in specs/extensions/*.boglc; do
	echo "$i -> extensions/`basename $i .boglc`.h"
	./boglc/build/boglc --input "$i" --output extensions/`basename $i .boglc`.h || exit 1
done

echo "Success."

