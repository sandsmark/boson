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
#ifndef BOSONORDERWIDGET_H
#define BOSONORDERWIDGET_H

#include <qwidget.h>
#include <qvaluelist.h>
#include <qpair.h>

#include "../global.h"

class Unit;
class Facility;
class Player;
class BosonOrderButton;
class BosonTiles;
template<class T> class QPtrList;

/**
 * This is scrollable widget in the commandframe that contains buttons of unit
 * actions (when single unit is selected) or selected units (when multiple
 * units are selected)
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonOrderWidget : public QWidget
{
	Q_OBJECT
public:
	BosonOrderWidget(QWidget* parent);
	~BosonOrderWidget();

	/**
	 * Initialize this widget for editor mode. In editor mode the widget
	 * also displays widgets for modifying cells.
	 **/
	void initEditor();

	/**
	 * Ensure that at least @p number buttons are available.
	 **/
	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideOrderButtons();

	/**
	 * Display @p number buttons per row
	 **/
	void setButtonsPerRow(int number);

	/**
	 * This function is used to display production options (e.g. when the
	 * unit has a @ref ProductionPlugin or in editor mode units should get
	 * placed). The list contains pairs, where the first element of the @ref
	 * QPair specifies whether the id is a unit or a technology. See @ref
	 * ProductionType. The second element of the pair specifies the id that
	 * fits the @ref ProductionType.
	 * @param produceList A list of production options (or placement options
	 * in editor mode) which all have a @ref ProductionType assigned which
	 * specifies which kind of production option the id is.
	 *
	 * See also @ref hideOrderButtons, @ref showUnits and @ref
	 * slotRedrawTiles
	 **/
	void setOrderButtons(QValueList<QPair<ProductionType, unsigned long int> > produceList, Player* owner, Facility* factory = 0);

	/**
	 * @overloaded
	 *
	 * This creates a list of @ref QPair where all elements of @p idList
	 * share the type @p type. See @ref setOrderButtons above.
	 **/
	void setOrderButtons(ProductionType type, QValueList<unsigned long int> idList, Player* owner, Facility* factory = 0);

	/**
	 * Display the @p units. This is used for multiple selections, i.e. when
	 * the player selected more than one unit.
	 *
	 * Use @ref setOrderButtons or @ref hideOrderButtons if the player
	 * selected one unit only.
	 **/
	void showUnits(QPtrList<Unit> units);

	void productionAdvanced(Unit* factory, double percentage);

	void setCellType(CellType type);
	void setTileSet(BosonTiles* tileSet);

	/**
	 * @return TRUE if the widget display production options (.e. order
	 * buttons), otherwise FALSE
	 **/
	OrderType orderType() const;

	/**
	 * Resets button by setting it's production count to 0 and making it
	 * not-grayed-out
	 **/
	void resetButton(BosonOrderButton* button);

public slots:
	/**
	 * Display the cells that can be placed on the map according to @ref
	 * setCellType and the config widgets.
	 *
	 * Note that you need to call @ref setTileSet at least once before you
	 * can use this slot.
	 **/
	void slotRedrawTiles();

protected:
	void resetLayout();

	/**
	 * In editor mode hide the widget to configure cells. Should be called
	 * when no cells are displayed.
	 **/
	void hideCellConfigWidgets();

	/**
	 * Call this once the user wants to place cells. This will show the
	 * cell configuration widgets. Note that you need to call @ref
	 * initEditor once before this has an effect.
	 **/
	void showCellConfigWidgets();

signals:
	void signalProduce(ProductionType type, unsigned long int id);
	void signalStopProduction(ProductionType type, unsigned long int id);
	void signalPlaceCell(int groundType);
	
	/**
	 * This unit should become the only selected unit. See @ref
	 * BosonOrderButton::signalSelectUnit
	 **/
	void signalSelectUnit(Unit*);

private:
	class BosonOrderWidgetPrivate;
	BosonOrderWidgetPrivate* d;
};

#endif

