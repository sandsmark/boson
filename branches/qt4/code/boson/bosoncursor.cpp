/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosoncursor.h"

#include "../bomemory/bodummymemory.h"
#include "botexture.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kcursor.h>

#include <qwidget.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <q3intdict.h>
#include <qimage.h>

#include "bosoncursor.moc"


BosonCursor::BosonCursor()
	: QObject()
{
 mMode = -1;
}

BosonCursor::~BosonCursor()
{
}

void BosonCursor::setCursor(int mode)
{
 mMode = mode;
 emit signalSetWidgetCursor(this);
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
 QString theme = QString::fromLatin1("futuristic");
 QString cursorDir = KGlobal::dirs()->findResourceDir("data",
		QString("boson/themes/cursors/%1/index.cursor").arg(theme));
 if (cursorDir.isEmpty()) {
	boError() << k_funcinfo << "cannot find default cursor theme " << theme << endl;
	return QString::null;
 }
 cursorDir += QString::fromLatin1("boson/themes/cursors/");
 cursorDir += theme;
 return cursorDir;
}

bool BosonCursor::insertDefaultModes(const QString& cursorDir)
{
 bool ret = true;
 if (!insertMode(CursorMove, cursorDir, QString::fromLatin1("move"))) {
	ret = false;
 }
 if (!insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"))) {
	ret = false;
 }
 if (!insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"))) {
	ret = false;
 }
 return ret;
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
 return Qt::ArrowCursor;
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

	Q3IntDict<BosonOpenGLCursorData> mCursors;
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
	if (qAbs(d->mCurrentRotate) >= 360) {
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
	const GLfloat w = 32.0f;
	const GLfloat h = 32.0f;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
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
	glPopAttrib();
 }
}



BosonCursorCollection::BosonCursorCollection(QObject* parent)
	: QObject(parent)
{
 mCursor = 0;
 mCursorType = -1;
}

BosonCursorCollection::~BosonCursorCollection()
{
 QMap<int, QMap<QString, BosonCursor*> >::iterator it;
 for (it = mCursors.begin(); it != mCursors.end(); ++it) {
	QMap<QString, BosonCursor*>::iterator it2;
	for (it2 = (*it).begin(); it2 != (*it).end(); ++it2) {
		delete *it2;
	}
 }
 mCursors.clear();
}

BosonCursor* BosonCursorCollection::loadCursor(int type, const QString& cursorDir_, QString& cursorDir)
{
 cursorDir = cursorDir_;
 if (cursorDir.isNull()) {
	cursorDir = BosonCursor::defaultTheme();
 }

 BosonCursor* b = (mCursors[type])[cursorDir_];
 if (b) {
	return b;
 }
 switch (type) {
	case CursorOpenGL:
		b = new BosonOpenGLCursor;
		break;
	default:
		type = CursorKDE;
	case CursorKDE:
		b = new BosonKDECursor;
		break;
 }

 if (!b->insertDefaultModes(cursorDir)) {
	boError() << k_funcinfo << "Could not load cursor type " << type << " from " << cursorDir << endl;
	delete b;
	if (type != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		return loadCursor(CursorKDE, QString::null, cursorDir);
	}
	boError() << k_funcinfo << "oops - loading CursorKDE failed. THIS MUST NEVER HAPPEN!" << endl;
	return 0;
 }
 (mCursors[type]).insert(cursorDir, b);

 return b;
}

BosonCursor* BosonCursorCollection::changeCursor(int type, const QString& cursorDir, QString* actualCursorDir)
{
 QString dir;
 BosonCursor* b = loadCursor(type, cursorDir, dir);
 if (actualCursorDir) {
	*actualCursorDir = dir;
 }

 if (b) {
	if (mCursor) {
		disconnect(mCursor, 0, this, 0);
	}
	mCursor = b;
	mCursorDir = dir;
	mCursorType = type;
	connect(mCursor, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
			this, SIGNAL(signalSetWidgetCursor(BosonCursor*)));

	// make sure this is called at least once with the correct cursor
	emit signalSetWidgetCursor(mCursor);
 } else {
	// will never happen, as loadCursor() falls back to CursorKDE.
	boError() << k_funcinfo << "loading cursor failed." << endl;
 }
 return b;
}



