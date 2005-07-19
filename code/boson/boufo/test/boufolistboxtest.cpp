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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <bogl.h>

#include <ufo/ufo.hpp>

#include "boufolistboxtest.h"
#include "boufolistboxtest.moc"

#include "../boufo.h"
#include <bodebug.h>

#include <klocale.h>

#include <qtimer.h>

#include <iostream>
#include <math.h>
#include <stdlib.h>


BoUfoListBoxTest::BoUfoListBoxTest(QWidget* parent, const char* name)
	: QGLWidget(parent, name, 0, Qt::WType_TopLevel | Qt::WDestructiveClose)
{
 setMouseTracking(true);

 mUfoManager = 0;

 QTimer* updateTimer = new QTimer(this);
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 updateTimer->start(100);
 setUpdatesEnabled(false);
}

BoUfoListBoxTest::~BoUfoListBoxTest()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
}

void BoUfoListBoxTest::initializeGL()
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

 BoUfoLabel* label = new BoUfoLabel(i18n("List box:"));
 mContentWidget->addWidget(label);

 BoUfoListBox* list = new BoUfoListBox();
 mContentWidget->addWidget(list);

 list->insertItem(i18n("First item"));
 for (int i = 2; i <= 100; i++) {
	list->insertItem(i18n("Item %1").arg(i));
 }
 list->insertItem(i18n("Last item"));

 recursive = false;
 initialized = true;
}

void BoUfoListBoxTest::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }
}

void BoUfoListBoxTest::paintGL()
{
 glClearColor(1, 1, 1, 0);
 glClear(GL_COLOR_BUFFER_BIT);
 glColor3f(1, 0, 0);

 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render(false);
 }
}

bool BoUfoListBoxTest::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void BoUfoListBoxTest::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoListBoxTest::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoListBoxTest::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoListBoxTest::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoListBoxTest::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void BoUfoListBoxTest::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
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

 BoUfoListBoxTest* main = new BoUfoListBoxTest(0);
 app.setMainWidget(main);
 main->show();

 return app.exec();
}

