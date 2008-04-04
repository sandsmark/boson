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

#ifndef BOSONEFFECTMANAGER_H
#define BOSONEFFECTMANAGER_H

#include "bo3dtools.h"

template<class T> class QPtrList;

class BosonEffect;
class BosonEffectProperties;
class UnitProperties;
class BosonWeaponProperties;

class BosonEffectManagerPrivate;
class BosonEffectManager
{
public:
	BosonEffectManager();
	~BosonEffectManager();

	bool loadUnitType(const UnitProperties* prop);
	bool loadWeaponType(const BosonWeaponProperties* prop);

	QPtrList<BosonEffect> newEffects(unsigned int id, const BoVector3Fixed& pos, bofixed zrot) const;

	QPtrList<BosonEffect> newDestroyedEffects(const UnitProperties* prop, float x, float y, float z) const;
	QPtrList<BosonEffect> newConstructedEffects(const UnitProperties* prop, float x, float y, float z) const;
	QPtrList<BosonEffect> newExplodingFragmentFlyEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const;
	QPtrList<BosonEffect> newExplodingFragmentHitEffects(const UnitProperties* prop, const BoVector3Fixed& pos) const;


	QPtrList<BosonEffect> newShootEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& topLeftPos, bofixed rotation) const;
	QPtrList<BosonEffect> newFlyEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos, bofixed rotation) const;
	QPtrList<BosonEffect> newHitEffects(const BosonWeaponProperties* prop, const BoVector3Fixed& pos) const;

private:
	BosonEffectManagerPrivate* d;
};

#endif // BOSONEFFECTMANAGER_H
