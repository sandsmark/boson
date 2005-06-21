/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonufominimap.h"
#include "bosonufominimap.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonglminimap.h"
#include "botexture.h"
#include "bosonmap.h"
#include "bosonprofiling.h"

#include <bodebug.h>

class UfoMiniMap : public BoUfoDrawable
{
public:
	UfoMiniMap() : BoUfoDrawable()
	{
		mMiniMap = 0;
	}

	void setMiniMap(BosonGLMiniMap* m)
	{
		mMiniMap = m;
	}

	virtual void render(int x, int y, int w, int h)
	{
		if (!mMiniMap) {
			return;
		}
		PROFILE_METHOD
		boTextureManager->invalidateCache();
		glPushMatrix();
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, -drawableHeight(), 0.0f);
		mMiniMap->renderMiniMap();
		glPopMatrix();
		boTextureManager->invalidateCache();
	}

	virtual int drawableWidth() const
	{
		if (mMiniMap) {
			return mMiniMap->miniMapWidth();
		}
		return 0;
	}
	virtual int drawableHeight() const
	{
		if (mMiniMap) {
			return mMiniMap->miniMapHeight();
		}
		return 0;
	}

private:
	BosonGLMiniMap* mMiniMap;
};

class BosonUfoMiniMapPrivate
{
public:
	BosonUfoMiniMapPrivate()
	{
		mGLMiniMap = 0;
		mUfoMiniMap = 0;
	}
	BosonGLMiniMap* mGLMiniMap;

	UfoMiniMap* mUfoMiniMap;
};

BosonUfoMiniMap::BosonUfoMiniMap() : BoUfoWidget()
{
 d = new BosonUfoMiniMapPrivate;
 setOpaque(true);

 setMouseEventsEnabled(true, true);

 d->mUfoMiniMap = new UfoMiniMap();
 setBackground(d->mUfoMiniMap);

 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
// connect(this, SIGNAL(signalMouseDragged(ufo::UMouseEvent*)),
//		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
// connect(this, SIGNAL(signalMouseClicked(ufo::UMouseEvent*)),
//		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
}

BosonUfoMiniMap::~BosonUfoMiniMap()
{
 boDebug() << k_funcinfo << endl;
 delete d->mUfoMiniMap;
 delete d;
}

void BosonUfoMiniMap::setMiniMap(BosonGLMiniMap* m)
{
 d->mGLMiniMap = m;
 d->mUfoMiniMap->setMiniMap(d->mGLMiniMap);

 // TODO: call these when the map changes
 setPreferredWidth(150);
 setPreferredHeight(150);
// setPreferredWidth(d->mGLMiniMap->miniMapWidth());
// setPreferredHeight(d->mGLMiniMap->miniMapHeight());
 setMinimumWidth(150);
 setMinimumHeight(150);
// setMinimumWidth(d->mGLMiniMap->miniMapWidth());
// setMinimumHeight(d->mGLMiniMap->miniMapHeight());

 // the above seem to be noops. this is the important call.
 setSize(150, 150);
// setSize(d->mGLMiniMap->miniMapWidth(), d->mGLMiniMap->miniMapHeight());
}

void BosonUfoMiniMap::slotMouseEvent(QMouseEvent* e)
{
 QPoint pos = e->pos();
 QPoint cell = widgetToCell(pos);

 // we accept all mouse events except mousemove events. this means that only
 // mouse move events are propagated to the parent (necessary for updating
 // cursor position)
 switch (e->type()) {
	case QMouseEvent::MouseMove:
		e->ignore();
		break;
	default:
		e->accept();
		break;
 }

 // FIXME: maybe we can do this using this->setEnabled(false/true) ?
 if (!d->mGLMiniMap->showMiniMap()) {
	return;
 }

 switch (e->type()) {
	case QMouseEvent::MouseButtonPress:
	{
		if (e->button() == Qt::LeftButton) {
			d->mGLMiniMap->emitSignalReCenterView(cell);
		} else if (e->button() == Qt::RightButton) {
			d->mGLMiniMap->emitSignalMoveSelection(cell);
		}
		break;
	}
	case QMouseEvent::MouseButtonRelease:
		break;
	case QMouseEvent::MouseMove:
		break;
//	case QMouseEvent::MouseClicked:
//		break;
//	case QMouseEvent::MouseDragged:
//		break;
	default:
		break;
 }
}

QPoint BosonUfoMiniMap::widgetToCell(const QPoint& pos)
{
 // TODO: the correct cell depends on the modelview matrix of the minimap (zooming, ...). we should use something like d->mGLMiniMap->widgetToCell(), which would take the mModelView of the minimap into account.
 if (!d->mGLMiniMap) {
	return QPoint();
 }
 BosonMap* map = d->mGLMiniMap->map();
 if (!map) {
	return QPoint();
 }
 if (d->mGLMiniMap->miniMapWidth() == 0 || d->mGLMiniMap->miniMapHeight() == 0) {
	return QPoint();
 }
 QPoint cell = QPoint((pos.x() * map->width()) / d->mGLMiniMap->miniMapWidth(),
		(pos.y() * map->height()) / d->mGLMiniMap->miniMapHeight());
 return cell;
}

