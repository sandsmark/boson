/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOMESH_H
#define BOMESH_H

#include "bo3dtools.h"
#include <bogl.h>

#include <qstring.h>

class BoMesh;
class BoMaterial;
class BoMeshRendererMeshData;
class BosonModel;
class QColor;
class BoTexture;



class BoMeshPrivate;
class BoMesh
{
public:
	BoMesh();
	~BoMesh();

	/**
	 * The size of a single points (vertex and texel). Size means
	 * the number of floats here.
	 **/
	static int pointSize();

	/**
	 * @return The position of the vertex in a point (see @ref pointSize)
	 */
	static int vertexPos();

	static int normalPos();

	static int texelPos();

	/**
	 * Use material @p mat when rendering this mesh.
	 **/
	void setMaterial(BoMaterial* mat) { mMaterial = mat; }

	BoMaterial* material() const { return mMaterial; }

	/**
	 * Set whether this mesh is a teamcolor object or not.
	 **/
	void setIsTeamColor(bool teamColor) { mIsTeamColor = teamColor; }

	/**
	 * @return TRUE if this is a teamcolor object (which also is not textured)
	 * and FALSE if it is not a teamcolor object.
	 **/
	bool isTeamColor() const { return mIsTeamColor; }

	/**
	 * @return material()->textureObject() if @ref material is non-null,
	 * otherwise 0.
	 **/
	BoTexture* textureObject() const;

	/**
	 * @return name of the mesh (loaded from file)
	 **/
	const QString& name() const { return mName; }
	void setName(const QString& name) { mName = name; }

	void renderMesh(const BoMatrix* matrix, const QColor* color);

	void renderVertexPoints(const BosonModel* model);

	/**
	 * Render a point for every vertex. The points are not connected
	 * and can therefore be used to see where vertices are, while the
	 * mesh is rendered as usual using @ref renderMesh.
	 **/
	void renderVertexPoints();

	/**
	 * @return The number of points in this mesh. See also @ref facesCount
	 **/
	unsigned int pointCount() const { return mPointCount; }
	void setPointCount(unsigned int c) { mPointCount = c; }
	/**
	 * @return Index of the first point of this mesh in the model's point array
	 **/
	unsigned int pointOffset() const { return mPointOffset; }
	void setPointOffset(unsigned int o) { mPointOffset = o; }

	/**
	 * Rendering mode that should be used to render this mesh.
	 * Usually it's GL_TRIANGLES
	 **/
	GLenum renderMode() const { return mRenderMode; }
	void setRenderMode(GLenum m) { mRenderMode = m; }

	inline BoMeshRendererMeshData* meshRendererMeshData() const
	{
		return mMeshRendererMeshData;
	}

	void setMeshRendererMeshData(BoMeshRendererMeshData* data);

private:
	void init();
	friend class BoMeshRenderer;

private:
	bool mIsTeamColor;
	BoMaterial* mMaterial;

	QString mName;

	unsigned int mPointCount;
	unsigned int mPointOffset;
	GLenum mRenderMode;

	BoMeshRendererMeshData* mMeshRendererMeshData;
};


/**
 * Same as @ref BoMeshRendererModelData, but this stores data for the @ref
 * BoMesh.
 *
 * See also @ref BoMesh::meshRendererMeshData and @ref
 * BoMeshRenderer::initMeshData.
 *
 * @short Simple storage class for @ref BoMesh and @ref BoMeshRenderer
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshRendererMeshData
{
public:
	BoMeshRendererMeshData()
	{
	}
	virtual ~BoMeshRendererMeshData()
	{
	}
};

#endif
