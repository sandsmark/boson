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

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <kcursor.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qcanvas.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qintdict.h>
#include <qlabel.h>

#include <X11/Xlib.h>

#include "bosoncursor.moc"


BosonCursor::BosonCursor()
{
 mMode = 0;
}

BosonCursor::~BosonCursor()
{
}

void BosonCursor::setCursor(int mode)
{
 if (mMode == mode) {
	return;
 }
// kdDebug() << k_funcinfo << mode << endl;
 mMode = (int)mode;
}

QCursor BosonCursor::cursor() const
{
 return QCursor(QCursor::ArrowCursor);
}

QRect BosonCursor::oldCursor() const
{
 return QRect(0,0,1,1);
}

void BosonCursor::move(double, double)
{
}

QPoint BosonCursor::pos() const
{
 return QCursor::pos();
}

QStringList BosonCursor::availableThemes()
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/themes/cursors/*/index.desktop");
 QStringList retList;
 for (unsigned int i = 0; i < list.count(); i++) {
	retList.append(list[i].left(list[i].length() - strlen("/index.desktop")));
 }

 return retList;
}

QString BosonCursor::defaultTheme()
{
 QString cursorDir = KGlobal::dirs()->findResourceDir("data",
		"boson/themes/cursors/default/index.desktop") +
		QString::fromLatin1("boson/themes/cursors/default");
 return cursorDir;
}

/////////////////////////////////////////
/////// BosonNormalCursor ///////////////
/////////////////////////////////////////
class BosonNormalCursor::BosonNormalCursorPrivate
{
public:
	BosonNormalCursorPrivate()
	{
	}

	QIntDict<QCursor> mQCursors;
};

BosonNormalCursor::BosonNormalCursor() : BosonCursor()
{
 d = new BosonNormalCursorPrivate;
 d->mQCursors.setAutoDelete(true);
}

BosonNormalCursor::~BosonNormalCursor()
{
 d->mQCursors.clear();
 delete d;
}

void BosonNormalCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
}

void BosonNormalCursor::setWidgetCursor(QWidget* w)
{
// kdDebug() << k_funcinfo << endl;
 w->setCursor(cursor());
}

QCursor BosonNormalCursor::cursor() const
{
 QCursor* c = d->mQCursors[cursorMode()];
 if (c) {
	return *c;
 }
 return QCursor(QCursor::ArrowCursor);
}

void BosonNormalCursor::insertMode(int mode, QCursor* cursor)
{
 if (d->mQCursors[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mQCursors.remove(mode);
 }
 if (cursor) {
	d->mQCursors.insert(mode, cursor);
 }
}

void BosonNormalCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 insertMode(mode, loadQCursor(baseDir, cursor));
}

QCursor* BosonNormalCursor::loadQCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 QCursor* qcursor = 0;
 QPixmap p;
 QString file = baseDir + cursor + QString::fromLatin1(".png");
 //TODO: hotspot!!
 if (p.load(file)) {
	p.setMask(p.createHeuristicMask());
	qcursor = new QCursor(p);
 } else {
	qcursor = new QCursor(Qt::arrowCursor);
 }
 return qcursor;
}



/////////////////////////////////////////
/////// BosonKDECursor //////////////////
/////////////////////////////////////////

BosonKDECursor::BosonKDECursor() : BosonCursor()
{
}

BosonKDECursor::~BosonKDECursor()
{
}

void BosonKDECursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
}

void BosonKDECursor::setWidgetCursor(QWidget* w)
{
// kdDebug() << k_funcinfo << endl;
 w->setCursor(cursor());
}

QCursor BosonKDECursor::cursor() const
{
 return KCursor::arrowCursor();
}

void BosonKDECursor::insertMode(int , QString , QString )
{
}

/////////////////////////////////////////
/////// BosonSpriteCursor ///////////////
/////////////////////////////////////////
class BosonSpriteCursor::BosonSpriteCursorPrivate
{
public:
	BosonSpriteCursorPrivate()
	{
		mCanvas = 0;
		mCurrentFrames = 0;
		mCurrentFrame = -1;

		mCursor = 0;
	}

	QCanvas* mCanvas;
	QTimer mAnimateTimer;
	QIntDict<QCanvasPixmapArray> mCursorPixmaps;
	QCanvasPixmapArray* mCurrentFrames;
	int mCurrentFrame;

	QCanvasSprite* mCursor;
};

BosonSpriteCursor::BosonSpriteCursor() : BosonCursor()
{
 d = new BosonSpriteCursorPrivate;
 d->mCursorPixmaps.setAutoDelete(true);
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));
}

BosonSpriteCursor::~BosonSpriteCursor()
{
 delete d->mCursor;
 d->mCursorPixmaps.clear();
 delete d;
}

void BosonSpriteCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
 d->mAnimateTimer.stop();
 if (mode >= 0) {
	QCanvasPixmapArray* a = d->mCursorPixmaps[cursorMode()];
//	d->mCursor->hide();
	if (a) {
		d->mCurrentFrames = a;
		d->mCursor->setSequence(a);
		d->mCursor->setFrame(0);
		d->mCursor->show();
		if (d->mCursor->frameCount() > 1) {
			d->mAnimateTimer.start(100);
		}
	} else {
		kdWarning() << k_funcinfo << "NULL pixmap array for " << mode << endl;
	}
 } else {
	d->mCursor->hide();
 }
}

void BosonSpriteCursor::setCanvas(QCanvas* canvas, int mode, int z)
{
 if (d->mCanvas) {
	kdError() << k_funcinfo << "Canvas already set" << endl;
	return;
 }
 d->mCanvas = canvas;
 if (d->mCursor) {
	d->mCursor->setCanvas(canvas);
 } else {
	d->mCursor = new QCanvasSprite(0, d->mCanvas);
	d->mCursor->setZ(z);
 }
 setCursor(mode);
}

void BosonSpriteCursor::setWidgetCursor(QWidget* w)
{
 if (w->cursor().shape() != Qt::BlankCursor) {
//	kdDebug() << k_funcinfo << endl;
	w->setCursor(Qt::BlankCursor);
 }
}

void BosonSpriteCursor::insertMode(int mode, QCanvasPixmapArray* array)
{
 if (d->mCursorPixmaps[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mCursorPixmaps.remove(mode);
 }
 if (array) {
	d->mCursorPixmaps.insert(mode, array);
 }
}

QCanvasSprite* BosonSpriteCursor::cursorSprite() const
{
 return d->mCursor;
}

void BosonSpriteCursor::move(double x, double y)
{
 d->mCursor->move(x, y);
}

void BosonSpriteCursor::slotAdvance()
{
 d->mCursor->setFrame((d->mCursor->frame() + 1 + d->mCursor->frameCount()) % d->mCursor->frameCount());
}

void BosonSpriteCursor::hideCursor()
{
 d->mCursor->hide();
}

void BosonSpriteCursor::showCursor()
{
 if (d->mCursor) {
	d->mCursor->show();
 }
}

void BosonSpriteCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 insertMode(mode, loadSpriteCursor(baseDir, cursor));
}

QCanvasPixmapArray* BosonSpriteCursor::loadSpriteCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 QCanvasPixmapArray* array = 0;
 KSimpleConfig c(baseDir + cursor + QString::fromLatin1("/index.desktop"));
 kdDebug() << baseDir << endl;
 if (!c.hasGroup("Boson Cursor")) {
	kdWarning() << k_funcinfo << "index.desktop is missing default group" << endl;
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
 if (!array) {
	QValueList<QPixmap> p;
	QPointArray points(1);
	points.setPoint(0, 0, 0);
	p.append(*KCursor::arrowCursor().bitmap());
	kdWarning() << "loading fallback cursor..." << endl;
	array = new QCanvasPixmapArray();
 }
 return array;
}

QPoint BosonSpriteCursor::pos() const
{
 return QPoint(d->mCursor->x(), d->mCursor->y());
}



/////////////////////////////////////////
/////// BosonExperimentalCursor /////////
/////////////////////////////////////////
class BosonExperimentalCursor::BosonExperimentalCursorPrivate
{
public:
	BosonExperimentalCursorPrivate()
	{
		mCurrentFrames = 0;
		mCurrentFrame = -1;

		mCursor = 0;

		mWidget = 0;
	}

	QTimer mAnimateTimer;
	QIntDict<QCanvasPixmapArray> mCursorPixmaps;
	QCanvasPixmapArray* mCurrentFrames;
	int mCurrentFrame;

	QLabel* mCursor;

	QWidget* mWidget;
	QRect mRect;
};

BosonExperimentalCursor::BosonExperimentalCursor() : BosonCursor()
{
 d = new BosonExperimentalCursorPrivate;
 d->mCursorPixmaps.setAutoDelete(true);
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));

 init();
}

BosonExperimentalCursor::~BosonExperimentalCursor()
{
 d->mCursorPixmaps.clear();
 delete d->mCursor;
 delete d;
}

void BosonExperimentalCursor::init()
{
 // from kdesktop/startupid.cpp
 d->mCursor = new QLabel(0, 0, 
		WX11BypassWM|
		WRepaintNoErase|
		WStaticContents|
		WResizeNoErase);
// XSetWindowAttributes attr;
// attr.save_under = True; // useful saveunder if possible to avoid redrawing
// XChangeWindowAttributes( qt_xdisplay(), d->mCursor->winId(), CWSaveUnder, &attr );
 d->mCursor->setStyle("Windows"); // set a simple style w/o background
 d->mCursor->hide();
}

void BosonExperimentalCursor::slotAdvance()
{
// d->mCursor->hide();
	
// d->mCursor->setFrame((d->mCursor->frame() + 1 + d->mCursor->frameCount()) % d->mCursor->frameCount());
 QCanvasPixmap* p;
 if ((int)d->mCurrentFrames->count() > d->mCurrentFrame + 1) {
	d->mCurrentFrame++;
 } else {
	d->mCurrentFrame = 0;
 }
 
 p = d->mCurrentFrames->image(d->mCurrentFrame);
 if (!p) {
	d->mCursor->hide();
	return;
 }
 d->mCursor->resize(p->width(), p->height());
// d->mCursor->setBackground(TransparentMode);
 d->mCursor->setMask(*p->mask());
 d->mCursor->setPixmap(*p);

 d->mCursor->show();


// paintCursor(0, QCursor::pos());
}

void BosonExperimentalCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
 d->mAnimateTimer.stop();
 if (mode >= 0) {
	QCanvasPixmapArray* a = d->mCursorPixmaps[cursorMode()];
//	d->mCursor->hide();
	if (a) {
		d->mCurrentFrames = a;
		if (a->count() > 1) {
			d->mAnimateTimer.start(100);
//			d->mAnimateTimer.start(1000);
			}
		slotAdvance();
		d->mCursor->show();
	} else {
		d->mCursor->hide();
		kdWarning() << k_funcinfo << "NULL pixmap array for " << mode << endl;
	}
 } else {
	d->mCursor->hide();
 }
}

void BosonExperimentalCursor::setWidgetCursor(QWidget* w)
{
 d->mWidget = w;
 if (!w) {
	return;
 }
 if (!d->mCursor->parent()) {
	d->mCursor->reparent(w, QPoint(0,0));
	d->mCursor->hide();
 }

 if (d->mWidget->cursor().shape() != Qt::BlankCursor) {
//	kdDebug() << k_funcinfo << endl;
	d->mWidget->setCursor(Qt::BlankCursor);
 }
}

void BosonExperimentalCursor::move(double x, double y)
{
	return;
 d->mCursor->move(x, y); // TODO: hotspot!

 // from kdesktop/startupod.cpp:
// XRaiseWindow(qt_xdisplay(), d->mCursor->winId());
// QApplication::flushX();

 paintCursor(0, QPoint(x, y));
}

void BosonExperimentalCursor::insertMode(int mode, QCanvasPixmapArray* array)
{
 if (d->mCursorPixmaps[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mCursorPixmaps.remove(mode);
 }
 if (array) {
	d->mCursorPixmaps.insert(mode, array);
 }
}

void BosonExperimentalCursor::hideCursor()
{
// d->mCursor->hide();
}

void BosonExperimentalCursor::showCursor()
{
// d->mCursor->show();
}

void BosonExperimentalCursor::removeOldCursor()
{
 if (!d->mWidget) {
	return;
 }
 kdDebug() << k_funcinfo << endl;
// d->mWidget->repaint(d->mRect);
}

QRect BosonExperimentalCursor::oldCursor() const
{
 return d->mRect;
}

void BosonExperimentalCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 insertMode(mode, loadCursor(baseDir, cursor));
}

void BosonExperimentalCursor::paintCursor(QPainter* /**obsolete**/, const QPoint& pos)
{
	return;
kdDebug() << k_funcinfo << endl;
 if (!d->mCurrentFrames) {
	return;
 }
 if (d->mCurrentFrame < 0 || (int)d->mCurrentFrames->count() <= d->mCurrentFrame) {
	return;
 }
 if (!d->mWidget) {
	return;
 }
 QPainter p(d->mWidget);
kdDebug() << k_funcinfo << endl;
 QPixmap* pix = d->mCurrentFrames->image(d->mCurrentFrame);
 if (!pix) {
	return;
 }
 d->mRect = QRect(pos, QSize(pix->width(), pix->height()));
 p.drawPixmap(d->mRect, *pix);
 p.end();
kdDebug() << k_funcinfo << "end" << endl;
}

QCanvasPixmapArray* BosonExperimentalCursor::loadCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 QCanvasPixmapArray* array = 0;
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
 if (!array) {
	QValueList<QPixmap> p;
	QPointArray points(1);
	points.setPoint(0, 0, 0);
	p.append(*KCursor::arrowCursor().bitmap());
	kdWarning() << "loading fallback cursor..." << endl;
	array = new QCanvasPixmapArray();
 }
 return array;
}



