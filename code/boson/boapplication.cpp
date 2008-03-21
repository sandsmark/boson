/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boapplication.h"

#include "../bomemory/bodummymemory.h"
#include "boglobal.h"
#include "imageio/boimageio.h"

#include <config.h>
#include <bogl.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>

#include <stdlib.h>

/**
 * @return @ref QApplication::applicationDirPath(), but it does not use the
 * argv[0] that is given to @ref QApplication, as it is broken (@ref
 * KCmdLineArgs::init strips the path out of it).
 **/
static QString fixedApplicationDirPath(const QCString& argv0);

/**
 * @internal
 * Required for @ref fixedApplicationDirPath
 **/
static QString resolveSymlinks( const QString& path, int depth = 0 );

BoApplication::BoApplication(const QCString& argv0, bool allowStyles, bool enableGUI)
	: KApplication(allowStyles, enableGUI)
{
 // this is for broken installations. people tend to install to /usr/local or
 // similar (which is 100% correct), but don't set $KDEDIRS (note that S)
 // correct. This is (I guess) a distribution bug in most (all?) distributions
 // out there.
 // we tell KDE here which our prefix is and add it this way to $KDEDIRS

 // AB: in KDE 3.2 there is a bug that overwrites custom resourceDir settings
 // (e.g. on debian config points to /etc/kde3) when calling addPrefix(). work
 // around this.
#if !BOSON_LINK_STATIC
 QStringList config = KGlobal::dirs()->resourceDirs("config");
 KGlobal::dirs()->addPrefix(BOSON_PREFIX);
 QStringList config2 = KGlobal::dirs()->resourceDirs("config");
 for (unsigned int i = 0; i < config.count(); i++) {
	if (!config2.contains(config[i])) {
		KGlobal::dirs()->addResourceDir("config", config[i]);
	}
 }
#else
 // AB: for static binaries we don't care about system settings of KDEDIR[S].
 //     all data that belongs to boson are in the same directory where the
 //     static boson binary resides.
 //     note that BOSON_PREFIX needs to be ignored as well, as nothing will get
 //     installed there!
 QString applicationDirPath = fixedApplicationDirPath(argv0);
 qDebug("Using prefix: %s", applicationDirPath.latin1()); // do NOT use boDebug() here. we need to have the prefix before using that
 KGlobal::dirs()->addPrefix(applicationDirPath);
#endif

 if (enableGUI) {
	if (!boglResolveGLSymbols()) {
//		boError() << k_funcinfo << "GL/GLU/GLX symbols could not be resolved" << endl;
		qDebug("ERROR: GL/GLU/GLX symbols could not be resolved");
	}
 }

 BoGlobal::initStatic();
 BoGlobal::boGlobal()->initGlobalObjects();
 BoImageIO::init();
}

BoApplication::~BoApplication()
{
 if (BoGlobal::boGlobal()) {
	BoGlobal::boGlobal()->destroyGlobalObjects();
 }
}

QString fixedApplicationDirPath(const QCString& _argv0)
{
 // AB: code has been shamelessy stolen from
 // QApplication::applicationFilePath()/applicationDirPath().
 QString applicationFilePath;

 QString argv0 = QFile::decodeName( _argv0 );
 QString absPath;

 if ( argv0[0] == '/' ) {
	/*
	  If argv0 starts with a slash, it is already an absolute
	  file path.
	*/
	absPath = argv0;
 } else if ( argv0.find('/') != -1 ) {
	/*
	  If argv0 contains one or more slashes, it is a file path
	  relative to the current directory.
	*/
	absPath = QDir::current().absFilePath( argv0 );
 } else {
	/*
	  Otherwise, the file path has to be determined using the
	  PATH environment variable.
	*/
	char *pEnv = getenv( "PATH" );
	QStringList paths( QStringList::split(QChar(':'), pEnv) );
	QStringList::const_iterator p = paths.begin();
	while ( p != paths.end() ) {
		QString candidate = QDir::current().absFilePath( *p + "/" + argv0 );
		if ( QFile::exists(candidate) ) {
			absPath = candidate;
			break;
		}
		++p;
	}
 }

 absPath = QDir::cleanDirPath( absPath );
 if ( QFile::exists(absPath) ) {
	applicationFilePath = resolveSymlinks( absPath );
 } else {
	applicationFilePath = QString::null;
 }

 return QFileInfo(applicationFilePath).dirPath();
}


QString resolveSymlinks( const QString& path, int depth )
{
 bool foundLink = FALSE;
 QString linkTarget;
 QString part = path;
 int slashPos = path.length();

 // too deep; we give up
 if ( depth == 128 )
	return QString::null;

 do {
	part = part.left( slashPos );
	QFileInfo fileInfo( part );
	if ( fileInfo.isSymLink() ) {
		foundLink = TRUE;
		linkTarget = fileInfo.readLink();
		break;
	}
 } while ( (slashPos = part.findRev('/')) != -1 );

 if ( foundLink ) {
	QString path2;
	if ( linkTarget[0] == '/' ) {
		path2 = linkTarget;
		if ( slashPos < (int) path.length() ) {
			path2 += "/" + path.right( path.length() - slashPos - 1 );
		}
	} else {
		QString relPath;
		relPath = part.left( part.findRev('/') + 1 ) + linkTarget;
		if ( slashPos < (int) path.length() ) {
			if ( !linkTarget.endsWith( "/" ) ) {
				relPath += "/";
			}
		relPath += path.right( path.length() - slashPos - 1 );
		}
		path2 = QDir::current().absFilePath( relPath );
	}
	path2 = QDir::cleanDirPath( path2 );
	return resolveSymlinks( path2, depth + 1 );
 } else {
	return path;
 }
}
