
#include "bosonbigdisplay.h"
#include "visualunit.h"
#include "bosoncanvas.h"
#include "player.h"
#include "unitproperties.h"

#include "defines.h"
#include "bosonmessage.h"

#include <kgame/kgameio.h>
#include <kdebug.h>

#include <qptrlist.h>
#include <qpoint.h>
#include <qpainter.h>

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
	VisualUnit* factory; // facility that constructs the unit (or NULL in editor mode)
	Player* owner; // the owner of the unit - probably only for editor mode.
	int groundType;
	unsigned char version;
};

class BosonBigDisplayPrivate
{
public:
	BosonBigDisplayPrivate()
	{
		mSelectionMode = BosonBigDisplay::SelectNone;
		mIsSelecting = false;

		mLocalPlayer = 0;
	}

	BosonBigDisplay::SelectionMode mSelectionMode;
	QPtrList<VisualUnit> mSelectionList;
	QPoint mSelectionStart;
	QPoint mSelectionEnd;
	bool mIsSelecting;

	Player* mLocalPlayer;

	ConstructUnit mConstruction;
	
};

BosonBigDisplay::BosonBigDisplay(QCanvas* c, QWidget* parent) : QCanvasView(c, parent)
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
}

BosonBigDisplay::~BosonBigDisplay()
{
 delete d;
}

void BosonBigDisplay::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
// *eatevent = true;//FIXME: it seems that this makes KGameIO *SEND* the input?
// e->accept() is not called for this?!
 QPoint pos = viewportToContents(e->pos());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
			removeSelectionRect();
		}
		e->accept();
		break;
	case QEvent::MouseMove:
	{
		if (e->state() & LeftButton) {
			moveSelectionRect(pos);
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
		if (e->button() == LeftButton) {
			startSelection(pos);
		} else if (e->button() == RightButton) {
		 	// AB: is *eatevent the correct parameter? KGame should
			// *not* send the stream if this is false!
			bool send = false;
		 	actionClicked(viewportToContents(e->pos()), stream, send);
			if (send) {
				*eatevent = true;
			}
		}
//		e->accept();
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		break;
 }
}

void BosonBigDisplay::startSelection(const QPoint& pos)
{
 // LMB clicked - either select the unit or start a selection rect
 VisualUnit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
 if (!unit) {
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
 addUnitSelection(unit);
 emit signalSingleUnitSelected(unit);
 //canvas->update(); // TODO?
}

void BosonBigDisplay::moveSelectionRect(const QPoint& newEnd)
{
 if (selectionMode() == SelectRect) {
	// remove an old selection rect
	if (d->mIsSelecting) {
		drawSelectionRect();
	}

	d->mSelectionEnd = newEnd;

	// draw the new selection rect
	drawSelectionRect();
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea(QRect(selectionStart(), pos));
 }
}

void BosonBigDisplay::removeSelectionRect()
{
 if (d->mIsSelecting) { // FIXME: can we se selectionMode() for this?
	// remove the rect:
	drawSelectionRect();
	// here as there is a performance problem in
	// mousemove:
	selectArea(QRect(selectionStart(), selectionEnd()));
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
 QPtrListIterator<VisualUnit> it(d->mSelectionList);
 while (it.current()) {
	it.current()->unselect();
	++it;
 }
 d->mSelectionList.clear();
}

void BosonBigDisplay::addUnitSelection(VisualUnit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->owner() != d->mLocalPlayer) {
	kdDebug() << k_funcinfo << ": not owner" << endl;
	return;
 }
 if (d->mSelectionList.contains(unit)) {
	return;
 }
 d->mSelectionList.append(unit);
 unit->select();
}

const QPoint& BosonBigDisplay::selectionStart() const
{
 return d->mSelectionStart;
}

const QPoint& BosonBigDisplay::selectionEnd() const
{
 return d->mSelectionEnd;
}

QPtrList<VisualUnit>& BosonBigDisplay::selection() const
{
 return d->mSelectionList;
}

void BosonBigDisplay::selectArea(const QRect& rect)
{
 QCanvasItemList list;
 QCanvasItemList::Iterator it;
// return;
 list = canvas()->collisions(rect.normalize());

 for (it = list.begin(); it != list.end(); ++it) {
	if ((*it)->rtti() >= RTTI::UnitStart) { // AKA isUnit
		VisualUnit* unit = (VisualUnit*)*it;
		if (unit->unitProperties()->isMobile()) { // no NULL check for performance
			addUnitSelection(unit);
		}
	}
 }
}

void BosonBigDisplay::drawSelectionRect()
{
// kdDebug() << k_funcinfo << endl;
 d->mIsSelecting = !d->mIsSelecting;
 QPen pen(red, 2); // FIXME: hardcoded
 QPainter painter;
 if (!painter.begin(viewport())) {
	kdError() << k_funcinfo << ": Cannot begin" << endl;
	return;
 }
 painter.setPen(pen);
 painter.setRasterOp(XorROP);

// might cause problems when viewport is moved:
 QPoint pos1 = contentsToViewport(selectionStart());
 QPoint pos2 = contentsToViewport(selectionEnd());
 QRect r(pos1, pos2);
 r.normalize();
 if (d->mIsSelecting) {
//	kdDebug() << "draw:   " << r.left() << " " << r.top() << " -> " <<
//			r.right() << " " << r.bottom() << endl;
 } else {
//	kdDebug() << "remove: " << r.left() << " " << r.top() << " -> " <<
//			r.right() << " " << r.bottom() << endl;
 }
 painter.drawRect(r);
 painter.end();
}

void BosonBigDisplay::slotReCenterView(const QPoint& pos)
{
// pos.x() and pos.y() are the cell numbers! *= BO_TILE_SIZE are the coordinates
 setContentsPos(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE);
}

void BosonBigDisplay::actionClicked(const QPoint& pos, QDataStream& stream, bool& send)
{// pos is already viewportToContents()'ed
// this method should not perform any tasks but rather send the input through
// the KGameIO. this way it is very easy (it should be at least) to write a
// computer player
 if (selection().isEmpty()) {
	return;
 }
 if (selectionMode() == SelectNone) {
	kdError() << k_funcinfo << ": SelectNone" << endl;
	return;
 }
 VisualUnit* unit = ((BosonCanvas*)canvas())->findUnitAt(pos);
 if (!unit) {
	if ((selection().first()->unitProperties()->isMobile())) { // move the selection to pos
		QPtrListIterator<VisualUnit> it(selection());
		// tell the clients we want to move units:
		stream << (Q_UINT32)BosonMessage::MoveMove;
		// tell them where to move to:
		stream << pos;
		// tell them how many units:
		stream << (Q_UINT32)selection().count();
		while (it.current()) {
			// tell them which unit to move:
			stream << (Q_ULONG)it.current()->id(); // MUST BE UNIQUE!
			++it;
		}
		send = true;
	} else { // place constructions
		VisualFacility* fac = (VisualFacility*)selection().first();
		if (fac->unitProperties()->canProduce() && fac->hasConstruction()) {
			// create the new unit
			stream << (Q_UINT32)BosonMessage::MoveConstruct;
			stream << (Q_UINT32)fac->id();
			stream << (Q_UINT32)fac->owner()->id();
//			kdDebug() << fac->id() << endl;
			stream << (Q_INT32)pos.x() / BO_TILE_SIZE;
			stream << (Q_INT32)pos.y() / BO_TILE_SIZE;
			send = true;
		}
	}
 } else { // there is a unit - attack it!
	QPtrListIterator<VisualUnit> it(selection());
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
 }
}

void BosonBigDisplay::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
}

void BosonBigDisplay::resizeEvent(QResizeEvent* e)
{
 QCanvasView::resizeEvent(e);
 emit signalSizeChanged(width(), height());
}

void BosonBigDisplay::slotEditorMouseEvent(QMouseEvent* e, bool* eatevent)
{
 *eatevent = true;
 QPoint pos = viewportToContents(e->pos());
 switch(e->type()) {
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		break;
	case QEvent::MouseButtonRelease:
		if (e->button() == LeftButton) {
			removeSelectionRect();
		}
		break;
	case QEvent::MouseMove:
	{
		if (e->state() & LeftButton) {
			moveSelectionRect(pos);
		}
		break;
	}
	case QEvent::MouseButtonPress:
		if (e->button() == LeftButton) {
			startSelection(pos);
		} else if (e->button() == RightButton) {
			editorActionClicked(pos);
		}
		break;
	default:
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		break;
 }
}

void BosonBigDisplay::editorActionClicked(const QPoint& pos)
{
// kdDebug() << k_funcinfo << endl;
 int x = pos.x() / BO_TILE_SIZE;
 int y = pos.y() / BO_TILE_SIZE;
// FIXME: should be done on left click?
 if (d->mConstruction.unitType > -1) {
	if (!d->mConstruction.owner) {
		kdWarning() << k_funcinfo << ": NO OWNER" << endl;
//		return;
	}
	
	emit signalConstructUnit(d->mConstruction.unitType, x, y,
			d->mConstruction.owner);
 } else if (d->mConstruction.groundType > -1) {
//	kdDebug() << "place ground " << d->mConstruction.groundType << endl;
	int version = 0; // FIXME: random()%4;
	// int version = d->mConstruction.version; // one of these lines - random here or in command frame?
	emit signalAddCell(x, y, d->mConstruction.groundType, version);
 }
}


void BosonBigDisplay::slotWillConstructUnit(int unitType, VisualUnit* factory, Player* owner)
{
 if (!owner) {
	kdDebug() << k_funcinfo << ": NULL owner" << endl;
	d->mConstruction.groundType = -1;
	d->mConstruction.unitType = -1;
	return;
 }
// kdDebug() << unitType << endl;
 d->mConstruction.unitType = unitType;
 d->mConstruction.factory = factory;
 d->mConstruction.owner = owner;
 d->mConstruction.groundType = -1;
 if (factory) {
	kdDebug() << "there is a factory " << endl;
	((VisualFacility*)factory)->addConstruction(unitType);
 }
}

void BosonBigDisplay::slotWillPlaceCell(int groundType, unsigned char version)
{
 d->mConstruction.unitType = -1;
 d->mConstruction.groundType = groundType;
 d->mConstruction.version = version;
}

void BosonBigDisplay::slotUnitChanged(VisualUnit* unit)
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
	}
	if (selectionMode() == SelectSingle) {
//		kdDebug() << "is only unit" << endl;
		emit signalSingleUnitSelected(unit); // update
	}
 }
}

