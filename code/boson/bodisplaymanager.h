/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BODISPLAYMANAGER_H
#define BODISPLAYMANAGER_H

#include <qwidget.h>
#include <qptrlist.h>

class BoBox;
class BosonBigDisplayBase;
class UnitBase;
class BosonCursor;
class Player;

class KPlayer;

class BosonCanvas;

/**
 * Since boson is able to provide different displays ("views") of the same map
 * we need to manage all of these display. THat is done here. You will add the
 * first display using @ref addInitialDisplay and then all following displays
 * using one of @ref splitActiveDisplayVertical or @ref
 * splitActiveDisplayHorizontal.
 *
 * The active display is the display that currently that receives the input from
 * the player (usually it also has the focus). E.g. if the player selected a
 * unit in two displays and then right-clicks on a point in the mini map only
 * the selected unit in the activeDisplay should move to that point.
 *
 * You can change the active display using @ref setActiveDisplay.
 * @short Manager for all displays ("views") of boson.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDisplayManager : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @param gameMode controls whether to create @ref BosonBigDisplay or
	 * @ref EditorBigDisplay widgets in @ref addDisplay. @ref
	 * BosonBigDisplay widgets are the default (TRUE)
	 **/
	BoDisplayManager(BosonCanvas* canvas, QWidget* parent, bool gameMode = true);
	~BoDisplayManager();

	BosonBigDisplayBase* addInitialDisplay();

	/**
	 * @return The currently active display. Use @ref
	 * BosonBigDisplayBase::makeActive to change it
	 **/
	BosonBigDisplayBase* activeDisplay() const;

	/**
	 * @return A list containing ALL displays. Try to avoid this function.
	 **/
	QPtrList<BosonBigDisplayBase> displays() const;

	/**
	 * Set the cursor for all displays
	 **/
	void setCursor(BosonCursor* cursor);

	/**
	 * Set the local player for all displays
	 **/
	void setLocalPlayer(Player* player);

	void quitGame();

	void removeActiveDisplay();
	BosonBigDisplayBase* splitActiveDisplayVertical();
	BosonBigDisplayBase* splitActiveDisplayHorizontal();

	void paintResources();
	void paintChatMessages();

public slots:


	void slotEditorWillPlaceCell(int);
	void slotEditorWillPlaceUnit(int unitType, UnitBase* fac, KPlayer*);


	void slotScrollUp() { slotScroll(ScrollUp); }
	void slotScrollRight() { slotScroll(ScrollRight); }
	void slotScrollDown() { slotScroll(ScrollDown); }
	void slotScrollLeft() { slotScroll(ScrollLeft); }
	/**
	 * Scroll the active display
	 * @param direction See @ref ScrollDirection
	 **/
	void slotScroll(int direction);

	void slotUpdateIntervalChanged(unsigned int);
	void slotCenterHomeBase();
	void slotResetViewProperties();

	void slotUpdate();
	void slotUpdateCanvas(); // same as above - but called from the canvas only

signals:
	/**
	 * Emitted when the currently active display changes.
	 * @param active The currently (newly) active display. See @ref
	 * activeDisplay
	 * @param old The previously active display (if any) or NULL if
	 * there was no.
	 **/
	void signalActiveDisplay(BosonBigDisplayBase* active, BosonBigDisplayBase* old);

protected:
	enum ScrollDirection {
		ScrollUp = 0,
		ScrollRight = 1,
		ScrollDown = 2,
		ScrollLeft = 3
	};

	BosonBigDisplayBase* addDisplay(QWidget* parent);
	BoBox* findBox(BosonBigDisplayBase*) const;
	void recreateLayout();

protected slots:
	void slotMakeActiveDisplay(BosonBigDisplayBase*);

private:
	void markActive(BosonBigDisplayBase* display, bool active);

private:
	class BoDisplayManagerPrivate;
	BoDisplayManagerPrivate* d;

	bool mGameMode;
};

#endif
