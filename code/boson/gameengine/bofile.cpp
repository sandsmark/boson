/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "bofile.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "defines.h"

#include <qfileinfo.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <ktar.h>

BoFile::BoFile(const QString& file, bool readOnly)
{
 mTar = new KTar(file, QString::fromLatin1("application/x-gzip"));
 // by default we keep the file open until destruction!!
 //
 // AB: open() parses the directory and file entries, but it does NOT read the
 // entire files.
 if (readOnly) {
	mTar->open(QIODevice::ReadOnly);
 } else {
	mTar->open(QIODevice::WriteOnly);
 }
}

BoFile::~BoFile()
{
 if (mTar) {
	mTar->close();
 }
 delete mTar;
}

bool BoFile::checkTar() const
{
 if (!mTar) {
	BO_NULL_ERROR(mTar);
	return false;
 }
 // if we can't access the file it'll not be open.
 if (!mTar->isOpen()) {
	boWarning() << k_funcinfo << "Archive not open" << endl;
	return false;
 }
 const KArchiveDirectory* dir = mTar->directory();
 if (!dir) {
	return false;
 }
 QStringList entries = dir->entries();
 if (entries.count() < 1) {
	boWarning() << k_funcinfo << "not a valid file (no toplevel entry)" << endl;
	return false;
 }
 if (entries.count() > 1) {
	boWarning() << k_funcinfo << "not a valid file (multiple toplevel entries)" << endl;
	return false;
 }
 if (!dir->entry(entries[0])->isDirectory()) {
	boWarning() << k_funcinfo << "not a valid file (toplevel entry is not a directory)" << endl;
	return false;
 }
 return true;
}

QString BoFile::fileName() const
{
 if (!mTar) {
	return QString();
 }
 return mTar->fileName();
}

const KArchiveDirectory* BoFile::topLevelDir() const
{
 if (!mTar) {
	return 0;
 }
 const KArchiveDirectory* dir = mTar->directory();
 if (!dir) {
	return 0;
 }
 QStringList entries = dir->entries();
 if (entries.count() != 1) {
	return 0;
 }
 if (!dir->entry(entries[0])->isDirectory()) {
	return 0;
 }
 return (const KArchiveDirectory*)mTar->directory()->entry(entries[0]);
}

QByteArray BoFile::fileData(const QString& fileName, const QString& subdir) const
{
 QByteArray b;
 if (!mTar) {
	return b;
 }
 if (!hasFile(fileName, subdir)) {
	return b;
 }
 const KArchiveDirectory* topLevel = topLevelDir();
 if (!topLevel) {
	boError() << k_funcinfo << "Oops - NULL topLevelDir??" << endl;
	return b;
 }
 const KArchiveDirectory* dir = 0;
 if (subdir.isEmpty()) {
	dir = topLevel;
 } else {
	// AB: a directory like "foo\/bar" is not allowed by us. I believe this
	// is only a very minor restriction :)
	QStringList dirs = subdir.split('/');
	QStringList::iterator it;

	const KArchiveDirectory* currentDir = topLevel;
	for (it = dirs.begin(); it != dirs.end(); ++it) {
		const KArchiveEntry* entry = currentDir->entry(*it);
		if (!entry || !entry->isDirectory()) {
			boError() << k_funcinfo << *it << " part of " << subdir << " is not a directory" << endl;
			return b;
		} else {
			currentDir = (const KArchiveDirectory*)entry;
		}
	}
	dir = currentDir;
 }
 if (!dir) {
	boError() << k_funcinfo << "Cannot find subdir " << subdir << endl;
	return b;
 }
 const KArchiveEntry* file = dir->entry(fileName);
 if (!file) {
	boError() << k_funcinfo << "Cannot find " << fileName << endl;
	return b;
 }
 if (!file->isFile()) {
	boError() << k_funcinfo << fileName << " is not a file" << endl;
	return b;
 }
 b = ((const KArchiveFile*)file)->data();
 return b;
}

bool BoFile::hasEntry(const QString& file, const QString& subdir, bool isFile) const
{
 const KArchiveDirectory* topLevel = topLevelDir();
 if (!topLevel) {
	return false;
 }
 const KArchiveDirectory* dir = 0;
 if (subdir.isNull()) {
	dir = topLevel;
 } else {
	const KArchiveEntry* e = topLevel->entry(subdir);
	if (!e) {
		return false;
	}
	if (!e->isDirectory()) {
		boError() << k_funcinfo << subdir << " exists, but is not a directory" << endl;
		return false;
	} else {
		dir = (const KArchiveDirectory*)e;
	}
 }
 if (!dir) {
	return false;
 }

 const KArchiveEntry* entry = dir->entry(file);
 if (!entry) {
	return false;
 }
 if (isFile) {
	return entry->isFile();
 } else {
	return entry->isDirectory();
 }
 return true;
}

bool BoFile::writeFile(const QString& fileName, const QByteArray& data, const QString& subdir)
{
 return writeFile(topDirName(), fileName, data.size(), data.data(), subdir);
}

bool BoFile::writeFile(const QString& fileName, const QString& data, const QString& subdir)
{
 QByteArray d = data.toUtf8();
 return writeFile(topDirName(), fileName, data.length(), d.constData(), subdir);
}

bool BoFile::writeFile(const QString& topDir, const QString& fileName, qint64 size, const char* data, const QString& subdir)
{
 if (!mTar) {
	BO_NULL_ERROR(mTar);
	return false;
 }
 const KArchiveDirectory* root = mTar->directory();
 if (!root) {
	boError() << k_funcinfo << "NULL root dir??" << endl;
	return false;
 }
 if (root->entries().count() != 0) {
	if (root->entries().count() > 1) {
		boError() << k_funcinfo << "more than one toplevel entry" << endl;
		return false;
	}
	if (root->entries()[0] != topDir) {
		boError() << k_funcinfo << "You cannot use two different toplevel dirs!" << endl;
		return false;
	}
 }
 QString user = root->user();
 QString group = root->group();
 QString file;
 if (subdir.isEmpty()) {
	file = QString::fromLatin1("%1/%2").arg(topDir).arg(fileName);
 } else {
	file = QString::fromLatin1("%1/%2/%3").arg(topDir).arg(subdir).arg(fileName);
 }
 return mTar->writeFile(file, user, group, data, size);
}

QString BoFile::topDirName() const
{
 QFileInfo f(fileName());
 QString top = f.fileName();
 if (top.right(7) == QString::fromLatin1(".tar.gz")) {
	// might be the case for debugging
	top = top.left(top.length() - 7);
 }
 if (top.lastIndexOf('.') > 0) {
	top.truncate(top.lastIndexOf('.'));
 }
 return top;
}



BPFFile::BPFFile(const QString& file, bool readOnly) : BoFile(file, readOnly)
{
 mIdentifier = fileNameToIdentifier(fileName());
}

BPFFile::~BPFFile()
{
}

bool BPFFile::checkTar() const
{
 if (!BoFile::checkTar()) {
	return false;
 }
 const KArchiveDirectory* top = topLevelDir();
 if (!top) {
	boWarning() << k_funcinfo << "NULL topLevelDir dir" << endl;
	return false;
 }

 return true;
}

QMap<QString, QByteArray> BPFFile::descriptionsData() const
{
 QMap<QString, QByteArray> descriptions;
 if (!hasFile(QString::fromLatin1("description.xml"), QString::fromLatin1("C"))) {
	// the default locale must always be present.
	boError() << k_funcinfo << "no description.xml for default language \"C\" found in map!" << endl;
	return descriptions;
 }

 const KArchiveDirectory* top = topLevelDir();
 QStringList entries = top->entries();
 QStringList::iterator it;
 for (it = entries.begin(); it != entries.end(); ++it) {
	if (!hasDirectory(*it)) {
		continue;
	}
	if (hasFile(QString::fromLatin1("description.xml"), *it)) {
		QByteArray b = fileData(QString::fromLatin1("description.xml"), *it);
		descriptions.insert(QString::fromLatin1("%1/description.xml").arg(*it), b);
	}
 }

 return descriptions;
}

QMap<QString, QByteArray> BPFFile::scriptsData() const
{
 QMap<QString, QByteArray> scripts;

 if (!hasDirectory("scripts")) {
	return scripts;
 }
 typedef QPair<QString, const KArchiveDirectory*> MyPair;
 Q3ValueList<MyPair> dirs;
 dirs.append(MyPair("scripts", (KArchiveDirectory*)topLevelDir()->entry("scripts")));
 while (!dirs.isEmpty()) {
	QPair<QString, const KArchiveDirectory*> pair = dirs[0];
	dirs.pop_front();

	QString dirName = pair.first;
	const KArchiveDirectory* dir = pair.second;

	QStringList entries = dir->entries();
	QStringList::iterator it;
	for (it = entries.begin(); it != entries.end(); ++it) {
		const KArchiveEntry* e = dir->entry(*it);
		if (!e) {
			continue;
		}
		QString entryName = dirName + "/" + *it;
		if (e->isFile()) {
			const KArchiveFile* file = (const KArchiveFile*)e;
			scripts.insert(entryName, file->data());
		} else if (e->isDirectory()) {
			const KArchiveDirectory* d = (const KArchiveDirectory*)e;
			dirs.append(MyPair(entryName, d));
		}
	}
 }

 return scripts;
}

QMap<QString, QByteArray> BPFFile::eventListenerData() const
{
 QMap<QString, QByteArray> eventListener;

 if (!hasDirectory("eventlistener")) {
	return eventListener;
 }
 typedef QPair<QString, const KArchiveDirectory*> MyPair;
 Q3ValueList<MyPair> dirs;
 dirs.append(MyPair("eventlistener", (KArchiveDirectory*)topLevelDir()->entry("eventlistener")));
 while (!dirs.isEmpty()) {
	QPair<QString, const KArchiveDirectory*> pair = dirs[0];
	dirs.pop_front();

	QString dirName = pair.first;
	const KArchiveDirectory* dir = pair.second;

	QStringList entries = dir->entries();
	QStringList::iterator it;
	for (it = entries.begin(); it != entries.end(); ++it) {
		const KArchiveEntry* e = dir->entry(*it);
		if (!e) {
			continue;
		}
		QString entryName = dirName + "/" + *it;
		if (e->isFile()) {
			const KArchiveFile* file = (const KArchiveFile*)e;
			eventListener.insert(entryName, file->data());
		} else if (e->isDirectory()) {
			const KArchiveDirectory* d = (const KArchiveDirectory*)e;
			dirs.append(MyPair(entryName, d));
		}
	}
 }

 return eventListener;
}

QString BPFFile::fileNameToIdentifier(const QString& fileName)
{
 QFileInfo fileInfo(fileName);
 return fileInfo.fileName();
}

