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

#include "rtti.h"
#include "botexture.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kcursor.h>

#include <qwidget.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qintdict.h>
#include <qimage.h>

#include "bosoncursor.moc"


BosonCursor::BosonCursor()
{
 mMode = -1;
}

BosonCursor::~BosonCursor()
{
}

void BosonCursor::setCursor(int mode)
{
 mMode = mode;
}

QCursor BosonCursor::cursor() const
{
 return QCursor(QCursor::ArrowCursor);
}

QStringList BosonCursor::availableThemes()
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/themes/cursors/*/index.cursor");
 QStringList retList;
 for (unsigned int i = 0; i < list.count(); i++) {
	retList.append(list[i].left(list[i].length() - strlen("/index.cursor")));
 }

 return retList;
}

QString BosonCursor::defaultTheme()
{
 QString cursorDir = KGlobal::dirs()->findResourceDir("data",
		"boson/themes/cursors/default/index.cursor") +
		QString::fromLatin1("boson/themes/cursors/default");
 return cursorDir;
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
// boDebug() << k_funcinfo << endl;
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
/////// BosonOpenGLCursor ///////////////
/////////////////////////////////////////

BosonOpenGLCursorData::BosonOpenGLCursorData()
{
 mTextureArray = 0;
 mTextureCount = 0;
 mHotspotX = 0;
 mHotspotY = 0;
 mAnimated = false;
 mAnimationSpeed = 0;
 mRotateDegree = 0;
}

BosonOpenGLCursorData::~BosonOpenGLCursorData()
{
 delete mTextureArray;
}

bool BosonOpenGLCursorData::loadTextures(QStringList files)
{
 mTextureArray = new BoTextureArray(files, BoTexture::UI);
 if (mTextureArray->count() == 0) {
	return false;
 }
 mTextureCount = mTextureArray->count();
 return true;
}

class BosonOpenGLCursor::BosonOpenGLCursorPrivate
{
public:
	BosonOpenGLCursorPrivate()
	{
		mCurrentFrame = 0;
		mCurrentRotate = 0;
	}

	QTimer mAnimateTimer;
	int mCurrentFrame;
	int mCurrentRotate;

	QIntDict<BosonOpenGLCursorData> mCursors;
};

BosonOpenGLCursor::BosonOpenGLCursor() : BosonCursor()
{
 d = new BosonOpenGLCursorPrivate;
 d->mCursors.setAutoDelete(true);
 connect(&d->mAnimateTimer, SIGNAL(timeout()), this, SLOT(slotAdvance()));
 mCurrentData = 0;
}

BosonOpenGLCursor::~BosonOpenGLCursor()
{
 d->mCursors.clear();
 delete d;
}

void BosonOpenGLCursor::setCursor(int mode)
{
 if (mode == cursorMode()) {
	return;
 }
 BosonCursor::setCursor(mode);
 d->mAnimateTimer.stop();
// boDebug() << k_funcinfo << endl;
 if (cursorMode() >= 0) {
	BosonOpenGLCursorData* data;
	data = d->mCursors[cursorMode()];
	if (data) {
//		boDebug() << k_funcinfo << mode << endl;
		if (data->mTextureCount == 0) {
			boError() << k_funcinfo << "textures not valid for mode=" << mode << endl;
			return;
		}
		setCurrentData(data);
	} else {
		boWarning() << k_funcinfo << "NULL array for " << mode << endl;
	}
 }
}

void BosonOpenGLCursor::setWidgetCursor(QWidget* w)
{
 // TODO: check if the cursor data (i.e. textures are valid - otherwise display
 // a default cursor!)
 if (w->cursor().shape() != Qt::BlankCursor) {
//	boDebug() << k_funcinfo << endl;
	w->setCursor(Qt::BlankCursor);
 }
}

bool BosonOpenGLCursor::insertMode(int mode, BosonOpenGLCursorData* data)
{
 if (d->mCursors[mode]) {
	boWarning() << k_funcinfo << "Mode already inserted - removing first" << endl;
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

void BosonOpenGLCursor::slotAdvance()
{
 if (!mCurrentData || mCurrentData->mTextureCount == 0) {
	return;
 }
 if (!mCurrentData->mAnimated || !mCurrentData->mAnimationSpeed) {
	return;
 }
 d->mCurrentFrame++;
 if (d->mCurrentFrame >= (int)mCurrentData->mTextureCount) {
	d->mCurrentFrame = 0;
	d->mCurrentRotate += mCurrentData->mRotateDegree;
	if (QABS(d->mCurrentRotate) >= 360) {
		d->mCurrentRotate = 0;
	}
 }
}

bool BosonOpenGLCursor::insertMode(int mode, QString baseDir, QString cursor)
{
 return insertMode(mode, loadSpriteCursor(baseDir, cursor));
}

BosonOpenGLCursorData* BosonOpenGLCursor::loadSpriteCursor(QString baseDir, QString cursor)
{
 if (baseDir.right(1) != QString::fromLatin1("/")) {
	baseDir += QString::fromLatin1("/");
 }
 KSimpleConfig c(baseDir + cursor + QString::fromLatin1("/index.cursor"));
 boDebug() << baseDir << endl;
 if (!c.hasGroup("Boson Cursor")) {
	boWarning() << k_funcinfo << "index.cursor is missing default group" << endl;
	return 0;
 }
 c.setGroup("Boson Cursor");
 QString filePrefix = c.readEntry("FilePrefix", QString::fromLatin1("cursor-"));
 unsigned int hotspotX = c.readUnsignedNumEntry(QString("HotspotX"), 0);
 unsigned int hotspotY = c.readUnsignedNumEntry(QString("HotspotY"), 0);
 bool animated = c.readBoolEntry("IsAnimated", false);
 unsigned int animationSpeed = 0;
 unsigned int frames = 1;
 int rotateDegree = 0;
 if (animated) {
	if (!c.hasGroup("Animation")) {
		animated = false;
	} else {
		c.setGroup("Animation");
		frames = c.readUnsignedNumEntry("FrameCount", 1);
		animationSpeed = c.readUnsignedNumEntry("Speed", 0);
		rotateDegree = c.readNumEntry("RotateDegree", 0);
	}
 }

 QStringList files;
 for (unsigned int j = 0; j < frames; j++) {
	QString number;
	number.sprintf("%04d", j);
	QString file = QString::fromLatin1("%1%2/%3%4.png").arg(baseDir).arg(cursor).arg(filePrefix).arg(number);
	files.append(file);
 }

 if (files.count() > 0) {
	BosonOpenGLCursorData* data = new BosonOpenGLCursorData;
	data->setHotspot(hotspotX, hotspotY);
	data->loadTextures(files);
	if (data->mTextureArray->count() == 0) {
		boError() << k_funcinfo << "Unable to load textures for cursor " << baseDir << "," << cursor << endl;
		delete data;
		return 0;
	}
	data->mAnimated = animated;
	data->mAnimationSpeed = animationSpeed;
	data->mRotateDegree = rotateDegree;
	return data;
 } else {
	boError() << k_funcinfo << "Could not load from " << baseDir + cursor << endl;
 }
 return 0;
}

void BosonOpenGLCursor::setCurrentData(BosonOpenGLCursorData* data)
{
 if (data == mCurrentData) {
	return;
 }
 if (!data) {
	boWarning() << k_funcinfo << "NULL data" << endl;
	mCurrentData = 0;
	return;
 }
 if (!data->mTextureArray || data->mTextureCount == 0) {
	boError() << k_funcinfo << "invalid textures" << endl;
	return;
 }
 mCurrentData = data;
 if (d->mCurrentFrame >= (int)mCurrentData->mTextureCount) {
	d->mCurrentFrame = 0;
 }
 d->mCurrentRotate = 0;

 if (mCurrentData->mAnimated && mCurrentData->mAnimationSpeed > 0) {
	d->mAnimateTimer.start(mCurrentData->mAnimationSpeed);
 }
}

void BosonOpenGLCursor::renderCursor(GLfloat x, GLfloat y)
{
 if (mCurrentData) {
	// FIXME: we currently depend on image width/height == BO_TILE_SIZE
	// we should use the actual width/height here instead!
	const GLfloat w = 48.0f;
	const GLfloat h = 48.0f;

	glEnable(GL_BLEND);
	glPushMatrix();
	glTranslatef(x - mCurrentData->mHotspotX, y - mCurrentData->mHotspotY, 0.0);
	glRotatef(d->mCurrentRotate, 0.0, 0.0, 1.0);
	mCurrentData->mTextureArray->texture(d->mCurrentFrame)->bind();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(0, h, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(w, h, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f(w, 0, 0.0);
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
 }
}

