/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONMODEL_H
#define BOSONMODEL_H

#include <GL/gl.h>

class KSimpleConfig;
class QColor;
class QString;
class QStringList;
class BoMatrix;
class BoMesh;
class BoMaterial;
class BosonModelLoaderData;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;
template<class T> class QPtrList;
template<class T, class T2> class QMap;
template<class T> class QIntDict;
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
	BosonAnimation(int start, unsigned int range, unsigned int speed)
	{
		mStart = start;
		mRange = range;
		mSpeed = speed;
	}

	inline int start() const { return mStart; }
	inline unsigned int range() const { return mRange; }
	inline unsigned int speed() const { return mSpeed; }

private:
	int mStart;
	unsigned int mRange;
	unsigned int mSpeed;
};

class BoFrame
{
public:
	BoFrame();

	/**
	 * Construct a copy of the first @p meshCount meshes from @p frame
	 *
	 * This can be used for the construction animations. Remember to set the
	 * correct display lists!
	 **/
	BoFrame(const BoFrame& frame, unsigned int firstMesh, unsigned int meshCount);

	/**
	 * @overload
	 * Just as above, but this version takes an array of mesh indices from
	 * @p frame that will get copied into thuis frame.
	 **/
	BoFrame(const BoFrame& frame, unsigned int* meshes, unsigned int meshCount);

	~BoFrame();

	/**
	 * Allocate matrices for all meshes in the model. These matrices modify
	 * the positions of the meshes for this frame.
	 **/
	void allocMeshes(int meshes);

	void setMesh(unsigned int index, BoMesh* mesh);

	/**
	 * To initialize the mesh you should call @ref setMesh before calling
	 * this!
	 * @return The mesh at index @p index. No validity check happens here!
	 **/
	inline BoMesh* mesh(int index) const { return mMeshes[index]; }

	/**
	 * You can use this function to change the matrix after calling @ref
	 * allocMeshes!
	 *
	 * The returned matrix should be applied to the modelview before
	 * rendering the mesh that is at @p index.
	 * @return A pointer to the matrix at index @p index (0 .. @ref meshCount). No validity check happens here!
	 **/
	BoMatrix* matrix(int index) const;

	/**
	 * Mark the mesh at @p indes (see @ref setMesh) as hidden, if @p hidden
	 * is TRUE. It will not be rendered, unless this is called again with @p
	 * hidden = FALSE (the default).
	 **/
	void setHidden(unsigned int index, bool hidden);

	/**
	 * @return Whether the mesh is hidden (i.e. should not be rendered) or
	 * not. By default it is not hidden.
	 */
	inline bool hidden(unsigned int index) const
	{
		return mHidden[index];
	}

	float depthMultiplier() const { return mDepthMultiplier; }
	void setDepthMultiplier(float d) { mDepthMultiplier = d; }
	void setRadius(float r) { mRadius = r; }

	//AB: this is the number of nodes (from 3ds files), NOT the number if
	//*different* meshes! we can have several pointers two the same mesh,
	//but they will have different matrices. e.g. we can have 4 wheels (-->
	//the same mesh, but 4 different positions)
	unsigned int meshCount() const { return mMeshCount; }

	/**
	 * @param lod See @ref BoMesh::renderMesh
	 **/
	void renderFrame(const QColor* teamColor, unsigned int lod = 0, int mode = GL_RENDER);

	void mergeMeshes();
	void sortByDepth();

private:
	void init();
	void copyMeshes(const BoFrame& frame, unsigned int* meshes, unsigned int count);

private:
	float mDepthMultiplier;
	float mRadius; // TODO

	unsigned int mMeshCount;
	BoMatrix** mMatrices;
	BoMesh** mMeshes;
	bool* mHidden;
};

class BosonModelPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BosonModel
{
public:
	/**
	 * @param width The final width of the unit. The model will be scaled to
	 * fit this value, if possible. Distortion will be avoided.
	 * @param height The final height width of the unit. The model will be scaled to
	 * fit this value, if possible. Distortion will be avoided.
	 **/
	BosonModel(const QString& dir, const QString& file, float width, float height);
	~BosonModel();

	/**
	 * @return The width of the model in the OpenGL coordinate system. This
	 * is the value provided in the constructor - most units will use 1.0 or
	 * 1.5 or 2.0 probably.
	 **/
	float width() const { return mWidth; }

	/**
	 * @return The height of the model in the OpenGL coordinate system. This
	 * is the value provided in the constructor - most units will use 1.0 or
	 * 1.5 or 2.0 probably.
	 **/
	float height() const { return mHeight; }

	/**
	 * Allocate @p count material objects. You must call this before you can
	 * use @ref setMaterial and @ref material.
	 *
	 * The material objects will be initialized as "default" materials.
	 **/
	void allocateMaterials(unsigned int count);

	/**
	 * @return The material object at @p index.
	 **/
	BoMaterial* material(unsigned int index) const;
	unsigned int materialCount() const;

	BoMesh* mesh(unsigned int index) const;
	unsigned int meshCount() const;
	QIntDict<BoMesh> allMeshes() const;

	void loadModel();

	/**
	 * Cleanup some variables. Deletes e.g. the 3ds file.
	 * Note that you must not call e.g. @ref loadModel or @ref
	 * generateConstructionFrames after this!
	 *
	 * You can continue using the generated display lists.
	 **/
	void finishLoading();

	/**
	 * Generate frames for construction steps. The unit will
	 * display more and more objects of the final model, when being
	 * constructed.
	 **/
	void generateConstructionFrames();

	/**
	 * @return The frame @p frame that resideds in the .3ds file. These are
	 * actual animation and model frames, not the construction animation.
	 *
	 * See also @ref constructionStep
	 **/
	BoFrame* frame(unsigned int frame) const;

	/**
	 * @return The number of actual frames (i.e. frames that reside in the
	 * 3ds files. @ref constructionStep frames don't count)
	 **/
	unsigned int frames() const;

	BoFrame* constructionStep(unsigned int step);
	unsigned int constructionSteps() const;

	/**
	 * Since .3ds files seem to supprt filenames of 8+3 chars only you can
	 * provide a map which assigns longer names here.
	 **/
	void setLongNames(QMap<QString, QString> names);

	/**
	 * Add an animation mode with the identitfier @p mode, starting at frame
	 * number @p start with the number of frames @p range. The frame should
	 * change after @p speed advance calls.
	 *
	 * Note that one mode with id 0 is already added. This mode has default
	 * settings start=0,range=1,speed=1, this is the only mode that can be
	 * replaced by calling insertAnimationMode. You must not use invalid
	 * settings for this mode!
	 *
	 * @param start The first frame (note that counting starts with 0) of
	 * this animation. Use -1 to disable this mode (i.e. mode 0 will be used
	 * instead)
	 * @param range How many frames are in this animation. 0 to disable
	 * this mode (i.e. use mode 0)
	 * @param speed How many advance calls have to be made until the frame
	 * changes to the next frame. 1 is the fastest speed - use 0 to disable
	 * this mode and use mode 0 instead.
	 **/
	void insertAnimationMode(int mode, int start, unsigned int range, unsigned int speed);

	/**
	 * Load an animation mode from a config file. This is a frontend to @ref
	 * insertAnimationMode.
	 * @param config The config file to load from
	 * @param name The name of the mode as it is used in the config file
	 * (e.g. "Idle" for entries like "FrameStartIdle")
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
	 * @return A pointer to the points (vertices and texture coordinates)
	 * for this model. You must call @ref mergeArraysbefore you can use
	 * this.
	**/
	float* pointArray() const;

	/**
	 * @return The number of points in @ref pointArray
	 **/
	unsigned int pointArraySize() const;

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
	 * @return How many LOD levels this model has
	 **/
	unsigned int lodCount() const;

	/**
	 * @return The level of detail that is preferred for the @p
	 * distanceFromCamera.
	 **/
	unsigned int preferredLod(float distanceFromCamera) const;

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

	/**
	 * Note that this will return useful values only once the model has been
	 * loaded completely!
	 * @return The vertex at @p v. @p v is the index in the model array,
	 * just like it is used in @ref BoFace::pointIndex. Note that in
	 * @ref BoMes::vertex the indices are local to the mesh!
	 **/
	BoVector3Float vertex(unsigned int v) const;

	/**
	 * @return The texel at @p t. Note that @p t is treated the same way as
	 * in @ref vertex!
	 **/
	BoVector3Float texel(unsigned int t) const;

protected:
	class BoHelper; // for computing width,height,.. of the model. this is a hack!

protected:
	/**
	 * Load the data from @p data into this class.
	 **/
	bool loadModelData(BosonModelLoaderData* data);
	void loadTextures(const QStringList& textures);

	/**
	 * Most 3ds files use dimensions like 40.0 width or more. We use 1.0 for
	 * a normal one-cell unit.
	 *
	 * So we need to compute the scaling factor. Only the first frame is
	 * taken into account currently! Note that we MUST use the SAME value
	 * for ALL frames - otherwise the model might get smaller and bigger
	 * when changing frames.
	 *
	 * The model is scaled to the values width and height that were
	 * specified to the constructor.
	 **/
	void applyMasterScale();

	void computeBoundings(BoFrame* frame, BoHelper* helper) const;

	/**
	 * Compute bounding objects for all meshes and all frames. Usually these
	 * objects will be boxes.
	 **/
	void computeBoundingObjects();

	/**
	 * Convert a 3ds texture name to a clean name. That means call
	 * QString::lower() on it, currently. It'll also map the "short name"
	 * from the .3ds file to the "long name" that can be specified e.g. in
	 * units index.unit files. See setLongNames
	 **/
	QString cleanTextureName(const char* name) const;

	/**
	 * @return The directory that contains the .3ds file. Usually the unit
	 * directory
	 **/
	const QString& baseDirectory() const;

	void generateLOD();

	/**
	 * @ref BoMesh allocates its own array(s) for vertices and texture
	 * coordinates. It is more efficient to maintain a single array that
	 * contains all of them. This does this.
	 *
	 * We might do the same for *all* models, so that we have a single array
	 * for all models in the game.
	 **/
	void mergeArrays();

	void mergeMeshesInFrames();
	void sortByDepth();

public:
	/**
	 * @return Whether the triangle @p face1 is adjacent to the triangle @p
	 * face2. That means that at least 2 points (i.e. vectors) are equal.
	 **/
	static bool isAdjacent(BoVector3Float* face1, BoVector3Float* face2);
	static int findPoint(const BoVector3Float& point, const BoVector3Float* array);

private:
	void init();

private:
	friend class KGameModelDebug;

private:
	BosonModelPrivate* d;

	float mWidth;
	float mHeight;
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

