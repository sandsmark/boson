/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bopixmaprenderer.h"

#include "../bomemory/bodummymemory.h"
#include "botexture.h"
#include "bosonglwidget.h"
#include <bodebug.h>

#include <qpixmap.h>
#include <q3valuelist.h>

class BoPixmapRendererPrivate
{
public:
	BoPixmapRendererPrivate()
	{
		mContext = 0;
		mOldContext = 0;
	}
	Q3ValueList<QPixmap> mPixmaps;
	QPixmap mPixmap;
	BoContext* mContext;
	BoContext* mOldContext;
};

BoPixmapRenderer::BoPixmapRenderer()
{
 d = new BoPixmapRendererPrivate;
 mGLWidget = 0;
}

void BoPixmapRenderer::setWidget(BosonGLWidget* w, int width, int height)
{
 if (mGLWidget) {
	boError() << k_funcinfo << "already a widget set" << endl;
	return;
 }
 mGLWidget = w;

 if (width < 0 || height < 0) {
	width = mGLWidget->width();
	height = mGLWidget->height();
 }
 d->mPixmap.resize(width, height);

 d->mOldContext = mGLWidget->context();
 if (d->mOldContext) {
	d->mOldContext->doneCurrent();
 }

 if (d->mPixmap.isNull()) {
	boError() << k_funcinfo << "NULL pixmap" << endl;
	return;
 }

 boDebug() << "new context" << endl;
 d->mContext = new BoContext(&d->mPixmap);
 d->mContext->create(false, false); // neither direct, nor double buffered
 boDebug() << "make context current" << endl;
 d->mContext->makeCurrent();
 boDebug() << "context current done" << endl;

 if (boTextureManager) {
	boTextureManager->reloadTextures();
 }

 if (!d->mContext->isValid()) {
	boError() << k_funcinfo << "invalid context" << endl;
	delete d->mContext;
	d->mContext = 0;
 }

 // the new context is a full drop-in replacement of the old context. all
 // context commands in BosonBigDisplayBase will be directed to the new context,
 // without any code changes.
 mGLWidget->switchContext(d->mContext);

 mGLWidget->resize(width, height);
}

BoPixmapRenderer::~BoPixmapRenderer()
{
 if (d->mContext) {
	d->mContext->doneCurrent();
 }
 delete d->mContext;
 if (mGLWidget && d->mOldContext) {
	mGLWidget->switchContext(d->mOldContext);
 }
 delete d;
}

QPixmap BoPixmapRenderer::getPixmap(bool store)
{
 if (!mGLWidget) {
	BO_NULL_ERROR(mGLWidget);
	return QPixmap();
 }
 startPixmap();
 mGLWidget->slotUpdateGL();
 return pixmapDone(store);
}

void BoPixmapRenderer::startPixmap()
{
 BO_CHECK_NULL_RET(d->mContext);
 d->mContext->makeCurrent();
}

QPixmap BoPixmapRenderer::pixmapDone(bool store)
{
 if (!mGLWidget || !d->mContext) {
	return QPixmap();
 }
 QPixmap p = d->mPixmap;
 // manually do this, as the GL methods won't do so!
 p.detach();

 if (store) {
	d->mPixmaps.append(p);
 }
 return p;
}

void BoPixmapRenderer::flush(const QString& prefix)
{
 boDebug() << k_funcinfo << "flushing " << d->mPixmaps.count() << " pixmaps. filename prefix " << prefix << endl;
 int i = 0;
 for (Q3ValueList<QPixmap>::iterator it = d->mPixmaps.begin(); it != d->mPixmaps.end(); ++it) {
	// TODO: use 001, 002, ..., 999 instead of 1, 2, .., 999
	QString file = prefix + QString("-%1.jpg").arg(i);
	if (!(*it).save(file, "JPEG", 90)) {
		boError() << k_funcinfo << "error saving to " << file << endl;
	}
 }
 d->mPixmaps.clear();
}

unsigned int BoPixmapRenderer::frameCount() const
{
 return d->mPixmaps.count();
}

QPixmap BoPixmapRenderer::frame(unsigned int i) const
{
 if (i >= d->mPixmaps.count()) {
	return QPixmap();
 }
 return d->mPixmaps[i];
}

