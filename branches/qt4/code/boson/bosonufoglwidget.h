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
#ifndef BOSONUFOGLWIDGET_H
#define BOSONUFOGLWIDGET_H

#include <bogl.h>
#include <QGLWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QWheelEvent>

class BoUfoManager;

/**
 * @short A @ref QGLWidget with support for @ref BoUfoManager
 *
 * This widget is just a @ref QGLWidget that provides one @ref BoUfoManager
 * object. You must call @ref initUfo in your initializeGL() method in order to
 * use it.
 *
 * For convenience all event method have been implemented to call the proper
 * even method in the @ref BoUfoManager. Remember to call them in your version
 * (especially @ref resizeGL which does nothing in @ref QGLWidget).
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoGLWidget : public QGLWidget
{
	Q_OBJECT
public:
	BosonUfoGLWidget(QWidget* parent = 0, bool direct = true);
	~BosonUfoGLWidget();

	BoUfoManager* ufoManager() const { return mUfoManager; }

	virtual void resizeGL(int, int);
	virtual void makeCurrent();

	/**
	 * If enabled, the events to this widget (see @ref mouseMoveEvent, @ref
	 * mousePressEvent, ...) are sent to the @ref ufoManager
	 * automatically.
	 *
	 * If @p send is FALSE, this feature is disabled.
	 *
	 * By default events are sent , i.e. @p send is TRUE.
	 **/
	void setSendEventsToUfo(bool send)
	{
		mSendEvents = send;
	}

protected slots:
	// TODO: remove. just a wrapper to make port to QGLWidget easier.
	void slotUpdateGL() { updateGL(); }


protected:
	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);

	virtual void paintEvent(QPaintEvent*);

protected:
	void initUfo();

private:
	BoUfoManager* mUfoManager;
	bool mSendEvents;
};


#endif

