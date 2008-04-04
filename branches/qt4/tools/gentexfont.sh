#!/bin/sh

if [ -z "$1" ]; then
	echo "First parameter must be the name of the font"
	exit 1
fi

if [ -z "$2" ]; then
	echo "Second parameter must be the output file"
	exit 1
fi

FONT=$1
FILE=$2

./gentexfont \
	-fn "$FONT" \
	-file $FILE \
	-glist \
	'`"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMN\
	OPQRSTUVWXYZ01234567890 \!@#$$%^&*()-=+/,.<>;:~{}[]' \
	-bitmap \
	-w 256 \
	-h 256 \
	-gap 3

