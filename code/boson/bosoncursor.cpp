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

#include <qcanvas.h>
#include <qbitmap.h>

#include <kcursor.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#define PIXMAP_CURSOR 1 // support for pixmaps as cursor

class BosonCursor::BosonCursorPrivate
{
public:
	BosonCursorPrivate()
	{
		mQCursor = 0;
		mCursor = 0;
		
		mCursorPixmaps = 0;

		mCanvas = 0;
	}

	CursorMode mMode;

	QCursor* mQCursor;
	QCanvasSprite* mCursor;

	QCanvasPixmapArray* mCursorPixmaps;

	QCanvas* mCanvas;

	QPixmap mCursorDefault;
	QPixmap mCursorMove;
	QPixmap mCursorAttack;
};


BosonCursor::BosonCursor()
{
 d = new BosonCursorPrivate;
}

BosonCursor::~BosonCursor()
{
 delete d->mQCursor;
 delete d->mCursor;
 delete d->mCursorPixmaps;
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
	case Hide:
//	case Default: // we do not (yet) have a pixmap for this
		d->mCursor->hide();
		break;
	default:
		d->mCursor->setFrame((int)mode);
		d->mCursor->show();
		break;

 }
#endif
 if (d->mQCursor) {
	delete d->mQCursor;
 }
 switch (d->mMode) {
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
 QString dir = KGlobal::dirs()->findResourceDir("data", "boson/themes/cursors/move.png") + QString::fromLatin1("boson/themes/cursors");
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
 }


#ifdef PIXMAP_CURSOR
 QValueList<QPixmap> pixmaps;
 QPointArray points;
 for (unsigned int i = 0; i < Hide; i++) {
	switch ((CursorMode)i) {
		case Attack:
		{
			pixmaps.append(d->mCursorAttack);
			points.resize(i + 1);
			KSimpleConfig c(dir + "/move.desktop");
			c.setGroup("Boson Cursor");
			points.setPoint(i, QPoint(
					c.readNumEntry("HotspotX", 0), 
					c.readNumEntry("HotspotY", 0)));
			break;
		}
		case Move:
		{
			pixmaps.append(d->mCursorMove);
			points.resize(i + 1);
			KSimpleConfig c(dir + "/move.desktop");
			c.setGroup("Boson Cursor");
			points.setPoint(i, QPoint(
					c.readNumEntry("HotspotX", 0),
					c.readNumEntry("HotspotY", 0)));
			break;
		}
		case Default:
		{
			pixmaps.append(d->mCursorDefault);
			KSimpleConfig c(dir + "/default.desktop");
			c.setGroup("Boson Cursor");
			points.setPoint(i, QPoint(
					c.readNumEntry("HotspotX, 0"),
					c.readNumEntry("HotspotY, 0")));
			points.resize(i + 1);
			break;
		}
		case Hide:
			break;
	}
 }
 d->mCursorPixmaps = new QCanvasPixmapArray(pixmaps, points);
 d->mCursor = new QCanvasSprite(d->mCursorPixmaps, d->mCanvas);
 d->mCursor->setZ(Z_CANVAS_CURSOR);
#endif
}

void BosonCursor::move(double x, double y)
{
#ifdef PIXMAP_CURSOR
 d->mCursor->move(x, y);
#endif
}

