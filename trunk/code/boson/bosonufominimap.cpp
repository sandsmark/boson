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

#include <ufo/ufo.hpp>
#include "bosonufominimap.h"
#include "bosonufominimap.moc"

#include "bosonglminimap.h"
#include "botexture.h"
#include "bosonmap.h"

#include <bodebug.h>

#include <ufo/events/umouseevent.hpp>

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

 d->mUfoMiniMap = new UfoMiniMap();
 setBackground(d->mUfoMiniMap);

 connect(this, SIGNAL(signalMouseEntered(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseExited(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseMoved(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseClicked(ufo::UMouseEvent*)),
		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
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
 widget()->setSize(150, 150);
// setSize(d->mGLMiniMap->miniMapWidth(), d->mGLMiniMap->miniMapHeight());
}

void BosonUfoMiniMap::slotMouseEvent(ufo::UMouseEvent* e)
{
 QPoint pos(e->getX(), e->getY());
 QPoint cell = widgetToCell(pos);

 // FIXME: maybe we can do this using this->setEnabled(false/true) ?
 if (!d->mGLMiniMap->showMiniMap()) {
	return;
 }

 switch (e->getType()) {
	case ufo::UMouseEvent::MousePressed:
	{
		if (e->getButton() == ufo::UMod::LeftButton) {
			d->mGLMiniMap->emitSignalReCenterView(cell);
		} else if (e->getButton() == ufo::UMod::RightButton) {
			d->mGLMiniMap->emitSignalMoveSelection(cell);
		}
		break;
	}
	case ufo::UMouseEvent::MouseReleased:
		break;
	case ufo::UMouseEvent::MouseMoved:
		break;
	case ufo::UMouseEvent::MouseClicked:
		break;
	case ufo::UMouseEvent::MouseEntered:
		break;
	case ufo::UMouseEvent::MouseExited:
		break;
	case ufo::UMouseEvent::MouseDragged:
		break;
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

