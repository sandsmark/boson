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
#include "player.h"
#include "speciestheme.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>

#include <kdebug.h>

class UnitBase::UnitBasePrivate
{
public:
	UnitBasePrivate()
	{
	}

	KGamePropertyHandler mProperties;

	KGameProperty<unsigned long int> mArmor;
	KGameProperty<unsigned long int> mShields;
	KGameProperty<unsigned long int> mId; // is a KGameProperty clever here?
	KGameProperty<unsigned int> mDeletionTimer;
	KGameProperty<unsigned int> mReloadState;
};


UnitBase::UnitBase(const UnitProperties* prop)
{
 d = new UnitBasePrivate;
 d->mProperties.setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 mOwner = 0;
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here

// PolicyLocal?
 mHealth.registerData(IdHealth, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Health");
 d->mArmor.registerData(IdArmor, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Armor");
 d->mShields.registerData(IdShields, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Shields");
 d->mId.registerData(IdId, dataHandler(), 
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
 d->mReloadState.registerData(IdReloadState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ReloadState");
 d->mDeletionTimer.registerData(IdDeletionTimer, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "DeletionTimer");
 d->mDeletionTimer.setEmittingSignal(false);


 mWork.setLocal((int)WorkNone);
 mAdvanceWork.setLocal((int)WorkNone);
 mHealth.setLocal(0); // initially destroyed
 d->mShields.setLocal(0); // doesn't have any shields
 d->mArmor.setLocal(0); // doesn't have any armor
 d->mId.setLocal(0);
 mWeaponDamage.setLocal(0);
 mWeaponRange.setLocal(0);
 mSightRange.setLocal(0);
 d->mReloadState.setLocal(0);
 d->mDeletionTimer.setLocal(0);
}

UnitBase::~UnitBase()
{
// kdDebug() << k_funcinfo << endl;
 dataHandler()->clear();
 delete d;
// kdDebug() << k_funcinfo << " done" << endl;
}


const QString& UnitBase::name() const
{
 return unitProperties()->name();
}

unsigned long int UnitBase::shields() const
{
 return d->mShields;
}

unsigned long int UnitBase::armor() const
{
 return d->mArmor;
}

void UnitBase::setId(unsigned long int id)
{
 d->mId = id;
}

unsigned long int UnitBase::id() const
{
 return d->mId;
}

void UnitBase::setArmor(unsigned long int a)
{
 d->mArmor = a;
}

void UnitBase::setShields(unsigned long int s)
{
 d->mShields = s;
}

KGamePropertyHandler* UnitBase::dataHandler() const
{
 return &d->mProperties;
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
 stream << (Q_INT32)unitProperties()->typeId();
 bool ret = dataHandler()->save(stream);
 return ret;
}

bool UnitBase::load(QDataStream& stream)
{
 Q_INT32 typeId;
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
 d->mDeletionTimer = d->mDeletionTimer + 1;
}

unsigned int UnitBase::deletionTimer() const
{
 return d->mDeletionTimer;
}

unsigned int UnitBase::reloadState() const
{
 return d->mReloadState;
}

void UnitBase::reloadWeapon()
{
 if (d->mReloadState > 0) {
	d->mReloadState = d->mReloadState - 1;
 }
}

void UnitBase::resetReload()
{
 d->mReloadState = unitProperties()->reload();
}


const PluginProperties* UnitBase::properties(int pluginType) const
{
 return unitProperties()->properties(pluginType);
}

