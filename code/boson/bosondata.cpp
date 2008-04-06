/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosondata.h"

#include "../bomemory/bodummymemory.h"
#include "boglobal.h"
#include "bodebug.h"

#include <k3staticdeleter.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <q3dict.h>

static BoGlobalObject<BosonData> globalDataObject(BoGlobalObjectBase::BoGlobalData);

BosonData* BosonData::mBosonData = 0;
static K3StaticDeleter<BosonData> sd;

class BosonDataPrivate
{
public:
	BosonDataPrivate()
	{
	}

	Q3Dict<BosonDataObject> mGroundThemes;
	Q3Dict<BosonDataObject> mPlayFieldPreviews;
	Q3Dict<BosonDataObject> mCampaigns;
};

BosonData::BosonData()
{
 d = new BosonDataPrivate;
 d->mGroundThemes.setAutoDelete(true);
 d->mPlayFieldPreviews.setAutoDelete(true);
 d->mCampaigns.setAutoDelete(true);
}

BosonData::~BosonData()
{
 clearData();
 delete d;
}

void BosonData::clearData()
{
 d->mCampaigns.clear();
 d->mPlayFieldPreviews.clear();
 d->mGroundThemes.clear();
}

BosonData* BosonData::bosonData()
{
 return BoGlobal::boGlobal()->bosonData();
}

QStringList BosonData::availableFiles(const QString& searchPattern, bool recursive)
{
 return KGlobal::dirs()->findAllResources("data", QString::fromLatin1("boson/") + searchPattern, KStandardDirs::Recursive);
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
 Q3DictIterator<BosonDataObject> it(d->mGroundThemes);
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

bool BosonData::insertPlayFieldPreview(BosonDataObject* preview)
{
 if (!preview) {
	return true;
 }
 if (d->mPlayFieldPreviews[preview->idString()]) {
	return false;
 }
 d->mPlayFieldPreviews.insert(preview->idString(), preview);
 return true;
}

BPFPreview* BosonData::playFieldPreview(const QString& id) const
{
 if (!d->mPlayFieldPreviews[id]) {
	return 0;
 }
 return (BPFPreview*)d->mPlayFieldPreviews[id]->pointer();
}

QStringList BosonData::availablePlayFields() const
{
 QStringList list;
 Q3DictIterator<BosonDataObject> it(d->mPlayFieldPreviews);
 for (; it.current(); ++it) {
	list.append(it.currentKey());
 }
 return list;
}


bool BosonData::insertCampaign(BosonDataObject* campaign)
{
 if (!campaign) {
	return true;
 }
 if (d->mCampaigns[campaign->idString()]) {
	return false;
 }
 d->mCampaigns.insert(campaign->idString(), campaign);
 return true;
}

BosonCampaign* BosonData::campaign(const QString& id) const
{
 if (!d->mCampaigns[id]) {
	return 0;
 }
 return (BosonCampaign*)d->mCampaigns[id]->pointer();
}

QStringList BosonData::availableCampaigns() const
{
 QStringList list;
 Q3DictIterator<BosonDataObject> it(d->mCampaigns);
 for (; it.current(); ++it) {
	list.append(it.currentKey());
 }
 return list;
}

QString BosonData::locateDataFile(const QString& fileName) const
{
 return KStandardDirs::locate("data", fileName);
}

