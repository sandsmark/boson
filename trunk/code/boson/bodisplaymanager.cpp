/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "no_player.h"
#include "defines.h"
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplayinputbase.h"
#include "bosonconfig.h"
#include "boselection.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "boaction.h"
#include "bocamerawidget.h"

#include <klocale.h>

#include <qstyle.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qintdict.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>

#include "bodisplaymanager.moc"


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
		mLayout = 0;

		mActiveDisplay = 0;

		mLightWidget = 0;
	}

	QVBoxLayout* mLayout;

	QPtrList<BosonBigDisplayBase> mDisplayList;
	QPtrList<BoBox> mBoxList;
	BosonBigDisplayBase* mActiveDisplay;

	QIntDict<BoSelection> mSelectionGroups;

	bool mGrabMovie;

	BoLightCameraWidget1* mLightWidget;
};

BoDisplayManager::BoDisplayManager(QWidget* parent) : QWidget(parent, "bosondisplaymanager")
{
 d = new BoDisplayManagerPrivate;
 d->mDisplayList.setAutoDelete(true);
 d->mBoxList.setAutoDelete(true);
 d->mGrabMovie = false;
 d->mLightWidget = 0;

 d->mSelectionGroups.setAutoDelete(true);
 for (int i = 0; i < 10; i++) {
	BoSelection* s = new BoSelection(this);
	d->mSelectionGroups.insert(i, s);
 }
}

BoDisplayManager::~BoDisplayManager()
{
 boDebug() << k_funcinfo << endl;
 delete d->mLightWidget;
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
 delete d->mLightWidget;
 d->mLightWidget = 0;
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
	boDebug() << k_funcinfo << "already have displays - returning first..." << endl;
	return d->mDisplayList.getFirst();
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
 BosonBigDisplayBase* b = new BosonBigDisplayBase(parent);
 connect(b, SIGNAL(signalSelectionChanged(BoSelection*)),
		this, SIGNAL(signalSelectionChanged(BoSelection*)));
 connect(b, SIGNAL(signalChangeViewport(BosonBigDisplayBase*,
		const QPoint&, const QPoint&, const QPoint&, const QPoint&)),
		this, SLOT(slotChangeViewport(BosonBigDisplayBase*,
		const QPoint&, const QPoint&, const QPoint&, const QPoint&)));

 d->mDisplayList.append(b);
 connect(b, SIGNAL(signalMakeActive(BosonBigDisplayBase*)),
		this, SLOT(slotMakeActiveDisplay(BosonBigDisplayBase*)));
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

void BoDisplayManager::setLocalPlayerIO(PlayerIO* io)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	boDebug() << k_funcinfo << endl;
	it.current()->setLocalPlayerIO(io);
	++it;
 }
}

void BoDisplayManager::setCanvas(BosonCanvas* c)
{
 boDebug() << k_funcinfo << endl;
 QIntDictIterator<BoSelection> selectIt(d->mSelectionGroups);
 for (; selectIt.current(); ++selectIt) {
	connect(c, SIGNAL(signalRemovedItem(BosonItem*)),
			selectIt.current(), SLOT(slotRemoveItem(BosonItem*)));
 }
 connect(c, SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotUnitRemoved(Unit*)));
}

void BoDisplayManager::quitGame()
{
 boDebug() << k_funcinfo << endl;
 delete d->mLightWidget;
 d->mLightWidget = 0;
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->quitGame();
	++it;
 }
 for (int i = 0; i < 10; i++) {
	slotClearGroup(i);
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

void BoDisplayManager::slotRotateLeft()
{
 BosonBigDisplayBase* active = activeDisplay();
 if (!active) {
	return;
 }
 active->rotateLeft();
}

void BoDisplayManager::slotRotateRight()
{
 BosonBigDisplayBase* active = activeDisplay();
 if (!active) {
	return;
 }
 active->rotateRight();
}

void BoDisplayManager::slotZoomIn()
{
 BosonBigDisplayBase* active = activeDisplay();
 if (!active) {
	return;
 }
 active->zoomIn();
}

void BoDisplayManager::slotZoomOut()
{
 BosonBigDisplayBase* active = activeDisplay();
 if (!active) {
	return;
 }
 active->zoomOut();
}

void BoDisplayManager::slotAdvance(unsigned int, bool)
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->setParticlesDirty(true);
	it.current()->advanceCamera();
	it.current()->advanceLineVisualization();
	++it;
 }
 grabMovieFrame();
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

void BoDisplayManager::slotPlaceUnit(unsigned long int unitType, Player* owner)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->placeUnit(unitType, owner);
}

void BoDisplayManager::slotPlaceGround(unsigned int textureCount, unsigned char* alpha)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->placeGround(textureCount, alpha);
}

void BoDisplayManager::slotMoveActiveSelection(int x, int y)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->slotMoveSelection(x, y); // FIXME: not a slot anymore
}

void BoDisplayManager::slotReCenterActiveDisplay(const QPoint& center)
{
 BO_CHECK_NULL_RET(activeDisplay());

 activeDisplay()->slotReCenterDisplay(center); // FIXME: not a slot anymore
}

void BoDisplayManager::slotActiveSelectSingleUnit(Unit* unit)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->selection());

 activeDisplay()->selection()->slotSelectSingleUnit(unit);
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

void BoDisplayManager::slotUnitChanged(Unit* unit)
{
 // this slot is meant for the case that unit has been destroyed but is
 // selected. we don't check for unit->isDestroyed() here (which would be
 // faster) but forward the pointer to the displays, to avoid including unit.h
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->slotUnitChanged(unit);
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

double BoDisplayManager::fps() const
{
 if (!activeDisplay()) {
	BO_NULL_ERROR(activeDisplay());
	return 0.0;
 }
 return activeDisplay()->fps();
}

void BoDisplayManager::slotChangeViewport(BosonBigDisplayBase* display, const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight)
{
 if (display != activeDisplay()) {
	return;
 }
 emit signalChangeActiveViewport(topLeft, topRight, bottomLeft, bottomRight);
}

void BoDisplayManager::slotAction(const BoSpecificAction& action)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->action(action);
}

QPtrList<BosonBigDisplayBase>* BoDisplayManager::displayList()
{
 return &d->mDisplayList;
}

void BoDisplayManager::slotUpdateOpenGLSettings()
{
 QPtrListIterator<BosonBigDisplayBase> it(d->mDisplayList);
 while (it.current()) {
	it.current()->updateOpenGLSettings();
	++it;
 }
}

void BoDisplayManager::slotChangeFont(const BoFontInfo& font)
{
 BO_CHECK_NULL_RET(activeDisplay());

 activeDisplay()->setFont(font);
}

void BoDisplayManager::slotSetGrabMovie(bool grab)
{
 BO_CHECK_NULL_RET(activeDisplay());

 d->mGrabMovie = grab;
}

void BoDisplayManager::grabMovieFrame()
{
 if (!d->mActiveDisplay) {
	return;
 }
 if (!d->mGrabMovie) {
	return;
 }
 QByteArray shot = d->mActiveDisplay->grabMovieFrame();

 if (shot.size() == 0) {
	return;
 }

 // Save frame
 static int frame = -1;
 QString file;
 if (frame == -1) {
	int i;
	for (i = 0; i <= 10000; i++) {
		file.sprintf("%s-%04d.%s", "boson-movie", i, "jpg");
		if (!QFile::exists(file)) {
			frame = i;
			break;
		}
	}
	if (i == 10000) {
		boWarning() << k_funcinfo << "Can't find free filename???" << endl;
		frame = 50000;
	}
 }
 file.sprintf("%s-%04d.%s", "boson-movie", frame++, "jpg");
 file = QFileInfo(file).absFilePath();

 //boDebug() << k_funcinfo << "Saving movie frame to " << file << endl;
 bool ok = QPixmap(shot).save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	return;
 }
 boDebug() << k_funcinfo << "Movie frame saved to file " << file << endl;

#if 0
 static QValueList<QByteArray> allMovieFrames;
 allMovieFrames.append(shot);


 // TODO: use a shortcut for this. do not do this after a certain number of
 // frames, but when a key was pressed.
 if (allMovieFrames.count() == 10) {
	boDebug() << k_funcinfo << "generating " << allMovieFrames.count() << " frames" << endl;
	d->mActiveDisplay->generateMovieFrames(allMovieFrames, "./11/");
	allMovieFrames.clear();
 }
#endif
}

void BoDisplayManager::slotShowLight0Widget()
{
 if (!d->mActiveDisplay) {
	return;
 }
 delete d->mLightWidget;
 d->mLightWidget = new BoLightCameraWidget1(0, true);
 d->mLightWidget->show();
 d->mLightWidget->setLight(d->mActiveDisplay->light(0), d->mActiveDisplay->context());
}

