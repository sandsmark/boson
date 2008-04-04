#!/bin/bash

# Instructions:
# 1. create a new directory (e.g. "static") and copy code and data into it.
# 2. copy qt, kdelibs, arts, python, openal, SDL into that directory ("static")
#    and prepare them as discussed in README.boson-static (see tools/installer).
# 3. modify code as discussed in README.boson-static
# 4. compile a static boson binary with srcdir!=builddir, i.e. do a
#    mkdir build ; cd build ; ../configure <parameters> instead of a ./configure
# 5. compile and install data into "static/data_installation"
# 6. copy loki_setupdb and loki_setup into "static". prepare and compile them as
#    discussed in README.loki (see also tools/installer/)
# 7. mkdir "loki" in "static".
# 8. cd loki
# 9. mkdir boson_installer
# 10. execute this script.
# 11. an installer should be in boson_installer/ now.
#
# some of these instructions are not strictly necessary, but by following them,
# you can sometimes avoid modifying the path variables below.

INSTALLER_VERSION="0.12pre"
OS="Linux"
ARCH="x86"
LIBC="glibc-2.1"
LOKI_SETUP="loki_setup"
CODE="code-head"
CODE_BUILD="$CODE/build"
DATA_INSTALLATION="data_installation"
KDELIBS_CODE="kdelibs"
PYTHON_CODE="python/python2.4"
PATCHES="../tools/installer/patches"
SETUP_XML="../tools/installer/setup.xml"
DESTINATION="boson_installer"

# binaries to copy
#BINARIES="boson/boson boson/boinfo boson/bocursor server/boserver boson/boufo/designer/boufodesigner" # boson/borender boson/bounit
BINARIES="boson/boson boson/boinfo" # boson/bocursor server/boserver boson/boufo/designer/boufodesigner" # boson/borender boson/bounit
# binaries to copy to a special "bin/" subdir (programs which are called by boson itself)
BINARIES_TO_BIN="bobmfconverter/bobmfconverter"

# files form code/boson/data/ that are copied to share/config/
CODE_DATA_TO_SHARE_CONFIG="bodebugrc bodebug.areas"

# files form code/boson/data/ that are copied to share/apps/boson/
CODE_DATA_TO_SHARE_APPS="*ui.rc"

# files from kdelibs that are copied to share/config/ui/ (usually no
# modification necessary)
KDELIBS_TO_SHARE_CONFIG_UI="kdeui/ui_standards.rc"
# TODO: modelcache
# TODO: music
# TODO: applnk
# TODO: icons
# TODO: copy $DATA/pics/biglogo.png (converted to xpm) to $DESTINATION/image/setup.data/splash.xml

if [ ! -r "$LOKI_SETUP" ]; then
	echo "Cannot read directory $LOKI_SETUP"
	exit 1
fi

FILE="$LOKI_SETUP/image/setup.data/bin/$OS/$ARCH/$LIBC/setup.gtk"
if [ ! -x "$FILE" ]; then
	echo "File $FILE does not exist or is not executable. Execute make and make install in $LOKI_SETUP first"
	exit 1
fi

if [ ! -r "$SETUP_XML" ]; then
	echo "File $SETUP_XML can not be read. Create it first!"
	exit 1
fi

if [ ! -r "$CODE" ]; then
	echo "Cannot read directory $CODE"
	exit 1
fi

if [ ! -r "$CODE_BUILD" ]; then
	echo "Cannot read directory $CODE_BUILD."
	exit 1
fi

for binary in $BINARIES $BINARIES_TO_BIN; do
	if [ ! -x "$CODE_BUILD/$binary" ]; then
		echo "No exectuable named \"$binary\" found in $CODE_BUILD."
		exit 1
	fi
done

if [ ! -d "$DESTINATION" ]; then
	echo "Destination directory $DESTINATION does not exist"
	exit 1
fi



copy_image() {
	cp -a "$LOKI_SETUP/image" "$DESTINATION/"
	cp "$SETUP_XML" "$DESTINATION/image/setup.data/"
}

copy_binaries() {
	binary_dest="$DESTINATION/image/bin/Linux/x86/glibc-2.1"
	mkdir -p $binary_dest
	for binary in $BINARIES; do
		ok="yes"
		cp "$CODE_BUILD/$binary" "$binary_dest/" || ok="no"
		if [ "$ok" != "yes" ]; then
			echo "Binary $binary could not be copied to $binary_dest"
			exit 1
		fi
	done
	mkdir -p $binary_dest/bin
	for binary in $BINARIES_TO_BIN; do
		ok="yes"
		cp "$CODE_BUILD/$binary" "$binary_dest/bin/" || ok="no"
		if [ "$ok" != "yes" ]; then
			echo "Binary $binary could not be copied to $binary_dest/bin/"
			exit 1
		fi
	done

	# finally replace the "boson" binary by a "boson" shellscript.
	# this script preloads libGL.so, which is necessary for some systems
	# (maybe nvidia only): dlopen()ing libGL.so after loading
	# libpthread (which boson links against), fails on these systems
	mv $binary_dest/boson $binary_dest/boson.bin
	echo "#!/bin/bash" > $binary_dest/boson
	echo "export LD_PRELOAD=libGL.so" >> $binary_dest/boson
	echo "\$0.bin \"\$@\"" >> $binary_dest/boson
	chmod +x $binary_dest/boson
}
copy_data() {
	mkdir -p "$DESTINATION/image/share/config"
	for i in $CODE_DATA_TO_SHARE_CONFIG; do
		ok="yes"
		cp $CODE/boson/data/$i "$DESTINATION/image/share/config/" || ok="no"
		if [ "$ok" != "yes" ]; then
			echo "File $i could not be copied"
			exit 1
		fi
	done

	mkdir -p "$DESTINATION/image/share/config/ui"
	for i in $KDELIBS_TO_SHARE_CONFIG_UI; do
		ok="yes"
		cp $KDELIBS_CODE/$i "$DESTINATION/image/share/config/ui" || ok="no"
		if [ "$ok" != "yes" ]; then
			echo "File $i could not be copied"
			exit 1
		fi
	done

	mkdir -p "$DESTINATION/image/share/apps/boson/"
	for i in $CODE_DATA_TO_SHARE_APPS; do
		ok="yes"
		cp $CODE/boson/data/$i "$DESTINATION/image/share/apps/boson/" || ok="no"
		if [ "$ok" != "yes" ]; then
			echo "File $i could not be copied"
			exit 1
		fi
	done

	if [ ! -r "$DATA_INSTALLATION" ]; then
		echo "Directory $DATA_INSTALLATION cannot be read"
		exit 1
	fi
	if [ ! -r "$DATA_INSTALLATION/share/apps/boson" ]; then
		echo "Directory $DATA_INSTALLATION/share cannot be read"
		exit 1
	fi
	ok="yes"
	mkdir -p "$DESTINATION/image/share/apps/boson"
	cp -r "$DATA_INSTALLATION/share/apps/boson" "$DESTINATION/image/share/apps/" || ok="no"
	if [ "$ok" != "yes" ]; then
		echo "Could not copy data"
		exit 1
	fi
}
copy_doc() {
	if [ ! -r "$DATA_INSTALLATION" ]; then
		echo "Directory $DATA_INSTALLATION cannot be read"
		exit 1
	fi
	if [ ! -r "$DATA_INSTALLATION/share/doc" ]; then
		echo "Directory $DATA_INSTALLATION/share/doc cannot be read"
		exit 1
	fi
	ok="yes"
	cp -r "$DATA_INSTALLATION/share/doc" "$DESTINATION/image/share/" || ok="no"
	if [ "$ok" != "yes" ]; then
		echo "Could not copy doc"
		exit 1
	fi
}

copy_music() {
	echo "TODO"
}

copy_pythonlib() {
	pythonlib_dir="$DESTINATION/image/share/apps/boson/themes/scripts/pythonlib"
	if [ ! -r "$PYTHON_CODE/Lib" ]; then
		echo "Directory $PYTHON_CODE/Lib cannot be read"
		exit 1
	fi
	mkdir -p "$pythonlib_dir"
	ok="yes"
	cp $PYTHON_CODE/Lib/*.py "$pythonlib_dir/" || ok="no"
	if [ "$ok" != "yes" ]; then
		echo "Could not copy pythonlib"
	fi
}

compress_bpf_files() {
	for i in `find $DESTINATION -name "*.bpf"`; do
		gzip "$i"
		mv "$i.gz" "$i"
	done
}
copy_modelcache() {
	echo "TODO: copy modelcache (probably generate on the fly)"
}
copy_patches() {
	if [ ! -r "$PATCHES" ]; then
		echo "Directory $PATCHES cannot be read"
		exit 1
	fi
	ok="yes"
	cp -r "$PATCHES" "$DESTINATION/image/" || ok="no"
	if [ "$ok" != "yes" ]; then
		echo "Patches could not be copied"
		exit 1
	fi
}

remove_cvs_directories() {
	if [ ! -d "$DESTINATION" ]; then
		echo "Destination directory $DESTINATION does not exist"
		exit 1
	fi
#	find $DESTINATION -name "CVS" -type d -exec echo {} \;
	find $DESTINATION -name "CVS" -type d -exec rm -r {} \; 2>/dev/null
}
create_installer() {
	MAKESELF="$LOKI_SETUP/makeself/makeself.sh"
	if [ ! -x "$MAKESELF" ]; then
		echo "Cannot execute makeself"
		exit 1
	fi
	if [ ! -d "$DESTINATION" ]; then
		echo "No destination directory $DESTINATION"
		exit 1
	fi

	FILENAME="boson-$INSTALLER_VERSION-$ARCH.run"
	ok="yes"
	$MAKESELF $DESTINATION/image $FILENAME "Boson" ./setup.sh || ok="no"
	if [ $ok != "yes" ]; then
		echo "makeself failed"
		exit 1
	fi
	mv $FILENAME $DESTINATION/
}

echo "Copying $LOKI_SETUP/image to $DESTINATION..."
copy_image
echo "$LOKI_SETUP/image copied."

echo "Copying binaries to $DESTINATION..."
copy_binaries
echo "Binaries copied."

echo "Copying data..."
copy_data
echo "data copied."

echo "Copying music..."
copy_music
echo "music copied."

echo "Copying pythonlib..."
copy_pythonlib
echo "pythonlib copied."

echo "Compressing .bpf files..."
compress_bpf_files
echo ".bpf files compressed."

echo "Copying modelcache..."
copy_modelcache
echo "modelcache copyied."

echo "Copying patches..."
copy_patches
echo "patches copyied."

echo "Removing CVS directories..."
remove_cvs_directories
echo "CVS directories removed."

echo "Creating installer..."
create_installer
echo "installer created."

