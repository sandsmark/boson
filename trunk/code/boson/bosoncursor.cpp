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

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qcursor.h>
#include <qcanvas.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qintdict.h>

#include "bosoncursor.moc"

//#define NO_PIXMAP_CURSOR 1 // use normal x-cursor ; not sprites.


class BosonCursor::BosonCursorPrivate
{
public:
	BosonCursorPrivate()
	{
#ifndef NO_PIXMAP_CURSOR
		mCursor = 0;
#endif
		
		mCanvas = 0;
	}

	int mMode;

	QIntDict<QCursor> mQCursors;

#ifndef NO_PIXMAP_CURSOR
	QCanvasSprite* mCursor;
	QIntDict<QCanvasPixmapArray> mCursorPixmaps;
	QTimer mAnimateTimer;
#endif

	QCanvas* mCanvas;
};


BosonCursor::BosonCursor()
{
 d = new BosonCursorPrivate;
#ifndef NO_PIXMAP_CURSOR
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));
 d->mCursorPixmaps.setAutoDelete(true);
#endif
 d->mQCursors.setAutoDelete(true);
}

BosonCursor::~BosonCursor()
{
#ifndef NO_PIXMAP_CURSOR
 delete d->mCursor;
 d->mCursorPixmaps.clear();
#endif
 d->mQCursors.clear();
 delete d;
}

void BosonCursor::setCursor(int mode)
{
 if (d->mMode == mode) {
	return;
 }
 d->mMode = (int)mode;
#ifndef NO_PIXMAP_CURSOR
 d->mAnimateTimer.stop();
 if (mode > 0) {
	QCanvasPixmapArray* a = d->mCursorPixmaps[d->mMode];
	if (!a) {
		kdWarning() << k_funcinfo << "NULL pixmap array for " << mode << endl;
		d->mCursor->hide();
	} else {
		d->mCursor->setSequence(a);
	}
	d->mCursor->show();
	if (d->mCursor->frameCount() > 1) {
		d->mAnimateTimer.start(100);
	}
 } else {
	d->mCursor->hide();
 }

// do NOT use QCanvasSprite::setAnimated(true)! we need to use
// our own stuff!
 d->mCursor->setAnimated(false);
#endif
}

void BosonCursor::setWidgetCursor(QWidget* w)
{
#ifndef NO_PIXMAP_CURSOR
 w->setCursor(QCursor::BlankCursor);
#else
 w->setCursor(cursor());
#endif
}

QCursor BosonCursor::cursor() const
{
 QCursor* c = d->mQCursors[d->mMode];
 if (c) {
	return *c;
 }
 return QCursor(QCursor::ArrowCursor);
}

QCanvasSprite* BosonCursor::cursorSprite() const
{
#ifndef NO_PIXMAP_CURSOR
 return d->mCursor;
#endif
 return 0;
}

int BosonCursor::cursorMode() const
{
 return d->mMode;
}

void BosonCursor::setCanvas(QCanvas* canvas, int mode, int z)
{
#ifndef NO_PIXMAP_CURSOR
 if (d->mCanvas) {
	kdError() << k_funcinfo << "Canvas already set" << endl;
	return;
 }
 d->mCanvas = canvas;
 if (d->mCursor) {
	d->mCursor->setCanvas(canvas);
 } else {
	d->mCursor = new QCanvasSprite(d->mCursorPixmaps[mode], d->mCanvas);
	d->mCursor->setZ(z);
 }
#endif
}

QCanvasPixmapArray* BosonCursor::loadSpriteCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 QCanvasPixmapArray* array = 0;
#ifndef NO_PIXMAP_CURSOR
 KSimpleConfig c(baseDir + cursor + QString::fromLatin1("/index.desktop"));
 if (!c.hasGroup("Boson Cursor")) {
	kdWarning() << k_funcinfo << "index.desktop is missing default group - sprite cursor disabled" << endl;
 } else {
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
		QString file = QString::fromLatin1("%1%2/%3%4.png").arg(baseDir).arg(cursor).arg(filePrefix).arg(number);
		if (p.load(file)) {
			QBitmap mask(file);
			p.setMask(mask);
		} else {
			kdError() << k_funcinfo << "Could not load " << file << endl;
		}
		pixmaps.append(p);
	}
	if (pixmaps.count() > 0) {
		array = new QCanvasPixmapArray(pixmaps, points);
	} else {
		kdError() << k_funcinfo << "No pixmaps loaded from " << baseDir + cursor << endl;
	}
 }
#endif
 return array;
// insertMode(mode, array, loadCursor(baseDir, cursor));
}

QCursor* BosonCursor::loadQCursor(QString baseDir, QString cursor)
{
 QCursor* qcursor = 0;
 QPixmap p;
 QString file = baseDir + cursor + QString::fromLatin1(".xpm"); // FIXME: only xpm??
 //TODO: hotspot!!
 if (p.load(file)) {
	qcursor = new QCursor(p);
 }
 return qcursor;
}

void BosonCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 insertMode(mode, loadSpriteCursor(baseDir, cursor), loadQCursor(baseDir, cursor));
}

void BosonCursor::insertMode(int mode, QCanvasPixmapArray* array, QCursor* cursor)
{
 if (d->mQCursors[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mQCursors.remove(mode);
 }
 if (cursor) {
	d->mQCursors.insert(mode, cursor);
 }
#ifndef NO_PIXMAP_CURSOR
 if (d->mCursorPixmaps[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mCursorPixmaps.remove(mode);
 }
 if (array) {
	d->mCursorPixmaps.insert(mode, array);
 }
#endif
}

void BosonCursor::move(double x, double y)
{
#ifndef NO_PIXMAP_CURSOR
 d->mCursor->move(x, y);
#endif
}

void BosonCursor::slotAdvance()
{
#ifndef NO_PIXMAP_CURSOR
 d->mCursor->setFrame((d->mCursor->frame() + 1 + d->mCursor->frameCount()) % d->mCursor->frameCount());
#endif
}
