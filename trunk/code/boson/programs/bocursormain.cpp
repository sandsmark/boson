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

#include "bocursormain.h"
#include "bocursormain.moc"

#include "../bomemory/bodummymemory.h"
#include "bosoncursoreditor.h"
#include "bosoncursor.h"
#include "bosonconfig.h"
#include "global.h"
#include "bosonglwidget.h"
#include "bodebug.h"
#include "bodebugdcopiface.h"
#include "boversion.h"
#include "boapplication.h"
#include "boglobal.h"
#include "botexture.h"
#include <bogl.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <qlayout.h>
#include <qcursor.h>
#include <qtimer.h>


static void postBosonConfigInit();

static const char *description =
    I18N_NOOP("Cursor Editor for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { 0, 0, 0 }
};

CursorPreview::CursorPreview(QWidget* parent) : BosonGLWidget(parent)
{
 mCursor = new BosonKDECursor;
 setMinimumSize(200, 200);
 setMouseTracking(true);
 mUpdateTimer = new QTimer(this);
 connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGL()));
 mIface = new BoDebugDCOPIface();
}

CursorPreview::~CursorPreview()
{
 delete mUpdateTimer;
 delete mCursor;
 BoTextureManager::deleteStatic();
}

void CursorPreview::initializeGL()
{
 BoGL::bogl()->initialize();
 glClearColor(0.0, 0.0, 0.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 resizeGL(width(), height());
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 mUpdateTimer->start(30);
 BoTextureManager::initStatic();
}

void CursorPreview::resizeGL(int w, int h)
{
 glViewport(0, 0, w, h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
 glMatrixMode(GL_MODELVIEW);
}

void CursorPreview::paintGL()
{
 glClear(GL_COLOR_BUFFER_BIT);
 if (!cursor()) {
	return;
 }
 glColor3ub(255, 255, 255);
 QPoint pos = mapFromGlobal(QCursor::pos());
 GLfloat x, y;
 x = (GLfloat)pos.x();
 y = height() - (GLfloat)pos.y();

 glEnable(GL_TEXTURE_2D);
 cursor()->renderCursor(x, y);

 // display the hotspot
 boTextureManager->disableTexturing();
 glBegin(GL_LINES);
	glVertex3f(x, 0.0, 0.0);
	glVertex3f(x, height(), 0.0);

	glVertex3f(0.0, y, 0.0);
	glVertex3f(width(), y, 0.0);
 glEnd();
}

void CursorPreview::slotChangeCursor(int mode, const QString& cursorDir_)
{
 BosonCursor* b;
 switch (mode) {
	case CursorOpenGL:
		makeCurrent();
		b = new BosonOpenGLCursor;
		break;
	case CursorKDE:
	default:
		b = new BosonKDECursor;
		break;
 }

 QString cursorDir = cursorDir_;
 if (cursorDir.isNull()) {
	cursorDir = BosonCursor::defaultTheme();
 }

 bool ok = true;
 if (!b->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"))) {
	ok = false;
 }
 if (!b->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"))) {
	ok = false;
 }
 if (!b->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"))) {
	ok = false;
 }
 if (!ok) {
	boError() << k_funcinfo << "Could not load cursor mode " << mode << " from " << cursorDir << endl;
	delete b;
	if (!cursor() && mode != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	return;
 }
 delete mCursor;
 mCursor = b;
}

void CursorPreview::slotChangeCursorType(int type)
{
 if (cursor()) {
	cursor()->setCursor(type);
 }
}

void CursorPreview::mouseMoveEvent(QMouseEvent* )
{
 if (mCursor) {
	mCursor->setWidgetCursor(this);
 }
}


int main(int argc, char **argv)
{
 KAboutData about("bocursor",
		I18N_NOOP("Boson Cursor Editor"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2002-2005 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Design & Coding"), "b_mann@gmx.de" );
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);

 BoApplication app(argv0);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QWidget* w = new QWidget(0);
 QHBoxLayout* l = new QHBoxLayout(w);
 BosonCursorEditor* editor = new BosonCursorEditor(w);
 l->addWidget(editor, 0);

 CursorPreview* preview = new CursorPreview(w);
 l->addWidget(preview, 1);

 QObject::connect(editor, SIGNAL(signalCursorChanged(int, const QString&)),
		preview, SLOT(slotChangeCursor(int, const QString&)));
 QObject::connect(editor, SIGNAL(signalCursorTypeChanged(int)),
		preview, SLOT(slotChangeCursorType(int)));

 editor->loadInitialCursor();

 app.setMainWidget(w);
 w->show();

 args->clear();

 boTextureManager->initOpenGL();

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 int r = app.exec();
 delete iface;
 delete editor;
 delete preview;
 delete w;
 return r;
}

static void postBosonConfigInit()
{
 BosonConfig* conf = BoGlobal::boGlobal()->bosonConfig();
 if (!conf) {
	boError() << k_funcinfo << "NULL BosonConfig object" << endl;
	return;
 }
 conf->setBoolValue("ForceDisableSound", true);
}

