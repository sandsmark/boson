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

#include "rtti.h" // if we ever make this class public we should remove dependancy on this class
#include "bosoncanvas.h" // see above .. but since we don't depend on QCanvas anymore thats just an illusion ;)

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

bool BosonNormalCursor::insertMode(int mode, QCursor* cursor)
{
 if (d->mQCursors[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mQCursors.remove(mode);
 }
 if (cursor) {
	d->mQCursors.insert(mode, cursor);
	return true;
 }
 return false;
}

bool BosonNormalCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 return insertMode(mode, loadQCursor(baseDir, cursor));
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

bool BosonKDECursor::insertMode(int , QString , QString )
{
 // always successful :-)
 return true;
}


/////////////////////////////////////////
/////// BosonSpriteCursor ///////////////
/////////////////////////////////////////

#ifndef NO_OPENGL
#include "bosontexturearray.h"
#include <qimage.h>

#else

class BoCursorSprite : public QCanvasSprite
{
public:
	BoCursorSprite(QCanvasPixmapArray* a,QCanvas* c) : QCanvasSprite(a,c) { }
	virtual bool collidesWith(const QCanvasItem*) const { return false; }
	virtual int rtti() const { return RTTI::SpriteCursor; }
};
#endif // !NO_OPENGL

class BosonSpriteCursor::BosonSpriteCursorPrivate
{
public:
	BosonSpriteCursorPrivate()
	{
		mCurrentFrame = 0;

#ifndef NO_OPENGL
		mCurrentTextures = 0;
#else
		mCanvas = 0;
		mCurrentFrames = 0;
		mCursor = 0;
#endif
	}

	QTimer mAnimateTimer;
	int mCurrentFrame;

#ifndef NO_OPENGL
	QIntDict<BosonTextureArray> mCursorPixmaps;
	BosonTextureArray* mCurrentTextures;
#else
	QCanvas* mCanvas;
	QIntDict<QCanvasPixmapArray> mCursorPixmaps;
	QCanvasPixmapArray* mCurrentFrames;

	BoCursorSprite* mCursor;
#endif

	unsigned int mHotspotX;
	unsigned int mHotspotY;
};

BosonSpriteCursor::BosonSpriteCursor() : BosonCursor()
{
 d = new BosonSpriteCursorPrivate;
 d->mCursorPixmaps.setAutoDelete(true);
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));
 mHotspotX = 0;
 mHotspotY = 0;
}

BosonSpriteCursor::~BosonSpriteCursor()
{
#ifdef NO_OPENGL
 delete d->mCursor;
#endif
 d->mCursorPixmaps.clear();
 delete d;
}

void BosonSpriteCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
#ifdef NO_OPENGL
 if (!d->mCursor) {
	kdError() << k_funcinfo << "NULL cursor sprite" << endl;
	return;
 }
#endif
 BosonCursor::setCursor(mode);
 d->mAnimateTimer.stop();
// kdDebug() << k_funcinfo << endl;
 if (mode >= 0) {
#ifndef NO_OPENGL
	BosonTextureArray * a;
#else
	QCanvasPixmapArray* a;
#endif
	a = d->mCursorPixmaps[cursorMode()];
	hideCursor();
	if (a) {
//		kdDebug() << k_funcinfo << mode << endl;
		if (!a->isValid()) {
			kdError() << k_funcinfo << "array not valid for mode=" << mode << endl;
			return;
		}
#ifndef NO_OPENGL
		setCurrentTextureArray(a);
//		d->mCursor->setFrame(0);
		showCursor();
		if (d->mCurrentTextures && d->mCurrentTextures->count() > 1) {
			d->mAnimateTimer.start(100);
		}
#else
		// workaround for a qt < 3.0.5 bug
		if (d->mCursor->frame() >= (int)a->count()) {
			d->mCursor->setFrame(0);
		}
		d->mCurrentFrames = a;
		d->mCursor->setSequence(a);
		d->mCursor->setFrame(0);
		showCursor();
		if (d->mCursor->frameCount() > 1) {
			d->mAnimateTimer.start(100);
		}
#endif
	} else {
		kdWarning() << k_funcinfo << "NULL array for " << mode << endl;
		hideCursor();
	}
 } else {
	hideCursor();
 }
}

void BosonSpriteCursor::setCanvas(BosonCanvas* canvas, int mode, int z)
{
#ifdef NO_OPENGL
 if (d->mCanvas) {
	kdError() << k_funcinfo << "Canvas already set" << endl;
	return;
 }
 d->mCanvas = canvas;
 if (d->mCursor) {
	d->mCursor->setCanvas(canvas);
 } else {
	d->mCursor = new BoCursorSprite(0, d->mCanvas);
	d->mCursor->setZ(z);
 }
#endif // NO_OPENGL
 setCursor(mode);
}

void BosonSpriteCursor::setWidgetCursor(QWidget* w)
{
 if (w->cursor().shape() != Qt::BlankCursor) {
//	kdDebug() << k_funcinfo << endl;
	w->setCursor(Qt::BlankCursor);
 }
}

#ifndef NO_OPENGL
bool BosonSpriteCursor::insertMode(int mode, BosonTextureArray* array)
#else
bool BosonSpriteCursor::insertMode(int mode, QCanvasPixmapArray* array)
#endif
{
 if (d->mCursorPixmaps[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mCursorPixmaps.remove(mode);
 }
 if (array) {
	d->mCursorPixmaps.insert(mode, array);
#ifndef NO_OPENGL
	if (!d->mCurrentTextures) {
		d->mCurrentTextures = array;
	}
#endif
	return true;
 }
 return false;
}

#ifdef NO_OPENGL
QCanvasSprite* BosonSpriteCursor::cursorSprite() const
{
 return d->mCursor;
}
#endif

void BosonSpriteCursor::move(double x, double y)
{
#ifdef NO_OPENGL
 d->mCursor->move(x, y);
#endif
}

void BosonSpriteCursor::slotAdvance()
{
#ifndef NO_OPENGL
 if (!d->mCurrentTextures) {
	return;
 }
 d->mCurrentFrame++;
 if (d->mCurrentFrame >= (int)d->mCurrentTextures->count()) {
	d->mCurrentFrame = 0;
 }
 // update the GL widget - not really nice, since its already updated pretty
 // often :(
 // but we need to do this, especially if game speed is low
 emit signalUpdate();
#else
 d->mCursor->setFrame((d->mCursor->frame() + 1 + d->mCursor->frameCount()) % d->mCursor->frameCount());

 // not a real ideal solution... this gets called very often anyway.
 // we need it here when the game speed is very low
 d->mCursor->canvas()->update();
#endif
}

void BosonSpriteCursor::hideCursor()
{
#ifdef NO_OPENGL
 if (d->mCursor) {
	d->mCursor->hide();
 }
#endif
}

void BosonSpriteCursor::showCursor()
{
#ifdef NO_OPENGL
 if (d->mCursor) {
	d->mCursor->show();
 }
#endif
}

bool BosonSpriteCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 return insertMode(mode, loadSpriteCursor(baseDir, cursor));
}

#ifndef NO_OPENGL
BosonTextureArray* BosonSpriteCursor::loadSpriteCursor(QString baseDir, QString cursor)
#else
QCanvasPixmapArray* BosonSpriteCursor::loadSpriteCursor(QString baseDir, QString cursor)
#endif
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 KSimpleConfig c(baseDir + cursor + QString::fromLatin1("/index.desktop"));
 kdDebug() << baseDir << endl;
#ifdef NO_OPENGL
 QCanvasPixmapArray* array = 0;
#endif
 if (!c.hasGroup("Boson Cursor")) {
	kdWarning() << k_funcinfo << "index.desktop is missing default group" << endl;
	return 0;
 }
 c.setGroup("Boson Cursor");
 QString filePrefix = c.readEntry("FilePrefix", QString::fromLatin1("cursor-"));
 unsigned int frames = c.readUnsignedNumEntry("FrameCount", 1);
#ifndef NO_OPENGL
 QValueList<QImage> images;
#else
 QValueList<QPixmap> pixmaps;
 QPointArray points(frames);
#endif

 mHotspotX = c.readUnsignedNumEntry(QString("HotspotX"), 0);
 mHotspotY = c.readUnsignedNumEntry(QString("HotspotY"), 0);
 for (unsigned int j = 0; j < frames; j++) {
	QString number;
	number.sprintf("%04d", j);
	QString file = QString::fromLatin1("%1%2/%3%4.png").arg(baseDir).arg(cursor).arg(filePrefix).arg(number);
#ifndef NO_OPENGL
	QImage image;
	if (!image.load(file)) {
		kdError() << k_funcinfo << "Could not load " << file << endl;
		// TODO: load dummy image
	}
	images.append(image);
#else
	points.setPoint(j, mHotspotX, mHotspotY);
	QPixmap p;
	if (p.load(file)) {
		QBitmap mask(file);
		p.setMask(mask);
	} else {
		kdError() << k_funcinfo << "Could not load " << file << endl;
	}
	pixmaps.append(p);
#endif // !NO_OPENGL
 }
#ifndef NO_OPENGL
 if (images.count() > 0) {
	kdDebug() << "create cursor texture array" << endl;
	BosonTextureArray* array = new BosonTextureArray(images);
	kdDebug() << "created texture array" << endl;
	return array;
#else
 if (pixmaps.count() > 0) {
	array = new QCanvasPixmapArray(pixmaps, points);
	return array;
#endif
 } else {
	kdError() << k_funcinfo << "Could not load from " << baseDir + cursor << endl;
 }
 return 0;
}

#ifndef NO_OPENGL
GLuint BosonSpriteCursor::currentTexture() const
{
 BosonTextureArray* a = d->mCurrentTextures;
 if (!a) {
	kdError() << k_funcinfo << "NULL current texture array" << endl;
	return 0;
 }
 return a->texture(d->mCurrentFrame);
}

void BosonSpriteCursor::setCurrentTextureArray(BosonTextureArray* array)
{
 if (!array) {
	kdError() << k_funcinfo << "NULL texture array" << endl;
	return;
 }
 if (!array->isValid()) {
	kdError() << k_funcinfo << "invalid texture array" << endl;
	return;
 }
 d->mCurrentTextures = array;
 if (d->mCurrentFrame >= (int)d->mCurrentTextures->count()) {
	d->mCurrentFrame = 0;
 }
}
#else
QPoint BosonSpriteCursor::pos() const
{
 return QPoint((int)d->mCursor->x(), (int)d->mCursor->y());
}
#endif

