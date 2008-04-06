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

#include <ufo/ufo.hpp>

#include "boufobuttontest.h"
#include "boufobuttontest.moc"

#include "../boufo.h"
#include <bodebug.h>

#include <qtimer.h>
#include <qimage.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include <iostream>
#include <math.h>
#include <stdlib.h>

//#define GL_FORMAT_OPTIONS (0)
#define GL_FORMAT_OPTIONS (0 | QGL::IndirectRendering)

BoUfoButtonTest::BoUfoButtonTest(QWidget* parent, const char* name)
	: QGLWidget(QGLFormat(GL_FORMAT_OPTIONS), parent, name, 0, Qt::WType_TopLevel | Qt::WDestructiveClose)
{
 setMouseTracking(true);

 mUfoManager = 0;

 QTimer* updateTimer = new QTimer(this);
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 updateTimer->start(100);
 setUpdatesEnabled(false);
}

BoUfoButtonTest::~BoUfoButtonTest()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
}

void BoUfoButtonTest::initializeGL()
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

 BoUfoPushButton* button1 = new BoUfoPushButton("Button1");
 BoUfoPushButton* button2 = new BoUfoPushButton("Button2");
 mContentWidget->addWidget(button1);
 mContentWidget->addWidget(button2);

 BoUfoWidget* hbox = new BoUfoWidget();
 hbox->setLayoutClass(BoUfoWidget::UHBoxLayout);
 mContentWidget->addWidget(hbox);
 BoUfoPushButton* button3 = new BoUfoPushButton("Button3 (HBox button 1)");
 BoUfoPushButton* button4 = new BoUfoPushButton("Button4 (HBox button 2)");
 hbox->addWidget(button3);
 hbox->addWidget(button4);

 BoUfoCheckBox* check = new BoUfoCheckBox("Checkbox");
 mContentWidget->addWidget(check);

 BoUfoWidget* container = new BoUfoWidget(); // AB: dummy container, to catch possible layout problems
 mContentWidget->addWidget(container);
 BoUfoButtonGroupWidget* buttonGroup = new BoUfoButtonGroupWidget();
 buttonGroup->setLayoutClass(BoUfoWidget::UHBoxLayout);
 container->addWidget(buttonGroup);
 BoUfoRadioButton* radio1 = new BoUfoRadioButton("Radio1");
 BoUfoRadioButton* radio2 = new BoUfoRadioButton("Radio2");
 buttonGroup->addWidget(radio1);
 buttonGroup->addWidget(radio2);

 mIcon1 = new BoUfoPushButton("Icon Button1");
 connect(mIcon1, SIGNAL(signalClicked()), this, SLOT(slotSetIcon()));
 mContentWidget->addWidget(mIcon1);
 QImage img(100, 100, 32);
 img.fill(Qt::red.rgb());
 BoUfoImage i(img);
 boDebug() << k_funcinfo << "IMAGE: " << i.image() << " BUTTON: " << mIcon1->button() << endl;
 mIcon1->setIcon(i);

 recursive = false;
 initialized = true;
}

void BoUfoButtonTest::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }
}

void BoUfoButtonTest::paintGL()
{
 glClearColor(1, 1, 1, 0);
 glClear(GL_COLOR_BUFFER_BIT);
 glColor3f(1, 0, 0);

 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render(false);
 }
}

bool BoUfoButtonTest::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void BoUfoButtonTest::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoButtonTest::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoButtonTest::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoButtonTest::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoButtonTest::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void BoUfoButtonTest::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
}

void BoUfoButtonTest::slotSetIcon()
{
 // apply a (new) icon to the button.
 // using this, we can find out whether setIcon() leaks memory.
 QImage img(100, 100, 32);
 img.fill(Qt::red.rgb());
 BoUfoImage i(img);
 mIcon1->setIcon(i);
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

 BoUfoButtonTest* main = new BoUfoButtonTest(0);
 app.setMainWidget(main);
 main->show();

 return app.exec();
}

