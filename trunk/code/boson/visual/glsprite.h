/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef GLSPRITE_H
#define GLSPRITE_H

#include "../defines.h"
#include "../bosonmodel.h"

#include <GL/gl.h>

class BosonCanvas;
class QRect;

class GLSprite
{
public:
	GLSprite(BosonModel* mode, BosonCanvas*);
	virtual ~GLSprite();

	virtual void setCanvas(BosonCanvas*);

	BosonCanvas* canvas() const { return mCanvas; }
	inline float x() const { return mX; }
	inline float y() const { return mY; }
	inline float z() const { return mZ; }
	inline void setX(float x)
	{
		mX = x;
	}
	inline void setY(float y)
	{
		mY = y;
	}
	inline void setZ(float z)
	{
		mZ = z;
	}

	inline BosonModel* model() const { return mModel; }

	// note: for GLunit all frames must have the same width/height
	inline int width() const { return model() ? model()->width(frame()) : 0; }
	inline int height() const { return model() ? model()->height(frame()) : 0; }

	// AB: we don't support a hotspot here!
	float leftEdge() const { return x(); }
	float topEdge() const { return y(); } // note: in OpenGL the top is y-coordinate + height!!
	float rightEdge() const { return (leftEdge() + width() - 1); }
	float bottomEdge() const { return (topEdge() + height() - 1); }
	QRect boundingRect() const; 
	QRect boundingRectAdvanced() const; 

	void show() { setVisible(true); }
	void hide() { setVisible(true); }
	void setVisible(bool v);
	bool isVisible() const { return mIsVisible; }
	virtual void setAnimated(bool a) = 0;
	
	void move(float x, float y) { move(x, y, 0.0); }
	void move(float nx, float ny, float nz) { moveBy(nx - x(), ny - y(), nz - z()); }
	void moveBy(float dx, float dy) { moveBy(dx, dy, 0.0); }
	virtual void moveBy(float dx, float dy, float dz)
	{
		// in Unit we MUST add a addToCells() / removeFromCells() !!
		if (dx || dy || dz) {
			setX(x() + dx);
			setY(y() + dy);
			setZ(z() + dz);
		}
	}
	void move(float x, float y, float z, int _frame)
	{
		if (isVisible()) {
			setVisible(false);
			move(x, y, z);
			if (_frame != frame()) {
				model()->setFrame(_frame);
			}
			setVisible(true);
		} else {
			move(x, y, z);
			if (_frame != frame()) {
				model()->setFrame(_frame);
			}
		}
	}

	int frame() const { return model()->frame(); }
	void setFrame(int frame) { move(x(), y(), z(), frame); }
	unsigned int frameCount() const { return model() ? model()->frames() : 0; }

	float xVelocity() const { return mXVelocity; }
	float yVelocity() const { return mYVelocity; }
	void setXVelocity(float vx) { mXVelocity = vx; }
	void setYVelocity(float vy) { mYVelocity = vy; }
	void setVelocity(float vx, float vy) 
	{
		mXVelocity = vx;
		mYVelocity = vy;
	}

	inline GLuint displayList() const 
	{
		return model()->displayList();
	}

private:
	// FIXME: use KGameProperty here. We can do so, since we don't use
	// QCanvasSprite anymore.
	BosonCanvas* mCanvas;
	float mX;
	float mY;
	float mZ;
	bool mIsVisible;
	float mXVelocity;
	float mYVelocity;
	BosonModel* mModel;
};

#endif // GLUNIT_H

