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
#ifndef __BOSONBIGDISPLAY_H__
#define __BOSONBIGDISPLAY_H__

#include <qcanvas.h>
#include <qptrlist.h>

class KGameIO;

class Unit;
class Player;

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

	enum SelectionMode {
		SelectNone = 0,
		SelectSingle = 1,
		SelectRect = 2
	};

	void setLocalPlayer(Player* p);

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
	void slotWillConstructUnit(int unitType, Unit* facility, Player* owner);

	void slotWillPlaceCell(int groundType, unsigned char version);

	void slotUnitChanged(Unit* unit);

	virtual void resizeContents(int w, int h);

signals:
	/**
	 * Emitted when a single unit (@ref selectionMode == Select_Single) is
	 * selected. The image of the unit should be displayed now.
	 *
	 * If this is a factory the order buttons also should be changed.
	 **/
	void signalSingleUnitSelected(Unit* unit);

	/**
	 * Emitted by @ref resizeEvent
	 **/
	void signalSizeChanged(int w, int h);

//	void signalEditorAddUnit(int type, int x, int y, int owner);
//	void signalEditorConstruction(int unitType, Player* owner);
	void signalAddCell(int x, int y, int groundType, unsigned char version);
	
	void signalConstructUnit(int type, Unit* facility, Player* owner);
	void signalConstructUnit(int type, int x, int y, Player* owner); // editor mode only

protected:
	/**
	 * Sets the selection mode after clearing any previous selection.
	 **/
	void setSelectionMode(SelectionMode mode);

	/**
	 * Clears the selection list and sets @ref selectionMode = Select_None
	 **/
	void clearSelection();

	/**
	 * Selects units in the specified rect. See also @ref addUnitSelection
	 **/
	void selectArea(const QRect& rect);

	/**
	 * @return The start point of the selection rect
	 **/
	const QPoint& selectionStart() const;

	/**
	 * @return The end point of the selection rect
	 **/
	const QPoint& selectionEnd() const;

	/**
	 * @return The currently selected units. See also @ref addUnitSelection
	 **/
	QPtrList<Unit>& selection() const;

	/**
	 * @return The selection mode. See @ref setSelectionMode
	 **/
	SelectionMode selectionMode() const;

	/**
	 * Add a unit to the selection list
	 **/
	void addUnitSelection(Unit* unit);

	/**
	 * Draw the selection rect from @ref selectionStart to @ref selectionEnd
	 **/
	void drawSelectionRect();

	/**
	 * Called when the user right-clicks on the big display.
	 * 
	 * Should e.g. move a unit
	 * @param point The point where the click was done
	 * @param stream The move should be placed here. A move should
	 * <em>not</em> be done in this method but rather sent to @ref KGame
	 * which performs the move on every client
	 * @param send Set to true if you actually want to send the stream
	 **/
	void actionClicked(const QPoint& point, QDataStream& stream, bool& send);

	void editorActionClicked(const QPoint& point);

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
	 *
	 * Do nothing if @ref selectionMode is not Select_Rect
	 **/
	void moveSelectionRect(const QPoint& newEnd);

	/**
	 * Remove a currently drawn selection rect. Do nothing if no selection
	 * rect is currently drawn.
	 **/
	void removeSelectionRect();
	
protected slots:
	void slotMouseEvent(KGameIO*, QDataStream& stream, QMouseEvent* e, bool *eatevent);

	void slotEditorMouseEvent(QMouseEvent* e, bool* eatevent);

private:
	void init();

private:
	class BosonBigDisplayPrivate;
	BosonBigDisplayPrivate* d;
};

#endif
