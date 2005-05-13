/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonufoglwidget.h"
#include "bosonufoglwidget.moc"

#include "boufo/boufo.h"
#include "bodebug.h"
#include "bosonprofiling.h"

BosonUfoGLWidget::BosonUfoGLWidget(QWidget* parent, const char* name, bool direct)
	: BosonGLWidget(parent, name, direct)
{
 mUfoManager = 0;
 mSendEvents = true;
}

BosonUfoGLWidget::~BosonUfoGLWidget()
{
 boDebug() << k_funcinfo << endl;
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
}

void BosonUfoGLWidget::makeCurrent()
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("makeCurrent()");
 BosonGLWidget::makeCurrent();
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
	mUfoManager->sendResizeEvent(width(), height());

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
	mUfoManager->sendMousePressEvent(e);
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
	mUfoManager->sendMouseReleaseEvent(e);
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
	mUfoManager->sendMouseMoveEvent(e);
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
	mUfoManager->sendWheelEvent(e);
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
	mUfoManager->sendKeyPressEvent(e);
	update();
 }
 BosonGLWidget::keyPressEvent(e);
 boProfiling->pop(); // "keyPressEvent()"
 boProfiling->popStorage();
}

void BosonUfoGLWidget::keyReleaseEvent(QKeyEvent* e)
{
 boProfiling->pushStorage("BoUfo");
 boProfiling->push("keyReleaseEvent()");
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendKeyReleaseEvent(e);
	update();
 }
 BosonGLWidget::keyReleaseEvent(e);
 boProfiling->pop(); // "keyReleaseEvent()"
 boProfiling->popStorage();
}

