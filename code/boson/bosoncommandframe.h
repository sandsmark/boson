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
#ifndef BOSONCOMMANDFRAME_H
#define BOSONCOMMANDFRAME_H

#include <qframe.h>
#include <qvaluelist.h>

class Unit;
class UnitBase;
class Facility;
class Player;
class CommandInput;

class QPixmap;
class KPlayer;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BoOrderWidget : public QWidget
{
	Q_OBJECT
public:
	BoOrderWidget(bool editor, QWidget* parent);
	~BoOrderWidget();

	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideOrderButtons();
	void setButtonsPerRow(int);

	void setOrderButtons(QValueList<int> produceList, Player* owner, Facility* factory = 0);

	void showUnit(Unit* unit); // TODO if this is the only unit -> use slotShowSingleUnit

	void productionAdvanced(Unit* factory, double percentage);


	void initEditor();
	void editorLoadTiles(const QString& dirName);
	void setOrderType(int);

protected:
	void resetLayout();

protected slots:
	void slotEditorLoadTiles();

public slots:
	void slotRedrawTiles();

signals:
	void signalProduceUnit(int unitType);
	void signalStopProduction(int unitType);
	void signalPlaceCell(int groundType);

private:
	class BoOrderWidgetPrivate;
	BoOrderWidgetPrivate* d;
};

/**
 * @short The frame where you can order units
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandFrame : public QFrame
{
	Q_OBJECT
public:

	BosonCommandFrame(QWidget* parent, bool editor = false);
	~BosonCommandFrame();

	/**
	 * @param p The player whose units can be produced here.
	 **/
	void setLocalPlayer(Player* p);

	/**
	 * Ok, I know this function is a hack. Since we use QToolBar as parent
	 * of BosonCommandFrame we need to do some ugly things - like
	 * reparenting the minimap. It is done only once, immediately after
	 * constructing the command frame.
	 *
	 * Seriously it does not even belong here. It belongs <em>next</em> to
	 * BosonCommandFrame onto the QToolBar. But hey - it doesn't hurt here
	 * and it's an easy solution for the background pixmap :-)
	 **/
	void reparentMiniMap(QWidget* map);
	
public slots:
	/**
	 * Show the selected unit in the @ref BosonUnitView
	 * @param unit The selected unit
	 **/
	void slotShowSingleUnit(Unit* unit);

	/**
	 * Sets e.g. the order buttons of possible production items, if this is
	 * a factory.
	 * @param unit The selected unit
	 **/
	void slotSetAction(Unit* unit);

	/**
	 * Called by @ref Editor when selecting a menu entry. Uses @ref
	 * OrderType to translate index
	 **/
	void slotEditorProduction(int index, Player* owner);

	/**
	 * Load the tile file (currenlty earth.png). Only used by the editor as
	 * the name implies.
	 **/
	void slotEditorLoadTiles(const QString& fileName);

	void slotShowUnit(Unit* unit); // TODO if this is the only unit -> use slotShowSingleUnit 

	void slotSetButtonsPerRow(int b);

	/**
	 * Should be called when the production of the factory changes, i.e. is
	 * stopped/paused or started.
	 **/
	void slotUpdateProduction(Facility* factory);

	/**
	 * If the selected unit is a facility that has not been constructed
	 * completely (see @ref Unit::isConstructionComplete) show how far the
	 * construction is currently.
	 **/
	void slotShowConstructionProgress(Facility* fac);

signals:
	/**
	 * Emitted when a unit should be produced.
	 **/
	void signalProduceUnit(int unitType, UnitBase* factory, KPlayer* owner);
	void signalStopProduction(int unitType, UnitBase* factory, KPlayer* owner);

	/**
	 * @param groundType The tile number. See @ref BosonTiles::tile to get
	 * the actual pixmap.
	 **/
	void signalCellSelected(int groundType);

	/**
	 * Center the base of the local player
	 **/
	void signalCenterBase();

protected slots:
	void slotProduceUnit(int unitType);
	void slotStopProduction(int unitType);

	void slotUpdate();

protected:
	virtual void resizeEvent(QResizeEvent*);

	void hideActions();

private:
	void init();
	void initEditor();

private:
	class BosonCommandFramePrivate;
	BosonCommandFramePrivate* d;
};

#endif
