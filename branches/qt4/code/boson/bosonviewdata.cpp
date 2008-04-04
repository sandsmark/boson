/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonviewdata.h"
#include "bosonviewdata.moc"

#include "bodebug.h"
#include "bosonconfig.h"
#include "gameengine/speciestheme.h"
#include "speciesdata.h"
#include "gameengine/bosonitem.h"
#include "gameengine/bosongroundtheme.h"
#include "bosongroundthemedata.h"

#include <qptrlist.h>
#include <qptrdict.h>

// AB: just a pointer to the actual object. we do NOT use a static deleter or so
//     here, the static pointer exists only to provide global access.
BosonViewData* BosonViewData::mGlobalViewData = 0;


BosonItemContainer::BosonItemContainer(BosonItem* item)
{
 mItem = item;
 mItemRenderer = 0;
 mEffects = 0;
}

BosonItemContainer::~BosonItemContainer()
{
 // AB: we cannot delete them here, as the classes are defined in gameview (and
 // we must not depend on gameview - otherwise borender would have to link to
 // it, too!)
 if (mItemRenderer) {
	boError() << k_funcinfo << "itemrenderer non-NULL - probably memory leak!!" << endl;
 }
 if (mEffects) {
	boError() << k_funcinfo << "effects non-NULL - probably memory leak!!" << endl;
 }
}



class BosonViewDataPrivate
{
public:
	BosonViewDataPrivate()
	{
	}

	QPtrDict<BosonItemContainer> mItem2ItemContainer;
	QPtrList<BosonItemContainer> mAllItemContainers;

	QMap<const SpeciesTheme*, SpeciesData*> mSpeciesTheme2SpeciesData;

	QPtrList<BosonGroundThemeData> mAllGroundThemeDatas;
	QMap<const BosonGroundTheme*, BosonGroundThemeData*> mGroundTheme2GroundThemeData;

};

BosonViewData::BosonViewData(QObject* parent)
	: QObject(parent)
{
 d = new BosonViewDataPrivate;
}

BosonViewData::~BosonViewData()
{
 if (!d->mAllItemContainers.isEmpty()) {
	boWarning() << k_funcinfo << "not all item containers deleted yet!" << endl;
	while (!d->mAllItemContainers.isEmpty()) {
		BosonItemContainer* c = d->mAllItemContainers.first();
		BosonItem* item = c->item();
		if (!item) {
			BO_NULL_ERROR(item);
			delete c;
		} else {
			slotRemoveItemContainerFor(item);
		}
	}
 }
 d->mItem2ItemContainer.clear();

 d->mSpeciesTheme2SpeciesData.clear();

 d->mGroundTheme2GroundThemeData.clear();
 if (!d->mAllGroundThemeDatas.isEmpty()) {
	boDebug() << k_funcinfo << "deleting remaining groundthemedata objects" << endl;
 }
 d->mAllGroundThemeDatas.setAutoDelete(true);
 d->mAllGroundThemeDatas.clear();

 delete d;

 if (mGlobalViewData == this) {
	setGlobalViewData(0);
 }
}

void BosonViewData::slotAddItemContainerFor(BosonItem* item)
{
 BO_CHECK_NULL_RET(item);
 BosonItemContainer* c = d->mItem2ItemContainer[item];
 if (c) {
	boError() << k_funcinfo << "item " << item << " already added" << endl;
	return;
 }
 c = new BosonItemContainer(item);
 d->mAllItemContainers.append(c);
 d->mItem2ItemContainer.insert(item, c);

 emit signalItemContainerAdded(c);
}

void BosonViewData::slotRemoveItemContainerFor(BosonItem* item)
{
 BO_CHECK_NULL_RET(item);
 BosonItemContainer* c = d->mItem2ItemContainer[item];
 BO_CHECK_NULL_RET(c);
 emit signalItemContainerAboutToBeRemoved(c);

 d->mItem2ItemContainer.remove(item);
 d->mAllItemContainers.setAutoDelete(true);
 d->mAllItemContainers.removeRef(c);
}

BosonItemContainer* BosonViewData::itemContainer(BosonItem* item)
{
 BosonItemContainer* c = d->mItem2ItemContainer[item];
 return c;
}

const QPtrList<BosonItemContainer>& BosonViewData::allItemContainers() const
{
 return d->mAllItemContainers;
}

void BosonViewData::addSpeciesTheme(const SpeciesTheme* theme)
{
 BO_CHECK_NULL_RET(theme);
 if (speciesData(theme)) {
	return;
 }

 // AB: note: we do NOT take ownership! SpeciesData::clearSpeciesData() will
 //           delete objects.
 SpeciesData* data = SpeciesData::speciesData(theme->themePath());
 BO_CHECK_NULL_RET(data);
 data->addTeamColor(theme->teamColor());
 d->mSpeciesTheme2SpeciesData.insert(theme, data);
}

void BosonViewData::removeSpeciesTheme(const SpeciesTheme* theme)
{
 BO_CHECK_NULL_RET(theme);
 SpeciesData* data = speciesData(theme);
 BO_CHECK_NULL_RET(data);
 data->removeTeamColor(theme->teamColor());
 d->mSpeciesTheme2SpeciesData.remove(theme);
}

SpeciesData* BosonViewData::speciesData(const SpeciesTheme* theme) const
{
 if (!d->mSpeciesTheme2SpeciesData.contains(theme)) {
	return 0;
 }
 return d->mSpeciesTheme2SpeciesData[theme];
}

void BosonViewData::addGroundTheme(const BosonGroundTheme* theme)
{
 BO_CHECK_NULL_RET(theme);
 if (groundThemeData(theme)) {
	return;
 }

 BosonGroundThemeData* data = new BosonGroundThemeData();
 if (!data->loadGroundTheme(theme)) {
	boError() << k_funcinfo << "unable to load ground theme " << theme->identifier() << " " << theme << endl;
	delete data;
	return;
 }
 d->mAllGroundThemeDatas.append(data); // takes ownership
 d->mGroundTheme2GroundThemeData.insert(theme, data);
}

void BosonViewData::removeGroundTheme(const BosonGroundTheme* theme)
{
 BO_CHECK_NULL_RET(theme);
 BosonGroundThemeData* data = groundThemeData(theme);
 BO_CHECK_NULL_RET(data);
 d->mGroundTheme2GroundThemeData.remove(theme);
 d->mAllGroundThemeDatas.setAutoDelete(true);
 d->mAllGroundThemeDatas.removeRef(data);
}

BosonGroundThemeData* BosonViewData::groundThemeData(const BosonGroundTheme* theme) const
{
 if (!d->mGroundTheme2GroundThemeData.contains(theme)) {
	return 0;
 }
 return d->mGroundTheme2GroundThemeData[theme];
}

