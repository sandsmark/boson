/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

class BoMesh;
class BoMaterial;
class BoAdjacentDataBase;
class BoMeshLOD;
class BoMeshRendererMeshData;
class BoMeshRendererMeshLODData;
class BosonModel;
class QColor;

template<class T> class QValueVector;

class BoFace
{
public:
	BoFace();
	BoFace(const BoFace& face)
	{
		*this = face;
	}

	BoFace& operator=(const BoFace& face);

	void setPointIndex(const int* points)
	{
		mPointIndex[0] = points[0];
		mPointIndex[1] = points[1];
		mPointIndex[2] = points[2];
	}

	const int* pointIndex() const
	{
		return mPointIndex;
	}

	bool hasPoint(int point) const
	{
		if((mPointIndex[0] == point) || (mPointIndex[1] == point) || (mPointIndex[2] == point)) {
			return true;
		}
		return false;
	}

	/**
	 * Use @þ normal for all vertices in this face
	 **/
	void setAllNormals(const BoVector3Float normal)
	{
		setNormal(0, normal);
		setNormal(1, normal);
		setNormal(2, normal);
	}
	void setNormal(unsigned int i, const BoVector3Float normal)
	{
		// values > 2 are not allowed
		i = i % 3;
		mNormals[i] = normal;
	}
	inline const BoVector3Float& normal(unsigned int i) const
	{
		// we don't do i = i % 3; as of performance reasons
		return mNormals[i];
	}

	void setSmoothGroup(unsigned long int group)
	{
		mSmoothGroup = group;
	}
	unsigned long int smoothGroup() const
	{
		return mSmoothGroup;
	}

private:
	int mPointIndex[3];

	unsigned long int mSmoothGroup;

	BoVector3Float mNormals[3];
};

/**
 * This class stores the way faces are connected. At the moment faces will
 * always be connected in a linear way (i.e. face1,face2,face3,...). But we
 * could also connect them so that we use triangle strips.
 **/
class BoFaceNode
{
public:
	/**
	 * Construct a node. Note that a node is 100% unusable without a valid
	 * @p BoFace object.
	 **/
	BoFaceNode(const BoFace* face);
	~BoFaceNode();

	void setPrevious(BoFaceNode* previous);
	void setNext(BoFaceNode* next);

	BoFaceNode* next() const
	{
		return mNext;
	}
	BoFaceNode* previous() const
	{
		return mPrevious;
	}

	int relevantPoint() const
	{
		return mRelevantPoint;
	}
	void setRelevantPoint(int p)
	{
		mRelevantPoint = p;
	}
	void encodeRelevantPoint(int firstPoint, int secondPoint, int thirdPoint)
	{
		int r = 0;
		r += firstPoint << 4;
		r += secondPoint << 2;
		r += thirdPoint;
		setRelevantPoint(r);
	}

	// the point is encoded for the *first* node only!
	// this will return stupid stuff for all other nodes!
	void decodeRelevantPoint(int* firstPoint, int* secondPoint, int* thirdPoint)
	{
		int r = relevantPoint();
		if (r < 0) {
			// oops!
			*firstPoint = -1;
			*secondPoint = -1;
			*thirdPoint = -1;
			return;
		}
		*firstPoint = r >> 4;
		r -= *firstPoint << 4;
		*secondPoint = r >> 2;
		r -= *secondPoint << 2;
		*thirdPoint = r;
	}

	/**
	 * @return The <em>local</em> index (i.e. 0..2) of the <em>point</em>
	 * index @p index (i.e. the index of the vertex in the vertex list). Or
	 * -1 if @p index is not a point of this node/face.
	 **/
	int findPointIndex(int index) const;

	inline const BoFace* face() const
	{
		return mFace;
	}
	const int* pointIndex() const
	{
		return face()->pointIndex();
	}

	void delNode();

	static bool isAdjacent(BoFaceNode* face1, BoFaceNode* face2);
	QString debugString() const;

private:
	void init();

private:
	BoFaceNode* mNext;
	BoFaceNode* mPrevious;
	int mRelevantPoint;
	const BoFace* mFace;
};


class BoMeshLODPrivate;
/**
 * This class is a collection of all data that depend on the current level of
 * detail (LOD).
 *
 * Note that some values/arrays (such as the point cache) are automatically
 * generated <em>after</em> the LOD has been generated. This happens for all
 * meshes and all LODs, so you don't have to worry about this for LOD. But it
 * also depends on the current LOD, so it is in this class.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshLOD
{
public:
	BoMeshLOD();
	~BoMeshLOD();

	void createFaces(unsigned int faces);

	inline BoFaceNode* nodes() const
	{
		return mNodes;
	}
	inline int type() const
	{
		return mType;
	}
	inline unsigned int* pointsCache() const
	{
		return mPointsCache;
	}
	inline unsigned int pointsCacheCount() const
	{
		return mPointsCacheCount;
	}

	/**
	 * @return The number of faces/triangles (i.e. nodes) in this mesh. Use
	 * the constructor to create the correct number.
	 **/
	unsigned int facesCount() const;

	void setFace(int index, const BoFace& face);
	const BoFace* face(unsigned int f) const;

	/**
	 * @param vertex The vertex in the face this normal applies to. This
	 * must be 0..2 or -1 for all vertices.
	 **/
	void setNormal(unsigned int face, int vertex, const BoVector3Float& normal);

	/**
	 * Called by @ref movePoints. This adds @p moveBy to every @ref
	 * BoFace::pointIndex.
	 **/
	void movePointIndices(int moveBy);

	void createPointCache();

	/**
	 * Ensure that all values in the point cache are valid. After this
	 * function is done, all entries in the point cache are greate or equal
	 * @p min and less or equal @p max.
	 *
	 * Note the "or equal" for @p max!
	 **/
	void ensurePointCacheValid(unsigned int min, unsigned int max);

	void disconnectNodes();
	void connectNodes();
	void addNodes();

	inline BoMeshRendererMeshLODData* meshRendererMeshLODData() const
	{
		return mMeshRendererMeshLODData;
	}

	void setMeshRendererMeshLODData(BoMeshRendererMeshLODData* data);

private:
	BoMeshLODPrivate* d;

private:
	BoFaceNode* mNodes;
	int mType;

	BoMeshRendererMeshLODData* mMeshRendererMeshLODData;

	// the list of points in the final order (after connectNodes() or
	// addNodes() was called). iterating through nodes() is equalivent (for
	// some modes the BoFaceNode::relevantPoint() will have to be used though)
	unsigned int* mPointsCache;
	unsigned int mPointsCacheCount;
};


class BoMeshPrivate;
class BoMesh
{
public:
	/**
	 * @param faces The number of faces (triangles) in this mesh to be
	 * created. You must use @ref setFace to initialize them.
	 **/
	BoMesh(unsigned int faces, const QString& name);
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

	static int texelPos();

	/**
	 * Prepare to load the points, i.e. allocate memory for them. You can
	 * set them using @ref setVertex and @ref setTexel.
	 **/
	void allocatePoints(unsigned int points);

	/**
	 * Move the points from the local array to the specified array. This
	 * will copy all points (vertices and texture coordinates). The points
	 * are inserted starting at @p index - all local indices are changed
	 * (i.e. increased by @p index).
	 *
	 * Note that the points are moved only, not changed. I.e. as long as you
	 * use the correct (maybe modified) index @ref point will return the
	 * same point.
	 *
	 * @return The number of points (!) that have been used. Note: a single
	 * point consists of @ref pointSize floats!
	 **/
	unsigned int movePoints(float* array, int index);

	/**
	 * Use material @p mat when rendering this mesh.
	 **/
	void setMaterial(BoMaterial* mat);

	BoMaterial* material() const;

	/**
	 * This adds the @p face to the default LOD (i.e. the full-detailed
	 * version) of this mesh. The face will be at @p index.
	 *
	 * Note that once @ref generateLOD was called you should not call
	 * setFace() anymore, as it will apply to the full-detailed version of
	 * the mesh only. As long as LODs aren't generated you don't have to
	 * care about LOD (@ref generateLOD will do everything).
	 **/
	void setFace(int index, const BoFace& face);

	/**
	 * @return BoMeshLOD::face for LOD 0 (i.e. full detailed mesh).
	 **/
	const BoFace* face(unsigned int f) const;

	/**
	 * You must call @ref allocatePoints before calling this!
	 *
	 * This methods sets the list of available vertices for this mesh. The
	 * faces (see @ref BoFace) may reference these.
	 *
	 * You are not allowed to call this twice!
	 * -> Some pre-parsing may occur at this point already (or on demand
	 *  after this point), and other methods/classes that use the generated
	 *  data might not know about later changes.
	 * @param vertices A value vector containing all vertices. The size of the
	 * vector must match exactly the size that was allocated using @ref
	 * allocatePoints.
	 **/
	// AB: if BoLODBuilder ever needs to add points/vertices, we should add
	// a addVertex() method, which _appends_ a vertex to the internal array.
	// existing vertices should not be removed.
	void setVertices(const QValueVector<BoVector3Float>& vertices);

	void calculateNormals();

	/**
	 * Just like @ref setVertices, just for texels. All rules for the
	 * @ref setVertices are valid here, too, but a zero size vector is
	 * valid.
	 *
	 * Note that the third component of every texel is discarded.
	 **/
	void setTexels(const QValueVector<BoVector3Float>& texels);

	/**
	 * Try to connect all faces in the mesh, so that we can use
	 * GL_TRIANGLE_STRIP. If that doesn't work this function will add all
	 * faces completely instead. See @ref addNodes.
	 *
	 * Note that this is not yet working!
	 **/
	void connectNodes();

	/**
	 * Add all nodes from @ref mesh, so that we can use GL_TRIANGLES. You
	 * should prefer @ref connectNodes usually (in case it was working).
	 **/
	void addNodes();

	/**
	 * Generate a point list (as it can be used by glDrawElements()) from
	 * the node list (see @ref nodes).
	 *
	 * The point cache is an array of all vertices that are referenced by
	 * the nodes/faces (exact: nodes, although the difference is not too
	 * big). So if face 1 references the vertices 2,4,7 and face 2
	 * references 7,5,9 then the point cache is 2,4,7,7,5,9
	 *
	 * The point cache can be used directly for the indices in
	 * glDrawElements().
	 *
	 * This cache will be invalid once @ref connectNodes or @ref addNodes
	 * gets called (i.e. the order of points get changed in any way).
	 **/
	void createPointCache();

	/**
	 * Disconnect all nodes to prepare another @ref connectNodes or @ref
	 * addNodes call. No node is deleted.
	 **/
	void disconnectNodes();

	/**
	 * Set whether this mesh is a teamcolor object or not.
	 **/
	void setIsTeamColor(bool teamColor);

	/**
	 * @return TRUE if this is a teamcolor object (which also is not textured)
	 * and FALSE if it is not a teamcolor object.
	 **/
	bool isTeamColor() const;

	/**
	 * @return material()->textureObject() if @ref material is non-null,
	 * otherwise 0.
	 **/
	GLuint textureObject() const;

	/**
	 * @return name of the mesh (loaded from file)
	 **/
	const QString& name() const;

	void renderMesh(const BoMatrix* matrix, const QColor* color, unsigned int lod = 0);

	/**
	 * Render the bounding object (usually a mesh) of this mesh
	 **/
	void renderBoundingObject();

	/**
	 * Render a point for every vertex. The points are not connected
	 * and can therefore be used to see where vertices are, while the
	 * mesh is rendered as usual using @ref renderMesh.
	 **/
	void renderVertexPoints(unsigned int lod = 0);

	/**
	 * Create a BoVector3 at index @p p from the vertex pool.
	 *
	 * @param p The index of the vertex in the vertex pool. Must
	 * be < @ref points.
	 *
	 * See also @ref BosonModel::vertex, which uses indices that are
	 * <em>global</em> to the model (as in @ref BoFace::pointIndex), while
	 * this method uses indices that are local to this mesh.
	 **/
	BoVector3Float vertex(unsigned int p) const;
	BoVector3Float texel(unsigned int p) const;


	/**
	 * @return The number of points in this mesh. See also @ref facesCount
	 **/
	unsigned int points() const;

	/**
	 * @return The maximal x value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxX() const;

	/**
	 * @return The minimal x value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minX() const;

	/**
	 * @return The maximal y value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxY() const;

	/**
	 * @return The minimal y value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minY() const;

	/**
	 * @return The maximal z value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxZ() const;

	/**
	 * @return The minimal z value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minZ() const;

	/**
	 * Return the bounding box in the specified parameters.
	 *
	 * The bounding box consists of the 8 points that are made by the
	 * possible combinations of the min/max x/y/z values.
	 *
	 * Note that the bounding box is axis aligned in all cases and might
	 * therefore be bigger than necessary.
	 *
	 * This method is equivalent to calling @ref minX, @ref maxX, ... for
	 * all specified parameters.
	 **/
	void getBoundingBox(float* minX, float* maxX, float* minY, float* maxY, float* minZ, float* maxZ) const;

	/**
	 * @overloaded
	 * This version behaves just like the above method, but it returns all
	 * combinations of min/max x/y/z in @p vertices. These 8 vertices are
	 * all vertices of the bounding box.
	 * @param vertices Must be an array of 8. The vertices of the bounding
	 * box are returned here.
	 **/
	void getBoundingBox(BoVector3Float* vertices) const;

	/**
	 * Return the bounding box after transforming the mesh by @p matrix. See
	 * the above version for more information.
	 *
	 * Note that the returned bounding box is still axis aligned and
	 * therefore is probably bigger than necessary!
	 *
	 * Also note that this bounding box is retrieved by transforming the
	 * bounding box returned by the above method by @p matrix, and therefore
	 * is probably <em>much</em> larger than necessary. For better results
	 * you should transform all vertices (see @ref vertex) by @ref matrix
	 * and find the min/max values from all resulting vertices.
	 **/
	void getBoundingBox(const BoMatrix& matrix, float* minX, float* maxX, float* minY, float* maxY, float* minZ, float* maxZ) const;

	/**
	 * Compute a bounding object (usually a box) for the mesh.
	 **/
	void computeBoundingObject();

	/**
	 * Called by @ref BosonModel to generate LODs for the mesh.
	 * @param lodCount How many levels of detail will be generated. Must be
	 * at least 1 and must be the same value for all meshes in the model.
	 **/
	void generateLOD(unsigned int lodCount, const BosonModel* parent);

	unsigned int facesCount(unsigned int lod) const;

	inline BoMeshRendererMeshData* meshRendererMeshData() const
	{
		return mMeshRendererMeshData;
	}

	void setMeshRendererMeshData(BoMeshRendererMeshData* data);

protected:
	// this is meant to check whether the something on the screen will
	// change if we draw this mesh now.
	bool checkVisible();

	BoMeshLOD* levelOfDetail(unsigned int lod) const;
	unsigned int lodCount() const;

	/**
	 * @overload
	 *
	 * This will create a @ref BoVector3 for the vertex @p i in @p face.
	 *
	 * Note: this is protected, because I don't want to make the @p _lod
	 * public. If possible people who write e.g. fileloaders should not cope
	 * with lod at all.
	 * @param lod See @ref levelOfDetail
	 **/
	BoVector3Float vertex(unsigned int face, unsigned int i, unsigned int lod) const;

	void calculateNormals(unsigned int lod);

	void setNormal(unsigned int face, int vertex, const BoVector3Float& normal);

	/**
	 * Calculate values for @ref maxZPoint and similar functions.
	 *
	 * Called by @ref setVertices
	 **/
	void calculateMaxMin();

private:
	void init();
	friend class BoMeshRenderer;

private:
	BoMeshPrivate* d;
	BoMeshRendererMeshData* mMeshRendererMeshData;
};


/**
 * Same as @ref BoMeshRendererModelData, but this stores data for the @ref
 * BoMesh.
 *
 * See also @ref BoMesh::meshRendererMeshData and @ref
 * BoMeshRenderer::initMeshData.
 *
 * If you are not sure whether your data belongs to BoMeshRendererMeshData or to
 * @ref BoMeshRendererMeshLODData, try @ref BoMeshRendererMeshLODData first.
 * There you will have less trouble retrieving certain data (such as normals).
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

/**
 * Same as @ref BoMeshRendererModelData, but this stores data for the @ref
 * BoMeshLOD.
 *
 * See also @ref BoMesh::meshRendererMeshData and @ref
 * BoMeshRenderer::initMeshLODData.
 *
 * You will most probably want to store your mesh relevant data in this class
 * (e.g. vertex buffer objects), if you are not sure whether you need @ref
 * BoMeshRendererMeshData or BoMeshRendererLODData, try this one first.
 * @short Simple storage class for @ref BoMeshLOD and @ref BoMeshRenderer
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshRendererMeshLODData
{
public:
	BoMeshRendererMeshLODData()
	{
	}
	virtual ~BoMeshRendererMeshLODData()
	{
	}
};

#endif
