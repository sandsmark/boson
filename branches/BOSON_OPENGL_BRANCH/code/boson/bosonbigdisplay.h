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
#ifndef BOSONBIGDISPLAY_H
#define BOSONBIGDISPLAY_H

#include "bosonbigdisplaybase.h"

class KGameIO;

class Unit;
class UnitBase;
class UnitProperties;
class Player;
class KPlayer;
class KGame;
class KGameChat;
class BosonCursor;
class BoSelection;

class QLabel;

/**
 * This is the "view" of the canvas - the part of the canvas that you can
 * actually see on the screen. See @ref QCanvasView documentation for more on
 * this.
 *
 * Here you can find most of the local visual stuff. E.g. the selection rect is
 * implemented here. The messages for the network (e.g. "move unit with ID z to
 * x,y") are generated here as well as they are a direct result of the input of
 * the local player on the view. Note that these messages are <em>not</em>
 * received or read here! This happens in the class @ref Boson which is
 * responsible for all of the network stuff. The so called "playerInput" is read
 * there (which was generated here) and the unit is moved according to the
 * message (if it is valid).
 *
 * Here you should not find much of the game logic - this should be done in @ref
 * Unit or @ref Boson if possible and senseful. All you can find here should not
 * influence any other client directly.
 * @short The main view of the game
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplay : public BosonBigDisplayBase
{
	Q_OBJECT
public:
	BosonBigDisplay(BosonCanvas* c, QWidget* parent);
	virtual ~BosonBigDisplay();

	virtual void unitAction(int actionType);

//	virtual void setLocalPlayer(Player* p);
	
public slots:
	void slotMoveSelection(int cellX, int cellY);

signals:

protected:
	/**
	 * Called when the user right-clicks on the big display.
	 * 
	 * Should e.g. move a unit
	 * @param action Contains information about the mouse event (position,
	 * additional buttons, ...)
	 * @param stream The move should be placed here. A move should
	 * <em>not</em> be done in this method but rather sent to @ref KGame
	 * which performs the move on every client
	 * @param send Set to true if you actually want to send the stream
	 **/
	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send);

	// AB: I'd prefer using BoAction instead of pos!
	bool actionMine(QDataStream& stream, const QPoint& pos);
	bool actionMove(QDataStream& stream, const QPoint& pos);
	bool actionBuild(QDataStream& stream, const QPoint& pos);
	bool actionAttack(QDataStream& stream, const QPoint& pos);
	bool actionRepair(QDataStream& stream, const QPoint& pos);
	bool actionRefine(QDataStream& stream, const QPoint& pos);

	virtual void updateCursor();

	virtual bool actionLocked() const;

//	void addMouseIO(Player* p);

protected slots:

private:
	void init();

private:
	class BosonBigDisplayPrivate;
	BosonBigDisplayPrivate* d;
};

#endif
