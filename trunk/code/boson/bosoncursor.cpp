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

#include "bosoncursor.h"
#include "defines.h"

#include <kcursor.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qcanvas.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qintdict.h>

#include "bosoncursor.moc"

#define PIXMAP_CURSOR 1 // support for pixmaps as cursor


class BosonCursor::BosonCursorPrivate
{
public:
	BosonCursorPrivate()
	{
		mQCursor = 0;
		mCursor = 0;
		
		mCanvas = 0;
	}

	CursorMode mMode;

	QCursor* mQCursor;
	QCanvasSprite* mCursor;

	QIntDict<QCanvasPixmapArray> mCursorPixmaps;

	QCanvas* mCanvas;

	QPixmap mCursorDefault;
	QPixmap mCursorMove;
	QPixmap mCursorAttack;

	QTimer mAnimateTimer;
};


BosonCursor::BosonCursor()
{
 d = new BosonCursorPrivate;
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));

 d->mCursorPixmaps.setAutoDelete(true);
}

BosonCursor::~BosonCursor()
{
 delete d->mQCursor;
 delete d->mCursor;
 d->mCursorPixmaps.clear();
 delete d;
}

void BosonCursor::setCursor(CursorMode mode)
{
 if (d->mMode == mode) {
	return;
 }
 d->mMode = mode;
#ifdef PIXMAP_CURSOR
 switch (d->mMode) {
	case Move:
	case Attack:
	case Default:
		d->mCursor->setSequence(d->mCursorPixmaps[(int)d->mMode]);
		break;
	case Hide:
	default:
		d->mCursor->hide();
		break;

 }
 if (d->mMode != Hide) { 
	d->mCursor->show();
	if (d->mCursor->frameCount() > 1) {
		d->mAnimateTimer.stop();
		d->mAnimateTimer.start(100);
	}

 }

// do NOT use QCanvasSprite::setAnimated(true)! we need to use
// our own stuff!
 d->mCursor->setAnimated(false);
#endif

 if (d->mQCursor) {
	delete d->mQCursor;
 }
 switch (d->mMode) {
	// TODO: hotspots!!
	case Attack:
		d->mQCursor = new QCursor(d->mCursorAttack);
		break;
	case Move:
		d->mQCursor = new QCursor(d->mCursorMove);
		break;
	case Hide:
		d->mQCursor = new QCursor(QCursor::BlankCursor);
		break;
	case Default:
		d->mQCursor = new QCursor(KCursor::arrowCursor());
		break;
 }
}

void BosonCursor::setCursor(QWidget* w)
{
#ifndef PIXMAP_CURSOR
 w->setCursor(cursor());
#else
 switch (cursorMode()) {
	case Default:
//		w->setCursor(cursor());
//		break;
	default:
		w->setCursor(QCursor::BlankCursor);
		break;
 }
#endif
}

const QCursor& BosonCursor::cursor() const
{
 return *d->mQCursor;
}

BosonCursor::CursorMode BosonCursor::cursorMode() const
{
 return d->mMode;
}

void BosonCursor::setCanvas(QCanvas* canvas)
{
 if (d->mCanvas) {
	kdError() << k_funcinfo << "Canvas already set" << endl;
	return;
 }
 d->mCanvas = canvas;
 loadCursors();
}

void BosonCursor::loadCursors()
{
 if (d->mCursor) {
	kdWarning() << "cursors already loaded..." << endl;
	delete d->mCursor;
 }
 QString dir = KGlobal::dirs()->findResourceDir("data", "boson/themes/cursors/move/index.desktop") + QString::fromLatin1("boson/themes/cursors");

 // normal cursor support is broken!
 /*
 QString move = dir + QString::fromLatin1("/move.png");
 QString attack = dir + QString::fromLatin1("/attack.png");
 QString defaultCursor = dir + QString::fromLatin1("/default.png");
 if (d->mCursorMove.load(move)) {
	QBitmap mask(move);
	d->mCursorMove.setMask(mask);
 } else {
	kdError() << k_funcinfo << "Could not load " << move << endl;
 }
 if (d->mCursorAttack.load(attack)) {
	QBitmap mask(move);
	d->mCursorAttack.setMask(mask);
 } else {
	kdError() << k_funcinfo << "Could not load " << attack << endl;
 }
 if (d->mCursorDefault.load(defaultCursor)) {
	QBitmap mask(defaultCursor);
	d->mCursorAttack.setMask(mask);
 } else {
	kdError() << k_funcinfo << "Could not load " << defaultCursor << endl;
 }*/


#ifdef PIXMAP_CURSOR
 for (unsigned int i = 0; i < Hide; i++) {
	QString cursorDir;
	switch ((CursorMode)i) {
		case Attack:
		{
			cursorDir = dir + QString::fromLatin1("/attack/");
			break;
		}
		case Move:
		{
			cursorDir = dir + QString::fromLatin1("/move/");
			break;
		}
		case Default:
		{
			cursorDir = dir + QString::fromLatin1("/default/");
			break;
		}
		case Hide:
			break;
	}
	if (cursorDir == QString::null) {
		continue;
	}

	KSimpleConfig c(cursorDir + QString::fromLatin1("index.desktop"));
	c.setGroup("Boson Cursor");
	QString filePrefix = c.readEntry("FilePrefix", QString::fromLatin1("cursor-"));
	unsigned int frames = c.readUnsignedNumEntry("FrameCount", 1);
	unsigned int hotspotX = 0;
	unsigned int hotspotY = 0;
	QValueList<QPixmap> pixmaps;
	QPointArray points(frames);
	for (unsigned int j = 0; j < frames; j++) {
		hotspotX = c.readUnsignedNumEntry(QString("HotspotX_%1").arg(j), hotspotX);
		hotspotY = c.readUnsignedNumEntry(QString("HotspotY_%1").arg(j), hotspotY);
		points.setPoint(j, hotspotX, hotspotY);
		QPixmap p;
		QString number;
		number.sprintf("%04d", j);
		QString file = cursorDir + filePrefix + number + QString::fromLatin1(".png");
		if (p.load(file)) {
			QBitmap mask(file);
			p.setMask(mask);
		} else {
			kdError() << k_funcinfo << "Could not load " << file << endl;
		}
		pixmaps.append(p);
	}
	d->mCursorPixmaps.insert(i, new QCanvasPixmapArray(pixmaps, points));
 }
 d->mCursor = new QCanvasSprite(d->mCursorPixmaps[Default], d->mCanvas);
 d->mCursor->setZ(Z_CANVAS_CURSOR);
#endif
}

void BosonCursor::move(double x, double y)
{
#ifdef PIXMAP_CURSOR
 d->mCursor->move(x, y);
#endif
}


void BosonCursor::slotAdvance()
{
 d->mCursor->setFrame((d->mCursor->frame() + 1 + d->mCursor->frameCount()) % d->mCursor->frameCount());
}
