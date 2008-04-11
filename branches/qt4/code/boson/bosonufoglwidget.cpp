/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonufoglwidget.h"
#include "bosonufoglwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "boufo/boufomanager.h"
#include "boufo/boufowidget.h"
#include "boufo/boufoprofiling.h"
#include "bodebug.h"
#include "bosonprofiling.h"

#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

class BoUfoRealProfiling : public BoUfoProfiling
{
public:
	BoUfoRealProfiling()
	{
	}
	void push(const QString& name)
	{
		boProfiling->push(name);
	}
	void pop()
	{
		boProfiling->pop();
	}
};

BosonUfoGLWidget::BosonUfoGLWidget(QWidget* parent, bool direct)
	: QGLWidget(QGLFormat(direct ? QGL::DirectRendering : QGL::IndirectRendering), parent)
{
 mUfoManager = 0;
 mSendEvents = true;

 qApp->installEventFilter(this);
}

BosonUfoGLWidget::~BosonUfoGLWidget()
{
 boDebug() << k_funcinfo << endl;
 qApp->removeEventFilter(this);

 delete mUfoManager;
 mUfoManager = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonUfoGLWidget::initUfo()
{
 if (mUfoManager) {
	boError() << k_funcinfo << "UFO manager not NULL" << endl;
	return;
 }

 makeCurrent();

 boProfiling->pushStorage("BoUfo");
 boProfiling->push("Create BoUfo Manager");
 mUfoManager = new BoUfoManager(width(), height());
 boProfiling->pop(); // "Create BoUfo Manager"
 boProfiling->popStorage();

 BoUfoProfiling::setProfiling(new BoUfoRealProfiling());
}

bool BosonUfoGLWidget::eventFilter(QObject* o, QEvent* e)
{
 if (!ufoManager()) {
	return QGLWidget::eventFilter(o, e);
 }
 switch (e->type()) {
	case QEvent::AccelOverride:
		// AB: Qt first sends an AccelOverride event, then an Accel
		//     event and then a KeyPress event. If the Accel event is
		//     accepted (which may happen in our KAccel objects), then
		//     the KeyPress event is skipped.
		//     If a widget accepts the AccelOverride event, then the
		//     Accel event is skipped and a KeyPress event is sent only.
		//     Therefore we must accept the AccelOverride event here, if
		//     a widget that takes key events has the keyboard focus.
		if (ufoManager()->focusedWidgetTakesKeyEvents()) {
			QKeyEvent* ke = (QKeyEvent*)e;
			ke->accept();
			return true;
		}
		break;
	default:
		break;
 }
 return QGLWidget::eventFilter(o, e);
}

void BosonUfoGLWidget::makeCurrent()
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("makeCurrent()");
 QGLWidget::makeCurrent();
 if (mUfoManager) {
	mUfoManager->makeContextCurrent();
 }
 boProfiling->pop(); // "makeCurrent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::resizeGL(int , int )
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("resizeGL()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();

	// WARNING: FIXME: we use size() as oldsize !!
	// (doesn't harm atm, as BoUfo does not use oldsize)
	QResizeEvent r(size(), size());
	mUfoManager->sendEvent(&r);

	// AB: is this necessary? if so then it should be in sendResizeEvent() !
	mUfoManager->contentWidget()->invalidate();
	update();
 }
 boProfiling->pop(); // "resizeGL()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::mousePressEvent(QMouseEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("mousePressEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 boProfiling->pop(); // "mousePressEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::mouseReleaseEvent(QMouseEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("mouseReleaseEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 boProfiling->pop(); // "mouseReleaseEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::mouseMoveEvent(QMouseEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("mouseMoveEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 boProfiling->pop(); // "mouseMoveEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::wheelEvent(QWheelEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("wheelEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 boProfiling->pop(); // "wheelEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::keyPressEvent(QKeyEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("keyPressEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 QGLWidget::keyPressEvent(e);
 boProfiling->pop(); // "keyPressEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::keyReleaseEvent(QKeyEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("keyReleaseEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendEvent(e);
	update();
 }
 QGLWidget::keyReleaseEvent(e);
 boProfiling->pop(); // "keyReleaseEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::paintEvent(QPaintEvent*)
{
 // nothing to do: we update the GL widget using a timer.
 // (we reimplement this method to prevent QGLWidget from calling paintGL())

}

