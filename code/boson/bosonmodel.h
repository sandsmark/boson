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

#include <qmap.h>
#include <qvaluelist.h>
#include <qintdict.h>
#include <GL/gl.h>

#include <lib3ds/types.h>

class BosonModelTextures;
class QColor;


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

	BoFrame* frame(unsigned int frame) const;
	inline unsigned int frames() const { return mFrames.count(); }

	BoFrame* constructionStep(unsigned int step);
	inline unsigned int constructionSteps() const { return mConstructionSteps.count(); }

	/**
	 * Since .3ds files seem to supprt filenames of 8+3 chars only you can
	 * provide a map which assigns longer names here.
	 **/
	void setLongNames(QMap<QString, QString> names) { mTextureNames = names; }

	static void reloadAllTextures();

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
	 * units index.desktop files. See setLongNames
	 **/
	QString cleanTextureName(const char* name);

	/**
	 * @return The directory that contains the .3ds file. Usually the unit
	 * directory
	 **/
	const QString& baseDirectory() const { return mDirectory; }

	/**
	 * @param v A single vector
	 **/
	void dumpVector(Lib3dsVector v);

	/**
	 * @param v An array of 3 Lib3dsVector
	 * @param texture none if 0, otherwise the textue object
	 * @param tex if texture is non-null this must be the texture
	 * coordinates (array of 3) as provided for glTexCoord*()
	 **/
	void dumpTriangle(Lib3dsVector* v, GLuint texture = 0, Lib3dsTexel* tex = 0);

	/**
	 * Calculate matrix*inputVector and return the result into resultVector.
	 *
	 * This can be used for calculations that are not speed-sensitive.
	 * @param resultVector An array of 3 (the forth coordinate is ignored
	 * because it is unused by boson)
	 * @param matrix An array of 16, as returned by e.g.
	 * glGetFloatv(GL_TEXTURE_MATRIX, matrix)
	 * @param inputVector An array of 3 that should get transformed
	 **/
	void myTransform(GLfloat* resultVector, GLfloat* matrix, GLfloat* inputVector);

private:
	void init();

private:
	static BosonModelTextures* mModelTextures;

	QString mDirectory;
	QString mFile;
	Lib3dsFile* m3ds;

	QMap<QString, QString> mTextureNames;
	QIntDict<BoFrame> mFrames;
	QIntDict<BoFrame> mConstructionSteps;
	QValueList<GLuint> mNodeDisplayLists;
	QColor* mTeamColor;

	float mWidth;
	float mHeight;
};
#endif

