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

#include <qptrlist.h>

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

	const QPtrList<BosonEffectProperties>* destroyedEffectProperties() const
	{
		return &mDestroyedEffectProperties;
	}
	const QPtrList<BosonEffectProperties>* constructedEffectProperties() const
	{
		return &mConstructedEffectProperties;
	}
	const QPtrList<BosonEffectProperties>* explodingFragmentFlyEffects() const
	{
		return &mExplodingFragmentFlyEffects;
	}
	const QPtrList<BosonEffectProperties>* explodingFragmentHitEffects() const
	{
		return &mExplodingFragmentHitEffects;
	}

private:
	QPtrList<BosonEffectProperties> mDestroyedEffectProperties;
	QPtrList<BosonEffectProperties> mConstructedEffectProperties;
	QPtrList<BosonEffectProperties> mExplodingFragmentFlyEffects;
	QPtrList<BosonEffectProperties> mExplodingFragmentHitEffects;
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

	const QPtrList<BosonEffectProperties>* shootEffectProperties() const
	{
		return &mShootEffectProperties;
	}
	const QPtrList<BosonEffectProperties>* flyEffectProperties() const
	{
		return &mFlyEffectProperties;
	}
	const QPtrList<BosonEffectProperties>* hitEffectProperties() const
	{
		return &mHitEffectProperties;
	}

private:
	QPtrList<BosonEffectProperties> mShootEffectProperties;
	QPtrList<BosonEffectProperties> mFlyEffectProperties;
	QPtrList<BosonEffectProperties> mHitEffectProperties;
};


class BosonEffectManagerPrivate
{
public:
	BosonEffectManagerPrivate()
	{
	}
	QIntDict<UnitPropertiesEffects> mUnitPropertiesEffects;
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

QPtrList<BosonEffect> BosonEffectManager::newEffects(unsigned int id, const BoVector3Fixed& pos, bofixed zrot) const
{
 QValueList<unsigned long int> ids;
 ids.append(id);
 QPtrList<BosonEffectProperties> effectProperties = BosonEffectProperties::loadEffectProperties(ids);
 return BosonEffectProperties::newEffects(&effectProperties, pos, BoVector3Fixed(0, 0, zrot));
}

QPtrList<BosonEffect> BosonEffectManager::newDestroyedEffects(const UnitProperties* prop, float x, float y, float z) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return QPtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->destroyedEffectProperties(), BoVector3Fixed(x, y, z));
}

QPtrList<BosonEffect> BosonEffectManager::newConstructedEffects(const UnitProperties* prop, float x, float y, float z) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return QPtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->constructedEffectProperties(), BoVector3Fixed(x, y, z));
}

QPtrList<BosonEffect> BosonEffectManager::newExplodingFragmentFlyEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return QPtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->explodingFragmentFlyEffects(), pos);
}

QPtrList<BosonEffect> BosonEffectManager::newExplodingFragmentHitEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mUnitPropertiesEffects[prop->typeId()]) {
	return QPtrList<BosonEffect>();
 }
 const UnitPropertiesEffects* e = d->mUnitPropertiesEffects[prop->typeId()];
 return BosonEffectProperties::newEffects(e->explodingFragmentHitEffects(), pos);
}



QPtrList<BosonEffect> BosonEffectManager::newShootEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos, bofixed rotation) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return QPtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->shootEffectProperties(), pos, BoVector3Fixed(0, 0, rotation));
}

QPtrList<BosonEffect> BosonEffectManager::newFlyEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos, bofixed rotation) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return QPtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->flyEffectProperties(), pos, BoVector3Fixed(0, 0, rotation));
}

QPtrList<BosonEffect> BosonEffectManager::newHitEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos) const
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return QPtrList<BosonEffect>();
 }
 if (!d->mWeaponPropertiesEffects.contains(prop)) {
	return QPtrList<BosonEffect>();
 }
 const WeaponPropertiesEffects* e = d->mWeaponPropertiesEffects[prop];
 return BosonEffectProperties::newEffects(e->hitEffectProperties(), pos);
}


