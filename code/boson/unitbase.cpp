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

#include <qrect.h>
#include <qptrlist.h>


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
};


UnitBase::UnitBase(const UnitProperties* prop)
{
 d = new UnitBasePrivate;
 mOwner = 0;
 mUnitProperties = prop;

// PolicyLocal?
 mHealth.registerData(IdHealth, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Health");
 d->mArmor.registerData(IdArmor, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Armor");
 d->mShields.registerData(IdShields, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Shields");
 d->mId.registerData(IdId, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ID"); // perhaps use dataHandler()->id() instead
 mRange.registerData(IdRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Range");
 mSightRange.registerData(IdSightRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "SightRange");
 mDamage.registerData(IdDamage, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Damage");
 mWork.registerData(IdWork, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Work");
 d->mDeletionTimer.registerData(IdDeletionTimer, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "DeletionTimer");
 d->mDeletionTimer.setEmittingSignal(false);


 mWork.setLocal((int)WorkNone);
 mHealth.setLocal(0); // initially destroyed
 d->mShields.setLocal(0); // doesn't have any shields
 d->mArmor.setLocal(0); // doesn't have any armor
 d->mId.setLocal(0);
 mDamage.setLocal(0);
 mRange.setLocal(0);
 mSightRange.setLocal(0);
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

inline int UnitBase::type() const
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
 mUnitProperties = speciesTheme()->unitProperties(typeId); // FIXME: make sure that speciesTheme() is != 0!!!
 bool ret = dataHandler()->load(stream);
 return ret;
}

inline SpeciesTheme* UnitBase::speciesTheme() const
{
 if (!owner()) {
	kdWarning() << k_funcinfo << ": NULL owner" << endl;
	return 0;
 }
 return owner()->speciesTheme();
}

inline bool UnitBase::isFacility() const
{
 return unitProperties()->isFacility();
}

inline bool UnitBase::isMobile() const
{
 return unitProperties()->isMobile();
}

inline bool UnitBase::isFlying() const
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
