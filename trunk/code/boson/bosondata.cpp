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

#include "bodebug.h"

#include <kstaticdeleter.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qdict.h>

BosonData* BosonData::mBosonData = 0;
static KStaticDeleter<BosonData> sd;

class BosonDataPrivate
{
public:
	BosonDataPrivate()
	{
	}

	QDict<BosonDataObject> mGroundThemes;
};

void BosonData::initBosonData()
{
 if (mBosonData) {
	return;
 }
 sd.setObject(mBosonData, new BosonData());
}

BosonData::BosonData()
{
 d = new BosonDataPrivate;
 d->mGroundThemes.setAutoDelete(true);
}

BosonData::~BosonData()
{
 delete d;
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

