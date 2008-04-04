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
#include "bounitdecorationrenderer.h"

#include "../gameengine/unit.h"
#include "../gameengine/rtti.h"
#include "../gameengine/bosonshot.h"
#include "../bocamera.h"
#include <bogl.h>
#include <bodebug.h>

class BoUnitDecorationRendererPrivate
{
public:
	BoUnitDecorationRendererPrivate()
	{
		mCamera = 0;
		mBackend = 0;
	}
	BoGameCamera* mCamera;

	BoUnitDecorationRendererBackend* mBackend;
};

BoUnitDecorationRenderer::BoUnitDecorationRenderer()
{
 d = new BoUnitDecorationRendererPrivate();

 setBackend(BackendDefault);
}

BoUnitDecorationRenderer::~BoUnitDecorationRenderer()
{
 delete d->mBackend;
 delete d;
}

void BoUnitDecorationRenderer::setCamera(BoGameCamera* camera)
{
 d->mCamera = camera;
}

BoGameCamera* BoUnitDecorationRenderer::camera() const
{
 return d->mCamera;
}

void BoUnitDecorationRenderer::setBackend(Backend backend)
{
 BoUnitDecorationRendererBackend* newBackend = 0;
 switch (backend) {
	case BackendDefault:
		newBackend = new BoUnitDecorationRendererBackendDefault();
		break;
	case BackendDebug:
		newBackend = new BoUnitDecorationRendererBackendDebug();
		break;
 }
 if (newBackend) {
	delete d->mBackend;
	d->mBackend = newBackend;

#if 0
	d->mBackend->setCamera(camera());
#endif
 }

 if (!d->mBackend) {
	boWarning() << k_funcinfo << "NULL backend. setting default backend" << endl;
	d->mBackend = new BoUnitDecorationRendererBackendDefault();
 }
}

void BoUnitDecorationRenderer::renderDecoration(BosonItem* item)
{
 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(d->mBackend);
 float x = (item->centerX());
 float y = -(item->centerY());
 float z = item->z();
 float w = (float)item->width();
 float h = (float)item->height();
 float depth = item->depth();

 glPushMatrix();
 glTranslatef(x, y, z);
 glScalef(w, h, depth);
 glRotatef(camera()->rotation(), 0.0f, 0.0f, 1.0f);

 if (RTTI::isUnit(item->rtti())) {
	d->mBackend->renderUnitDecoration((Unit*)item);
 } else if (RTTI::isShot(item->rtti())) {
	d->mBackend->renderShotDecoration((BosonShot*)item);
 }

 glPopMatrix();
}


void BoUnitDecorationRendererBackendDebug::renderUnitDecoration(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
}

void BoUnitDecorationRendererBackendDebug::renderShotDecoration(BosonShot* shot)
{
 BO_CHECK_NULL_RET(shot);
}

void BoUnitDecorationRendererBackendDefault::renderUnitDecoration(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
}

void BoUnitDecorationRendererBackendDefault::renderShotDecoration(BosonShot* shot)
{
 BO_CHECK_NULL_RET(shot);
}

