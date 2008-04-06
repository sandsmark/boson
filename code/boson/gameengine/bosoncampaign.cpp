/*
    This file is part of the Boson game
    Copyright (C) 2003-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosoncampaign.h"

#include "../bomemory/bodummymemory.h"
#include "bosonplayfield.h"
#include "bosondata.h"
#include "bodebug.h"
#include "bpfloader.h" // BPFPreview TODO: dedicated file

#include <q3ptrlist.h>
#include <qstringlist.h>

class BosonCampaignData : public BosonDataObject
{
public:
	BosonCampaignData(const QString& identifier, BosonCampaign* campaign)
		: BosonDataObject(identifier)
	{
		mCampaign = campaign;
		mId = identifier;
	}
	virtual ~BosonCampaignData()
	{
		delete mCampaign;
	}
	virtual QString idString() const
	{
		return mId;
	}
	virtual void* pointer() const
	{
		return (void*)campaign();
	}
	BosonCampaign* campaign() const
	{
		return mCampaign;
	}
	virtual bool load()
	{
		return true;
	}

private:
	BosonCampaign* mCampaign;
	QString mId;
};


class BosonCampaignPrivate
{
public:
	BosonCampaignPrivate()
	{
	}
	Q3PtrList<BPFPreview> mPlayFields;
	QString mName;
	QString mIdentifier;
};

BosonCampaign::BosonCampaign(const QString& identifier, const QString& name)
{
 d = new BosonCampaignPrivate;
 d->mName = name;
 d->mIdentifier = identifier;
 if (d->mIdentifier.isNull()) {
	// null strings are invalid in QMaps, so we use an empty string
	d->mIdentifier = QString::fromLatin1("");
 }
}

BosonCampaign::~BosonCampaign()
{
 d->mPlayFields.clear();
 delete d;
}

void BosonCampaign::addPlayField(BPFPreview* p)
{
 if (!p) {
	return;
 }
 d->mPlayFields.append(p);
}

void BosonCampaign::removePlayField(BPFPreview* p)
{
 if (!p) {
	return;
 }
 d->mPlayFields.removeRef(p);
}

unsigned int BosonCampaign::playFieldCount() const
{
 return d->mPlayFields.count();
}

BosonDataObject* BosonCampaign::campaignDataObject(BosonCampaign* campaign)
{
 BO_CHECK_NULL_RET0(campaign);
 BosonCampaignData* data = new BosonCampaignData(campaign->identifier(), campaign);
 return data;
}

QString BosonCampaign::name() const
{
 return d->mName;
}

QString BosonCampaign::identifier() const
{
 return d->mIdentifier;
}

QStringList BosonCampaign::playFields() const
{
 QStringList list;
 Q3PtrListIterator<BPFPreview> it(d->mPlayFields);
 for (; it.current(); ++it) {
	list.append(it.current()->identifier());
 }
 return list;
}

