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

#include "bosonmodel.h"

#include "defines.h"
#include "bosonmodeltextures.h"
#include "bosonprofiling.h"
#include "bo3dtools.h"

#include <kdebug.h>

#include <qimage.h>
#include <qgl.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/material.h>

BosonModelTextures* BosonModel::mModelTextures = 0;

BoFrame::BoFrame()
{
 mDisplayList = 0;
 mRadius = 0.0;
}

BoFrame::BoFrame(const BoFrame& f)
{
 mDisplayList = f.mDisplayList;
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;
}

BoFrame::~BoFrame()
{
 if (mDisplayList != 0) {
	glDeleteLists(mDisplayList, 1);
 }
}

class BosonModel::BoHelper
{
public:
	BoHelper()
	{
		mMaxX = 0.0;
		mMinX = 0.0;
		mMaxY = 0.0;
		mMinY = 0.0;
		mMaxZ = 0.0;
		mMinZ = 0.0;
	}
	
	void addPoint(float x, float y, float z)
	{
		if (x > mMaxX) {
			mMaxX = x;
		} else if (x < mMinX) {
			mMinX = x;
		}
		if (y > mMaxY) {
			mMaxY = y;
		} else if (y < mMinY) {
			mMinY = y;
		}
		if (z > mMaxZ) {
			mMaxZ = z;
		} else if (z < mMinZ) {
			mMinZ = z;
		}
	}

	// radius of the bounding sphere
//	float radius() const
//	{
	//TODO
//	}

	// well in theory mMaxX == -mMinX
	// but usually..
	float diffX() const { return (mMinX - mMaxX); }
	float diffY() const { return (mMinY - mMaxY); }
	float diffZ() const { return (mMinZ - mMaxZ); }

	float lengthX() const { return (mMaxX - mMinX); }
	float lengthY() const { return (mMaxY - mMinY); }
	float lengthZ() const { return (mMaxZ - mMinZ); }

	float scale(float w, float h) const
	{
		float scaleX = w / lengthX();
		float scaleY = h / lengthY();
		// we don't care about z-size here!
		return QMIN(scaleX, scaleY);
	}



	float mMaxX;
	float mMinX; // lowest (i.e. negative) x
	float mMaxY;
	float mMinY;
	float mMaxZ;
	float mMinZ;
};

// store the current values (e.g. current texture, color) to avoid redundant
// OpenGL calls
class BoRenderData
{
public:
	BoRenderData()
	{
		mColor[0] = mColor[1] = mColor[2] = 255;
	}

	bool colorChanged(GLubyte* c)
	{
		bool changed = c[0] != mColor[0] || c[1] != mColor[1] || c[2] != mColor[2];
		if (changed) {
			mColor[0] = c[0];
			mColor[1] = c[1];
			mColor[2] = c[2];
		}
		return changed;
	}

	bool colorToDefault()
	{
		GLubyte c[3];
		c[0] = c[1] = c[2] = 255;
		return colorChanged(c);
	}

	GLubyte mColor[3];
};

BosonModel::BosonModel(const QString& dir, const QString& file, float width, float height)
{
 init();
 mDirectory = dir;
 mFile = file;
 mWidth = width;
 mHeight = height;
}

void BosonModel::init()
{
 m3ds = 0;
 mTeamColor = 0;
 mWidth = 0;
 mHeight = 0;
 mFrames.setAutoDelete(true);
 mConstructionSteps.setAutoDelete(true);
 if (!mModelTextures) { // TODO static deleter!
	mModelTextures = new BosonModelTextures();
 }
}

BosonModel::~BosonModel()
{
 kdDebug() << k_funcinfo << endl;
 finishLoading();
 mModelTextures->removeModel(this);
 kdDebug() << k_funcinfo << "delete " << mFrames.count() << " frames" << endl;
 mFrames.clear();
 kdDebug() << k_funcinfo << "delete " << mConstructionSteps.count() << " construction frames" << endl;
 mConstructionSteps.clear();
 kdDebug() << k_funcinfo << "delete " << mNodeDisplayLists.count() << " child display lists" << endl;
 QValueList<GLuint>::Iterator it = mNodeDisplayLists.begin();
 for (; it != mNodeDisplayLists.end(); ++it) {
	glDeleteLists((*it), 1);
 }
 kdDebug() << k_funcinfo << "done" << endl;
}

void BosonModel::loadModel()
{
 if (mFile.isEmpty() || mDirectory.isEmpty()) {
	kdError() << k_funcinfo << "No file has been specified for loading" << endl;
	return;
 }
 boProfiling->start(BosonProfiling::LoadModel);
 QString fullFile = baseDirectory() + mFile;
 m3ds = lib3ds_file_load(fullFile);
 if (!m3ds) {
	kdError() << k_funcinfo << "Can't load " << fullFile << endl;
	boProfiling->stop(BosonProfiling::LoadModel);
	return;
 }

 Lib3dsNode* node = m3ds->nodes;
 if (!node) {
	kdError() << k_funcinfo << "Could not load file " << fullFile << " correctly" << endl;
	boProfiling->stop(BosonProfiling::LoadModel);
	return;
 }
 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures();
 boProfiling->stop(BosonProfiling::LoadModelTextures);
 boProfiling->start(BosonProfiling::LoadModelDisplayLists);
 loadNodes();
 createDisplayLists();
 boProfiling->stop(BosonProfiling::LoadModelDisplayLists);

 kdDebug() << k_funcinfo << "loaded from " << fullFile << endl;

 boProfiling->stop(BosonProfiling::LoadModel);
}

QString BosonModel::cleanTextureName(const char* name)
{
 QString s = QString(name).lower();
 if (mTextureNames.contains(s)) {
	return mTextureNames[s];
 }
 return s;
}

void BosonModel::loadTextures()
{
 if (!m3ds) {
	kdError() << k_funcinfo << "File was not yet loaded" << endl;
	return;
 }
 //FIXME: do we need to loop through frames? i guess even in other frames there
 //shouldn't be other textures?!
 Lib3dsMaterial* mat;
 for (mat = m3ds->materials; mat; mat = mat->next) {
	Lib3dsTextureMap* t = &mat->texture1_map;
	QString texName = cleanTextureName(t->name);
	if (texName.isEmpty()) {
		continue;
	}
	mModelTextures->insert(this, texName);
 }
}

void BosonModel::createDisplayLists()
{
 if (!QGLContext::currentContext()) {
	kdError() << k_funcinfo << "NULL current context" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "creating " << m3ds->frames << " lists" << endl;
 GLuint listBase = glGenLists(m3ds->frames);
 if (listBase == 0) {
	kdError() << k_funcinfo << "NULL display lists created" << endl;
	return;
 }
 float scale = 1.0; // note: all frame must share the same scaling factor. this means that the model mustn't grow in x or y direction (width or height). It can grow/shrink in z-direction however.

 // ok so lets start now. A .3ds file can contain several frames. A different
 // frame of the same file looks slightly differnt - e.g. useful for animation
 // (mainly even).
 // You can read a frame with lib3ds by calling lib3ds_file_eval() first for
 // every frame.
 for (int i = 0; i < m3ds->frames; i++) {
	Lib3dsNode* p;
	m3ds->current_frame = i;

	// prepare the file for rendering the next frame.
	// Note: as far as I understand the lib3ds code this does *not* change
	// the object itself, but rather node->data.object.*
	// this means for us that we can continue to use the display lists in
	// node->user.d ; but we *have* to ensure that the new values from
	// node->data are used, i.e. they are not stored in that display list.
	// they should reside in the final list.
	// UPDATE: this is achieved by calling loadNode() before renderNode().
	// loadNode() generates the user.d display lists, renderNode() renders
	// the nodes with the recent positions for the frame from node->data
	lib3ds_file_eval(m3ds, m3ds->current_frame);

	// ok, this way (iterating all nodes again) is not really efficient. but
	// it is done only once, so it does not really matter. and it is worth
	// it :)
	BoHelper helper;
	for (p = m3ds->nodes; p; p = p->next) {
		computeBoundings(p, &helper);
	}

	GLuint list = listBase + m3ds->current_frame;
	glNewList(list, GL_COMPILE);
		glPushMatrix();


		// TODO: use BosonHelper::diff*() ! they can be used as
		// glTranslate here! (the center of the gl coordinate system
		// should actually be the *center* of the model)
		
		// note: we use BO_GL_CELL_SIZE as default size in *all*
		// directions. one day we should use UnitProperties::unitWidth()
		// and so on here - but for now we use this and re-scale it
		// later (in Unit or so).
		if (i == 0) {
			// we compute scaling for the first frame only.
			scale = helper.scale(mWidth, mHeight);
			kdDebug() << "master scale: " << scale << endl;
		}

		// FIXME: can we do our own calculations here, instead of OpenGL
		// scaling? i.e. try to use lib3ds scaling!
		glScalef(scale, scale, scale);

		// we render from bottom to top - but for x and y in the center!
		// FIXME: this doesn't work 100% correctly - the quad e.g. is
		// still partially (parts of the wheels) in the grass.
		glTranslatef(0.0, 0.0, -helper.mMinZ * scale);


		// a .3ds file consists of nodes. I don't *know* it, but I'm
		// pretty sure that a 3ds node is simply an object of the whole
		// object file. One object (node) for example may be a "wheel".
		// A node can have child-nodes, too.
		// Here we parse all top-level nodes. child nodes get parsed in
		// renderNode().
		for (p = m3ds->nodes; p; p = p->next) {
			renderNode(p);
		}
		glPopMatrix();
	glEndList();

	BoFrame* frame = new BoFrame;
	frame->setDisplayList(list);
	frame->setDepthMultiplier(helper.lengthZ() * scale / BO_GL_CELL_SIZE);
//	frame->setDepthMultiplier(helper.lengthZ() / BO_GL_CELL_SIZE);
	mFrames.insert(m3ds->current_frame, frame);
 }
}

void BosonModel::generateConstructionLists()
{
 if (!m3ds) {
	kdError() << k_funcinfo << "NULL file" << endl;
	return;
 }
 // construction lists are always generated from the 1st frame!
 m3ds->current_frame = 0;
 lib3ds_file_eval(m3ds, m3ds->current_frame);

 BoFrame* frame0 = mFrames[0];
 if (!frame0) {
	kdError() << k_funcinfo << "No frame was loaded yet!" << endl;
	return;
 }
 unsigned int nodes = 0;
 Lib3dsNode* node;
 for (node = m3ds->nodes; node; node = node->next) {
	nodes++;
 }
 kdDebug() << k_funcinfo << "Generating " << nodes << " construction lists" << endl;

 // again this iterating... it is exactly the same as it was for the first frame
 // in createDisplayLists() but I don't want to store it somewhere. Especially
 // not for *all* frames - would take quite same memory. after startup we don't
 // need this helper anymore!
 BoHelper helper;
 for (node = m3ds->nodes; node; node = node->next) {
	computeBoundings(node, &helper);
 }

 float scale = helper.scale(mWidth, mHeight);
 GLuint base = glGenLists(nodes);
 if (base == 0) {
	kdError() << k_funcinfo << "NULL display lists created" << endl;
	return;
 }
 for (unsigned int i = 0; i < nodes; i++) {
	GLuint list = base + i;
	BoFrame* step = new BoFrame(*frame0);

	unsigned int j = 0;
	// AB: FIXME: this code is pretty much redundant. we use the same code as
	// in createDisplayLists(), but for a limted number of nodes only. we
	// could merge both in a new function!
	glNewList(list, GL_COMPILE);
		glPushMatrix();
		glScalef(scale, scale, scale);
		glTranslatef(0.0, 0.0, -helper.mMinZ * scale);
		for (node = m3ds->nodes; node && j <= i; node = node->next) {
			renderNode(node);
			j++;
		}
		glPopMatrix();
	glEndList();
	step->setDisplayList(list);
	mConstructionSteps.insert(i, step);
 }
}

void BosonModel::computeBoundings(Lib3dsNode* node, BosonModel::BoHelper* helper)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		computeBoundings(p, helper);
	}
 }

 if (node->type == LIB3DS_OBJECT_NODE) {
	if (strcmp(node->name, "$$$DUMMY") == 0) {
		return;
	}
	Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name);
	if (!mesh) {
		return;
	}
	unsigned int p;
	Lib3dsMatrix invMeshMatrix;
	lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
	lib3ds_matrix_inv(invMeshMatrix);

	for (p = 0; p < mesh->faces; ++p) {
		Lib3dsFace* f = &mesh->faceL[p];

		Lib3dsVector v[3];
		for (int i = 0; i < 3; i++) {
			lib3ds_vector_transform(v[i], invMeshMatrix, mesh->pointL[f->points[i]].pos);
			Lib3dsObjectData* d = &node->data.object;
			Lib3dsVector tmp, actualPoint;
			lib3ds_vector_copy(actualPoint, v[i]);
			lib3ds_vector_sub(actualPoint, actualPoint, d->pivot);
			lib3ds_vector_copy(tmp, actualPoint);
			lib3ds_vector_transform(actualPoint, node->matrix, tmp);
			helper->addPoint(actualPoint[0], actualPoint[1], actualPoint[2]);
		}
	}
 }
}

void BosonModel::loadNodes(bool reload)
{
 if (!QGLContext::currentContext()) {
	kdError() << k_funcinfo << "NULL current context" << endl;
	return;
 }
 if (reload) {
	kdDebug() << k_funcinfo << "reloading all nodes" << endl;
 } else {
	kdDebug() << k_funcinfo << "loading all nodes" << endl;
 }
 Lib3dsNode* p;
 for (p = m3ds->nodes; p; p = p->next) {
	loadNode(p, reload);
 }
}

void BosonModel::loadNode(Lib3dsNode* node, bool reload)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		loadNode(p, reload);
	}
 }
 if (node->type == LIB3DS_OBJECT_NODE) {
	if (strcmp(node->name, "$$$DUMMY") == 0) {
		return;
	}
	// we create a display list for *every single* node.
	// This might look like a lot of overhead (both memory and speed - we
	// need to use glCallList() every time which means more function calls)
	// but it is actually less - e.g. in animations we don't have separate
	// display lists for node that didn't change at all (less memory!)

	Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name);
	if (!mesh) {
		return;
	}
	if (!node->user.d) {
		node->user.d = glGenLists(1);
		if (node->user.d == 0) {
			kdError() << k_funcinfo << "NULL display list created" << endl;
			return;
		}
	} else {
		if (reload) {
			// it is important that the list is deleted, but the
			// newly generated list must have the *same* number as
			// it had before!
			glDeleteLists(node->user.d, 1);
		} else {
			kdWarning() << k_funcinfo << "node was already loaded before" << endl;
			return;
		}
	}

	unsigned int p;
	Lib3dsMatrix invMeshMatrix;
	lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
	lib3ds_matrix_inv(invMeshMatrix);
	GLuint myTex = 0;
	bool resetColor = false; // needs to be true after we changed the current color

	// AB: we have *lots* of faces! in numbers the maximum i found
	// so far (only a short look) was about 25 toplevel nodes and
	// rarely child nodes. sometimes 2 child nodes or so - maybe 10
	// per model (if at all).
	// but we have up to (short look only) 116 faces *per node*
	// usually it's about 10-20 faces (minimum) per node!
	//
	// so optimization should happen here - if possible at all...


	// AB: WARNING: from now on we demand that all faces in a mesh
	// use the same material and therefore the same texture!
	// --> maybe we want to change this one day in order to provide
	// more details or so. but i hope for now we gain some speed of
	// it. so we can group all faces into a single glBegin() call!
	Lib3dsMaterial* mat = 0;
	for (p = 0; p < mesh->faces; ++p) {
		Lib3dsFace* f = &mesh->faceL[p];
		Lib3dsMaterial* mat2 = 0;
		if (f->material[0]) {
			mat2 = lib3ds_file_material_by_name(m3ds, f->material);
			if (p == 0) {
				mat = mat2;
			}
			if (mat != mat2) {
				kdWarning() << "face " << p << " uses different material than previous faces" << endl;
			}
		}
	}

	if (mat) {
		// this is the texture map of the object.
		// t->name is the (file-)name and in
		// mesh->texelL you can find the texture
		// coordinates for glTexCoord*()
		// note that mesh->texels can be 0 - then the
		// mesh doesn't have any texture. otherwise it
		// must be equal to mesh->points
		Lib3dsTextureMap* t = &mat->texture1_map;
		QString texName = cleanTextureName(t->name);
		if (texName.isEmpty()) {
			myTex = 0;
		} else {
			myTex = mModelTextures->texture(texName);
			if (!myTex) {
				kdWarning() << k_funcinfo << "Texture " << t->name << " was not loaded" << endl;
			}
		}
	} else {
		//...
		myTex = 0;
	}

	BoMatrix texMatrix;
	if (mat && myTex) {
		// *ggg* this is a nice workaround.
		// it's hard to do this with a Lib3dsMatrix by several
		// reasons - so we do these calculations in OpenGL
		// (*NOT* in a display list - immediate mode) and then
		// get the texture matrix.
		// With this matrix we can easily calculate the actual
		// texture coordinates.
		// btw: this means that all calculations are done only
		// once (on startup) and therefore we'll have some
		// speedup

		// AB: this part isn't time critical anymore. you can
		// call even OpenGL commands without runtime slowdowns.
		// We calculate the actual texture map coordinates only
		// once and then use the final calculations in the
		// display list.
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity(); // should already be there
		Lib3dsTextureMap* t = &mat->texture1_map;
		if (t->scale[0] || t->scale[1]) {
			glScalef(t->scale[0], t->scale[1], 1.0);
		}
		glTranslatef(t->offset[0], t->offset[1], 0.0);
		if (t->rotation != 0.0) {
			glRotatef(t->rotation, 0.0, 0.0, 1.0);
		}
		glTranslatef(mesh->map_data.pos[0], mesh->map_data.pos[1], mesh->map_data.pos[2]);
		float scale = mesh->map_data.scale;
		if (scale != 0.0 && scale != 1.0) {
			// doesn't seem to be used in our models
			glScalef(scale, scale, 1.0);
		}
		texMatrix.loadMatrix(GL_TEXTURE_MATRIX);
		if (texMatrix.isNull()) {
			kdWarning() << k_funcinfo << "Invalid texture matrix was generated!" << endl;
			texMatrix.loadIdentity();
		}

		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	} else {
		// we use the default identity matrix - this is just "in case".
		// we must not use this matrix at all!
	}

	// now start the actual display list for this node.
	glNewList(node->user.d, GL_COMPILE);
	mNodeDisplayLists.append(node->user.d);
	glBindTexture(GL_TEXTURE_2D, myTex);

	glBegin(GL_TRIANGLES); // note: you shouldn't do calculations after a glBegin() but we compile a display list only, so its ok
	for (p = 0; p < mesh->faces; p++) {
		Lib3dsFace* f = &mesh->faceL[p];
		Lib3dsVector v[3];
		Lib3dsTexel tex[3];
		for (int i = 0; i < 3; i++) {
			lib3ds_vector_transform(v[i], invMeshMatrix, mesh->pointL[ f->points[i] ].pos);
			if (mesh->texels != mesh->points) {
				if (mesh->texels != 0) {
					kdWarning() << k_funcinfo << "hmm.. points: " << mesh->points
							<< " , texels: " << mesh->texels << endl;
				}
				myTex = 0;
			} else {
				// mesh->texelL[f->points[i]] is our
				// vector. it has x and y only, z is
				// therefore 0.0
				BoVector3 a;
				BoVector3 b;
				a.set(mesh->texelL[f->points[i]][0], mesh->texelL[f->points[i]][1], 0.0);
				texMatrix.transform(&b, &a);
				tex[i][0] = b[0];
				tex[i][1] = b[1];
			}
		}
		if (QString::fromLatin1(mesh->name).find("teamcolor", 0, false) == 0) {
			myTex = 0; // teamcolor objects are *not* textured
			if (mTeamColor) {
				glColor3ub((GLubyte)mTeamColor->red(), (GLubyte)mTeamColor->green(), (GLubyte)mTeamColor->blue());
				resetColor = true;
			}
		}
		if (myTex) {
			glTexCoord2fv(tex[0]); glVertex3fv(v[0]);
			glTexCoord2fv(tex[1]); glVertex3fv(v[1]);
			glTexCoord2fv(tex[2]); glVertex3fv(v[2]);
		} else {
			glVertex3fv(v[0]);
			glVertex3fv(v[1]);
			glVertex3fv(v[2]);
		}
		
	}
	glEnd();
	if (resetColor) {
		glColor3f(1.0, 1.0, 1.0);
	}
	glEndList();
 }
}

void BosonModel::renderNode(Lib3dsNode* node)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		renderNode(p);
	}
 }
 if (node->type == LIB3DS_OBJECT_NODE) {
	if (strcmp(node->name, "$$$DUMMY") == 0) {
		return;
	}
	if (node->user.d) {
		glPushMatrix();
		Lib3dsObjectData* d = &node->data.object; // these can get changed in different frames even for the same nodes. so we can't place the following parts into the display lists for the nodes themselves (see loadNode())
		
		// I assume thats e.g. the rotation of the node. maybe
		// even scaling.
		glMultMatrixf(&node->matrix[0][0]);

		// the pivot point is the center of the object, I guess.
		glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);

		// finally call the list, which was created in loadNode()
		glCallList(node->user.d);

		glPopMatrix();
	}
 }
}

BoFrame* BosonModel::frame(unsigned int frame) const
{
 return mFrames[frame];
}

BoFrame* BosonModel::constructionStep(unsigned int step)
{
 if (step >= mConstructionSteps.count()) {
	step = mConstructionSteps.count() - 1;
 }
 return mConstructionSteps[step];
}

void BosonModel::setTeamColor(const QColor& c)
{
 delete mTeamColor;
 mTeamColor = new QColor(c);
}

void BosonModel::finishLoading()
{
 delete mTeamColor;
 mTeamColor = 0;
 if (m3ds) {
	lib3ds_file_free(m3ds);
	m3ds = 0;
 }
 delete mTeamColor;
 mTeamColor = 0;
 mTextureNames.clear();
}

void BosonModel::dumpVector(Lib3dsVector v)
{
 kdDebug() << "Vector: " << v[0] << "," << v[1] << "," << v[2] << endl;
}

void BosonModel::dumpTriangle(Lib3dsVector* v, GLuint texture, Lib3dsTexel* tex)
{
 QString text = "triangle: ";
 for (int i = 0; i < 3; i++) {
	text += QString("%1,%2,%3").arg(v[i][0]).arg(v[i][1]).arg(v[i][2]);
	text += " ; ";
 }
 if (texture && tex) {
	text += QString("texture=%1-->").arg(texture);
	for (int i = 0; i < 3; i++) {
		text += QString("%1,%2").arg(tex[i][0]).arg(tex[i][1]);
		if (i < 2) {
			text += " ; ";
		}
	}
 } else {
	text += "(no texture)";
 }
 kdDebug() << text << endl;
}

void BosonModel::reloadAllTextures()
{
 kdDebug() << k_funcinfo << endl;
 if (!mModelTextures) {
	kdError() << k_funcinfo << "NULL model textures ?!?!" << endl;
	return;
 }
 mModelTextures->reloadTextures();
}

