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

#include <klocale.h>

#include <qlayout.h>
#include <qintdict.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>

#include "bodisplaymanager.moc"


class BoDisplayManager::BoDisplayManagerPrivate
{
public:
	BoDisplayManagerPrivate()
	{
		mLayout = 0;

		mActiveDisplay = 0;
	}

	QVBoxLayout* mLayout;

	BosonBigDisplayBase* mActiveDisplay;

	QIntDict<BoSelection> mSelectionGroups;

	bool mGrabMovie;
};

BoDisplayManager::BoDisplayManager(QWidget* parent) : QWidget(parent, "bosondisplaymanager")
{
 d = new BoDisplayManagerPrivate;
 d->mGrabMovie = false;

 d->mSelectionGroups.setAutoDelete(true);
 for (int i = 0; i < 10; i++) {
	BoSelection* s = new BoSelection(this);
	d->mSelectionGroups.insert(i, s);
 }
 d->mLayout = new QVBoxLayout(this);
}

BoDisplayManager::~BoDisplayManager()
{
 boDebug() << k_funcinfo << endl;
 d->mSelectionGroups.clear();
 boDebug() << k_funcinfo << "deleting display" << endl;
 delete d->mActiveDisplay;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

BosonBigDisplayBase* BoDisplayManager::activeDisplay() const
{
 return d->mActiveDisplay;
}

BosonBigDisplayBase* BoDisplayManager::addInitialDisplay()
{
 if (d->mActiveDisplay) {
	boDebug() << k_funcinfo << "already have displays - returning first..." << endl;
	return d->mActiveDisplay;
 }
 boDebug() << k_funcinfo << endl;
 BosonBigDisplayBase* b = new BosonBigDisplayBase(this);
 d->mActiveDisplay = b;
 connect(b, SIGNAL(signalSelectionChanged(BoSelection*)),
		this, SIGNAL(signalSelectionChanged(BoSelection*)));

 d->mLayout->addWidget(b);
 d->mLayout->activate();

 return b;
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
 if (d->mActiveDisplay) {
	d->mActiveDisplay->quitGame();
 }
 for (int i = 0; i < 10; i++) {
	slotClearGroup(i);
 }
}

void BoDisplayManager::slotAdvance(unsigned int, bool)
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->setParticlesDirty(true);
 d->mActiveDisplay->advanceCamera();
 d->mActiveDisplay->advanceLineVisualization();
 grabMovieFrame();
}

void BoDisplayManager::slotUpdateIntervalChanged(unsigned int ms)
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->setUpdateInterval(ms);
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

void BoDisplayManager::slotUnitRemoved(Unit* u)
{
 for(int i = 0; i < 10; i++) {
	d->mSelectionGroups[i]->removeUnit(u);
 }
}

void BoDisplayManager::unlockAction()
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->displayInput()->unlockAction();
}

void BoDisplayManager::setToolTipUpdatePeriod(int ms)
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->setToolTipUpdatePeriod(ms);
}

void BoDisplayManager::setToolTipCreator(int type)
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->setToolTipCreator(type);
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

 if (d->mActiveDisplay) {
	d->mActiveDisplay->loadFromXML(display);
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
 if (d->mActiveDisplay) {
	d->mActiveDisplay->saveAsXML(display);
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

void BoDisplayManager::slotAction(const BoSpecificAction& action)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->action(action);
}

void BoDisplayManager::slotUpdateOpenGLSettings()
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->updateOpenGLSettings();
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

