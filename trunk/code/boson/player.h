/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <kgame/kplayer.h>

#include <qcolor.h>

class QCanvasPixmapArray;
class Unit;
class SpeciesTheme;
class UnitProperties;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Player : public KPlayer
{
	Q_OBJECT
public:
	Player();
	virtual ~Player();

	/**
	 * @return A @ref QCanvasPixmapArray for the specified unit (see @ref
	 * UnitProperties::typeId) of the species of this player
	 **/
	QCanvasPixmapArray* pixmapArray(int unitType) const;

	void loadTheme(const QString& species, const QRgb& teamColor);

	void addUnit(Unit* unit);
	void unitDestroyed(Unit* unit);
	SpeciesTheme* speciesTheme() const { return mSpecies; }

	Unit* findUnit(unsigned long int unitId) const;

	virtual bool load(QDataStream& stream);
	virtual bool save(QDataStream& stream);

	/**
	 * @return <em>All</em> units of this player. Please don't use this as
	 * it is very unclean. This is meant for KGameUnitDebug only.
	 **/
	QPtrList<Unit> allUnits() const;

	/**
	 * Called by @ref Unit to sync the positions of the units on
	 * several clients. Shouldn't be necessary if it was implemented
	 * cleanly!!
	 **/
	void sendStopMoving(Unit* unit);

	/**
	 * Convenience method for theme()->unitProperties()
	 **/
	const UnitProperties* unitProperties(int unitType) const;

signals:
	void signalCreateUnit(Unit*& unit, int unitType, Player* owner); // obsolete
	void signalLoadUnit(int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(Unit* unit);

public slots:
	void slotUnitPropertyChanged(KGamePropertyBase* prop);
	void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

private:
	class PlayerPrivate;
	PlayerPrivate* d;

	SpeciesTheme* mSpecies;
};

#endif
