/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann <b_mann@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <qdir.h> // must be first, due to a header conflict otherwise

#include "bogl.h"
#include "boglx.h"
#include "bogl_private.h"

#include "bodebug.h"
#include "myqlibrary.h"

#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>

#include <stdlib.h>

static MyQLibrary* loadLibrary(const QString& fileName);
static MyQLibrary* loadLibraryFromFile(const QString& file);
static void scanLdSoConf(QStringList* dirs, const QString& file, QStringList* scannedFiles = 0);
static QStringList resolveWildcards(const QString& argument);
static void resolveWildcards(QStringList* files, const QString& argument);

bool boglResolveGLSymbols()
{
 return BoGL::bogl()->resolveGLSymbols();
}

 // QLibrary assumes that the library ends with .so, however e.g. on
 // vanilla debian systems libGLU.so may not exist. Only libGLU.so.1.3 does.
 // Therefore we must check whether the library got loaded and if not, we must
 // try to use all libGL.* (libGLU.*) files found.
 //
 // UPDATE: we use MyQLibrary now (not QLibrary anymore). same problem though,
 //         as MyQLibrary will expect an absolute filename.
static MyQLibrary* loadLibrary(const QString& name)
{
 if (name.startsWith("/")) {
	MyQLibrary* lib = loadLibraryFromFile(name);
	if (lib) {
		return lib;
	}
	boError() << k_funcinfo << "library " << name << " could not be loaded" << endl;
	return 0;
 } else if (name.contains("/")) {
	boError() << k_funcinfo << "library " << name << " could not be loaded. filename guessing is not supported for relative paths." << endl;
	return 0;
 }

// boDebug() << k_funcinfo << "Trying to guess correct filename for libGL" << endl;

 // we are trying to emulate the search order of dlopen() now:
 // 1. if exectuable file contains a DT_RPATH tag and no DT_RUNPATH, then
 //    search the dirs listed in DT_RPATH
 //    -> I have no idea how to emulate this, probably we cannot do so. Probably
 //       we don't need it anyway
 //    --> we do not emulate this.
 // 2. LD_LIBRARY_PATH dirs
 // 3. if the executable contains a DT_RUNPATH tag, then search the dirs listed
 //    in there
 //    -> same problem as with DT_RPATH
 // 4. the cache file /etc/ld.so.cache is checked for the file-
 //    -> I have no idea how to parse that file (and don't think it would be a
 //       good idea), so we use /etc/ld.so.conf instead
 // 5. /lib
 // 6. /usr/lib
 //
 // in addition we also search in /usr/X11R6/lib
 QStringList dirs;

 // AB: shamelessy stolen from qprocess_unix.cpp
#if defined(Q_OS_MACX)
 QString ld_library_path("DYLD_LIBRARY_PATH");
#else
 QString ld_library_path("LD_LIBRARY_PATH");
#endif

 dirs += QStringList::split(':', QString(getenv(ld_library_path)));

 scanLdSoConf(&dirs, "/etc/ld.so.conf");

 dirs.append("/lib");
 dirs.append("/usr/lib");

 // AB: these are not listed in the manpage, but according to "gg:ld.so.conf
 // lib64", these _are_ builtin into ldconfig
 dirs.append("/lib64");
 dirs.append("/usr/lib64");

 // AB: dlopen() emulation (at least following the manpage) ends here.
 //     the following dirs are our own additions.

 dirs.append("/usr/X11R6/lib");

 MyQLibrary* lib = 0;
 QString suffix = ".so";
 for (QStringList::iterator dirit = dirs.begin(); dirit != dirs.end(); ++dirit) {
	QString dirname = *dirit;
//	boDebug() << "searching in dir " << dirname << endl;

	QDir dir;
	if (!dir.cd(dirname)) {
		boDebug() << "cannot enter directory " << dirname << endl;
		continue;
	}
	dir.setFilter(QDir::Files | QDir::Readable);


	// this part is highly system dependent.
	// I don't know how to handle other systems (Q_OS_MACX, Q_WS_WIN,
	// Q_OS_HPUX) correctly, so probably this solution is pretty much
	// a noop on these systems.


	// AB: we match ("name" is the name of the library, .so the suffix)
	// - name.so*, i.e. name.so, name.so.1, name.so.1.3, ...
	//   -> "name" may start with "lib" already
	// - libname.so*, i.e. libname.so, libname.so.1, ...
	//   -> this is the usual case
	QString filter1 = QString("%1%2*").arg(name).arg(suffix);
	QString filter2 = QString("lib%1%2*").arg(name).arg(suffix);
	dir.setNameFilter(QString("%1 %2").arg(filter1).arg(filter2));

	QStringList files = dir.entryList();
	for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
		QString file = dir.absPath() + "/" + *it;
		lib = loadLibraryFromFile(file);
		if (lib) {
//			boDebug() << "using file " << file << endl;
			return lib;
		}
		boWarning() << k_funcinfo << "library file " << file << " exists but cannot be loaded" << endl;
	}
 }

 if (!lib) {
	boError() << k_funcinfo << "unable to load library " << name << endl;
	return 0;
 }
 return lib;
}

static MyQLibrary* loadLibraryFromFile(const QString& file)
{
 MyQLibrary* lib = new MyQLibrary(file);
 if (!lib->isLoaded() && !lib->load()) {
	delete lib;
	lib = 0;
 }
 return lib;
}

static void scanLdSoConf(QStringList* dirs, const QString& file, QStringList* scannedFiles)
{
 QStringList scanned;
 if (!dirs) {
	BO_NULL_ERROR(dirs);
	return;
 }
 if (!scannedFiles) {
	scannedFiles = &scanned;
 }
 if (scannedFiles->contains(file)) {
	return;
 }
 if (!QFile::exists(file)) {
	return;
 }
 scannedFiles->append(file);
 QFile conf(file);
 if (conf.open(IO_ReadOnly)) {
	// AB: we use lines that begin with '/' only (whitespaces
	//     ignored)
	// AB: we also use lines that start with "include", which is used by a
	//     fedora core patch to glibc.
	QTextStream s(&conf);
	while (!s.atEnd()) {
		QString line;
		line = s.readLine();

		// remove comments
		if (line.find('#') >= 0) {
			line = line.left(line.find('#') + 1);
		}

		line = line.stripWhiteSpace();
		if (line.startsWith("include")) {
			QString inc = line.right(line.length() - QString("include").length());
			inc = inc.stripWhiteSpace();
			if (!inc.startsWith("/")) {
				inc = QString("/etc/") + inc;
			}
			QDir dir;
			QStringList incFiles = resolveWildcards(inc);
			for (unsigned int i = 0; i < incFiles.count(); i++) {
				scanLdSoConf(dirs, incFiles[i], scannedFiles);
			}

			continue;
		} else if (!line.startsWith("/")) {
			continue;
		}

		dirs->append(line);
	}
 }
}

QStringList resolveWildcards(const QString& argument)
{
 QStringList list;
 resolveWildcards(&list, argument);
 return list;
}

void resolveWildcards(QStringList* files, const QString& argument)
{
 if (!files) {
	BO_NULL_ERROR(files);
	return;
 }
 if (argument.isEmpty()) {
	return;
 }
 if (argument[0] != '/') {
	boDebug() << k_funcinfo << "argument \"" << argument << "\" did not start with '/', relative filenames are not supported." << endl;
	return;
 }
 int index1 = argument.find("*");
 int index2 = argument.find("?");
 if (index1 < 0 && index2 < 0) {
	files->append(argument);
	return;
 }
 int index = index1;
 if (index2 >= 0 && index2 < index1 || index1 < 0) {
	index = index2;
 }
 if (index < 0) {
	boError() << k_funcinfo << "oops" << endl;
	return;
 }
 QString prefix = argument.left(index); // everything before the first '*' or '?'
 QString dirname = prefix.left(prefix.findRev("/")); // the dirname in prefix
 QDir dir(dirname);
 if (!dir.exists()) {
	boDebug() << "directory " << dirname << " does not exist" << endl;
	return;
 }
 QString afterDir = argument.right(argument.length() - (dirname.length() + 1));
 QString inDir;
 QString suffixDir;
 if (afterDir.find('/') >= 0) {
	// note: suffixDir includes leading '/'
	suffixDir = afterDir.right(afterDir.length() - (afterDir.find('/')));
	inDir = afterDir.left(afterDir.find('/'));
 } else {
	inDir = afterDir;
 }

 // argument == dirname + '/' + inDir + suffixDir

 QStringList entries = dir.entryList(inDir, QDir::Readable | QDir::Files | QDir::Dirs);
 for (QStringList::iterator it = entries.begin(); it != entries.end(); ++it) {
	resolveWildcards(files, dirname + '/' + *it + suffixDir);
 }
}


bool BoGL::resolveGLSymbols()
{
 if (isResolved()) {
	return true;
 }

 bool ret = true;

#if BOGL_DO_DLOPEN

 QString libGL;
 QString libGLU;

 MyQLibrary* gl = loadLibrary("GL");
 if (!gl) {
	return false;
 }
 if (ret) {
	ret = boglResolveLibGLSymbols(gl);
 }
 libGL = gl->library();
 if (libGL.isEmpty()) {
	ret = false;
 }
 if (ret && libGL.left(1) != "/") {
	boError() << k_funcinfo << "library returned by loadLibrary() does not use absolute filename: " << libGL << endl;
	ret = false;
 }
 delete gl;
 gl = 0;
 if (!ret) {
	return false;
 }

 MyQLibrary* glu = loadLibrary("GLU");
 if (!glu) {
	return false;
 }
 if (ret) {
	ret = boglResolveLibGLUSymbols(glu);
 }
 libGLU = glu->library();
 if (libGLU.isEmpty()) {
	ret = false;
 }
 if (ret && libGLU.left(1) != "/") {
	boError() << k_funcinfo << "library returned by loadLibrary() does not use absolute filename: " << libGLU << endl;
	ret = false;
 }
 delete glu;
 glu = 0;
 if (!ret) {
	return false;
 }

 d->mOpenGLLibraryFile = libGL;
 d->mGLULibraryFile = libGLU;

 boDebug() << "Resolved GL symbols from file " << OpenGLFile() << endl;
 boDebug() << "Resolved GLU symbols from file " << GLUFile() << endl;
#else // BOGL_DO_DLOPEN
 ret = boglResolveLibGLSymbols(0);
 if (!ret) {
	boError() << k_funcinfo << "resolving GL symbols failed";
	return false;
 }
 ret = boglResolveLibGLUSymbols(0);
 if (!ret) {
	boError() << k_funcinfo << "resolving GLU symbols failed";
	return false;
 }
 boDebug() << "Resolved GL symbols (GL/GLU linked to binary)" << endl;
#endif // BOGL_DO_DLOPEN

 d->mIsResolved = true;


 return true;
}


