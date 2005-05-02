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

BosonUfoGLWidget::BosonUfoGLWidget(QWidget* parent, const char* name, bool direct)
	: BosonGLWidget(parent, name, direct)
{
 mUfoManager = 0;
 mSendEvents = true;
}

BosonUfoGLWidget::~BosonUfoGLWidget()
{
 // FIXME: atm this causes a segfault
// delete mUfoManager;
}

void BosonUfoGLWidget::initUfo()
{
 if (mUfoManager) {
	boError() << k_funcinfo << "UFO manager not NULL" << endl;
	return;
 }

 makeCurrent();
 mUfoManager = new BoUfoManager(width(), height());
}

void BosonUfoGLWidget::makeCurrent()
{
 BosonGLWidget::makeCurrent();
 if (mUfoManager) {
	mUfoManager->makeContextCurrent();
 }
}

void BosonUfoGLWidget::resizeGL(int , int )
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendResizeEvent(width(), height());

	// AB: is this necessary? if so then it should be in sendResizeEvent() !
	mUfoManager->contentWidget()->invalidate();
 }
 update();
}

void BosonUfoGLWidget::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendMousePressEvent(e);
	update();
 }
}

void BosonUfoGLWidget::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendMouseReleaseEvent(e);
	update();
 }
}

void BosonUfoGLWidget::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendMouseMoveEvent(e);
	update();
 }
}

void BosonUfoGLWidget::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendWheelEvent(e);
	update();
 }
}

void BosonUfoGLWidget::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendKeyPressEvent(e);
	update();
 }
 BosonGLWidget::keyPressEvent(e);
}

void BosonUfoGLWidget::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager && mSendEvents) {
	makeCurrent();
	mUfoManager->sendKeyReleaseEvent(e);
	update();
 }
 BosonGLWidget::keyReleaseEvent(e);
}

