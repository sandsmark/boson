/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <bogl.h>

#include "boufotestmain.h"
#include "boufotestmain.moc"

#include "../boufo.h"
#include "boufofontselectionwidget.h"
#include <bodebug.h>

#include <qtimer.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#include <iostream>
#include <math.h>
#include <stdlib.h>


BoUfoTest::BoUfoTest(QWidget* parent, const char* name)
	: QGLWidget(parent, name, 0, Qt::WType_TopLevel | Qt::WDestructiveClose)
{
 setMouseTracking(true);
 setMinimumSize(800, 200);

 mUfoManager = 0;

 QTimer* updateTimer = new QTimer(this);
// connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(mySlotUpdateGL()));
 updateTimer->start(100);
 setUpdatesEnabled(false);
}

BoUfoTest::~BoUfoTest()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
}

void BoUfoTest::initializeGL()
{
 static bool recursive = false;
 static bool initialized = false;
 if (recursive) {
	return;
 }
 if (initialized) {
	return;
 }
 recursive = true;
 makeCurrent();

 glDisable(GL_DITHER);

 boDebug() << k_funcinfo << endl;
 mUfoManager = new BoUfoManager(width(), height(), true);
 mContentWidget = mUfoManager->contentWidget();

 mContentWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);

#define MULTILINE_TEST 0
#if MULTILINE_TEST

 QString s;
 for (int i = 0; i < 100; i++) {
	s += QString("foobar%1 ").arg(i + 1);
 }

#if 1
 BoUfoLabel* label2 = new BoUfoLabel();
// label2->setConstraints("north");
 mContentWidget->addWidget(label2);
 label2->setText(s);

// ufo::UDimension dim = label2->widget()->getPreferredSize();
// printf("label2 preferred size: %d,%d\n", dim.w, dim.h);
#endif

#if 0
 ufo::ULabel* label3 = new ufo::ULabel();
 mContentWidget->widget()->add(label3);
 label3->setText(s.latin1());
 ufo::UDimension dim3 = label3->getPreferredSize();
 printf("label3 preferred size: %d,%d\n", dim3.w, dim3.h);
#endif

#endif // MULTILINE_TEST

#define FONT_SELECTION_TEST 1
#if FONT_SELECTION_TEST
 BoUfoInternalFrame* frame = new BoUfoInternalFrame(mUfoManager, "Font selection");
 BoUfoFontSelectionWidget* selection = new BoUfoFontSelectionWidget(mUfoManager);
 frame->addWidget(selection);
 frame->setSize(frame->preferredWidth(), frame->preferredHeight());
 connect(selection, SIGNAL(signalFontSelected(const BoUfoFontInfo&)),
		this, SLOT(slotChangeFont(const BoUfoFontInfo&)));
#endif // FONT_SELECTION_TEST

 initUfoActions();

 recursive = false;
 initialized = true;
}

void BoUfoTest::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }

 glViewport(0, 0, width(), height());
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0, width(), 0, height());
 glMatrixMode(GL_MODELVIEW);
}

void BoUfoTest::paintGL()
{
 glClearColor(1, 1, 1, 0);
 glClear(GL_COLOR_BUFFER_BIT);
 glColor3f(1, 0, 0);

 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render();
 }

#if 0

#if 0
//right:
 glVertex3f(-0.868108, -0.00800257, -0.496311, 13.4017);
//left:
 glVertex3f(0.586832,  0.639786,    -0.496311, 2.97388);
//bottom:
 glVertex3f(-0.405266, 0.910257,    0.0848017, 15.4496);
//top:
 glVertex3f(0.172763,  -0.388037,   -0.905307, -1.91351);
//far:
 glVertex3f(0.232564,  -0.522039,   0.820603,  986.325);
//near:
 glVertex3f(-0.232504, 0.52222,    -0.820505,  13.4861);
#endif
//right:
 glBegin(GL_POINTS);
 glVertex3f(-0.868108, -0.00800257, -0.496311);
//left:
 glVertex3f(0.586832,  0.639786,    -0.496311);
//bottom:
 glVertex3f(-0.405266, 0.910257,    0.0848017);
//top:
 glVertex3f(0.172763,  -0.388037,   -0.905307);
//far:
 glVertex3f(0.232564,  -0.522039,   0.820603);
//near:
 glVertex3f(-0.232504, 0.52222,    -0.820505);

 glVertex2i(10, 10);
 glEnd();
#endif
}

void BoUfoTest::mySlotUpdateGL()
{
 QTime t;
 t.start();
 updateGL();
// boDebug() << k_funcinfo << "elapsed: " << t.elapsed() << endl;
}

bool BoUfoTest::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void BoUfoTest::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoTest::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoTest::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoTest::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoTest::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void BoUfoTest::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
}

void BoUfoTest::slotChangeFont(const BoUfoFontInfo& font)
{
 mUfoManager->setGlobalFont(font);
}

void BoUfoTest::initUfoActions()
{
}

int main(int argc, char **argv)
{
 std::cout << "resolving GL, GLX and GLU symbols" << std::endl;
 if (!boglResolveGLSymbols()) {
	// TODO: open a messagebox
	std::cerr << "Could not resolve all symbols!" << std::endl;
	return 1;
 }
 std::cout << "GL, GLX and GLU symbols successfully resolved" << std::endl;

 QApplication app(argc, argv);
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

 BoUfoTest* main = new BoUfoTest(0);
 app.setMainWidget(main);
 main->show();

 return app.exec();
}

