/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bodisplaymanager.h"
#include "bodisplaymanager.moc"

#include "bosonbigdisplay.h"
#include "bosonconfig.h"
#include "player.h"
#include "defines.h"

#include <klocale.h>

#include <qstyle.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qpainter.h>

#include <kdebug.h>

class BoBox : public QWidget
{
public:
	BoBox(QWidget* parent) : QWidget(parent, "bosondisplaybox")
	{
		mLayout = 0;
	}
	~BoBox()
	{
	}

	bool hasDisplay(BosonBigDisplay* display)
	{
		return mDisplays.containsRef(display);
	}

	void insert(unsigned int index, BosonBigDisplay* display)
	{
		if (hasDisplay(display)) {
			kdError() << k_funcinfo << "already have that display" << endl;
			remove(display);
		}
		mDisplays.insert(index, display);
		recreateLayout();
	}
	
	void remove(BosonBigDisplay* b)
	{
		if (!hasDisplay(b)) {
			kdError() << k_funcinfo << "don't have that display" << endl;
			return;
		}
		mDisplays.removeRef(b);
		recreateLayout();
	}
	
	unsigned int count() const
	{
		return mDisplays.count();
	}

	int find(BosonBigDisplay* b)
	{
		return mDisplays.findRef(b);
	}

protected:
	void recreateLayout()
	{
		delete mLayout;
		mLayout = new QHBoxLayout(this);
		QPtrListIterator<BosonBigDisplay> it(mDisplays);
		while (it.current()) {
			mLayout->addWidget(it.current());
			++it;
		}
		mLayout->activate();
	}

private:
	QPtrList<BosonBigDisplay> mDisplays;
	QHBoxLayout* mLayout;
};

class BoDisplayManager::BoDisplayManagerPrivate
{
public:
	BoDisplayManagerPrivate()
	{
		mCanvas = 0;
		mActiveDisplay = 0;

		mLayout = 0;
	}

	QPtrList<BosonBigDisplay> mDisplayList;
	QPtrList<BoBox> mBoxList;

	QCanvas* mCanvas;
	BosonBigDisplay* mActiveDisplay;

	QVBoxLayout* mLayout;
};

BoDisplayManager::BoDisplayManager(QCanvas* canvas, QWidget* parent) : QWidget(parent, "bosondisplaymanager")
{
 d = new BoDisplayManagerPrivate;
 d->mDisplayList.setAutoDelete(true);
 d->mBoxList.setAutoDelete(true);
 d->mCanvas = canvas;
}

BoDisplayManager::~BoDisplayManager()
{
 d->mDisplayList.clear();
 d->mBoxList.clear();
 delete d;
}

void BoDisplayManager::slotMakeActiveDisplay(BosonBigDisplay* display)
{
 if (display == d->mActiveDisplay) {
	return;
 }
 kdDebug() << k_funcinfo << endl;
 BosonBigDisplay* old = d->mActiveDisplay;
 d->mActiveDisplay = display;
 
 if (old) {
	markActive(old, false);
 }
 markActive(d->mActiveDisplay, true);
 emit signalActiveDisplay(d->mActiveDisplay, old);
}

void BoDisplayManager::markActive(BosonBigDisplay* display, bool active)
{
 if (!display) {
	kdWarning() << k_funcinfo << "NULL display" << endl;
	return;
 }
 display->setActive(active);
 if (active) {
	if (d->mDisplayList.count() > 1) {
		display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) + 3);
	} else {
		display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this));
	}
 } else {
	display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this));
 }
}

BosonBigDisplay* BoDisplayManager::activeDisplay() const
{
 return d->mActiveDisplay;
}

QPtrList<BosonBigDisplay> BoDisplayManager::displays() const
{
 return d->mDisplayList;
}

void BoDisplayManager::removeActiveDisplay()
{
 if (d->mDisplayList.count() <  2) {
	kdWarning() << k_funcinfo << "need at lest two displays" << endl;
	return;
 }
 if (!d->mActiveDisplay) {
	return;
 }
 BosonBigDisplay* old = d->mActiveDisplay;
 QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	if (it.current() != old) {
		it.current()->makeActive();
		break;
	}
	++it;
 }
 BoBox* box = findBox(old);
 if (!box) {
	kdError() << k_funcinfo << "Cannot find parent box" << endl;
	return;
 }

 box->remove(old);
 d->mDisplayList.removeRef(old);

 if (box->count() == 0) {
	d->mBoxList.removeRef(box);
 }

 // we need to mark twice - once above and once here - it may be that
 // d->mDisplayList.count() is 1 now
 markActive(d->mActiveDisplay, true);
 recreateLayout();
}

BosonBigDisplay* BoDisplayManager::splitActiveDisplayVertical()
{
 if (!activeDisplay()) {
	return 0;
 }
 kdDebug() << k_funcinfo << endl;
 
// we are not actually splitting the view but the entire row...
// ok splitting the view only is a TODO. but not an important one
 int index = d->mBoxList.findRef(findBox(activeDisplay()));
 if (index < 0) {
	kdDebug() << k_funcinfo << "Cannot find parent box for active display" << endl;
	return 0;
 }
 BoBox* box = new BoBox(this);
 BosonBigDisplay* b = addDisplay(box);
 box->insert(0, b);
 box->show();
 d->mBoxList.insert(index + 1, box);
 recreateLayout();
 return b;
}

BosonBigDisplay* BoDisplayManager::splitActiveDisplayHorizontal()
{
 if (!activeDisplay()) {
	return 0;
 }
 kdDebug() << k_funcinfo << endl;
 BoBox* box = findBox(activeDisplay());
 if (!box) {
	kdDebug() << k_funcinfo << "Cannot find parent box for active display" << endl;
	return 0;
 }
 BosonBigDisplay* b = addDisplay(box);
 box->insert(box->find(activeDisplay()) + 1, b);
 return b;
}

BosonBigDisplay* BoDisplayManager::addInitialDisplay()
{
 if (d->mDisplayList.count() != 0) {
	kdError() << k_funcinfo << "already have displays" << endl;
	return 0;
 }
 BoBox* box = new BoBox(this);
 d->mBoxList.append(box);
 BosonBigDisplay* b = addDisplay(box);
 box->insert(0, b);
 box->show();
 recreateLayout();
 return b;
}

BosonBigDisplay* BoDisplayManager::addDisplay(QWidget* parent)
{
 if (!parent) {
	kdError() << k_funcinfo << "parent must not be 0" << endl;
	return 0;
 }
 kdDebug() << k_funcinfo << endl;
 BosonBigDisplay* b = new BosonBigDisplay(d->mCanvas, parent);
 d->mDisplayList.append(b);
 connect(b, SIGNAL(signalMakeActive(BosonBigDisplay*)), 
		this, SLOT(slotMakeActiveDisplay(BosonBigDisplay*)));
 b->show();
 return b;
}

void BoDisplayManager::slotEditorWillPlaceCell(int c)
{
 QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	it.current()->slotWillPlaceCell(c);
	++it;
 }
}

void BoDisplayManager::slotEditorWillPlaceUnit(int type, UnitBase* fac, KPlayer* p)
{
 QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	it.current()->slotWillConstructUnit(type, fac, p);
	++it;
 }
}

void BoDisplayManager::setCursor(BosonCursor* cursor)
{
 QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setCursor(cursor);
	++it;
 }
}

void BoDisplayManager::setLocalPlayer(Player* p)
{
 QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setLocalPlayer(p);
	++it;
 }
}

void BoDisplayManager::quitGame()
{
  QPtrListIterator<BosonBigDisplay> it(d->mDisplayList);
 while (it.current()) {
	it.current()->quitGame();
	++it;
 }
}

BoBox* BoDisplayManager::findBox(BosonBigDisplay* b) const
{
 QPtrListIterator<BoBox> it(d->mBoxList); 
 while (it.current()) {
	if (it.current()->hasDisplay(b)) {
		return it.current();
	}
	++it;
 }
 return 0;
}

void BoDisplayManager::recreateLayout()
{
 delete d->mLayout;
 d->mLayout = new QVBoxLayout(this);
 QPtrListIterator<BoBox> it(d->mBoxList);
 while (it.current()) {
	d->mLayout->addWidget(it.current());
	++it;
 }
 d->mLayout->activate();
}

void BoDisplayManager::slotScroll(int dir)
{
 BosonBigDisplay* active = activeDisplay();
 if (!active) {
	return;
 }
 switch ((ScrollDirection)dir) {
	case ScrollUp:
		active->scrollBy(0, -boConfig->arrowKeyStep());
		break;
	case ScrollRight:
		active->scrollBy(boConfig->arrowKeyStep(), 0);
		break;
	case ScrollDown:
		active->scrollBy(0, boConfig->arrowKeyStep());
		break;
	case ScrollLeft:
		active->scrollBy(-boConfig->arrowKeyStep(), 0);
		break;
	default:
		return;
 }
}

void BoDisplayManager::paintResources()
{
#ifndef NO_BOSON_CANVASTEXT
//FIXME: paint on upper right display, not on active
 BosonBigDisplay* b = activeDisplay();
 if (!b) {
	kdError() << k_funcinfo << "NULL display" << endl;
	return;
 }
 Player* p = b->localPlayer();
 if (!p) {
	return;
 }
 //AB: we could make this configurable
 QFont font;
 QColor color = white;

 QPainter painter(b->viewport());
 painter.setFont(font);
 painter.setPen(color);

 QFontMetrics metrics(font);
 QString mineralText = i18n("Minerals: ");
 QString oilText = i18n("Oil: ");
 QString minerals = QString::number(p->minerals());
 QString oil = QString::number(p->oil());
 int w = QMAX(metrics.width(mineralText + minerals), metrics.width(oilText + oil)) + 5;
 int x = b->visibleWidth() - w - 10;
 int y = 10;
 painter.drawText(x, y, w, metrics.height(), AlignLeft, mineralText);
 painter.drawText(x, y, w, metrics.height(), AlignRight, minerals);
 y += metrics.height() + 5;
 painter.drawText(x, y, w, metrics.height(), AlignLeft, oilText);
 painter.drawText(x, y, w, metrics.height(), AlignRight, minerals);
 painter.end();
 kdDebug() << k_funcinfo << endl;
#endif
}

void BoDisplayManager::paintChatMessages()
{
 // TODO
}

