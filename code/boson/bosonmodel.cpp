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
#include "bosontexturearray.h"

#include <kdebug.h>

#include <qimage.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/material.h>

BosonModel::BosonModel(GLuint list, int width, int height)
{
 init();
 mDisplayList = list;
 mFrames.insert(0, list);
 mWidth = width;
 mHeight = height;
}

BosonModel::BosonModel(const QString& dir, const QString& file)
{
 init();
 mDirectory = dir;
 QString fullFile = baseDirectory() + file;
 m3ds = lib3ds_file_load(fullFile);
 if (!m3ds) {
	kdError() << k_funcinfo << "Can't load " << fullFile << endl;
	return;
 }
// kdDebug() << k_funcinfo << "current frame: " << m3ds->current_frame << endl;
 lib3ds_file_eval(m3ds, m3ds->current_frame);

 Lib3dsNode* node = m3ds->nodes;
 if (!node) {
	kdError() << k_funcinfo << "Could not load file " << fullFile << " correctly" << endl;
	return;
 }
 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
 loadTextures();
 createDisplayLists();
 
 if (!mDisplayList) {
	kdError() << k_funcinfo << "Still null display list" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "loaded from " << fullFile << endl;

 // WARNING: FIXME!
 mWidth = BO_TILE_SIZE;
 mHeight = BO_TILE_SIZE;
}

void BosonModel::init()
{
 m3ds = 0;
 mDisplayList = 0;
 mTextureArray = 0;
 mWidth = 0;
 mHeight = 0;
 mFrame = 0;
}

BosonModel::~BosonModel()
{
 if (m3ds) {
	lib3ds_file_free(m3ds);
	m3ds = 0;
 }
 mTextures.clear();
 delete mTextureArray;
}

QString BosonModel::textureDirectory() const
{
 return baseDirectory() + QString::fromLatin1("textures/"); 
}

QString BosonModel::cleanTextureName(const char* name)
{
 QString s = QString(name).lower();
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
 QValueList<QImage> images;
 QValueList<QString> names;
 Lib3dsMaterial* mat;
 for (mat = m3ds->materials; mat; mat = mat->next) {
	Lib3dsTextureMap* t = &mat->texture1_map;
	QString texName = cleanTextureName(t->name);
	QImage image(textureDirectory() + texName);
	if (image.isNull()) {
		kdError() << k_funcinfo << "NULL image: " << textureDirectory() + texName << endl;
		image = QImage(64, 64, 32);
		image.fill(Qt::red.rgb()); // evil dummy image
	}
	images.append(image);
	names.append(t->name);
 }
 mTextureArray = new BosonTextureArray(images);
 for (unsigned int i = 0; i < mTextureArray->count(); i++) {
	GLuint list = mTextureArray->texture(i);
	mTextures.insert(names[i], list);
 }
}

void BosonModel::createDisplayLists()
{
 Lib3dsNode* node = m3ds->nodes;

 kdDebug() << k_funcinfo << "creating " << m3ds->frames << " lists" << endl;
 GLuint listBase = glGenLists(m3ds->frames); 
 for (int i = 0; i < m3ds->frames; i++) {
	m3ds->current_frame = i;
	lib3ds_file_eval(m3ds, m3ds->current_frame);
	glNewList(listBase + m3ds->current_frame, GL_COMPILE);
		glPushMatrix();
		glScalef(0.004, 0.004, 0.004); // FIXME
		for (; node; node = node->next) {
			renderNode(node);
		}
		glPopMatrix();
	glEndList();
	mFrames.insert(m3ds->current_frame, listBase + m3ds->current_frame);
 }
 mDisplayList = listBase; //AB: FIXME
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
#if 0
	// the original implementation creates a display list for ALL nodes.
	// to use this with frames we'd have to re-assign 0 to user.d between
	// all frames.
	// I'm not sure which is better - storing all GL calls in a single
	// display list or using a lot of glCallList() in the main list.
	//
	// I am sure that re-rendering every time is slower on loading.. but
	// what about runtime?
	if (!node->user.d) {
#endif
		Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name);
		if (!mesh) {
			return;
		}
#if 0
		node->user.d = glGenLists(1);
		glNewList(node->user.d, GL_COMPILE);
#endif

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
				Lib3dsTextureMap* t = &mat->texture1_map;
				if (!mTextures.contains(t->name)) {
					if (QString(t->name) != "") {
						kdWarning() << k_funcinfo << "Texture " << t->name << " was not loaded" << endl;
					}
					myTex = 0;
				} else {
					myTex = mTextures[t->name];
				}
			} else {
				//...
				myTex = 0;
			}
	
			{
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
				if (myTex) {
					glBindTexture(GL_TEXTURE_2D, myTex);
					glBegin(GL_TRIANGLES);
						glTexCoord2fv(tex[0]); glVertex3fv(v[0]);
						glTexCoord2fv(tex[1]); glVertex3fv(v[1]);
						glTexCoord2fv(tex[2]); glVertex3fv(v[2]);
					glEnd();
				} else {
					glBegin(GL_TRIANGLES);
						glVertex3fv(v[0]);
						glVertex3fv(v[1]);
						glVertex3fv(v[2]);
					glEnd();
				}
			}
		}
#if 0
		glEndList();
#endif
	}
#if 0
	if (node->user.d) {
		glPushMatrix();
		Lib3dsObjectData* d = &node->data.object;
		glMultMatrixf(&node->matrix[0][0]);
		glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);
		glCallList(node->user.d);
		glPopMatrix();
	}
 }
#endif
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
}

