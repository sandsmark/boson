/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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


#include "unitbase.h"

#include "unitproperties.h"
#include "pluginproperties.h"
#include "player.h"
#include "speciestheme.h"

#include <kdebug.h>


UnitBase::UnitBase(const UnitProperties* prop)
{
 mProperties = new KGamePropertyHandler();
 mProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 mOwner = 0;
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here

// PolicyLocal?
 mHealth.registerData(IdHealth, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Health");
 mArmor.registerData(IdArmor, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Armor");
 mShields.registerData(IdShields, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Shields");
 mId.registerData(IdId, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ID"); // perhaps use dataHandler()->id() instead
 mWeaponRange.registerData(IdWeaponRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "WeaponRange");
 mSightRange.registerData(IdSightRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "SightRange");
 mWeaponDamage.registerData(IdWeaponDamage, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "WeaponDamage");
 mWork.registerData(IdWork, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Work");
 mAdvanceWork.registerData(IdAdvanceWork, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "AdvanceWork");
 mReloadState.registerData(IdReloadState, dataHandler(),
		KGamePropertyBase::PolicyLocal, "ReloadState");
 mDeletionTimer.registerData(IdDeletionTimer, dataHandler(),
		KGamePropertyBase::PolicyLocal, "DeletionTimer");
 mDeletionTimer.setEmittingSignal(false);


 mWork.setLocal((int)WorkNone);
 mAdvanceWork.setLocal((int)WorkNone);
 mHealth.setLocal(0); // initially destroyed
 mShields.setLocal(0); // doesn't have any shields
 mArmor.setLocal(0); // doesn't have any armor
 mId.setLocal(0);
 mWeaponDamage.setLocal(0);
 mWeaponRange.setLocal(0);
 mSightRange.setLocal(0);
 mReloadState.setLocal(0);
 mDeletionTimer.setLocal(0);
}

UnitBase::~UnitBase()
{
// kdDebug() << k_funcinfo << endl;
 dataHandler()->clear();
// kdDebug() << k_funcinfo << " done" << endl;
}


const QString& UnitBase::name() const
{
 return unitProperties()->name();
}

unsigned long int UnitBase::shields() const
{
 return mShields;
}

unsigned long int UnitBase::armor() const
{
 return mArmor;
}

void UnitBase::setId(unsigned long int id)
{
 mId = id;
}

unsigned long int UnitBase::id() const
{
 return mId;
}

void UnitBase::setArmor(unsigned long int a)
{
 mArmor = a;
}

void UnitBase::setShields(unsigned long int s)
{
 mShields = s;
}

KGamePropertyHandler* UnitBase::dataHandler() const
{
 return mProperties;
}

unsigned long int UnitBase::type() const
{
 return unitProperties()->typeId();
}

void UnitBase::setOwner(Player* owner)
{
 mOwner = owner;
}

bool UnitBase::save(QDataStream& stream)
{
 // TODO: we need to save and load Unit::mCurrentPlugin->pluginType() !!
 // note that multiple plugins of the same type are not *yet* supported! but
 // they might be one day..
 stream << (Q_UINT32)unitProperties()->typeId();
 bool ret = dataHandler()->save(stream);
 return ret;
}

bool UnitBase::load(QDataStream& stream)
{
 Q_UINT32 typeId;
 stream >> typeId;
 if (!speciesTheme()) {
	kdError() << k_funcinfo << "NULL speciesTheme" << endl;
	return false;
 }
 mUnitProperties = speciesTheme()->unitProperties(typeId);
 bool ret = dataHandler()->load(stream);
 return ret;
}

SpeciesTheme* UnitBase::speciesTheme() const
{
 if (!owner()) {
	kdWarning() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 return owner()->speciesTheme();
}

bool UnitBase::isFacility() const
{
 return unitProperties()->isFacility();
}

bool UnitBase::isMobile() const
{
 return unitProperties()->isMobile();
}

bool UnitBase::isFlying() const
{
 return (unitProperties() ? unitProperties()->isAircraft() : false);
}

void UnitBase::increaseDeletionTimer()
{
 mDeletionTimer = mDeletionTimer + 1;
}

unsigned int UnitBase::deletionTimer() const
{
 return mDeletionTimer;
}

unsigned int UnitBase::reloadState() const
{
 return mReloadState;
}

void UnitBase::reloadWeapon()
{
 if (mReloadState > 0) {
	mReloadState = mReloadState - 1;
 }
}

void UnitBase::resetReload()
{
 mReloadState = unitProperties()->reload();
}

const PluginProperties* UnitBase::properties(int pluginType) const
{
 return unitProperties()->properties(pluginType);
}


