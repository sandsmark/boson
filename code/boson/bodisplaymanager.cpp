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
#include "bodebug.h"
#include "boaction.h"

#include <klocale.h>

#include <qlayout.h>
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

	bool mGrabMovie;
};

BoDisplayManager::BoDisplayManager(QWidget* parent) : QWidget(parent, "bosondisplaymanager")
{
 d = new BoDisplayManagerPrivate;
 d->mGrabMovie = false;

 d->mLayout = new QVBoxLayout(this);
}

BoDisplayManager::~BoDisplayManager()
{
 delete d->mActiveDisplay;
 delete d;
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
 connect(b, SIGNAL(signalSetGrabMovie(bool)),
		this, SLOT(slotSetGrabMovie(bool)));

 d->mLayout->addWidget(b);
 d->mLayout->activate();

 return b;
}

void BoDisplayManager::slotAdvance(unsigned int, bool)
{
 BO_CHECK_NULL_RET(d->mActiveDisplay);
 d->mActiveDisplay->setParticlesDirty(true);
 d->mActiveDisplay->advanceCamera();
 d->mActiveDisplay->advanceLineVisualization();
 grabMovieFrame();
}

void BoDisplayManager::slotAction(const BoSpecificAction& action)
{
 BO_CHECK_NULL_RET(activeDisplay());
 BO_CHECK_NULL_RET(activeDisplay()->displayInput());

 activeDisplay()->displayInput()->action(action);
}

void BoDisplayManager::slotSetGrabMovie(bool grab)
{
 BO_CHECK_NULL_RET(activeDisplay());

 d->mGrabMovie = grab;
}

void BoDisplayManager::grabMovieFrame()
{
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

