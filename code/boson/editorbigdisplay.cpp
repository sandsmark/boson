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
#include "editorbigdisplay.h"
#include "editorbigdisplay.moc"

#include "unit.h"
#include "unitplugins.h"
#include "bosoncanvas.h"
#include "player.h"
#include "unitproperties.h"
#include "cell.h"
#include "bosonmessage.h"
#include "kgamecanvaschat.h"
#include "bosoncursor.h"
#include "bosonmusic.h"
#include "bosonconfig.h"
#include "global.h"
#include "kspritetooltip.h"
#include "boselection.h"
#include "defines.h"

#include <kgame/kgameio.h>
#include <kdebug.h>
#include <klocale.h>

#include <qptrlist.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qcursor.h>

// this just stores a *selection* for constructing. This means e.g. if you click
// on a unit (in the command frame!) the unit type is placed here as well as the
// facility that shall construct the unit. We'll see if this makes sense at
// all...
// for editor mode the facility is 0. As soon as the player left-clicks on the
// map the unit is placed there.
// owner should be for Editor mode only.
struct ConstructUnit
{
	int unitType; // to be constructed unit
	Unit* factory; // facility that constructs the unit (or NULL in editor mode)
	Player* owner; // the owner of the unit - probably only for editor mode.
	int groundType;
};

class EditorBigDisplay::EditorBigDisplayPrivate
{
public:
	EditorBigDisplayPrivate()
	{
		mMouseIO = 0;
	}

	KGameMouseIO* mMouseIO;

	ConstructUnit mConstruction;
};

EditorBigDisplay::EditorBigDisplay(BosonCanvas* c, QWidget* parent) 
		: BosonBigDisplayBase(c, parent)
{
 init();
}

void EditorBigDisplay::init()
{
 d = new EditorBigDisplayPrivate;

 d->mConstruction.unitType = -1; // FIXME: 0 would be better but this is a unit...
 d->mConstruction.groundType = -1;
}

EditorBigDisplay::~EditorBigDisplay()
{
 delete d;
}

void EditorBigDisplay::setLocalPlayer(Player* p)
{
 if (localPlayer() == p) {
	return;
 }
 if (localPlayer()) {
	//AB: in theory the IO gets removed from the players' IO list. if we
	//ever use this, then test it!
	delete d->mMouseIO;
	d->mMouseIO = 0;
 }
 BosonBigDisplayBase::setLocalPlayer(p);
 if (localPlayer()) {
	addMouseIO(localPlayer());
 }
}

void EditorBigDisplay::actionClicked(const BoAction& action, QDataStream& , bool* )
//void EditorBigDisplay::editorActionClicked(const BoAction& action)
{
// kdDebug() << k_funcinfo << endl;
 int x = action.canvasPos().x() / BO_TILE_SIZE;
 int y = action.canvasPos().y() / BO_TILE_SIZE;
 if (d->mConstruction.unitType > -1) {
	if (!d->mConstruction.owner) {
		kdWarning() << k_funcinfo << "NO OWNER" << endl;
//		return;
	}
	
	emit signalBuildUnit(d->mConstruction.unitType, x, y,
			d->mConstruction.owner);
//	setModified(true); // TODO: in BosonPlayField
 } else if (d->mConstruction.groundType > -1) {
//	kdDebug() << "place ground " << d->mConstruction.groundType << endl;
	int version = 0; // FIXME: random()%4;
	emit signalAddCell(x, y, d->mConstruction.groundType, version);
	if (Cell::isBigTrans(d->mConstruction.groundType)) {
		emit signalAddCell(x + 1, y, 
				d->mConstruction.groundType + 1, version);
		emit signalAddCell(x, y + 1, 
				d->mConstruction.groundType + 2, version);
		emit signalAddCell(x + 1, y + 1, 
				d->mConstruction.groundType + 3, version);
	}
//	setModified(true); // TODO: in BosonPlayField
 }
}

/*
void EditorBigDisplay::slotWillConstructUnit(int unitType, UnitBase* factory, KPlayer* owner)
{
 if (!owner) {
	kdDebug() << k_funcinfo << "NULL owner" << endl;
	d->mConstruction.groundType = -1;
	d->mConstruction.unitType = -1;
	return;
 }
// kdDebug() << unitType << endl;
 if (factory) { // usual production
//	kdDebug() << "there is a factory " << endl;
	return;
	kdError() << "obsolete??" << endl;
	// this MUST be sent over network - therefore use CommandInput!
//	((Facility*)factory)->addProduction(unitType); //AB: seems to be
//	obsolete.. //TODO: test if it is used in any way!
 } else { // we are in editor mode!
	//FIXME
	// should be sent over network
	d->mConstruction.unitType = unitType;
	d->mConstruction.factory = (Unit*)factory;
	d->mConstruction.owner = (Player*)owner;
	d->mConstruction.groundType = -1;
 }
}

void EditorBigDisplay::slotWillPlaceCell(int groundType)
{
 d->mConstruction.unitType = -1;
 d->mConstruction.groundType = groundType;
}*/

void EditorBigDisplay::updateCursor()
{
 // does nothing. 
}

/*
void EditorBigDisplay::addMouseIO(Player* p)
{
//AB: make sure that both editor and game mode can share the same IO !
 if (!localPlayer()) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (d->mMouseIO) {
	kdError() << "This view already has a mouse io!!" << endl;
	return;
 }
 d->mMouseIO = new KGameMouseIO(viewport(), true);
 connect(d->mMouseIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, 
		QMouseEvent*, bool*)),
		this, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*,
		bool*)));
 localPlayer()->addGameIO(d->mMouseIO);
}
*/
