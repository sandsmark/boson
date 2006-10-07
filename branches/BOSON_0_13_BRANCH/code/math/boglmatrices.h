/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOGLMATRICES_H
#define BOGLMATRICES_H

#include "math/bomath.h"
#include "math/bofrustum.h"

/**
 * @short A collection of the most important GL matrices
 *
 * Often a class needs read access to the matrices that are currently used - for
 * example the canvas renderer needs them to do frustum culling. This class is a
 * convenience class that provides them, so that we don't have to provide them
 * all separately in the constructor of every class that needs them.
 *
 * Note that this class stores references to the actual matrices, so they always
 * reflect the current values and don't require updates.
 *
 * @author Andreas Beckermann <b_mann@gmx.de
 **/
class BoGLMatrices
{
public:
	BoGLMatrices(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const BoFrustum& viewFrustum, const int* viewport, const float& fovY, const float& aspect);

	const BoMatrix& modelviewMatrix() const { return mModelviewMatrix; }
	const BoMatrix& projectionMatrix() const { return mProjectionMatrix; }
	const BoFrustum& viewFrustum() const { return mViewFrustum; }
	const int* viewport() const { return mViewport; }
	const float& fovY() const { return mFovY; }
	const float& aspect() const { return mAspect; }

private:
	const BoMatrix& mModelviewMatrix;
	const BoMatrix& mProjectionMatrix;
	const BoFrustum& mViewFrustum;
	const int* mViewport;
	const float& mFovY;
	const float& mAspect;
};

#endif
