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

#include "bosoneffectmanager.h"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosoneffect.h"
#include "bosoneffectproperties.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/bosonweapon.h"

#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3ValueList>

// effects that are stored per-unittype.
class UnitPropertiesEffects
{
public:
	UnitPropertiesEffects()
	{
	}

	bool loadUnitType(const UnitProperties* prop)
	{
		mDestroyedEffectProperties = BosonEffectProperties::loadEffectProperties(prop->destroyedEffectIds());
		mConstructedEffectProperties = BosonEffectProperties::loadEffectProperties(prop->constructedEffectIds());
		mExplodingFragmentFlyEffects = BosonEffectProperties::loadEffectProperties(prop->explodingFragmentFlyEffectIds());
		mExplodingFragmentHitEffects = BosonEffectProperties::loadEffectProperties(prop->explodingFragmentHitEffectIds());


		return true;
	}

	const Q3PtrList<BosonEffectProperties>* destroyedEffectProperties() const
	{
		return &mDestroyedEffectProperties;
	}
	const Q3PtrList<BosonEffectProperties>* constructedEffectProperties() const
	{
		return &mConstructedEffectProperties;
	}
	const Q3PtrList<BosonEffectProperties>* explodingFragmentFlyEffects() const
	{
		return &mExplodingFragmentFlyEffects;
	}
	const Q3PtrList<BosonEffectProperties>* explodingFragmentHitEffects() const
	{
		return &mExplodingFragmentHitEffects;
	}

private:
	Q3PtrList<BosonEffectProperties> mDestroyedEffectProperties;
	Q3PtrList<BosonEffectProperties> mConstructedEffectProperties;
	Q3PtrList<BosonEffectProperties> mExplodingFragmentFlyEffects;
	Q3PtrList<BosonEffectProperties> mExplodingFragmentHitEffects;
};

class WeaponPropertiesEffects
{
public:
	WeaponPropertiesEffects()
	{
	}

	bool loadWeaponType(const BosonWeaponProperties* prop)
	{
		mShootEffectProperties = BosonEffectProperties::loadEffectProperties(prop->shootEffectIds());
		mFlyEffectProperties = BosonEffectProperties::loadEffectProperties(prop->flyEffectIds());
		mHitEffectProperties = BosonEffectProperties::loadEffectProperties(prop->hitEffectIds());


		return true;
	}

	const Q3PtrList<BosonEffectProperties>* shootEffectProperties() const
	{
		return &mShootEffectProperties;
	}
	const Q3PtrList<BosonEffectProperties>* flyEffectProperties() const
	{
		return &mFlyEffectProperties;
	}
	const Q3PtrList<BosonEffectProperties>* hitEffectProperties() const
	{
		return &mHitEffectProperties;
	}

private:
	Q3PtrList<BosonEffectProperties> mShootEffectProperties;
	Q3PtrList<BosonEffectProperties> mFlyEffectProperties;
	Q3PtrList<BosonEffectProperties> mHitEffectProperties;
};


class BosonEffectManagerPrivate
{
public:
	BosonEffectManagerPrivate()
	{
	}
	Q3IntDict<UnitPropertiesEffects> mUnitPropertiesEffects;
	QMap<const BosonWeaponProperties*, WeaponPropertiesEffects*> mWeaponPropertiesEffects;
};

BosonEffectManager::BosonEffectManager()
{
 d = new BosonEffectManagerPrivate;
}

BosonEffectManager::~BosonEffectManager()
{
 d->mUnitPropertiesEffects.setAutoDelete(true);
 d->mUnitPropertiesEffects.clear();
 for (QMap<const BosonWeaponProperties*, WeaponPropertiesEffects*>::iterator it = d->mWeaponPropertiesEffects.begin(); it != d->mWeaponPropertiesEffects.end(); ++it) {
	delete it.data();
 }
 d->mWeaponPropertiesEffects.clear();
 delete d;
}

bool BosonEffectManager::loadUnitType(const UnitProperties* prop)
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (d->mUnitPropertiesEffects[prop->typeId()]) {
	return true;
 }
 UnitPropertiesEffects* e = new UnitPropertiesEffects();

 if (!e->loadUnitType(prop)) {
	boError() << k_funcinfo << "unable to load effects for unit type " << prop->typeId() << endl;
	delete e;
	return false;
 }
 d->mUnitPropertiesEffects.insert(prop->typeId(), e);


 return true;
}

bool BosonEffectManager::loadWeaponType(const BosonWeaponProperties* prop)
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (d->mWeaponPropertiesEffects.contains(prop)) {
	return true;
 }
 WeaponPropertiesEffects* e = new WeaponPropertiesEffects();

 if (!e->loadWeaponType(prop)) {
	boError() << k_funcinfo << "unable to load effects for weapon " << prop << endl;
	delete e;
	return false;
 }
 d->mWeaponPropertiesEffects.insert(prop, e);

 return true;
}

Q3PtrList<BosonEffect> BosonEffectManager::newEffects(unsigned int id, const BoVector3Fixed& pos, bofixed zrot) const
{
 Q3ValueList<unsigned long int> ids;
 ids.append(id);
 Q3PtrList<BosonEffectProperties> effectProperties = BosonEffectProperties::loadEffectProperties(ids);
 return BosonEffectProperties::newEffects(&effectProperties, pos, BoVector3Fixed(0, 0, zrot));
}

Q3PtrList<BosonEffect> BosonEffectManager::newDestroyedEffects(const UnitProperties* prop, float x, float y, float z) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return Q3PtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->destroyedEffectProperties(), BoVector3Fixed(x, y, z));
}

Q3PtrList<BosonEffect> BosonEffectManager::newConstructedEffects(const UnitProperties* prop, float x, float y, float z) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return Q3PtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->constructedEffectProperties(), BoVector3Fixed(x, y, z));
}

Q3PtrList<BosonEffect> BosonEffectManager::newExplodingFragmentFlyEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return Q3PtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->explodingFragmentFlyEffects(), pos);
}

Q3PtrList<BosonEffect> BosonEffectManager::newExplodingFragmentHitEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return Q3PtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->explodingFragmentHitEffects(), pos);
}



// topLeftPos.x() == leftEdge(), topLeftPos.y() == topEdge(), topEdge.z() == z()
Q3PtrList<BosonEffect> BosonEffectManager::newShootEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& topLeftPos, bofixed rotation) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return Q3PtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->shootEffectProperties(), topLeftPos, BoVector3Fixed(0, 0, rotation));
}

Q3PtrList<BosonEffect> BosonEffectManager::newFlyEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos, bofixed rotation) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return Q3PtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->flyEffectProperties(), pos, BoVector3Fixed(0, 0, rotation));
}

Q3PtrList<BosonEffect> BosonEffectManager::newHitEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return Q3PtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return Q3PtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->hitEffectProperties(), pos);
}


