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
#ifndef BOSONMODEL_H
#define BOSONMODEL_H

#include "../global.h"

#include <bogl.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3PtrList>

class KSimpleConfig;
class QColor;
class QString;
class Q3CString;
class QStringList;
class BoMatrix;
class BoMesh;
class BoMaterial;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;
template<class T> class Q3PtrList;
template<class T> class Q3ValueVector;
template<class T, class T2> class QMap;
template<class T> class Q3IntDict;
class BoMeshRendererModelData;

/**
 * This class represents the Frame* entries in the index.unit files. Here you
 * can find where this animation starts and ends and so.
 * See @ref BosonModel::insertAnimationMode
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Information about an animation mode.
 **/
class BosonAnimation
{
public:
	BosonAnimation(unsigned int start, unsigned int end, float speed, bool loop)
	{
		mStart = start;
		mEnd = end;
		mSpeed = speed;
		mLoop = loop;
	}

	/**
	 * @return The number of the first frame of this animation
	 **/
	inline unsigned int start() const { return mStart; }

	/**
	 * @return The number of the last frame of this animation
	 **/
	inline unsigned int end() const { return mEnd; }

	/**
	 * @return The number of frames in this animation
	 **/
	inline unsigned int range() const { return mEnd - mStart + 1; }

	inline float speed() const { return mSpeed; }

	/**
	 * @return TRUE if if the animation should restart at @ref start once
	 * @ref end was exceeded
	 **/
	inline bool loop() const { return mLoop; }

private:
	unsigned int mStart;
	unsigned int mEnd;
	float mSpeed;
	bool mLoop;
};

class BoFrame
{
public:
	BoFrame();

	/**
	 * Construct a copy of the first @p nodeCount meshes from @p frame
	 *
	 * This can be used for the construction animations. Remember to set the
	 * correct display lists!
	 **/
	BoFrame(const BoFrame& frame, unsigned int firstNode, unsigned int nodeCount);

	/**
	 * @overload
	 * Just as above, but this version takes an array of node indices from
	 * @p frame that will get copied into this frame.
	 **/
	BoFrame(const BoFrame& frame, unsigned int* nodes, unsigned int nodeCount);

	~BoFrame();

	/**
	 * Allocate matrices for all nodes in the model. These matrices modify
	 * the positions of the meshes for this frame.
	 **/
	void allocNodes(int nodes);

	void setMesh(unsigned int index, BoMesh* mesh);

	/**
	 * To initialize the mesh you should call @ref setMesh before calling
	 * this!
	 * @return The mesh for node @p index. No validity check happens here!
	 **/
	inline BoMesh* mesh(int index) const { return mMeshes[index]; }

	/**
	 * You can use this function to change the matrix after calling @ref
	 * allocNodes!
	 *
	 * The returned matrix should be applied to the modelview before
	 * rendering the mesh of node @p index.
	 * @return A pointer to the matrix for node @p index (0 .. @ref nodeCount). No validity check happens here!
	 **/
	BoMatrix* matrix(int index) const;

	void setRadius(float r) { mRadius = r; }

	unsigned int nodeCount() const { return mNodeCount; }

	/**
	 * @param lod See @ref BoMesh::renderMesh
	 **/
	void renderFrame(const Q3ValueVector<const BoMatrix*>& itemMatrices, const QColor* teamColor, bool transparentmeshes = false, RenderFlags flags = Default, int mode = GL_RENDER);

private:
	void init();
	void copyNodes(const BoFrame& frame, unsigned int* nodes, unsigned int count);

private:
	float mDepthMultiplier;
	float mRadius; // TODO

	unsigned int mNodeCount;
	BoMatrix** mMatrices;
	BoMesh** mMeshes;
};


class BoLOD
{
public:
	BoLOD();
	~BoLOD();

	void allocateMeshes(unsigned int count);
	unsigned int meshCount() const { return mMeshCount; }
	BoMesh* mesh(unsigned int i) const;

	void allocateFrames(unsigned int count);
	unsigned int addFrames(unsigned int count);
	unsigned int frameCount() const { return mFrameCount; }
	BoFrame* frame(unsigned int i) const;
	void setFrame(unsigned int i, BoFrame* f);

	bool hasTransparentMeshes() const { return mHasTransparentMeshes; }
	void setHasTransparentMeshes(bool has) { mHasTransparentMeshes = has; }


private:
	unsigned int mMeshCount;
	BoMesh** mMeshes;
	unsigned int mFrameCount;
	BoFrame** mFrames;
	bool mHasTransparentMeshes;
};


class BosonModelPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BosonModel
{
public:
	BosonModel(const QString& dir, const QString& file);
	~BosonModel();

	unsigned int id() const;
	static unsigned int maxId();

#if 0
	/**
	 * @return The width of the model in the OpenGL coordinate system. Most units
	 * will use 1.0 or 1.5 or 2.0 probably.
	 **/
	float width() const { return mWidth; }

	/**
	 * @return The height of the model in the OpenGL coordinate system. Most units
	 * will use 1.0 or 1.5 or 2.0 probably.
	 **/
	float height() const { return mHeight; }
#endif

	/**
	 * Allocate @p count material objects. You must call this before you can
	 * use @ref setMaterial and @ref material.
	 *
	 * The material objects will be initialized as "default" materials.
	 **/
	 // TODO: maybe move materials to some central place (BoMaterialManager?
	 //  BosonBigDisplayBase?) so that they can be shared between models?
	void allocateMaterials(unsigned int count);

	/**
	 * @return The material object at @p index.
	 **/
	BoMaterial* material(unsigned int index) const;
	unsigned int materialCount() const;

	void allocateLODs(unsigned int count);
	BoLOD* lod(unsigned int index) const;
	/**
	 * Minimum distance that has to be between camera and model for lod @p index
	 *  to be used.
	 **/
	float lodDistance(unsigned int index) const;
	void setLodDistance(unsigned int index, float distance) const;
	unsigned int lodCount() const;

	/**
	 * @return The level of detail that is preferred for the @p
	 * distanceFromCamera.
	 **/
	unsigned int preferredLod(float distanceFromCamera) const;

	bool hasTransparentMeshes(unsigned int lod = 0);

	bool loadModel(const QString& configfile);

	/**
	 * Generate animation for construction steps. The unit will
	 * display more and more objects of the final model, when being
	 * constructed.
	 **/
	void generateConstructionAnimation(unsigned int steps);


	/**
	 * Add an animation mode with the identitfier @p mode, starting at frame
	 * number @p start and ending at frame @p end. Every advance call, current
	 * frame number should be increased by @p speed and if @p loop is true,
	 * animation should loop.
	 *
	 * Note that one mode with id 0 is already added. This mode has default
	 * settings start=0,end=0,speed=0,loop=true, this is the only mode that can be
	 * replaced by calling insertAnimationMode. You must not use invalid
	 * settings for this mode!
	 *
	 * @param start The first frame (note that counting starts with 0) of
	 * this animation.
	 * @param end The last frame (note that counting starts with 0) of
	 * this animation.
	 * @param speed By how many frames to advance the animation every advance
	 * call. 1 changes to next frame every advance call, 2 skips every other
	 * frame, 0.5 shows every frame for two advance calls, etc
	 * @param loop If true, then when the end of the animation is reached,
	 * playing continues from the beginning of the animation. If false, then once
	 * the end frame is reached, animation stops.
	 **/
	void insertAnimationMode(int mode, unsigned int start, unsigned int end, float speed, bool loop);

	/**
	 * Load an animation mode from a config file. This is a frontend to @ref
	 * insertAnimationMode.
	 * @param config The config file to load from
	 * @param name The name of the mode as it is used in the config file
	 * (e.g. "Idle" for entries like "Animation-Idle-Start")
	 **/
	void loadAnimationMode(int mode, KSimpleConfig* config, const QString& name);

	/**
	 * @return The animation assigned to @p mode. See @ref
	 * insertAnimationMode.
	 **/
	BosonAnimation* animation(int mode) const;

	/**
	 * @return The absolute filename to the .3ds file of this model.
	 **/
	QString file() const;

	/**
	 * @return A pointer to the points (vertices, normals and texture coordinates)
	 * for this model.
	**/
	float* pointArray() const;

	/**
	 * @return The number of points in @ref pointArray
	 **/
	unsigned int pointArraySize() const;

	void allocatePointArray(unsigned int size);

	/**
	 * @return A pointer to the indices for this model.
	 * Note that you'll need to cast the array into either QUINT16* or QUINT32*,
	 *  depending on what @ref indexArrayType returns.
	 **/
	unsigned char* indexArray() const;

	/**
	 * @return The number of indices in @ref indexArray
	 **/
	unsigned int indexArraySize() const;

	unsigned int indexArrayType() const;

	/**
	 * Allocates index array, consisting of size elements of type type.
	 * Type can be either GL_UNSIGNED_SHORT or GL_UNSIGNED_INT.
	 **/
	void allocateIndexArray(unsigned int size, unsigned int type);

	float boundingSphereRadius() const;
	void setBoundingSphereRadius(float r);

	const BoVector3Float& boundingBoxMin() const;
	const BoVector3Float& boundingBoxMax() const;
	void setBoundingBox(const BoVector3Float& min, const BoVector3Float& max);

	/**
	 * Called before the very first model in a set of models is rendered.
	 * See @ref BoMeshRenderer::startModelRendering
	 **/
	static void startModelRendering();

	/**
	 * See @ref BoMeshRenderer::stopModelRendering
	 **/
	static void stopModelRendering();

	/**
	 * Prepare this model for being rendered next. This must be called once
	 * before rendering anything (frame, mesh, ..) in this model.
	 *
	 * This will e.g. enable @ref pointArray pointer to be used for
	 * vertex arrays. The client state must have been set already.
	 **/
	void prepareRendering();


	/**
	 * @return The @ref BoMeshRenderer specific data of the model for the
	 * current renderer. Such data may e.g. be display lists, vertex buffers,
	 * ...
	 **/
	inline BoMeshRendererModelData* meshRendererModelData() const
	{
		return mMeshRendererModelData;
	}

	void setMeshRendererModelData(BoMeshRendererModelData* data);

protected:
	void loadTextures(const QStringList& textures);

	/**
	 * @return The directory that contains the .3ds file. Usually the unit
	 * directory
	 **/
	const QString& baseDirectory() const;

private:
	void init();

private:
	friend class KGameModelDebug;

private:
	BosonModelPrivate* d;

	BoMeshRendererModelData* mMeshRendererModelData;
};

/**
 * This class stores data that is used by the current meshrenderer only. See
 * @ref BoMeshRenderer for information on the meshrenderer.
 *
 * A derived class can contain any data that you like/need (including but not
 * limited to point arrays, vertex buffer objects, ...). It is created and
 * initialized in @ref BoMeshRenderer::initModelData.
 *
 * The default implementation does not store anything.
 *
 * A data object can be retrieved from a @ref BosonModel using @ref
 * BosonModel::meshRendererModelData.
 * @short A simple data storage class for @ref BosonModel and @ref
 * BoMeshRenderer
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshRendererModelData
{
public:
	BoMeshRendererModelData() {}
	virtual ~BoMeshRendererModelData() {}
};

#endif

