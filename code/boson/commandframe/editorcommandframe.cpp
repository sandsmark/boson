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

#include <klocale.h>
#include <kgameprogress.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qvbox.h>

class EditorCommandFrame::EditorCommandFramePrivate
{
public:
	EditorCommandFramePrivate()
	{
	}

};

EditorCommandFrame::EditorCommandFrame(QWidget* parent) : BosonCommandFrameBase(parent)
{
 init();
}

void EditorCommandFrame::init()
{
 d = new EditorCommandFramePrivate;
 orderWidget()->initEditor();

// the order buttons
/*
 connect(orderWidget(), SIGNAL(signalProduceUnit(unsigned long int)),
		this, SLOT(slotProduceUnit(unsigned long int)));
 connect(orderWidget(), SIGNAL(signalStopProduction(unsigned long int)),
		this, SLOT(slotStopProduction(unsigned long int)));
*/
}

EditorCommandFrame::~EditorCommandFrame()
{
 delete d;
}

void EditorCommandFrame::setAction(Unit* unit)
{
 kdDebug() << k_funcinfo << endl;
 BosonCommandFrameBase::setAction(unit);
 if (!selectedUnit()) {
	return;
 }
 if (selectedUnit() != unit) {
	kdError() << k_funcinfo << "selectedUnit() != unit" << endl;
	return;
 }
 Player* owner = unit->owner();
/*
 if (d->mConstructionProgress->showUnit(unit)) {
	startStopUpdateTimer();
	return;
 }

 // Show unit's actions (move, attack, stop...
 // TODO: these can be displayed (at least most of them) for groups, too!
 showUnitActions(unit);

 kdDebug() << k_funcinfo << endl;

 if (selectedUnit()->plugin(UnitPlugin::Production)) {
	if (!selectedUnit()->properties(PluginProperties::Production)) {
		// must not happen if the units has the production
		// plugin
		kdError() << k_funcinfo << "no production properties!" << endl;
		return;
	}
	ProductionProperties* pp = (ProductionProperties*)selectedUnit()->properties(PluginProperties::Production);
	QValueList<unsigned long int> produceList = selectedUnit()->speciesTheme()->productions(pp->producerList());
	// Filter out things that player can't actually build (requirements aren't
	//  met yet)
	QValueList<unsigned long int>::Iterator it;
	it = produceList.begin();
	while(it != produceList.end()) {
		if(!owner->canBuild(*it)) {
			it = produceList.remove(it);
		} else {
			it++;
		}
	}
	orderWidget()->setOrderButtons(produceList, owner, (Facility*)unit);
	orderWidget()->show();
 }
 d->mMinerWidget->showUnit(selectedUnit());
 startStopUpdateTimer();
 */
}

void EditorCommandFrame::slotUpdate()
{
 BosonCommandFrameBase::slotUpdate();
 if (!selectedUnit()) {
	return;
 }
 /*
 if (!d->mConstructionProgress->isHidden()) {
	if (d->mConstructionProgress->showUnit(selectedUnit())) {
		if (!selectedUnit()->isFacility()) {
			// can't happen, since d->mConstructionProgress already
			// checks this
			kdError() << k_funcinfo << "No facility" << endl;
			return;
		}
	} else {
		// construction has been completed=!
		setAction(selectedUnit());
	}
 }
 if (!orderWidget()->isHidden()) { // FIXME
	ProductionPlugin* production = (ProductionPlugin*)selectedUnit()->plugin(UnitPlugin::Production);
	if (!production || !production->hasProduction()) {
		slotUpdateProduction(selectedUnit());
	}
 }
 */
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

void EditorCommandFrame::showUnitActions(Unit* unit)
{
}

void EditorCommandFrame::slotSetButtonsPerRow(int b)
{
 BosonCommandFrameBase::slotSetButtonsPerRow(b);
// d->mUnitActions->setButtonsPerRow(b);
}

void EditorCommandFrame::placeCells(CellType type)
{
 if (!orderWidget()) {
	kdError() << k_funcinfo << "NULL orderwidget" << endl;
	return;
 }
 orderWidget()->hideOrderButtons();
 orderWidget()->setCellType(type);
 orderWidget()->slotRedrawTiles();
 orderWidget()->show();
}

void EditorCommandFrame::setTileSet(BosonTiles* t)
{
 orderWidget()->setTileSet(t);
}


