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
#ifndef __BOSONCOMMANDFRAME_H__
#define __BOSONCOMMANDFRAME_H__

#include <qframe.h>
#include <qvaluelist.h>

class Unit;
class Player;

class QPixmap;

class BosonCommandFramePrivate;
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

signals:
	/**
	 * Emitted when an order button is clicked. This unit is marked for
	 * construction, i.e. the construction should be started if possible.
	 * @param unitType The unit to construct
	 * @param factory Where to construct
	 * @param owner The owner of the factory (i.e. the local player)
	 **/
	void signalUnitSelected(int unitType, Unit* factory, Player* owner);

	void signalCellSelected(int groundType, unsigned char version);

protected slots:
	/**
	 * A button has been clicked. Emit the correct signal.
	 **/
	void slotHandleOrder(int button);

	/**
	 * If the order buttons should currently display tiles (cells) this
	 * updates the buttons.
	 **/
	void slotRedrawTiles();

protected:
	/**
	 * Set the orderbutton button to produce unitType
	 **/
	void setOrderButton(unsigned int button, int unitType, Player* owner);

	/**
	 * Call @ref setOrderButton for a list of unitTypes
	 * @param produceList A list containing UnitTypeIDs.
	 **/
	void setOrderButtons(QValueList<int> produceList, Player* owner);

	/**
	 * Make sure that at least no order buttons exist. Create order buttons
	 * if not.
	 **/
	void initOrderButtons(unsigned int no);

	/**
	 * @param id The order button to hide or -1 for all
	 **/
	void hideOrderButtons();
	
	/**
	 * Set the pixmap of the order button. See @ref
	 * SpeciesTheme::smallOverview
	 **/
	void setOrderPixmap(unsigned int id, const QPixmap& p);

	/**
	 * Set the tooltip of the button. The tooltip should show the name of
	 * the unit that can be constructed with this button.
	 **/
	void setOrderTooltip(unsigned int id, const QString& text);

private:
	void init();
	void initEditor();

private:
	BosonCommandFramePrivate* d;
};

#endif
