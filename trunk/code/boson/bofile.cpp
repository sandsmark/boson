/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bofile.h"

#include "bodebug.h"
#include "defines.h"

//#include <qdom.h>
#include <qfileinfo.h>

#include <ktar.h>

BoFile::BoFile(const QString& file, bool readOnly)
{
 mTar = new KTar(file, QString::fromLatin1("application/x-gzip"));
 // by default we keep the file open until destruction!!
 if (readOnly) {
	mTar->open(IO_ReadOnly);
 } else {
	mTar->open(IO_WriteOnly);
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
 if (!mTar->isOpened()) {
	boWarning() << k_funcinfo << "Archive not open" << endl;
	return false;
 }
 const KArchiveDirectory* dir = mTar->directory();
 if (!dir) {
	return false;
 }
 QStringList entries = dir->entries();
 if (entries.count() != 1) {
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
	return QString::null;
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
	// currently we support only a single subdir, i.e. "de" but not
	// "de/foobar". if we change that we could us QStringList::split() with
	// '/'. But keep in mind that directories with a slash in the name might
	// be possible!
	const KArchiveEntry* e = topLevel->entry(subdir);
	if (!e->isDirectory()) {
		boError() << k_funcinfo << subdir << " is not a directory" << endl;
		return b;
	} else {
		dir = (const KArchiveDirectory*)e;
	}
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
	if (!e->isDirectory()) {
		boError() << k_funcinfo << subdir << " is not a directory" << endl;
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
 return writeFile(topDirName(), fileName, data.length(), data.data(), subdir);
}

bool BoFile::writeFile(const QString& topDir, const QString& fileName, int size, const char* data, const QString& subdir)
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
 return mTar->writeFile(file, user, group, size, data);
}

QString BoFile::topDirName() const
{
 QFileInfo f(fileName());
 QString top = f.fileName();
 if (top.right(7) == QString::fromLatin1(".tar.gz")) {
	// might be the case for debugging
	top = top.left(top.length() - 7);
 }
 if (top.findRev('.') > 0) {
	top.truncate(top.findRev('.'));
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

QByteArray BPFFile::descriptionData() const
{
// TODO: pick locales dir e.g. for german "de" probably KLocale should be useful
// here

 QString locale;
 if (!locale.isEmpty()) {
	if (hasFile(QString::fromLatin1("description.xml"), locale)) {
		return fileData(QString::fromLatin1("description.xml"), locale);
	} else {
		boDebug() << k_funcinfo << "Could not find description.xml in " << locale << " will try C instead" << endl;
	}
 }
 if (!hasFile(QString::fromLatin1("description.xml"), QString::fromLatin1("C"))) {
	boError() << k_funcinfo << "no description.xml for default language \"C\" found in map!" << endl;
	return QByteArray();
 }
 return fileData(QString::fromLatin1("description.xml"), QString::fromLatin1("C"));
}

QString BPFFile::fileNameToIdentifier(const QString& fileName)
{
 QFileInfo fileInfo(fileName);
 return fileInfo.fileName();
}


BSGFile::BSGFile(const QString& fileName, bool readOnly) : BoFile(fileName, readOnly)
{
}

BSGFile::~BSGFile()
{
}

bool BSGFile::checkTar() const
{
 if (!BoFile::checkTar()) {
	return false;
 }
 return true;
}

