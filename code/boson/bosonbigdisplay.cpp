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

#include "bosonbigdisplay.h"

#include "unit.h"
#include "bosoncanvas.h"
#include "player.h"
#include "unitproperties.h"
#include "cell.h"
#include "bosonmessage.h"
#include "defines.h"
#include "kgamecanvaschat.h"
#include "bosoncursor.h"
#include "bosonmusic.h"

#include <kgame/kgameio.h>
#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qptrlist.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtimer.h>

#include "bosonbigdisplay.moc"

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

class BosonBigDisplay::BosonBigDisplayPrivate
{
public:
	BosonBigDisplayPrivate()
	{
		mSelectionMode = BosonBigDisplay::SelectNone;
		mIsRMBMove = false;

		mLocalPlayer = 0;

		mChat = 0;

		mSelectionRect = 0;

		mCursor = 0;
	}

	BosonBigDisplay::SelectionMode mSelectionMode;
	QPtrList<Unit> mSelectionList;
	QPoint mSelectionStart;
	QPoint mSelectionEnd;
	bool mIsRMBMove;
	QPoint mRMBMove; // position where RMB move started

	Player* mLocalPlayer;

	ConstructUnit mConstruction;
	
	BosonCursor* mCursor;

	QCanvasText* mMinerals;
	QCanvasText* mOil;

	bool mIsModified; // for editor mode


	KGameCanvasChat* mChat;


	QCanvasRectangle* mSelectionRect;
};

BosonBigDisplay::BosonBigDisplay(QCanvas* c, QWidget* parent) : QCanvasView(c,
		parent, "bigdisplay", 
		WRepaintNoErase |// this should remove some flickering
		WStaticContents | 
		WResizeNoErase
		)
{
 init();
}
BosonBigDisplay::BosonBigDisplay(QWidget* parent) : QCanvasView(parent)
{
 init();
}

void BosonBigDisplay::init()
{
 d = new BosonBigDisplayPrivate;

// setSizePolicy(QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ));
// setResizePolicy(QScrollView::AutoOne);
 setVScrollBarMode(AlwaysOff);
 setHScrollBarMode(AlwaysOff);
 d->mConstruction.unitType = -1; // FIXME: 0 would be better but this is a unit...
 d->mConstruction.groundType = -1;

 d->mIsModified = false;

 connect(this, SIGNAL(contentsMoving(int, int)), 
		this, SLOT(slotContentsMoving(int, int)));

 d->mMinerals = new QCanvasText(QString::null, canvas());
 d->mMinerals->setZ(Z_CANVASTEXT);
 d->mMinerals->setColor(white);
 d->mMinerals->show();
 d->mOil = new QCanvasText(QString::null, canvas());
 d->mOil->setZ(Z_CANVASTEXT);
 d->mOil->setColor(white);
 d->mOil->show();

 d->mChat = new KGameCanvasChat(this);
 d->mChat->setCanvas(canvas());
 d->mChat->setZ(Z_CANVASTEXT);

 d->mCursor = new BosonCursor;
 QString cursorDir = KGlobal::dirs()->findResourceDir("data", "boson/themes/cursors/move/index.desktop") + QString::fromLatin1("boson/themes/cursors");
 d->mCursor->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"));
 d->mCursor->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"));
 d->mCursor->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"));
 d->mCursor->setCanvas(canvas(), CursorDefault, Z_CANVAS_CURSOR);
}

BosonBigDisplay::~BosonBigDisplay()
{
 delete d->mSelectionRect;
 delete d->mChat;
 delete d;
}

void BosonBigDisplay::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
// *eatevent = true;//FIXME: it seems that this makes KGameIO *SEND* the input?
// e->accept() is not called for this?!
 QWMatrix wm = inverseWorldMatrix(); // for zooming
 QPoint pos = viewportToContents(e->pos());
 wm.map(pos.x(), pos.y(), &pos.rx(), &pos.ry());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
			removeSelectionRect();
		} else if (e->button() == RightButton) {
			if (d->mIsRMBMove) {
				d->mIsRMBMove = false;
			} else {
			 	// AB: is *eatevent the correct parameter? KGame should
				// *not* send the stream if this is false!
				bool send = false;
				// pos must be viewportToContents'ed and mapped using
				// the inverse matrix!
			 	actionClicked(pos, stream, send);
				if (send) {
					*eatevent = true;
				}
			}
		}
		e->accept();
		break;
	case QEvent::MouseMove:
	{
		if (e->state() & LeftButton) {
			moveSelectionRect(pos);
		} else if (e->state() & RightButton) {
			d->mIsRMBMove = true;
			scrollBy(pos.x() - d->mRMBMove.x(), pos.y() - d->mRMBMove.y());
			d->mRMBMove = pos;
		} else if (selectionMode() == SelectRect || selectionMode() == SelectSingle) {
			if (selection().count() == 0) {
				kdWarning() << "mode=" << selectionMode() << " but nothing selected" << endl;
				break;
			}
			if (selection().first()->owner() == d->mLocalPlayer) {
				Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
				if (unit && !unit->isDestroyed()) {
					if (unit->owner() == d->mLocalPlayer) {
						d->mCursor->setCursor(CursorDefault);
						d->mCursor->setWidgetCursor(this);
					} else {
						kdDebug() << "a" << endl;
						d->mCursor->setCursor(CursorAttack);
						d->mCursor->setWidgetCursor(this);
					}
				} else if (selection().first()->isMobile()) {
					d->mCursor->setCursor(CursorMove);
					d->mCursor->cursorSprite()->show();
					d->mCursor->setWidgetCursor(this);
				}
			} else {
				d->mCursor->setCursor(CursorDefault);
				d->mCursor->setWidgetCursor(this);
			}
		}
		d->mCursor->move(pos.x(), pos.y());
		canvas()->update();
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
		if (e->button() == LeftButton) {
			startSelection(pos);
		} else if (e->button() == MidButton) {
			center(pos.x(), pos.y());
		} else if (e->button() == RightButton) {
			d->mRMBMove = pos;
		}
		e->accept();
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		break;
 }
}

void BosonBigDisplay::startSelection(const QPoint& pos)
{
 // LMB clicked - either select the unit or start a selection rect
 Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
 if (!unit || unit->isDestroyed()) {
	// nothing has been found : it's a ground-click
	// Here, we have to draw a "selection box"...
	setSelectionMode(SelectRect);
	d->mSelectionStart = pos;
	d->mSelectionEnd = pos;
	// the box is drawn on mouse move

	emit signalSingleUnitSelected(0);
	return;
 }

 setSelectionMode(SelectSingle);
 boMusic->playSound(unit, Unit::SoundOrderSelect);
 addUnitSelection(unit);
 //canvas->update(); // TODO?
}

void BosonBigDisplay::moveSelectionRect(const QPoint& newEnd)
{
 if (selectionMode() == SelectRect) {
	d->mSelectionEnd = newEnd;

	// draw the new selection rect
	drawSelectionRect();
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonBigDisplay::removeSelectionRect()
{
 if (selectionMode() == SelectRect) {
	// here as there is a performance problem in
	// mousemove:
	selectArea();

	// remove the rect:
	if (d->mSelectionRect) {
		delete d->mSelectionRect;
		d->mSelectionRect = 0;
	}
	if (selection().isEmpty()) {
		setSelectionMode(SelectNone);
	} else {
		Unit* u = selection().first();
		if (u->owner() == d->mLocalPlayer) {
			boMusic->playSound(u, Unit::SoundOrderSelect);
		}
	}
 }
}

void BosonBigDisplay::setSelectionMode(SelectionMode mode)
{
 clearSelection();
 d->mSelectionMode = mode;
}

BosonBigDisplay::SelectionMode BosonBigDisplay::selectionMode() const
{
 return d->mSelectionMode;
}

void BosonBigDisplay::clearSelection()
{
 d->mSelectionMode = SelectNone;
 QPtrListIterator<Unit> it(d->mSelectionList);
 while (it.current()) {
	it.current()->unselect();
	++it;
 }
 d->mSelectionList.clear();
 d->mCursor->setCursor(CursorDefault);
 d->mCursor->setWidgetCursor(this);
 emit signalSingleUnitSelected(0);
}

void BosonBigDisplay::addUnitSelection(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (selectionMode() != SelectSingle && unit->owner() != d->mLocalPlayer) {
	kdDebug() << k_funcinfo << ": not owner" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	kdDebug() << k_lineinfo << "don't select destroyed unit" << endl;
	return;
 }
 if (d->mSelectionList.contains(unit)) {
	return;
 }
 d->mSelectionList.append(unit);
 unit->select();
 if (selectionMode() == SelectSingle) {
	emit signalSingleUnitSelected(unit);
 } else {
	emit signalSelectUnit(unit);
 }
}

const QPoint& BosonBigDisplay::selectionStart() const
{
 return d->mSelectionStart;
}

const QPoint& BosonBigDisplay::selectionEnd() const
{
 return d->mSelectionEnd;
}

QPtrList<Unit>& BosonBigDisplay::selection() const
{
 return d->mSelectionList;
}

void BosonBigDisplay::selectArea()
{
 if (!d->mSelectionRect) {
	return;
 }
 QCanvasItemList list;
 QCanvasItemList unitList;
 QCanvasItemList::Iterator it;
 list = d->mSelectionRect->collisions(true);
 for (it = list.begin(); it != list.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* unit = (Unit*)*it;
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->unitProperties()->isMobile()) {
		unitList.append(*it);
	}
	
 }

 if (unitList.count() == 1) {
	setSelectionMode(SelectSingle);
 }
 for (it = unitList.begin(); it != unitList.end(); ++it) {
	addUnitSelection((Unit*)*it);
 }
}

void BosonBigDisplay::drawSelectionRect()
{
// kdDebug() << k_funcinfo << endl;
 QPen pen(red, 2); // FIXME: hardcoded
 


 if (!d->mSelectionRect) {
	d->mSelectionRect = new QCanvasRectangle(QRect(selectionStart(), selectionEnd()), canvas());
	d->mSelectionRect->setPen(pen);
	d->mSelectionRect->setZ(Z_CANVASTEXT); // canvas text is always top so we can use it here, too
 }
 d->mSelectionRect->setVisible(true);
 d->mSelectionRect->setSize(selectionEnd().x() - selectionStart().x(),
		selectionEnd().y() - selectionStart().y());

}

void BosonBigDisplay::slotReCenterView(const QPoint& pos)
{
// pos.x() and pos.y() are the cell numbers! *= BO_TILE_SIZE are the coordinates
 center(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE);
}

void BosonBigDisplay::actionClicked(const QPoint& pos, QDataStream& stream, bool& send)
{// pos is already viewportToContents()'ed
// this method should not perform any tasks but rather send the input through
// the KGameIO. this way it is very easy (it should be at least) to write a
// computer player
 if (selection().isEmpty()) {
//	d->mChat->sendMessage(i18n("hi"));
	return;
 }
 if (selectionMode() == SelectNone) {
	kdError() << k_funcinfo << ": SelectNone" << endl;
	return;
 }
 Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
 if (!unit) {
	if ((selection().first()->unitProperties()->isMobile())) { // move the selection to pos
		QPtrListIterator<Unit> it(selection());
		// tell the clients we want to move units:
		stream << (Q_UINT32)BosonMessage::MoveMove;
		// tell them where to move to:
		stream << pos;
		// tell them how many units:
		stream << (Q_UINT32)selection().count();
		Unit* unit = 0;
		while (it.current()) {
			if (!unit) {
				unit = it.current();
			}
			// tell them which unit to move:
			stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
			++it;
		}
		boMusic->playSound(unit, Unit::SoundOrderMove);
		send = true;
	} else { // place constructions
		// FIXME: another option: add the waypoint to the facility and
		// apply it to any unit that gets constructed by that facility.
		// For this we'd probably have to use LMB for unit placing
		Facility* fac = (Facility*)selection().first();
		if (!fac->hasProduction() || !fac->canPlaceProductionAt(pos)) {
			return;
		}
		// create the new unit
		stream << (Q_UINT32)BosonMessage::MoveBuild;
		stream << (Q_UINT32)fac->id();
		stream << (Q_UINT32)fac->owner()->id();
		stream << (Q_INT32)pos.x() / BO_TILE_SIZE;
		stream << (Q_INT32)pos.y() / BO_TILE_SIZE;
		send = true;
	}
 } else { // there is a unit - attack it!
	QPtrListIterator<Unit> it(selection());
	// tell the clients we want to attack:
	stream << (Q_UINT32)BosonMessage::MoveAttack;
	// tell them which unit to attack:
	stream << (Q_ULONG)unit->id();
	// tell them how many units attack:
	stream << (Q_UINT32)selection().count();
	while (it.current()) {
		// tell them which unit is going to attack:
		stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
		++it;
	}
	send = true;
	Unit* u = selection().first();
	boMusic->playSound(u, Unit::SoundOrderAttack);
 }
}

void BosonBigDisplay::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 if (p) {
//	d->mChat->setFromPlayer(p);
 }
}

void BosonBigDisplay::resizeEvent(QResizeEvent* e)
{
 QCanvasView::resizeEvent(e);
 // FIXME: is width()/height() correct? rather the ones from viewport()!
 // use the same line as im resizeContents()!
 emit signalSizeChanged(width(), height());
 slotContentsMoving(contentsX(), contentsY());
}

void BosonBigDisplay::slotEditorMouseEvent(QMouseEvent* e, bool* eatevent)
{
 *eatevent = true;
 QWMatrix wm = inverseWorldMatrix(); // for zooming
 QPoint pos = viewportToContents(e->pos());
 wm.map(pos.x(), pos.y(), &pos.rx(), &pos.ry());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
			removeSelectionRect();
		} else if (e->button() == RightButton) {
			if (d->mIsRMBMove) {
				d->mIsRMBMove = false;
			} else {
				editorActionClicked(pos);
			}
		}
		break;
	case QEvent::MouseMove:
	{
		if (e->state() & LeftButton) {
			moveSelectionRect(pos);
		} else if (e->state() & RightButton) {
			d->mIsRMBMove = true;
			scrollBy(pos.x() - d->mRMBMove.x(), pos.y() - d->mRMBMove.y());
			d->mRMBMove = pos;
		}
		break;
	}
	case QEvent::MouseButtonPress:
		if (e->button() == LeftButton) {
			startSelection(pos);
		} else if (e->button() == MidButton) {
			center(pos.x(), pos.y());
		} else if (e->button() == RightButton) {
			d->mRMBMove = pos;
		}
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		break;
 }
 e->accept();
}

void BosonBigDisplay::editorActionClicked(const QPoint& pos)
{
// FIXME: should be done on left click?

// kdDebug() << k_funcinfo << endl;
 int x = pos.x() / BO_TILE_SIZE;
 int y = pos.y() / BO_TILE_SIZE;
 if (d->mConstruction.unitType > -1) {
	if (!d->mConstruction.owner) {
		kdWarning() << k_funcinfo << ": NO OWNER" << endl;
//		return;
	}
	
	emit signalBuildUnit(d->mConstruction.unitType, x, y,
			d->mConstruction.owner);
	setModified(true);
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
	setModified(true);
 }
}


void BosonBigDisplay::slotWillConstructUnit(int unitType, UnitBase* factory, KPlayer* owner)
{
 if (!owner) {
	kdDebug() << k_funcinfo << ": NULL owner" << endl;
	d->mConstruction.groundType = -1;
	d->mConstruction.unitType = -1;
	return;
 }
// kdDebug() << unitType << endl;
 if (factory) { // usual production
//	kdDebug() << "there is a factory " << endl;
	return;
	// this MUST be sent over network - therefore use CommandInput!
	((Facility*)factory)->addProduction(unitType);
 } else { // we are in editor mode!
	//FIXME
	// should be sent over network
	d->mConstruction.unitType = unitType;
	d->mConstruction.factory = (Unit*)factory;
	d->mConstruction.owner = (Player*)owner;
	d->mConstruction.groundType = -1;
 }
}

void BosonBigDisplay::slotWillPlaceCell(int groundType)
{
 d->mConstruction.unitType = -1;
 d->mConstruction.groundType = groundType;
}

void BosonBigDisplay::slotUnitChanged(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
// kdDebug() << k_funcinfo << endl;
 if (selection().contains(unit)) {
//	kdDebug() << "is selected" << endl;
	if (unit->isDestroyed()) {
		d->mSelectionList.removeRef(unit);
		unit->unselect();
		emit signalUnselectUnit(unit);
		if (selection().isEmpty()) {
			setSelectionMode(SelectNone);
		}
	}
	if (selectionMode() == SelectSingle) {
//		kdDebug() << "is only unit" << endl;
		emit signalSingleUnitSelected(unit); // update
	}
 }
}

void BosonBigDisplay::resizeContents(int w, int h)
{
 QCanvasView::resizeContents(w, h);
 QWMatrix wm = inverseWorldMatrix();
 QRect br = wm.mapRect(QRect(0,0,width(),height()));
 emit signalSizeChanged(br.width(), br.height());
}

void BosonBigDisplay::slotUpdateMinerals(int minerals)
{
 d->mMinerals->setText(i18n("Minerals: %1").arg(minerals));
}

void BosonBigDisplay::slotUpdateOil(int oil)
{
 d->mOil->setText(i18n("Oil: %1").arg(oil));
}

void BosonBigDisplay::slotContentsMoving(int x, int y)
{
 d->mMinerals->move(x + visibleWidth() - 5 - d->mMinerals->boundingRect().width(), y + 5);
 d->mOil->move     (x + visibleWidth() - 5 - d->mOil->boundingRect().width(), y + 5 + d->mMinerals->boundingRect().height());
 canvas()->update();

 d->mChat->move(x + 10, y + visibleHeight() - 10); // FIXME: hardcoded!
}

bool BosonBigDisplay::isModified() const
{
 return d->mIsModified;
}

void BosonBigDisplay::setModified(bool m)
{
 d->mIsModified = m;
}

void BosonBigDisplay::setKGameChat(KGameChat* c)
{
 d->mChat->setChat(c);
}

void BosonBigDisplay::addChatMessage(const QString& message)
{
 d->mChat->addMessage(message);
}
