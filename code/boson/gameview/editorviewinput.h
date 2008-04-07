/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef EDITORVIEWINPUT_H
#define EDITORVIEWINPUT_H

#include "bosongameviewinputbase.h"

class BoSelection;
class BoSpecificAction;
class BosonMessageEditorMove;

class EditorViewInputPrivate;


class EditorViewInput : public BosonGameViewInputBase
{
	Q_OBJECT
public:
	EditorViewInput();
	~EditorViewInput();


	//AB: this should generate an error when owner != localPlayer()
	/**
	 * The specified unitType is marked to be placed whenever the user
	 * clicks on the map.
	 **/
	virtual void placeUnit(quint32 unitType, Player* owner);

	virtual void placeGround(unsigned int textureCount, unsigned char* alpha);

	/**
	 * Delete the currently selected units
	 **/
	virtual void deleteSelectedUnits();

	/**
	 * Called when the user right-clicks on the big display.
	 *
	 * Should e.g. move a unit
	 * @param event Contains information about the mouse event (position,
	 * additional buttons, ...)
	 **/
	virtual void actionClicked(const BoMouseEvent& event);

	virtual void updatePlacementPreviewData();
	virtual void action(const BoSpecificAction& action);

	/**
	 * In editor mode this does just nothing.
	 * UPDATE: it calls @ref BosonCursor::setWidgetCursor() on the
	 * bigDisplay.
	 **/
	virtual void updateCursor();

	virtual bool selectAll(const UnitProperties* prop, bool replace);

	virtual void undo();
	virtual void redo();

public slots:
	virtual void slotMoveSelection(int cellX, int cellY);

protected slots:
	void slotClearUndoStack();
	void slotClearRedoStack();
	void slotNewUndoMessage(const BosonMessageEditorMove&, bool fromRedo);
	void slotNewRedoMessage(const BosonMessageEditorMove&);

protected:
	//AB: use BoMouseEvent as 1st parameter
	bool actionPlace(const BoVector3Fixed& pos, bool exact, bool force);
	bool actionChangeHeight(const BoVector3Fixed& pos, bool up);

	/**
	 * Editor mode can select just everything, even destroyed units
	 * (otherwise we can't delete them anymore).
	 * @return CanSelectMultipleOk
	 **/
	virtual CanSelectUnit canSelect(Unit* unit) const { Q_UNUSED(unit); return CanSelectMultipleOk; }

	QString messageName(const BosonMessageEditorMove* m) const;

private:
	EditorViewInputPrivate* d;
};

#endif

