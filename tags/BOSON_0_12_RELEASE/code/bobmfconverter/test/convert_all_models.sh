#!/bin/bash

CONVERTER=$1
KDEDIR=$2
OUTPUT_DIR=$3
DELETE_OUTPUT_DIR="no"
CONVERTER_PARAMS="-lods 5 -keepframes -texnametolower -useboth -dontloadtex"

function exit_script()
{
 exit_code=$1
 if [ -z "$exit_code" ]; then
	exit_code="0"
 fi
 if [ "x$DELETE_OUTPUT_DIR" = "xyes" ]; then
	rm -rf $OUTPUT_DIR
 fi
 exit $exit_code
}
function usage()
{
	echo "Usage: $0 /path/to/bobmfconverter /path/to/kdedir [/path/to/output/directory]"
	exit_script 1
}

if [ -z $KDEDIR ]; then
	KDEDIR="/opt/kde3"
fi

if [ ! -d $KDEDIR ]; then
	echo "ERROR: KDEDIR containing unit files not found"
	usage
fi

if [ -z $CONVERTER ]; then
	CONVERTER="../../build/bobmfconverter/bobmfconverter"
	echo $CONVERTER
fi
if [ ! -x $CONVERTER ]; then
	echo "ERROR: bobmfconverter binary not found"
	usage
fi
if [ -z "$OUTPUT_DIR" ]; then
	OUTPUT_DIR="$PWD/converter_output"
	if [ -d $OUTPUT_DIR ]; then
		echo "Default output dir $OUTPUT_DIR already exists. delete it first"
		exit_script 1
	fi

	mkdir $OUTPUT_DIR || exit_script 1
	DELETE_OUTPUT_DIR="yes"
fi

THEMESDIR="$KDEDIR/share/apps/boson/themes"
MODELS=`find $THEMESDIR -name "unit.3ds"`
if [ -z "$MODELS" ]; then
	echo "Did not find any model files in $THEMESDIR"
	exit_script 1
fi

for model in $MODELS; do
	dir=`dirname $model`
	config="$dir/index.unit"
	unit=`basename $dir`
	$CONVERTER $CONVERTER_PARAMS -o "$OUTPUT_DIR/$unit.bmf" -c "$config" "$model"
	exit_code=$?
	rm -f "$OUTPUT_DIR/$unit.bmf"

	if [ "$exit_code" -ne "0" ]; then
		echo "bobmfconverter returned an error for model file $model"
		exit_script 1
	fi
done

exit_script 0

