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
#include <kcursor.h>
#include <kdebug.h>

#include <qpainter.h>
#include <qwidget.h>
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

#include "bosontexturearray.h"
#include <qimage.h>

BosonSpriteCursorData::BosonSpriteCursorData()
{
 mArray = 0;
}

BosonSpriteCursorData::~BosonSpriteCursorData()
{
 delete mArray;
}

class BosonSpriteCursor::BosonSpriteCursorPrivate
{
public:
	BosonSpriteCursorPrivate()
	{
		mCurrentFrame = 0;
	}

	int mCurrentFrame;
	QTimer mAnimateTimer;

	QIntDict<BosonSpriteCursorData> mCursors;
};

BosonSpriteCursor::BosonSpriteCursor() : BosonCursor()
{
 d = new BosonSpriteCursorPrivate;
 d->mCursors.setAutoDelete(true);
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));
 mCurrentData = 0;
}

BosonSpriteCursor::~BosonSpriteCursor()
{
 d->mCursors.clear();
 delete d;
}

void BosonSpriteCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
 d->mAnimateTimer.stop();
// kdDebug() << k_funcinfo << endl;
 if (cursorMode() >= 0) {
	BosonSpriteCursorData* data;
	data = d->mCursors[cursorMode()];
	hideCursor();
	if (data) {
//		kdDebug() << k_funcinfo << mode << endl;
		if (!data->mArray || !data->mArray->isValid()) {
			kdError() << k_funcinfo << "array not valid for mode=" << mode << endl;
			return;
		}
		setCurrentData(data);
	} else {
		kdWarning() << k_funcinfo << "NULL array for " << mode << endl;
		hideCursor();
	}
 } else {
	hideCursor();
 }
}

void BosonSpriteCursor::setWidgetCursor(QWidget* w)
{
 // TODO: check if the cursor data (i.e. textures are valid - otherwise display
 // a default cursor!)
 if (w->cursor().shape() != Qt::BlankCursor) {
//	kdDebug() << k_funcinfo << endl;
	w->setCursor(Qt::BlankCursor);
 }
}

bool BosonSpriteCursor::insertMode(int mode, BosonSpriteCursorData* data)
{
 if (d->mCursors[mode]) {
	kdWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
	d->mCursors.remove(mode);
 }
 if (data) {
	d->mCursors.insert(mode, data);
	if (!mCurrentData) {
		setCurrentData(data);
	}
	return true;
 }
 return false;
}

void BosonSpriteCursor::slotAdvance()
{
 if (!mCurrentData || !mCurrentData->mArray) {
	return;
 }
 d->mCurrentFrame++;
 if (d->mCurrentFrame >= (int)mCurrentData->mArray->count()) {
	d->mCurrentFrame = 0;
 }
 mCurrentTexture = mCurrentData->mArray->texture(d->mCurrentFrame);
}

void BosonSpriteCursor::hideCursor()
{
//TODO
}

void BosonSpriteCursor::showCursor()
{
//TODO
}

bool BosonSpriteCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 return insertMode(mode, loadSpriteCursor(baseDir, cursor));
}

BosonSpriteCursorData* BosonSpriteCursor::loadSpriteCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 KSimpleConfig c(baseDir + cursor + QString::fromLatin1("/index.desktop"));
 kdDebug() << baseDir << endl;
 if (!c.hasGroup("Boson Cursor")) {
	kdWarning() << k_funcinfo << "index.desktop is missing default group" << endl;
	return 0;
 }
 c.setGroup("Boson Cursor");
 QString filePrefix = c.readEntry("FilePrefix", QString::fromLatin1("cursor-"));
 unsigned int frames = c.readUnsignedNumEntry("FrameCount", 1);
 QValueList<QImage> images;

 unsigned int hotspotX = c.readUnsignedNumEntry(QString("HotspotX"), 0);
 unsigned int hotspotY = c.readUnsignedNumEntry(QString("HotspotY"), 0);
 for (unsigned int j = 0; j < frames; j++) {
	QString number;
	number.sprintf("%04d", j);
	QString file = QString::fromLatin1("%1%2/%3%4.png").arg(baseDir).arg(cursor).arg(filePrefix).arg(number);
	QImage image;
	if (!image.load(file)) {
		kdError() << k_funcinfo << "Could not load " << file << endl;
		// TODO: load dummy image
	}
	images.append(image);
 }

 if (images.count() > 0) {
	BosonSpriteCursorData* data = new BosonSpriteCursorData;
	data->mArray = new BosonTextureArray(images, false);
	data->mHotspotX = hotspotX;
	data->mHotspotY = hotspotY;
	return data;
 } else {
	kdError() << k_funcinfo << "Could not load from " << baseDir + cursor << endl;
 }
 return 0;
}

void BosonSpriteCursor::setCurrentData(BosonSpriteCursorData* data)
{
 if (data == mCurrentData) {
	return;
 }
 if (!data) {
	kdWarning() << k_funcinfo << "NULL data" << endl;
	hideCursor();
	mCurrentData = 0;
	return;
 }
 if (!data->mArray || !data->mArray->isValid()) {
	kdError() << k_funcinfo << "invalid texture array" << endl;
	return;
 }
 mCurrentData = data;
 mCurrentTexture = data->mArray->texture(0);
 if (d->mCurrentFrame >= (int)mCurrentData->mArray->count()) {
	d->mCurrentFrame = 0;
 }
 if (mCurrentData->mArray->count() > 1) {
	d->mAnimateTimer.start(100);
 }
 showCursor();
}

