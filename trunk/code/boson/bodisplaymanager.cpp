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

#include "defines.h"
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplayinput.h"
#include "editorbigdisplayinput.h"
#include "bosonconfig.h"
#include "boselection.h"
#include "bodebug.h"

#include <klocale.h>

#include <qstyle.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qintdict.h>
#include <qdom.h>

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

	bool hasDisplay(BosonBigDisplayBase* display)
	{
		return mDisplays.containsRef(display);
	}

	void insert(unsigned int index, BosonBigDisplayBase* display)
	{
		if (hasDisplay(display)) {
			boError() << k_funcinfo << "already have that display" << endl;
			remove(display);
		}
		mDisplays.insert(index, display);
		recreateLayout();
	}
	
	void remove(BosonBigDisplayBase* b)
	{
		if (!hasDisplay(b)) {
			boError() << k_funcinfo << "don't have that display" << endl;
			return;
		}
		mDisplays.removeRef(b);
		recreateLayout();
	}
	
	unsigned int count() const
	{
		return mDisplays.count();
	}

	int find(BosonBigDisplayBase* b)
	{
		return mDisplays.findRef(b);
	}

protected:
	void recreateLayout()
	{
		delete mLayout;
		mLayout = new QHBoxLayout(this);
		QPtrListIterator<BosonBigDisplayBase> it(mDisplays);
		while (it.current()) {
			mLayout->addWidget(it.current());
			++it;
		}
		mLayout->activate();
	}

private:
	QPtrList<BosonBigDisplayBase> mDisplays;
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

	QPtrList<BosonBigDisplayBase> mDisplayList;
	QPtrList<BoBox> mBoxList;

	BosonCanvas* mCanvas;
	BosonBigDisplayBase* mActiveDisplay;

	QVBoxLayout* mLayout;

	QIntDict<BoSelection> mSelectionGroups;
};

BoDisplayManager::BoDisplayManager(BosonCanvas* canvas, QWidget* parent, bool gameMode) : QWidget(parent, "bosondisplaymanager")
{
 d = new BoDisplayManagerPrivate;
 d->mDisplayList.setAutoDelete(true);
 d->mBoxList.setAutoDelete(true);
 d->mCanvas = canvas;
 mGameMode = gameMode;
 d->mSelectionGroups.setAutoDelete(true);
 for (int i = 0; i < 10; i++) {
	BoSelection* s = new BoSelection(this);
	d->mSelectionGroups.insert(i, s);
 }
}

BoDisplayManager::~BoDisplayManager()
{
 boDebug() << k_funcinfo << endl;
 d->mSelectionGroups.clear();
 boDebug() << k_funcinfo << "clearing display list" << endl;
 d->mDisplayList.clear();
 boDebug() << k_funcinfo << "clearing box list" << endl;
 d->mBoxList.clear();
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BoDisplayManager::slotMakeActiveDisplay(BosonBigDisplayBase* display)
{
 if (display == d->mActiveDisplay) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 BosonBigDisplayBase* old = d->mActiveDisplay;
 d->mActiveDisplay = display;
 
 if (old) {
	markActive(old, false);
 }
 markActive(d->mActiveDisplay, true);
 emit signalActiveDisplay(d->mActiveDisplay, old);
}

void BoDisplayManager::markActive(BosonBigDisplayBase* display, bool active)
{
 if (!display) {
	boWarning() << k_funcinfo << "NULL display" << endl;
	return;
 }
 display->setActive(active);
 // obsolete for OpenGL?
 /*
 if (active) {
	if (d->mDisplayList.count() > 1) {
		display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) + 3);
	} else {
		display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this));
	}
 } else {
	display->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, this));
 }
 */
}

BosonBigDisplayBase* BoDisplayManager::activeDisplay() const
{
 return d->mActiveDisplay;
}

/*
QPtrList<BosonBigDisplayBase> BoDisplayManager::displays() const
{
 return d->mDisplayList;
}*/

void BoDisplayManager::removeActiveDisplay()
{
 if (d->mDisplayList.count() <  2) {
	boWarning() << k_funcinfo << "need at lest two displays" << endl;
	return;
 }
 if (!d->mActiveDisplay) {
	return;
 }
 BosonBigDisplayBase* old = d->mActiveDisplay;
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	if (it.current() != old) {
		it.current()->makeActive();
		break;
	}
	++it;
 }
 BoBox* box = findBox(old);
 if (!box) {
	boError() << k_funcinfo << "Cannot find parent box" << endl;
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

BosonBigDisplayBase* BoDisplayManager::splitActiveDisplayVertical()
{
 if (!activeDisplay()) {
	return 0;
 }
 boDebug() << k_funcinfo << endl;
 
// we are not actually splitting the view but the entire row...
// ok splitting the view only is a TODO. but not an important one
 int index = d->mBoxList.findRef(findBox(activeDisplay()));
 if (index < 0) {
	boDebug() << k_funcinfo << "Cannot find parent box for active display" << endl;
	return 0;
 }
 BoBox* box = new BoBox(this);
 BosonBigDisplayBase* b = addDisplay(box);
 box->insert(0, b);
 box->show();
 d->mBoxList.insert(index + 1, box);
 recreateLayout();
 return b;
}

BosonBigDisplayBase* BoDisplayManager::splitActiveDisplayHorizontal()
{
 if (!activeDisplay()) {
	return 0;
 }
 boDebug() << k_funcinfo << endl;
 BoBox* box = findBox(activeDisplay());
 if (!box) {
	boDebug() << k_funcinfo << "Cannot find parent box for active display" << endl;
	return 0;
 }
 BosonBigDisplayBase* b = addDisplay(box);
 box->insert(box->find(activeDisplay()) + 1, b);
 return b;
}

BosonBigDisplayBase* BoDisplayManager::addInitialDisplay()
{
 if (d->mDisplayList.count() != 0) {
	boError() << k_funcinfo << "already have displays" << endl;
	return 0;
 }
 BoBox* box = new BoBox(this);
 d->mBoxList.append(box);
 BosonBigDisplayBase* b = addDisplay(box);
 box->insert(0, b);
 box->show();
 recreateLayout();
 return b;
}

BosonBigDisplayBase* BoDisplayManager::addDisplay(QWidget* parent)
{
 if (!parent) {
	boError() << k_funcinfo << "parent must not be 0" << endl;
	return 0;
 }
 boDebug() << k_funcinfo << endl;
 //TODO: what about editor widgets??
 BosonBigDisplayBase* b = 0;
 b = new BosonBigDisplayBase(d->mCanvas, parent);
 if (mGameMode) {
	b->setDisplayInput(new BosonBigDisplayInput(b));
 } else {
	b->setDisplayInput(new EditorBigDisplayInput(b));
 }
 connect(b->displayInput(), SIGNAL(signalLockAction(bool)), this, SIGNAL(signalLockAction(bool)));
 d->mDisplayList.append(b);
 connect(b, SIGNAL(signalMakeActive(BosonBigDisplayBase*)), 
		this, SLOT(slotMakeActiveDisplay(BosonBigDisplayBase*)));
 b->show();
 return b;
}

void BoDisplayManager::setCursor(BosonCursor* cursor)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setCursor(cursor);
	++it;
 }
}

void BoDisplayManager::setLocalPlayer(Player* p)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	boDebug() << k_funcinfo << endl;
	it.current()->setLocalPlayer(p);
	++it;
 }
}

void BoDisplayManager::quitGame()
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->quitGame();
	++it;
 }
}

BoBox* BoDisplayManager::findBox(BosonBigDisplayBase* b) const
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
 BosonBigDisplayBase* active = activeDisplay();
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

void BoDisplayManager::slotUpdateIntervalChanged(unsigned int ms)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setUpdateInterval(ms);
	++it;
 }
}

void BoDisplayManager::slotCenterHomeBase()
{
 if (activeDisplay()) {
	activeDisplay()->slotCenterHomeBase();
 }
}

void BoDisplayManager::slotResetViewProperties()
{
 if (activeDisplay()) {
	activeDisplay()->slotResetViewProperties();
 }
}

void BoDisplayManager::slotSelectGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 if (!activeDisplay()) {
	boError() << k_funcinfo << "NULL active display" << endl;
	return;
 }
 activeDisplay()->selection()->copy(d->mSelectionGroups[number]);
}

void BoDisplayManager::slotCreateGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 if (!activeDisplay()) {
	boError() << k_funcinfo << "NULL active display" << endl;
	return;
 }
 d->mSelectionGroups[number]->copy(activeDisplay()->selection());
}

void BoDisplayManager::slotClearGroup(int number)
{
 if (number < 0 || number >= 10) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!d->mSelectionGroups[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 d->mSelectionGroups[number]->clear();
}

void BoDisplayManager::slotUnitAction(int action)
{
 activeDisplay()->unitAction(action);
}

void BoDisplayManager::slotPlaceUnit(unsigned long int unitType, Player* owner)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->placeUnit(unitType, owner);
}

void BoDisplayManager::slotPlaceCell(int tile)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->placeCell(tile);
}

void BoDisplayManager::slotDeleteSelectedUnits()
{
 if (!activeDisplay()) {
	boError() << k_funcinfo << "NULL active display" << endl;
	return;
 }
 activeDisplay()->displayInput()->deleteSelectedUnits();
}

void BoDisplayManager::addChatMessage(const QString& text)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->addChatMessage(text);
	++it;
 }
}

void BoDisplayManager::slotUnitRemoved(Unit* u)
{
 for(int i = 0; i < 10; i++) {
	d->mSelectionGroups[i]->removeUnit(u);
 }
}

void BoDisplayManager::mapChanged()
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->mapChanged();
	++it;
 }
}

void BoDisplayManager::slotSetDebugMapCoordinates(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugMapCoordinates(debug);
	++it;
 }
}

void BoDisplayManager::slotSetDebugShowCellGrid(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugShowCellGrid(debug);
	++it;
 }
}

void BoDisplayManager::slotSetDebugMatrices(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugMatrices(debug);
	++it;
 }
}

void BoDisplayManager::slotSetDebugItemWorks(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugItemWorks(debug);
	++it;
 }
}

void BoDisplayManager::slotSetDebugCamera(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugCamera(debug);
	++it;
 }
}

void BoDisplayManager::slotSetDebugRenderCounts(bool debug)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setDebugRenderCounts(debug);
	++it;
 }
}

void BoDisplayManager::unlockAction()
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->displayInput()->unlockAction();
	++it;
 }
}

void BoDisplayManager::setToolTipUpdatePeriod(int ms)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setToolTipUpdatePeriod(ms);
	++it;
 }
}

void BoDisplayManager::setToolTipCreator(int type)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setToolTipCreator(type);
	++it;
 }
}

void BoDisplayManager::loadFromXML(const QDomElement& root)
{
 // Load displays (camera)
 QDomElement displays = root.namedItem(QString::fromLatin1("Displays")).toElement();
 if (displays.isNull()) {
	boError(260) << k_funcinfo << "no displays" << endl;
	return;
 }
 QDomElement display = displays.namedItem(QString::fromLatin1("Display")).toElement();
 if (display.isNull()) {
	boError(260) << k_funcinfo << "no display" << endl;
	return;
 }

 if (d->mDisplayList.at(0)) {
	d->mDisplayList.at(0)->loadFromXML(display);
 } else {
	boError(260) << k_funcinfo << "No displays ?!" << endl;
 }

 // Load unitgroups
 QDomElement unitgroups = root.namedItem(QString::fromLatin1("UnitGroups")).toElement();
 if (unitgroups.isNull()) {
	boError(260) << k_funcinfo << "no unitgroups " << endl;
	return;
 }
 QDomNodeList list = unitgroups.elementsByTagName(QString::fromLatin1("Group"));
 if (list.count() == 0) {
	boWarning(260) << k_funcinfo << "no unitgroups" << endl;
	return;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(260) << k_funcinfo << i << " is not an element" << endl;
		return;
	}
	if (!e.hasAttribute("Id")) {
		boError(260) << k_funcinfo << "missing attribute: Id for Group " << i << endl;
		continue;
	}
	int id;
	bool ok;
	id = e.attribute("Id").toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id for Group " << i << endl;
		continue;
	}
	d->mSelectionGroups[id]->loadFromXML(e);
 }
}

void BoDisplayManager::saveAsXML(QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 // Save displays
 // FIXME: only first display is saved here as we currently don't support more.
 //  if we will ever support multiple displays again, their layout etc. must be
 //  saved here as well.
 QDomElement displays = doc.createElement(QString::fromLatin1("Displays"));
 QDomElement display = doc.createElement(QString::fromLatin1("Display"));
 if (d->mDisplayList.at(0)) {
	d->mDisplayList.at(0)->saveAsXML(display);
 } else {
	boError(260) << k_funcinfo << "No displays ?!" << endl;
 }
 displays.appendChild(display);
 root.appendChild(displays);

 // Save unitgroups
 QDomElement unitgroups = doc.createElement(QString::fromLatin1("UnitGroups"));
 for(int i = 0; i < 10; i++) {
	QDomElement group = doc.createElement(QString::fromLatin1("Group"));
	group.setAttribute("Id", i);
	d->mSelectionGroups[i]->saveAsXML(group);
	unitgroups.appendChild(group);
 }
 root.appendChild(unitgroups);
}

