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

 mUfoManager = new BoUfoManager(width(), height());
}

void BosonUfoGLWidget::makeCurrent()
{
 BosonGLWidget::makeCurrent();
 if (mUfoManager) {
	mUfoManager->makeContextCurrent();
 }
}

void BosonUfoGLWidget::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	mUfoManager->postResizeEvent(width(), height());

	// AB: is this necessary? if so then it should be in postResizeEvent() !
	mUfoManager->contentWidget()->invalidate();
 }
 update();
}

void BosonUfoGLWidget::mousePressEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postMousePressEvent(e);
 }
 update();
}

void BosonUfoGLWidget::mouseReleaseEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postMouseReleaseEvent(e);
 }
 update();
}

void BosonUfoGLWidget::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postMouseMoveEvent(e);
 }
 update();
}

void BosonUfoGLWidget::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postWheelEvent(e);
 }
 update();
}

void BosonUfoGLWidget::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postKeyPressEvent(e);
 }
// if (puKeyboard(e->ascii(), PU_DOWN)) {
//	e->accept();
// else {
	BosonGLWidget::keyPressEvent(e);
//
 update();
}

void BosonUfoGLWidget::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	makeCurrent();
	mUfoManager->postKeyReleaseEvent(e);
 }
// if (puKeyboard(e->ascii(), PU_UP)) {
//	e->accept();
// } else {
	BosonGLWidget::keyReleaseEvent(e);
// }
 update();
}

