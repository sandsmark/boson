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
#include <qintdict.h>
#include <GL/gl.h>

#include <lib3ds/types.h>

class BosonModelTextures;
class BoFrame;


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BosonModel
{
public:
	/**
	 * Construct a model using an already created display list
	 **/
	BosonModel(GLuint list, float width, float height);

	/**
	 * @param width The final width of the unit. The model will be scaled to
	 * fit this value, if possible. Distortion will be avoided.
	 * @param height The final height width of the unit. The model will be scaled to
	 * fit this value, if possible. Distortion will be avoided.
	 **/
	BosonModel(const QString& dir, const QString& file, float width, float height);
	~BosonModel();

	void loadModel();

	/**
	 * Generate display lists for steps construction steps. The unit will
	 * display more and more objects of the final model, when being
	 * constructed.
	 **/
	void generateConstructionLists();

	/**
	 * Note: as long as @ref constructionStep is < @ref constructionSteps
	 * you can't call this function!
	 *
	 * (note also that by default @ref constructionStep == @ref
	 * constructionSteps == 0)
	 **/
	void setFrame(unsigned int frame);
	unsigned int frame() const { return mFrame; }
	inline unsigned int frames() const { return mFrames.count(); }

	void setConstructionStep(unsigned int step);
	inline unsigned int constructionSteps() const { return mConstructionSteps.count(); }
	inline unsigned int constructionStep() const { return mConstructionStep; }

	/**
	 * @return The display list of the current @ref frame.
	 **/
	inline GLuint displayList() const
	{
		return mDisplayList;
	}
	
	/**
	 * @return The factor BO_GL_CELL_SIZE needs to be multiplied with to get
	 * the actual depth (height in z-direction) of the unit
	 **/
	float depthMultiplier() const { return mDepthMultiplier; }

	/**
	 * Since .3ds files seem to supprt filenames of 8+3 chars only you can
	 * provide a map which assigns longer names here.
	 **/
	void setLongNames(QMap<QString, QString> names) { mTextureNames = names; }

protected:
	class BoHelper; // for computing width,height,.. of the model. this is a hack!

protected:
	void loadTextures();
	void createDisplayLists();
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

private:
	void init();
	void setCurrentFrame(BoFrame* frame);

private:
	Lib3dsFile* m3ds;
	static BosonModelTextures* mModelTextures;
	QMap<QString, QString> mTextureNames;
	GLuint mDisplayList;
	QIntDict<BoFrame> mFrames;
	QIntDict<BoFrame> mConstructionSteps;
	QString mDirectory;
	QString mFile;
	unsigned int mFrame;
	unsigned int mConstructionStep;

	float mWidth;
	float mHeight;
	float mDepthMultiplier;
};
#endif

