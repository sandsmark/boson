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

#include "editorcommandframe.h"
#include "editorcommandframe.moc"

#include "bosonorderwidget.h"
#include "../unit.h"
#include "../unitplugins.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../pluginproperties.h"
#include "../boselection.h"
#include "../defines.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgameprogress.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qscrollview.h>

class EditorCommandFrame::EditorCommandFramePrivate
{
public:
	EditorCommandFramePrivate()
	{
		mPlacementWidget = 0;
	}

	BosonOrderWidget* mPlacementWidget;
};

EditorCommandFrame::EditorCommandFrame(QWidget* parent) : BosonCommandFrameBase(parent)
{
 init();
}

void EditorCommandFrame::init()
{
 d = new EditorCommandFramePrivate;

 QScrollView* scrollView = addPlacementView();
 d->mPlacementWidget = new BosonOrderWidget(scrollView->viewport());
 d->mPlacementWidget->setBackgroundOrigin(WindowOrigin);
 d->mPlacementWidget->initEditor();
 scrollView->addChild(d->mPlacementWidget);

// the order buttons
 connect(d->mPlacementWidget, SIGNAL(signalProduce(ProductionType, unsigned long int)),
		this, SLOT(slotPlaceUnit(ProductionType, unsigned long int)));
 connect(d->mPlacementWidget, SIGNAL(signalPlaceCell(int)),
		this, SIGNAL(signalPlaceCell(int)));
}

EditorCommandFrame::~EditorCommandFrame()
{
 delete d;
}

void EditorCommandFrame::showUnitActions(Unit* unit)
{
 // currently unused in editor mode.
 // we might display configuration buttons (e.g. for health) here
 // (maybe they'd fit best in setSelectedUnit() as plugins)
 if (!unit) {
	return;
 }
}

void EditorCommandFrame::setProduction(Unit* unit)
{
 // same as above - we might use this for configuration widgets. but we'll
 // better use plugins (see setSelectedUnit())
 boDebug() << k_funcinfo << endl;
 BosonCommandFrameBase::setProduction(0); // will hide all order buttons
 if (!unit) {
	return;
 }
}

void EditorCommandFrame::setSelectedUnit(Unit* unit)
{
 // currently we don't provide any plugins here. just like above we might
 // provide plugins for configurations.
 boDebug() << k_funcinfo << endl;
 BosonCommandFrameBase::setSelectedUnit(unit);
 if (!selectedUnit()) {
	return;
 }
 if (selectedUnit() != unit) {
	boError() << k_funcinfo << "selectedUnit() != unit" << endl;
	return;
 }
}

void EditorCommandFrame::slotUpdate()
{
 BosonCommandFrameBase::slotUpdate();
 if (!selectedUnit()) {
	return;
 }
}

bool EditorCommandFrame::checkUpdateTimer() const
{
 if (!selectedUnit()) {
	return false;
 }
 if (BosonCommandFrameBase::checkUpdateTimer()) {
	return true;
 }
 return false;
}

void EditorCommandFrame::slotSetButtonsPerRow(int b)
{
 BosonCommandFrameBase::slotSetButtonsPerRow(b);
 d->mPlacementWidget->setButtonsPerRow(b);
}

void EditorCommandFrame::setTileSet(BosonTiles* t)
{
 d->mPlacementWidget->setTileSet(t);
}

void EditorCommandFrame::placeCells(CellType type)
{
 d->mPlacementWidget->setCellType(type);
 d->mPlacementWidget->slotRedrawTiles();
}

void EditorCommandFrame::placeMobiles(Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	boError() << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }
 d->mPlacementWidget->setOrderButtons(ProduceUnit, theme->allMobiles(), owner);
}

void EditorCommandFrame::placeFacilities(Player* owner)
{
 if (!owner) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	boError() << k_funcinfo << "NULL speciestheme" << endl;
	return;
 }
 d->mPlacementWidget->setOrderButtons(ProduceUnit, theme->allFacilities(), owner);
}

