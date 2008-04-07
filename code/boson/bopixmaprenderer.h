//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
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

#ifndef BOPIXMAPRENDERER_H
#define BOPIXMAPRENDERER_H

class QGLWidget;

class QString;
template<class T> class Q3ValueList;
class QPixmap;

class BoPixmapRendererPrivate;
class BoPixmapRenderer
{
public:
	BoPixmapRenderer();
	~BoPixmapRenderer();

	void setWidget(QGLWidget* w, int width = -1, int height = -1);

	/**
	 * Uses @ref startPixmap, @ref BosonGLWidget::slotUpdateGL and @ref
	 * pixmapDone to retrieve a pixmap.
	 **/
	QPixmap getPixmap(bool store = false);

	void startPixmap();

	/**
	 * Finalize the pixmap. The pixmap is returned and stored in an internal
	 * list, see @ref frame and @ref flush.
	 **/
	QPixmap pixmapDone(bool store = true);

	unsigned int frameCount() const;
	QPixmap frame(unsigned int index) const;

	/**
	 * Write all pixmaps (frames) in memory to the disk, to files named
	 * fileNamePrefix-number.jpg
	 **/
	void flush(const QString& fileNamePrefix);

private:
	BoPixmapRendererPrivate* d;
	QGLWidget* mGLWidget;
};

#endif

