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

#include "bpffile.h"

#include "bodebug.h"
#include "../defines.h"

#include <qdom.h>
#include <qfileinfo.h>

BPFFile::BPFFile(const QString& file, bool readOnly) : KTar(file, QString::fromLatin1("application/x-gzip"))
{
 // by default we keep the file open until destruction!!
 if (readOnly) {
	open(IO_ReadOnly);
 } else {
	open(IO_WriteOnly);
 }
 QFileInfo fileInfo(fileName());
 mIdentifier = fileInfo.fileName();
}

BPFFile::~BPFFile()
{
 close();
}

bool BPFFile::checkTar() const
{
 const KArchiveDirectory* dir = directory();
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
 const KArchiveDirectory* top = (const KArchiveDirectory*)dir->entry(entries[0]);
 entries = top->entries();
 if (!entries.contains(QString::fromLatin1("map.xml"))) {
	boError() << k_funcinfo << "can't find map.xml" << endl;
	return false;
 }
 if (!entries.contains(QString::fromLatin1("C"))) { // default language
	boError() << k_funcinfo << "can't find C" << endl;
	return false;
 }
 return true;
}

QByteArray BPFFile::descriptionData() const
{
 const KArchiveDirectory* dir = (const KArchiveDirectory*)directory()->entry(directory()->entries()[0]);
 if (!dir) {
	boError() << k_funcinfo << "Oops - NULL dir??" << endl;
	return QByteArray();
 }
 QStringList entries = dir->entries();
 const KArchiveDirectory* localeDir = 0;
// TODO: pick locales dir e.g. for german "de" probably KLocale should be useful
// here

 if (!localeDir) {
	if (!entries.contains(QString::fromLatin1("C"))) {
		boError() << k_funcinfo << "no default language found in map!" << endl;
		return QByteArray();
	}
	const KArchiveEntry* e = dir->entry(QString::fromLatin1("C"));
	if (!e->isDirectory()) {
		boError() << k_funcinfo << "default language entry is not a directory" << endl;
		return QByteArray();
	}
	localeDir = ((const KArchiveDirectory*)e);
 }
 const KArchiveEntry* desc = localeDir->entry(QString::fromLatin1("description.xml"));
 if (!desc) {
	boError() << k_funcinfo << "NULL description.xml in language directory " << localeDir->name() << endl;
	return QByteArray();
 }
 if (!desc->isFile()) {
	boError() << k_funcinfo << "description.xml in language directory " << localeDir->name() << " is not a file" << endl;
	return QByteArray();
 }
 return ((const KArchiveFile*)desc)->data();
}

QByteArray BPFFile::fileData(const QString& fileName) const
{
 QByteArray b;
 const KArchiveDirectory* dir = (const KArchiveDirectory*)directory()->entry(directory()->entries()[0]);
 if (!dir) {
	boError() << k_funcinfo << "Oops - NULL dir??" << endl;
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

