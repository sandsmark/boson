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

#include <qcanvas.h>
#include <qptrlist.h>

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
class BoAction;

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
class BosonBigDisplay : public QCanvasView
{
	Q_OBJECT
public:
	BosonBigDisplay(QCanvas* c, QWidget* parent);
	BosonBigDisplay(QWidget* parent);
	~BosonBigDisplay();

	void setLocalPlayer(Player* p);
	
	/**
	 * Used by @ref BoDisplayManager - don't use anywhere else, if possible.
	 *
	 * This is meant to return the local player of this display. One day it
	 * might be possible to have a different player in a different display
	 * (think e.g. of allied players).
	 * @return The local player of this display
	 **/
	Player* localPlayer() const;

	/**
	 * Set the @ref KGameChat object to the @ref KGameCanvasChat.
	 **/
	void setKGameChat(KGameChat* c);
	
	void addChatMessage(const QString&);

	void setCursor(BosonCursor* cursor);

	/**
	 * Called by @ref BoDisplayManager.
	 * 
	 * Do NOT call directly!
	 **/
	void setActive(bool);

	/**
	 * Emit @ref signalMakeActive to inform @ref BosonWidget and @ref
	 * BoDisplayManager that this display should become the active display.
	 *
	 * Use this if you want to change the active status of the display!
	 **/
	void makeActive();

	BoSelection* selection() const;

	/**
	 * Clear the selection and so on.
	 **/
	void quitGame();

public slots:
	/**
	 * @param pos The new position - cell coordinates! so you have to
	 * multiply the coordinates with BO_TILE_SIZE
	 **/
	void slotReCenterView(const QPoint& pos);

	/**
	 * Mark the unitType for construction. Inform boson about this and place
	 * the unit on the screen if in editor mode - otherwise delay.
	 **/
	void slotWillConstructUnit(int unitType, UnitBase* facility, KPlayer* owner);

	void slotWillPlaceCell(int groundType);

	void slotUnitChanged(Unit* unit);

	virtual void resizeContents(int w, int h);

	void slotMoveSelection(int cellX, int cellY);

	virtual void setContentsPos(int x, int y);

signals:
	/**
	 * Emitted by @ref resizeEvent
	 **/
	void signalSizeChanged(int w, int h);

	void signalAddCell(int x, int y, int groundType, unsigned char version);
	
	void signalBuildUnit(int type, Unit* facility, Player* owner);
	void signalBuildUnit(int type, int x, int y, Player* owner); // editor mode only

	/**
	 * Make this display the currently active view
	 **/
	void signalMakeActive(BosonBigDisplay*);

protected:
	/**
	 * Selects units in the specified rect. See also @ref BoSelection
	 **/
	void selectArea();

	/**
	 * @return The start point of the selection rect
	 **/
	const QPoint& selectionStart() const;

	/**
	 * @return The end point of the selection rect
	 **/
	const QPoint& selectionEnd() const;

	/**
	 * Select all units (of the local player) that have this unit properties
	 * (i.e. are of the same type)
	 * @return TRUE if we were successfull, or FALSE if the player is not
	 * meant to select ALL units of this type. E.g. for facilities where it
	 * doesn't make sense. THe unit that was clicked is meant to be selected
	 * then.
	 **/
	bool selectAll(const UnitProperties* prop);

	/**
	 * Draw the selection rect from @ref selectionStart to @ref selectionEnd
	 **/
	void drawSelectionRect();

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
	void actionClicked(const BoAction* action, QDataStream& stream, bool& send);

	void editorActionClicked(const BoAction* action);

	virtual void resizeEvent(QResizeEvent*);

	/**
	 * Start the selection at pos. Either draw the selection rect here or
	 * select a single unit (if at pos is a unit)
	 * @param pos The start point of the selection rect or the position of a
	 * unit
	 **/
	void startSelection(const QPoint& pos);

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now newEnd
	 **/
	void moveSelectionRect(const QPoint& newEnd);

	/**
	 * Remove a currently drawn selection rect. Do nothing if no selection
	 * rect is currently drawn.
	 **/
	void removeSelectionRect();

	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	virtual bool eventFilter(QObject*, QEvent*);

	void updateCursor();

protected slots:
	void slotMouseEvent(KGameIO*, QDataStream& stream, QMouseEvent* e, bool *eatevent);
	void slotEditorMouseEvent(QMouseEvent* e, bool* eatevent);

	void slotContentsMoving(int x, int y);

	void slotCursorEdgeTimeout();

private:
	void init();

private:
	class BosonBigDisplayPrivate;
	BosonBigDisplayPrivate* d;
};

#endif
