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
#ifndef BOSONMODEL_H
#define BOSONMODEL_H

#include <GL/gl.h>

#include <lib3ds/types.h>

class BosonModelTextures;
class KSimpleConfig;
class QColor;
class QString;
class QStringList;
class BoVector3;
template<class T> class QPtrList; //hmm is this working for all compilers? can templates be forwarded safely?
template<class T, class T2> class QMap;

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
	BoFrame(const BoFrame& f);
	~BoFrame();

	void setDisplayList(GLuint l) { mDisplayList = l; }
	GLuint displayList() const { return mDisplayList; }
	float depthMultiplier() const { return mDepthMultiplier; }
	void setDepthMultiplier(float d) { mDepthMultiplier = d; }
	void setRadius(float r) { mRadius = r; }

private:
	GLuint mDisplayList;
	float mDepthMultiplier;
	float mRadius; // TODO
};


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
	 * Will use c as color for all meshes of the model with the name
	 * "teamcolor". Note that this <em>must</em> be called before @ref
	 * loadModel
	 **/
	void setTeamColor(const QColor& c);

	void loadModel();

	/**
	 * Cleanup some variables. Deletes e.g. the 3ds file and the teamcolor
	 * object. Note that you must not call e.g. @ref loadModel or @ref
	 * generateConstructionLists after this!
	 *
	 * You can continue using the generated display lists.
	 **/
	void finishLoading();

	/**
	 * Generate display lists for steps construction steps. The unit will
	 * display more and more objects of the final model, when being
	 * constructed.
	 **/
	void generateConstructionLists();

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

	static void reloadAllTextures();

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
	 * @return A list of all textures used in this model. Note that these
	 * are the plain names, i.e. not necessarily the final filenames. Use
	 * @ref cleanTextureName to receive the final fileName.
	 **/
	static QStringList textures(Lib3dsFile* file);

protected:
	class BoHelper; // for computing width,height,.. of the model. this is a hack!

protected:
	void loadTextures();
	/**
	 * Call @ref loadNode for all nodes in this model - also the child
	 * nodes.
	 * @param reload Overwrite any existing display lists (i.e. replace
	 * them) if TRUE, otherwise abort if a display list already exists
	 * (default)
	 **/
	void loadNodes(bool reload = false);

	/**
	 * Generate the "normal" display lists, for all frames. This includes
	 * all nodes of the file. See also @ref generateConstructionLists, which
	 * doesn't use all nodes.
	 **/
	void createDisplayLists();

	/**
	 * Generate the display list for this node, including all child nodes.
	 * The resulting display list will be placed into node->user.d
	 * @param reload See @ref loadNodes
	 **/
	void loadNode(Lib3dsNode* node, bool reload);

	/**
	 * Render the specified node according to the values for the current
	 * frame. You should call lib3ds_file_eval(frameNumber) before calling
	 * renderNode().
	 *
	 * Note that only one node (+ all child nodes) will be rendered - you
	 * should iterate all toplevel nodes and call renderNode() for all of
	 * them usually.
	 **/
	void renderNode(Lib3dsNode* node);

	void computeBoundings(Lib3dsNode* node, BoHelper* helper);

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

public:
	/**
	 * @return Whether the triangle @p face1 is adjacent to the triangle @p
	 * face2. That means that at least 2 points (i.e. vectors) are equal.
	 **/
	static bool isAdjacent(BoVector3* face1, BoVector3* face2);

	/**
	 * Find adjacent faces in @p mesh and place them into @p adjacentFaces.
	 * The search will start at @p search if @p search is non-NULL,
	 * otherwise the first face of @p mesh is used.
	 *
	 * Once an adjacent face is found this function also searches for
	 * adjacent faces of the new face and so on.
	 **/
	static void findAdjacentFaces(QPtrList<Lib3dsFace>* adjacentFaces, Lib3dsMesh* mesh, Lib3dsFace* search = 0);

	/**
	 * @param v A single vector
	 **/
	void dumpVector(Lib3dsVector v);

	/**
	 * @param v An array of 3 BoVector3
	 * @param texture none if 0, otherwise the textue object
	 * @param tex if texture is non-null this must be the texture
	 * coordinates (array of 3) as provided for glTexCoord*()
	 **/
	static void dumpTriangle(BoVector3* v, GLuint texture = 0, Lib3dsTexel* tex = 0);
	static void dumpTriangle(Lib3dsVector* v, GLuint texture = 0, Lib3dsTexel* tex = 0);

private:
	void init();

private:
	class Private;
	friend class KGameModelDebug;

private:
	static BosonModelTextures* mModelTextures;

	Private* d;
	Lib3dsFile* m3ds;

	QColor* mTeamColor;

	float mWidth;
	float mHeight;
};


#endif

