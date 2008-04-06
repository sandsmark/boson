/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

//#include <ufo/ufo.hpp>

#include "boufowidgets.h"
#include "boufowidgets.moc"

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

#define GL_FORMAT_OPTIONS (0)
//#define GL_FORMAT_OPTIONS (0 | QGL::IndirectRendering)

BoUfoWidgets::BoUfoWidgets(QWidget* parent, const char* name)
	: QGLWidget(QGLFormat(GL_FORMAT_OPTIONS), parent, name, 0, Qt::WType_TopLevel | Qt::WDestructiveClose)
{
 setMouseTracking(true);

 mUfoManager = 0;

 QTimer* updateTimer = new QTimer(this);
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 updateTimer->start(100);
 setUpdatesEnabled(false);
}

BoUfoWidgets::~BoUfoWidgets()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
}

void BoUfoWidgets::initializeGL()
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

 addBoUfoWidget(new BoUfoPushButton(tr("Button")), tr("BoUfoPushButton"));
 addBoUfoWidget(new BoUfoLabel(tr("Label")), tr("BoUfoLabel"));
 addBoUfoWidget(new BoUfoCheckBox(tr("Checkbox")), tr("BoUfoCheckBox"));
 addBoUfoWidget(new BoUfoRadioButton(tr("Radio button")), tr("BoUfoRadioButton"));
 addBoUfoWidget(new BoUfoNumInput(), tr("BoUfoNumInput"));

 addProgressWidgets(false);
 addProgressWidgets(true);


 recursive = false;
 initialized = true;
}

void BoUfoWidgets::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }
}

void BoUfoWidgets::paintGL()
{
 glClearColor(1, 1, 1, 0);
 glClear(GL_COLOR_BUFFER_BIT);
 glColor3f(1, 0, 0);

 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render(false);
 }
}

bool BoUfoWidgets::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void BoUfoWidgets::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoWidgets::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoWidgets::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoWidgets::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void BoUfoWidgets::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void BoUfoWidgets::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
}

void BoUfoWidgets::addBoUfoWidget(BoUfoWidget* w, const QString& label)
{
#if 1
 BoUfoHBox* hbox = new BoUfoHBox();
 mContentWidget->addWidget(hbox);

 BoUfoVBox* vbox = new BoUfoVBox();
 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 hbox->addWidget(vbox);
 hbox->addWidget(stretch);

 BoUfoLabel* l = new BoUfoLabel(label);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);

 vbox->addWidget(l);
 vbox->addWidget(w);
 vbox->addWidget(stretch);
#else
 mContentWidget->addWidget(w);
#endif
}

void BoUfoWidgets::addProgressWidgets(bool extended)
{
 QString name;
 if (extended) {
	name = "BoUfoExtendedProgress";
 } else {
	name = "BoUfoProgress";
 }
 BoUfoProgress* p = 0;
 p = createProgress(Qt::Horizontal, extended);
 p->setValue(100.0);
 addBoUfoWidget(p, tr("%1 (horizontal, 100%)").arg(name));
 p = createProgress(Qt::Horizontal, extended);
 p->setValue(90.0);
 addBoUfoWidget(p, tr("%1 (horizontal, 90%)").arg(name));
 p = createProgress(Qt::Horizontal, extended);
 p->setValue(50.0);
 addBoUfoWidget(p, tr("%1 (horizontal, 50%)").arg(name));
 p = createProgress(Qt::Horizontal, extended);
 p->setValue(10.0);
 addBoUfoWidget(p, tr("%1 (horizontal, 10%)").arg(name));
 p = createProgress(Qt::Horizontal, extended);
 p->setValue(0.0);
 addBoUfoWidget(p, tr("%1 (horizontal, 0%)").arg(name));

 p = createProgress(Qt::Vertical, extended);
 p->setValue(100.0);
 addBoUfoWidget(p, tr("%1 (vertical, 100%)").arg(name));
 p = createProgress(Qt::Vertical, extended);
 p->setValue(90.0);
 addBoUfoWidget(p, tr("%1 (vertical, 90%)").arg(name));
 p = createProgress(Qt::Vertical, extended);
 p->setValue(50.0);
 addBoUfoWidget(p, tr("%1 (vertical, 50%)").arg(name));
 p = createProgress(Qt::Vertical, extended);
 p->setValue(10.0);
 addBoUfoWidget(p, tr("%1 (vertical, 10%)").arg(name));
 p = createProgress(Qt::Vertical, extended);
 p->setValue(0.0);
 addBoUfoWidget(p, tr("%1 (vertical, 0%)").arg(name));
}

BoUfoProgress* BoUfoWidgets::createProgress(Qt::Orientation o, bool extended)
{
 BoUfoProgress* p;
 if (extended) {
	BoUfoExtendedProgress* p1;
	p1 = new BoUfoExtendedProgress(o);
	p1->setStartExtensionSizeFactor(0.1); // 10% of the bar
	p1->setEndExtensionSizeFactor(0.2); // 20% of the bar

	p1->setStartExtensionValueRange(10);
	p1->setEndExtensionValueRange(20);
	p = p1;
 } else {
	p = new BoUfoProgress(o);
 }
 p->setFrameColor(Qt::black);
 return p;
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

 BoUfoWidgets* main = new BoUfoWidgets(0);
 app.setMainWidget(main);
 main->show();

 return app.exec();
}

