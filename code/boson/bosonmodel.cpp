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

BosonModel::BosonModel(GLuint list, int width, int height)
{
 // dummy implementation - we don't load a model, but rather use the provided
 // list.
 // Mostly used because we do not yet have models for all units
 boProfiling->start(BosonProfiling::LoadModelDummy);
 init();
 mDisplayList = list;
 mFrames.insert(0, list);
 mWidth = width;
 mHeight = height;
 boProfiling->stop(BosonProfiling::LoadModelDummy);
}

BosonModel::BosonModel(const QString& dir, const QString& file)
{
 init();
 mDirectory = dir;
 mFile = file;
}

void BosonModel::init()
{
 m3ds = 0;
 mDisplayList = 0;
 mWidth = 0;
 mHeight = 0;
 mFrame = 0;
 if (!mModelTextures) { // TODO static deleter!
	mModelTextures = new BosonModelTextures();
 }
}

BosonModel::~BosonModel()
{
 kdDebug() << k_funcinfo << endl;
 mModelTextures->removeModel(this);
 if (m3ds) {
	lib3ds_file_free(m3ds); // we can probably free already after creating the display lists!
	m3ds = 0;
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
// kdDebug() << k_funcinfo << "current frame: " << m3ds->current_frame << endl;
 lib3ds_file_eval(m3ds, m3ds->current_frame);

 Lib3dsNode* node = m3ds->nodes;
 if (!node) {
	kdError() << k_funcinfo << "Could not load file " << fullFile << " correctly" << endl;
	boProfiling->stop(BosonProfiling::LoadModel);
	return;
 }
 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures();
 boProfiling->stop(BosonProfiling::LoadModelTextures);
 boProfiling->start(BosonProfiling::LoadModelDisplayLists);
 createDisplayLists();
 boProfiling->stop(BosonProfiling::LoadModelDisplayLists);

 if (!mDisplayList) {
	kdError() << k_funcinfo << "Still null display list" << endl;
	boProfiling->stop(BosonProfiling::LoadModel);
	return;
 }
 kdDebug() << k_funcinfo << "loaded from " << fullFile << endl;

 // WARNING: FIXME!
 mWidth = BO_TILE_SIZE;
 mHeight = BO_TILE_SIZE;
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
 int allPoints = 0;

 // ok so lets start now. A .3ds file can contain several frames. A different
 // frame of the same file looks slightly differnt - e.g. useful for animation
 // (mainly even).
 // You can read a frame with lib3ds by calling lib3ds_file_eval() first for
 // every frame.
 for (int i = 0; i < m3ds->frames; i++) {
	Lib3dsNode* p;
	m3ds->current_frame = i;
	// prepare the file for rendering the next frame.
	// Note: as far as I understund the lib3ds code this does *not* change
	// the object itself, but rather node->data.object.*
	// this means for us that we can continue to use the display lists in
	// node->user.d ; but we *have* to ensure that the new values from
	// node->data are used, i.e. they are not stored in that display list.
	// they should reside in the final list.
	lib3ds_file_eval(m3ds, m3ds->current_frame);

	// we render the same node twice - the first run creates the display
	// lists, the second just calls glCallList().
	// In the second call we store all display lists in a single list, which
	// gets finally called in boson.
	for (p = m3ds->nodes; p; p = p->next) {
		renderNode(p);
	}

	mPoints = 0;
	GLuint list = listBase + m3ds->current_frame;
	glNewList(list, GL_COMPILE);
		glPushMatrix();
		glScalef(0.004, 0.004, 0.004); // FIXME

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
	mFrames.insert(m3ds->current_frame, list);
//	kdDebug() << k_funcinfo << "points in frame " << m3ds->current_frame << ": " << mPoints << endl;
	allPoints += mPoints;
 }
 setFrame(frame());
 kdDebug() << k_funcinfo << "points in model (all frames): " << allPoints << endl;
}

// AB: note that this function was nearly completely copied from the lib3ds
// examples!
// i.e. it is very bad and probably too slow for us.
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
	// we create a display list for *every single* node.
	// This might look like a lot of overhead (both memory and speed - we
	// need to use glCallList() every time which means more function calls)
	// but it is actually less - e.g. in animations we don't have separate
	// display lists for node that didn't change at all (less memory!)

	if (!node->user.d) {
		//AB: what exactly is a "mesh" ?
		Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name);
		if (!mesh) {
			return;
		}
		node->user.d = glGenLists(1);
		glNewList(node->user.d, GL_COMPILE);

		unsigned int p;
		Lib3dsMatrix invMeshMatrix;
		lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
		lib3ds_matrix_inv(invMeshMatrix);
		GLuint myTex = 0;

		for (p = 0; p < mesh->faces; ++p) {
			Lib3dsFace* f = &mesh->faceL[p];
			Lib3dsMaterial* mat = 0;
			if (f->material[0]) {
				mat = lib3ds_file_material_by_name(m3ds, f->material);
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
	
			{
				mPoints += 3;
				Lib3dsVector v[3];
				Lib3dsTexel tex[3];
				for (int i = 0; i < 3; i++) {
					lib3ds_vector_transform(v[i], invMeshMatrix, mesh->pointL[f->points[i]].pos);
					if (mesh->texels != mesh->points) {
						if (mesh->texels != 0) {
							kdWarning() << k_funcinfo << "hmm.. points: " << mesh->points 
									<< " , texels: " << mesh->texels << endl;
						}
						myTex = 0;
					} else {
						tex[i][0] = mesh->texelL[f->points[i]][0];
						tex[i][1] = mesh->texelL[f->points[i]][1];
					}
				}
				if (QString::fromLatin1(mesh->name).lower() == QString::fromLatin1("teamcolor")) {
					myTex = 0; // teamcolor objects are *not* textured
//					glColor3f(); // TODO
				}
				if (myTex) {
					glBindTexture(GL_TEXTURE_2D, myTex);
					glBegin(GL_TRIANGLES);
						glTexCoord2fv(tex[0]); glVertex3fv(v[0]);
						glTexCoord2fv(tex[1]); glVertex3fv(v[1]);
						glTexCoord2fv(tex[2]); glVertex3fv(v[2]);
					glEnd();
				} else {
					glBindTexture(GL_TEXTURE_2D, 0);
					glBegin(GL_TRIANGLES);
						glVertex3fv(v[0]);
						glVertex3fv(v[1]);
						glVertex3fv(v[2]);
					glEnd();
				}
			}
		}
		glEndList();
	}
	if (node->user.d) {
		glPushMatrix();
		Lib3dsObjectData* d = &node->data.object; // these can get changed in different frames even for the same nodes. so we can't place the following parts into the display lists for the nodes themselves.
		
		// I assume thats e.g. the rotation of the node. maybe
		// even scaling.
		glMultMatrixf(&node->matrix[0][0]);
		
		// the pivot point is the center of the object, I guess.
		glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);

		// finally call the list
		glCallList(node->user.d);

		glPopMatrix();
	}
 }
}

void BosonModel::setFrame(unsigned int frame)
{
 if (!mFrames.contains(frame)) {
	kdWarning() << k_funcinfo << "Invalid frame " << frame << endl;
	if (!mFrames.contains(mFrame)) {
		mFrame = 0;
	}
	return;
 }
 mFrame = frame;
 mDisplayList = mFrames[mFrame];
}

