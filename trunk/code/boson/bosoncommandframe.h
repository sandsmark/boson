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
#ifndef __BOSONCOMMANDFRAME_H__
#define __BOSONCOMMANDFRAME_H__

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
 * @short The frame where you can order units
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandFrame : public QFrame
{
	Q_OBJECT
public:

	//FIXME: this is not the ideal position for this enum. Dunno where it
	//should be placed though... the correct class must be accessible by
	//BosonCommandFrame, EditorTop and Top
	enum OrderType {
		Facilities = 0,
		Mobiles = 1,
		PlainTiles = 2,
		Small = 3,
		Big1 = 4,
		Big2 = 5,
		
		OrderLast // should always be the last item - used by loops
	};

	BosonCommandFrame(QWidget* parent, bool editor = false);
	~BosonCommandFrame();

	/**
	 * @param p The player whose units can be constructed here.
	 **/
	void setLocalPlayer(Player* p);

public slots:
	/**
	 * Show the selected unit in the @ref BosonUnitView
	 * @param unit The selected unit
	 **/
	void slotShowSingleUnit(Unit* unit);

	/**
	 * Set the orderbuttons to display the possible constructions of this
	 * unit. Hide all buttons if none are possible
	 * @param unit The selected unit
	 **/
	void slotSetConstruction(Unit* unit);

	/**
	 * Called by @ref Editor when selecting a menu entry. Uses @ref
	 * OrderType to translate index
	 **/
	void slotEditorConstruction(int index, Player* owner);

	/**
	 * Load the tile file (currenlty earth.png). Only used by the editor as
	 * the name implies.
	 **/
	void slotEditorLoadTiles(const QString& fileName);

	void slotShowUnit(Unit* unit); // TODO if this is the only unit -> use slotShowSingleUnit 

signals:
	/**
	 * Emitted when a unit should be produced.
	 **/
	void signalProduceUnit(int unitType, UnitBase* factory, KPlayer* owner);

	/**
	 * @param groundType The tile number. See @ref BosonTiles::tile to get
	 * the actual pixmap.
	 **/
	void signalCellSelected(int groundType);

protected slots:
	/**
	 * If the order buttons should currently display tiles (cells) this
	 * updates the buttons.
	 **/
	void slotRedrawTiles();

	void slotProduceUnit(int unitType);

	void slotProductionAdvanced(Unit* factory, double percentage);

	/**
	 * Gray out the order buttons that can currently not be used, as another
	 * units is being produced. (maybe even disable the buttons)
	 **/
	void slotFacilityProduces(Facility* factory);

	/**
	 * Re-Enable the order buttons. See @ref slotFacilityProduces
	 **/
	void slotProductionCompleted(Facility* factory);

protected:
	/**
	 * Set the orderbuttons to containt a list of producable units.
	 * @param produceList A list containing UnitTypeIDs.
	 * @param owner The owner of the producable units
	 * @param factory if NULL all buttons will be enabled. if non-NULL only
	 * the producable items are enabled. So if the factory already produces,
	 * e.g. item #2, then all except #2 are disabled.
	 **/
	void setOrderButtons(QValueList<int> produceList, Player* owner, Facility* factory = 0);

	/**
	 * Make sure that at least @p count order buttons exist. 
	 **/
	void initOrderButtons(unsigned int count);

	/**
	 * Hide all buttons
	 **/
	void hideOrderButtons();
	
private:
	void init();
	void initEditor();

private:
	class BosonCommandFramePrivate;
	BosonCommandFramePrivate* d;
};

#endif
