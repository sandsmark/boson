/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSOOGLVIEW_H
#define BOSOOGLVIEW_H

//#include <bogl.h>
//#include <QGLWidget>
#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsView;
class QGLFormat;
class MyGLWidget;
class BosonFPSCounter;

// provides
// * a QGraphicsView
// * a QGraphicsScene
// * a QGLWidget
// * a FPS counter
// * checkIfGLDriverIsBroken()
// * BoGL initializing
// -> and of course code to couple these together
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGLView : public QGraphicsView
{
	Q_OBJECT
public:
	BosonGLView(QWidget* parent, bool directRendering = true);
	~BosonGLView();

	bool initializeGL();

	BosonFPSCounter* fpsCounter() const
	{
		return mFPSCounter;
	}

	QGLFormat format() const;

	void addWidgetToScene(QWidget* widget, qreal zValue);

	/**
	 * This method checks if the OpenGL driver is believed to be broken by
	 * any reason. This is in particular the case when NVidia drivers are
	 * used, but a MESA libGL.so is picked up.
	 *
	 * Note that this method CANNOT be 100% accurate: it may report a GL
	 * driver that is believed to be working, although it is still broken.
	 * However if a broken driver is reported, that usually is accurate.
	 *
	 * @return A i18n'ed error message describing the problem if there is
	 * any, otherwise a null string.
	 **/
	QString checkIfGLDriverIsBroken();

public slots:
	void slotRenderFrame();

private:
	bool mIsInitialized;
	QGraphicsScene* mScene;
	MyGLWidget* mGLWidget;
	BosonFPSCounter* mFPSCounter;
};


#endif

