/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOMESH_H
#define BOMESH_H

#include "../bo3dtools.h"
#include "../global.h"
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
	 * @return TRUE if this is a teamcolor object
	 * and FALSE if it is not a teamcolor object.
	 **/
	bool isTeamColor() const { return mIsTeamColor; }

	/**
	 * @return material()->textureObject()
	 **/
	BoTexture* textureObject() const;

	/**
	 * @return name of the mesh (loaded from file)
	 **/
	const QString& name() const { return mName; }
	void setName(const QString& name) { mName = name; }

	void renderMesh(const BoMatrix* itemMeshMatrix, const BoMatrix* matrix, const QColor* color, RenderFlags flags);

	void renderVertexPoints(const BosonModel* model);

	/**
	 * Render a point for every vertex. The points are not connected
	 * and can therefore be used to see where vertices are, while the
	 * mesh is rendered as usual using @ref renderMesh.
	 **/
	void renderVertexPoints();

	bool useIndices() const { return mUseIndices; }
	void setUseIndices(bool use) { mUseIndices = use; }

	/**
	 * @return The number of points in this mesh.
	 **/
	unsigned int pointCount() const { return mPointCount; }
	void setPointCount(unsigned int c) { mPointCount = c; }
	/**
	 * @return Index of the first point of this mesh in the model's point array
	 **/
	unsigned int pointOffset() const { return mPointOffset; }
	void setPointOffset(unsigned int o) { mPointOffset = o; }

	/**
	 * @return The number of indices in this mesh.
	 **/
	unsigned int indexCount() const { return mIndexCount; }
	void setIndexCount(unsigned int c) { mIndexCount = c; }

	unsigned char* indices() const { return mIndices; }
	void setIndices(unsigned char* i) { mIndices = i; }

	/**
	 * Rendering mode that should be used to render this mesh.
	 * Usually it's GL_TRIANGLES
	 **/
	GLenum renderMode() const { return mRenderMode; }
	void setRenderMode(GLenum m) { mRenderMode = m; }

	const BoVector3Float& boundingBoxMin() const { return mMinCoord; }
	const BoVector3Float& boundingBoxMax() const { return mMaxCoord; }
	void setBoundingBox(const BoVector3Float& min, const BoVector3Float& max) { mMinCoord = min; mMaxCoord = max; }


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
	unsigned int mIndexCount;
	unsigned char* mIndices;
	bool mUseIndices;
	GLenum mRenderMode;

	BoVector3Float mMinCoord;
	BoVector3Float mMaxCoord;

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
