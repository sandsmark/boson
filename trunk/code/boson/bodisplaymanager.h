/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

class BoBox;
class BosonBigDisplayBase;
class UnitBase;
class BosonCursor;
class Player;
class PlayerIO;
class Unit;
class BosonCanvas;
class BoSelection;
class BoSpecificAction;
class BoFontInfo;

class KPlayer;
class QDomElement;
template<class T> class QPtrList;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDisplayManager : public QWidget
{
	Q_OBJECT
public:
	/**
	 **/
	BoDisplayManager(QWidget* parent);
	~BoDisplayManager();

	BosonBigDisplayBase* addInitialDisplay();

	/**
	 * @return The currently active display. Use @ref
	 * BosonBigDisplayBase::makeActive to change it
	 **/
	BosonBigDisplayBase* activeDisplay() const;

	/**
	 * Make the displaymanager use this canvas (e.g. the selections need
	 * this).
	 *
	 * WARNING: this is NOT applied to the displays!
	 **/
	void setCanvas(BosonCanvas* canvas);

	void quitGame();

	/**
	 * See @ref BosonBigDisplayInputBase::unlockAction
	 **/
	void unlockAction();

	void setToolTipCreator(int type);
	void setToolTipUpdatePeriod(int ms);

	void saveAsXML(QDomElement& root);
	void loadFromXML(const QDomElement& root);

	/**
	 * @return BosonBigDisplayBase::fps for the @ref activeDisplay
	 **/
	double fps() const;

public slots:
	/**
	 * Select the specified group to the active display.
	 * @param number The group to be selected. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotSelectGroup(int number);

	/**
	 * Copy the current selection (of the active display) to the specified
	 * group.
	 * @param number The group to be created. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotCreateGroup(int number);

	/**
	 * Clear the specified group.
	 * @param number The group to be created. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotClearGroup(int number);

	/**
	 * Editor mode only: specifies the unitType that will be placed on the
	 * map when the user clicks the next time
	 **/
	void slotPlaceUnit(unsigned long int unitType, Player* owner);

	/**
	 * Editor mode only: specifies the texture values that will be placed on the
	 * map when the user clicks the next time
	 **/
	void slotPlaceGround(unsigned int textureCount, unsigned char* alpha);

	void slotShowPlaceFacilities(Player*);
	void slotShowPlaceMobiles(Player*);
	void slotShowPlaceGround();

	/**
	 * Editor mode only: delete all currently selected units.
	 **/
	void slotDeleteSelectedUnits();

	void slotUpdateIntervalChanged(unsigned int);

	void slotUnitRemoved(Unit* u);

	/**
	 * Called by @ref Boson::signalAdvance.
	 *
	 * Note that it is <em>not</em> ensured, that @ref
	 * BosonCanvas::slotAdvance is called first. It might be possible that
	 * this slot gets called before @ref BosonCanvas::slotAdvance but the
	 * other way round might be possible as well.
	 *
	 * Also note that this should <em>not</em> be used for game logic parts
	 * that the network might depend on. Use it for OpenGL or similar
	 * operations (input/output on the local client) only.
	 **/
	void slotAdvance(unsigned int, bool);

	/**
	 * Move the selection of the @ref activeDisplay to the cell at (x,y).
	 * See also @ref BosonBigDisplayInputBase::slotMoveSelection
	 **/
	void slotMoveActiveSelection(int x, int y);

	/**
	 * Select the a single unit in the @ref activeDisplay. See also @ref
	 * BoSelection::slotSelectSingleUnit
	 **/
	void slotActiveSelectSingleUnit(Unit*);

	void slotAction(const BoSpecificAction&);

	void slotUpdateOpenGLSettings();

	void slotChangeFont(const BoFontInfo& font);

	void slotSetGrabMovie(bool grab);

signals:
	/**
	 * See @ref BosonBigDisplayInputBase::signalLockAction
	 **/
	void signalLockAction(bool);

	/**
	 * This signal is emitted when the selection of a display changes, see
	 * @ref BosonBigDisplayBase::signalSelectionChanged. One day this might
	 * be changed so that this is emitted when the selection of the @ref
	 * activeDisplay changes only (in case we support several displays again
	 * one day).
	 **/
	void signalSelectionChanged(BoSelection*);

	void signalToggleStatusbar(bool);

protected:
	void grabMovieFrame();

private:
	class BoDisplayManagerPrivate;
	BoDisplayManagerPrivate* d;
};

#endif
