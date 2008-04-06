/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOSONPLAYERINPUTHANDLER_H
#define BOSONPLAYERINPUTHANDLER_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

class QDataStream;
class Player;
class Boson;
class Unit;
class UnitOrder;
class BosonCanvas;
class BosonMessageEditorMove;
class BosonMessageEditorMoveDeleteItems;
class bofixed;
template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

template<class T> class Q3PtrStack;
template<class T> class Q3PtrList;
template<class T> class Q3ValueList;
template<class T> class Q3ValueVector;

/**
 * @short Helper class for @ref Boson
 * This class handles player input in boson. Here we handle <em>received</em>
 * input only, sending player input is handled elsewhere.
 *
 * When a player sends a player input message using @ref KGameIO::sendInput it
 * is transmitted through network and delivered to @ref Boson::playerInput which
 * forwards it to this class.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayerInputHandler : public QObject
{
	Q_OBJECT
public:
	BosonPlayerInputHandler(Boson* game);
	~BosonPlayerInputHandler();

	bool playerInput(QDataStream& stream, Player* player);

protected:
	/**
	 * Convenience method for mGame->findUnit(id, searchIn)
	 **/
	Unit* findUnit(unsigned long int id, Player* searchIn) const;

	/**
	 * Convenience method for (Player*)mGame->findPlayerByKGameId(id)
	 **/
	Player* findPlayerByKGameId(unsigned long int id) const;

	/**
	 * Convenience method for (Player*)mGame->findPlayerByUserId(id)
	 **/
	Player* findPlayerByUserId(int id) const;

	BosonCanvas* canvas() const;

	/**
	 * WARNING: return value differs from @ref playerInput!
	 * @return TRUE if the message was processed in here, otherwise FALSE
	 **/
	bool gamePlayerInput(quint32 msgid, QDataStream& stream, Player* player);

	/**
	 * WARNING: return value differs from @ref playerInput!
	 * @return TRUE if the message was processed in here, otherwise FALSE
	 **/
	bool editorPlayerInput(quint32 msgid, QDataStream& stream, Player* player);

	void editorDeleteItems(const Q3ValueList<Q_ULONG>& items);
	Unit* editorPlaceUnitAtTopLeftPos(quint32 owner, quint32 unitType, const BoVector2Fixed& pos, const bofixed& rotation);

	void editorChangeHeight(const Q3ValueVector<quint32>& cornersX, const Q3ValueVector<quint32>& cornersY, const Q3ValueVector<bofixed>& heights);

	BosonMessageEditorMove* createNewUndoDeleteItemsMessage(const BosonMessageEditorMoveDeleteItems&) const;

	void giveOrder(const Q3PtrList<Unit>& units, const UnitOrder& order, bool replace = true);
	void giveOrder(Unit* unit, const UnitOrder& order, bool replace = true);

signals:
	void signalChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha);
	void signalChangeHeight(int x, int y, float height);
	void signalEditorNewUndoMessage(const BosonMessageEditorMove&, bool fromRedo);
	void signalEditorNewRedoMessage(const BosonMessageEditorMove&);

private:
	Boson* mGame;
};

#endif
