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
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplaybase.moc"

#include "defines.h"
#include "unit.h"
#include "unitplugins.h"
#include "bosoncanvas.h"
#include "player.h"
#include "unitproperties.h"
#include "cell.h"
#include "bosonmessage.h"
#include "bosoncursor.h"
#include "bosonmusic.h"
#include "bosonconfig.h"
#include "global.h"
#include "kspritetooltip.h"
#include "boselection.h"
#include "visual/bosonchat.h"
#include "visual/bosoncanvaschat.h"

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

class BosonBigDisplayBase::BosonBigDisplayBasePrivate
{
public:
	BosonBigDisplayBasePrivate()
	{
		mMouseIO = 0;
		
		mIsRMBMove = false;

		mLocalPlayer = 0;
		mCursor = 0;
		mUnitTips = 0;

		mChat = 0;

		mSelection = 0;
		mSelectionRect = 0;
	}

	KGameMouseIO* mMouseIO;

	bool mIsRMBMove;
	QPoint mRMBMove; // position where RMB move started
	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;

	Player* mLocalPlayer;
	BosonCursor* mCursor;
	KSpriteToolTip* mUnitTips;

	BosonChat* mChat;

	BoSelection* mSelection;
	QCanvasRectangle* mSelectionRect;
	QPoint mSelectionStart;
	QPoint mSelectionEnd;
};

BosonBigDisplayBase::BosonBigDisplayBase(QCanvas* c, QWidget* parent) : QCanvasView(c,
		parent, "bigdisplay", 
		WRepaintNoErase |// this should remove some flickering
		WStaticContents | 
		WResizeNoErase
		)
{
 init();
}

void BosonBigDisplayBase::init()
{
 d = new BosonBigDisplayBasePrivate;

// setSizePolicy(QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ));
// setResizePolicy(QScrollView::AutoOne);
 setVScrollBarMode(AlwaysOff);
 setHScrollBarMode(AlwaysOff);
 d->mCursorEdgeCounter = 0;

 connect(this, SIGNAL(contentsMoving(int, int)), 
		this, SLOT(slotContentsMoving(int, int)));
 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()),
		this, SLOT(slotCursorEdgeTimeout()));

 d->mChat = (BosonChat*)new BosonCanvasChat(this);
 ((BosonCanvasChat*)d->mChat)->setCanvas(canvas());
 ((BosonCanvasChat*)d->mChat)->setZ(Z_CANVASTEXT);

 d->mUnitTips = new KSpriteToolTip(this);

 d->mSelection = new BoSelection(this);

 disconnect(this, SIGNAL(contentsMoving(int,int)), this, SLOT(cMoving(int,int)));

 // the following needs to be done only once for all displays. we can safely
 // call it for every new display, it'll just be ignored.
 KSpriteToolTip::ignore(RTTI::SelectPart);
 KSpriteToolTip::ignore(RTTI::BoShot);
 KSpriteToolTip::ignore(RTTI::FogOfWar);// we don't display "fog of war" or so...
 KSpriteToolTip::ignore(RTTI::SpriteCursor);// we don't display "fog of war" or so...
}

BosonBigDisplayBase::~BosonBigDisplayBase()
{
 delete d->mUnitTips;
 delete d->mSelectionRect;
 delete d->mChat;
 delete d;
}

void BosonBigDisplayBase::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
//TODO: use a KGameIO for the editor !!! very important, since we need to set *eatevent = true otherwise

// *eatevent = true;//FIXME: it seems that this makes KGameIO *SEND* the input?
// e->accept() is not called for this?!
 QWMatrix wm = inverseWorldMatrix(); // for zooming
 QPoint pos = viewportToContents(e->pos());
 wm.map(pos.x(), pos.y(), &pos.rx(), &pos.ry());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
	{
		makeActive();
		if (e->button() == LeftButton) {
			Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
			if (unit) {
				if (!selectAll(unit->unitProperties())) {
					d->mSelection->selectUnit(unit);
				}
			}
		}
		e->accept();
		break;
	}
	case QEvent::Wheel:
		// qt already gives us wheel support :-)
		e->ignore();
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
			removeSelectionRect();
		} else if (e->button() == RightButton) {
			if (d->mIsRMBMove) {
				d->mIsRMBMove = false;
			} else {
				//TODO: port editor to KGameIO - otherwise
				//eatevent and send are useless

			 	// AB: is *eatevent the correct parameter? KGame should
				// *not* send the stream if this is false!
				bool send = false;
				// pos must be viewportToContents'ed and mapped using
				// the inverse matrix!
				BoAction action;
				action.setPos(pos);
				if (e->state() & ControlButton) {
					action.setForceAttack(true);
				}
			 	actionClicked(&action, stream, send);
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
			if (boConfig->rmbMove()) {
				d->mIsRMBMove = true;
				scrollBy(pos.x() - d->mRMBMove.x(), pos.y() - d->mRMBMove.y());
				d->mRMBMove = pos;
			}
		}
		updateCursor();
		canvas()->update();
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
		makeActive();
		if (e->button() == LeftButton) {
			if(actionLocked()) {
				// If action is locked then it means that user clicked on an action
				//  button and wants to perform specific action
				bool send = false;
				// pos must be viewportToContents'ed and mapped using
				// the inverse matrix!
				BoAction action;
				action.setPos(pos);
			 	actionClicked(&action, stream, send);
				if (send) {
					*eatevent = true;
				}
			}
			else {
				startSelection(pos);
			}
		} else if (e->button() == MidButton) {
			if (boConfig->mmbMove()) {
				//TODO can we move the cursor-replacing code
				//directly into center() ?
				center(pos.x(), pos.y());
				updateCursor();
				canvas()->update();
			}
		} else if (e->button() == RightButton) {
			d->mRMBMove = pos;
		}
		e->accept();
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		break;
 }
}

void BosonBigDisplayBase::startSelection(const QPoint& pos)
{
 // LMB clicked - either select the unit or start a selection rect
 Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
 if (!unit) {
	// nothing has been found : it's a ground-click
	// Here, we have to draw a "selection box"...
	d->mSelectionStart = pos;
	d->mSelectionEnd = pos;
	// create the rect so it is drawn once the mouse is moved
	drawSelectionRect();
	return;
 }

 d->mSelection->selectUnit(unit);

 // cannot be placed into d->mSelection cause we don't have d->mLocalPlayer
 // there
 if (d->mLocalPlayer == unit->owner()) {
	boMusic->playSound(unit, Unit::SoundOrderSelect);
 }
 //canvas->update(); // TODO?
}

void BosonBigDisplayBase::moveSelectionRect(const QPoint& newEnd)
{
 if (d->mSelectionRect && d->mSelectionRect->isVisible()) {
	d->mSelectionEnd = newEnd;

	// draw the new selection rect
	drawSelectionRect();
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonBigDisplayBase::removeSelectionRect()
{
 if (d->mSelectionRect && d->mSelectionRect->isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	selectArea();

	// remove the rect:
	delete d->mSelectionRect;
	d->mSelectionRect = 0;
	if (!d->mSelection->isEmpty()) {
		Unit* u = d->mSelection->leader();
		if (u->owner() == d->mLocalPlayer) {
			//TODO: do not play sound here
			//instead make virtual and play in derived class
			boMusic->playSound(u, Unit::SoundOrderSelect);
		}
	}
 }
}

bool BosonBigDisplayBase::selectAll(const UnitProperties* prop)
{
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return false;
 }
 if (prop->isFacility()) {
	// we don't select all facilities, but only the one that was
	// double-clicked. it makes no sense for facilities
	return false;
 }
 QPtrList<Unit> allUnits = d->mLocalPlayer->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> it(allUnits);
 while (it.current()) {
	if (it.current()->unitProperties() == prop) {
		list.append(it.current());
	}
	++it;
 }
 d->mSelection->selectUnits(list);
 return true;
}

const QPoint& BosonBigDisplayBase::selectionStart() const
{
 return d->mSelectionStart;
}

const QPoint& BosonBigDisplayBase::selectionEnd() const
{
 return d->mSelectionEnd;
}

void BosonBigDisplayBase::selectArea()
{
 if (!d->mSelectionRect) {
	kdDebug() << "no rect" << endl;
	return;
 }
 if (boConfig->debugMode() == BosonConfig::DebugSelection) {
	QCanvasItemList list;
	list = d->mSelectionRect->collisions(true);
	QCanvasItemList::Iterator it;
	kdDebug() << "Selection count: " << list.count() << endl;
	for (it = list.begin(); it != list.end(); ++it) {
		QString s = QString("Selected: RTTI=%1").arg((*it)->rtti());
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			s += QString(" Unit ID=%1").arg(u->id());
			if (u->isDestroyed()) {
				s += QString("(destroyed)");
			}
		}
		kdDebug() << s << endl;
	}
 }
 QCanvasItemList list;
 QPtrList<Unit> unitList;
 Unit* fallBackUnit= 0; // in case no localplayer mobile unit can be found we'll select this instead
 QCanvasItemList playerUnitList;
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
		if (unit->owner() == d->mLocalPlayer) {
			unitList.append(unit);
		} else {
			fallBackUnit = unit;
		}
	} else {
		fallBackUnit = unit; 
	}
	
 }

 if (unitList.count() > 0) {
	d->mSelection->selectUnits(unitList);
 } else if (fallBackUnit) {
	d->mSelection->selectUnit(fallBackUnit);
 } else {
	d->mSelection->clear();
 }
}

void BosonBigDisplayBase::drawSelectionRect()
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

void BosonBigDisplayBase::slotReCenterView(const QPoint& pos)
{
// pos.x() and pos.y() are the cell numbers! *= BO_TILE_SIZE are the coordinates
 center(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE);
}

void BosonBigDisplayBase::setLocalPlayer(Player* p)
{
 if (d->mLocalPlayer == p) {
	return;
 }
 d->mLocalPlayer = p;
}

Player* BosonBigDisplayBase::localPlayer() const
{
 return d->mLocalPlayer;
}

void BosonBigDisplayBase::resizeEvent(QResizeEvent* e)
{
 QCanvasView::resizeEvent(e);
 // FIXME: is width()/height() correct? rather the ones from viewport()!
 // use the same line as im resizeContents()!
 emit signalSizeChanged(width(), height());

 slotContentsMoving(contentsX(), contentsY());
}

/*
void BosonBigDisplayBase::slotEditorMouseEvent(QMouseEvent* e, bool* eatevent)
{
 *eatevent = true;
 QWMatrix wm = inverseWorldMatrix(); // for zooming
 QPoint pos = viewportToContents(e->pos());
 wm.map(pos.x(), pos.y(), &pos.rx(), &pos.ry());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
		makeActive();
		break;
	case QEvent::Wheel:
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
//			removeSelectionRect();
		} else if (e->button() == RightButton) {
			if (d->mIsRMBMove) {
				d->mIsRMBMove = false;
			} else {
				BoAction action;
				action.setPos(pos);
				editorActionClicked(&action);
			}
		}
		break;
	case QEvent::MouseMove:
	{
		if (e->state() & LeftButton) {
//			moveSelectionRect(pos);
		} else if (e->state() & RightButton) {
			if (boConfig->rmbMove()) {
				d->mIsRMBMove = true;
				scrollBy(pos.x() - d->mRMBMove.x(), pos.y() - d->mRMBMove.y());
				d->mRMBMove = pos;
			}
		}
		break;
	}
	case QEvent::MouseButtonPress:
		makeActive();
		if (e->button() == LeftButton) {
			if (((BosonCanvas*)canvas())->findUnitAt(pos)) {
				startSelection(pos);
			}
		} else if (e->button() == MidButton) {
			if (boConfig->mmbMove()) {
				center(pos.x(), pos.y());
			}
		} else if (e->button() == RightButton) {
			d->mRMBMove = pos;
		}
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		break;
 }
 e->accept();
}*/

void BosonBigDisplayBase::slotUnitChanged(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (d->mSelection->contains(unit)) {
	if (unit->isDestroyed()) {
		d->mSelection->removeUnit(unit);
	}
 }
}

void BosonBigDisplayBase::resizeContents(int w, int h)
{
 QCanvasView::resizeContents(w, h);
 QWMatrix wm = inverseWorldMatrix();
 QRect br = wm.mapRect(QRect(0,0,width(),height()));
 emit signalSizeChanged(br.width(), br.height());
}

void BosonBigDisplayBase::slotContentsMoving(int newx, int newy)
{
// d->mChat->move(newx + 10, newy + visibleHeight() - 10);
}

void BosonBigDisplayBase::setKGameChat(KGameChat* c)
{
 d->mChat->setChat(c);
}

void BosonBigDisplayBase::addChatMessage(const QString& message)
{
 d->mChat->addMessage(message);
}

void BosonBigDisplayBase::enterEvent(QEvent*)
{
 if (!cursor()) {
	// we do *not* complain (i.e. genereate an error) here
	// cursor() == 0 may be possible in editor one day
	return;
 }
 cursor()->showCursor();
}

void BosonBigDisplayBase::leaveEvent(QEvent*)
{
 if (!cursor()) {
	// we do *not* complain (i.e. genereate an error) here
	// cursor() == 0 may be possible in editor one day
	return;
 }
 cursor()->hideCursor();
}

void BosonBigDisplayBase::setCursor(BosonCursor* cursor)
{
 d->mCursor = cursor;
}

/*
void BosonBigDisplayBase::slotMoveSelection(int cellX, int cellY)
{
 if (!d->mLocalPlayer) {
	kdError() << "NULL local player" << endl;
	return;
 }
 if (d->mSelection->isEmpty()) {
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 bool send = false;
 BoAction action;
 action.setPos(QPoint(cellX * BO_TILE_SIZE + BO_TILE_SIZE / 2, cellY * BO_TILE_SIZE + BO_TILE_SIZE / 2));
 actionClicked(&action, stream, send);
 if (send) {
	QDataStream msg(buffer, IO_ReadOnly);
	d->mLocalPlayer->forwardInput(msg, true);
 }
}
*/

void BosonBigDisplayBase::makeActive()
{
 emit signalMakeActive(this);
}

void BosonBigDisplayBase::setActive(bool a)
{
 if (a) {
	qApp->setGlobalMouseTracking(true);
	qApp->installEventFilter(this);
 } else {
	qApp->setGlobalMouseTracking(false);
	qApp->removeEventFilter(this);
 }
 d->mSelection->activate(a);
}

void BosonBigDisplayBase::setContentsPos(int x, int y)
{
// this hack is not really perfect. we still have some flickering in the
// resources display since it is redrawn on *every* QCanvas::update() call now.
// But hey - at least its working now :-)
#ifndef NO_BOSON_CANVASTEXT
	kdDebug() << k_funcinfo<<endl;
 viewport()->setUpdatesEnabled(false);
 QScrollView::setContentsPos(x, y);
 viewport()->setUpdatesEnabled(true);
 viewport()->update();
#else
 QScrollView::setContentsPos(x, y);
#endif
}

void BosonBigDisplayBase::slotCursorEdgeTimeout()
{
 int x = 0;
 int y = 0;
 const int sensity = boConfig->cursorEdgeSensity();
 QWidget* w = qApp->mainWidget();
 QPoint pos = w->mapFromGlobal(QCursor::pos());

 const int move = 15; // FIXME hardcoded - use BosonConfig instead
 if (pos.x() <= sensity && pos.x() > -1) {
	x = -move;
 } else if (pos.x() >= w->width() - sensity && pos.x() <= w->width()) {
	x = move;
 }
 if (pos.y() <= sensity && pos.y() > -1) {
	y = -move;
 } else if (pos.y() >= w->height() - sensity && pos.y() <= w->height()) {
	y = move;
 }
 if (!x && !y || !sensity) {
	d->mCursorEdgeTimer.stop();
	d->mCursorEdgeCounter = 0;
 } else {
	if (!d->mCursorEdgeTimer.isActive()) {
		d->mCursorEdgeTimer.start(20);
	}
	d->mCursorEdgeCounter++;
	if (d->mCursorEdgeCounter > 30) {
		scrollBy(x, y);
	}
	
 }
}

bool BosonBigDisplayBase::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseMove:
		if (!d->mCursorEdgeTimer.isActive()) {
			slotCursorEdgeTimeout();
		}
		break;
	default:
		break;
 }
 return QCanvasView::eventFilter(o, e);
}

BoSelection* BosonBigDisplayBase::selection() const
{
 return d->mSelection;
}

void BosonBigDisplayBase::quitGame()
{
 d->mSelection->clear();
}

void BosonBigDisplayBase::addMouseIO(Player* p)
{
//AB: make sure that both editor and game mode can share the same IO !
 if (!d->mLocalPlayer) {
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
 d->mLocalPlayer->addGameIO(d->mMouseIO);
}

BosonCursor* BosonBigDisplayBase::cursor() const
{
 return d->mCursor;
}
