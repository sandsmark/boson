/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosondata.h"

#include "boglobal.h"
#include "bodebug.h"

#include <kstaticdeleter.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qdict.h>

static BoGlobalObject<BosonData> globalDataObject(BoGlobalObjectBase::BoGlobalData);

BosonData* BosonData::mBosonData = 0;
static KStaticDeleter<BosonData> sd;

class BosonDataPrivate
{
public:
	BosonDataPrivate()
	{
	}

	QDict<BosonDataObject> mGroundThemes;
	QDict<BosonDataObject> mPlayFields;
};

BosonData::BosonData()
{
 d = new BosonDataPrivate;
 d->mGroundThemes.setAutoDelete(true);
 d->mPlayFields.setAutoDelete(true);
}

BosonData::~BosonData()
{
 d->mPlayFields.clear();
 d->mGroundThemes.clear();
 delete d;
}

BosonData* BosonData::bosonData()
{
 return BoGlobal::boGlobal()->bosonData();
}

QStringList BosonData::availableFiles(const QString& searchPattern)
{
 return KGlobal::dirs()->findAllResources("data", QString::fromLatin1("boson/") + searchPattern);
}

bool BosonData::insertGroundTheme(BosonDataObject* theme)
{
 if (!theme) {
	return true;
 }
 if (d->mGroundThemes[theme->idString()]) {
	return false;
 }
 d->mGroundThemes.insert(theme->idString(), theme);
 return true;
}

BosonGroundTheme* BosonData::groundTheme(const QString& id) const
{
 if (!d->mGroundThemes[id]) {
	return 0;
 }
 return (BosonGroundTheme*)d->mGroundThemes[id]->pointer();
}

QStringList BosonData::availableGroundThemes() const
{
 QStringList list;
 QDictIterator<BosonDataObject> it(d->mGroundThemes);
 for (; it.current(); ++it) {
	list.append(it.currentKey());
 }
 return list;
}

bool BosonData::loadGroundTheme(const QString& id)
{
 if (!d->mGroundThemes[id]) {
	boWarning() << k_funcinfo << "no ground theme with id=" << id << endl;
	return false;
 }
 return d->mGroundThemes[id]->load();
}

bool BosonData::insertPlayField(BosonDataObject* field)
{
 if (!field) {
	return true;
 }
 if (d->mPlayFields[field->idString()]) {
	return false;
 }
 d->mPlayFields.insert(field->idString(), field);
 return true;
}

BosonPlayField* BosonData::playField(const QString& id) const
{
 if (!d->mPlayFields[id]) {
	return 0;
 }
 return (BosonPlayField*)d->mPlayFields[id]->pointer();
}

QStringList BosonData::availablePlayFields() const
{
 QStringList list;
 QDictIterator<BosonDataObject> it(d->mPlayFields);
 for (; it.current(); ++it) {
	list.append(it.currentKey());
 }
 return list;
}

bool BosonData::loadPlayField(const QString& id)
{
 if (!d->mPlayFields[id]) {
	boWarning() << k_funcinfo << "no playField with id=" << id << endl;
	return false;
 }
 return d->mPlayFields[id]->load();
}

