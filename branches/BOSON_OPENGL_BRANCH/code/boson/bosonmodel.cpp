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

#include <kdebug.h>

#include <lib3ds/file.h>
#include <lib3ds/camera.h> // we probably don't need this!
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>

BosonModel::BosonModel(GLuint list)
{
 init();
 mDisplayList = list;
}

BosonModel::BosonModel(const QString& f)
{
 init();
 QString file = f;
// QString file = "/home/guest/3ds/3ds/andi3d/sphere.3DS"; // AB: temporary for debugging
 m3ds = lib3ds_file_load(file);
 if (!m3ds) {
	kdError() << k_funcinfo << "Can't load " << file << endl;
	return;
 }
// kdDebug() << k_funcinfo << "current frame: " << m3ds->current_frame << endl;
 lib3ds_file_eval(m3ds, m3ds->current_frame);
 
 Lib3dsNode* node = m3ds->nodes;
 if (!node) {
	kdError() << k_funcinfo << "Could not load file " << file << " correctly" << endl;
	return;
 }
 renderNode(node);
 
 mDisplayList = node->user.d;

 if (!mDisplayList) {
	kdError() << k_funcinfo << "Still null display list" << endl;
	return;
 }
 kdDebug() << k_funcinfo << "loaded from " << file << endl;
}

void BosonModel::init()
{
 m3ds = 0;
 mDisplayList = 0;
}

BosonModel::~BosonModel()
{
 if (m3ds) {
	lib3ds_file_free(m3ds);
	m3ds = 0;
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
	if (!node->user.d) {
		Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name); // FIXME: what is this?
		if (!mesh) {
			return;
		}
	glColor3f(1.0,0.0,0.0);
		node->user.d = glGenLists(1);
		glNewList(node->user.d, GL_COMPILE);

		unsigned int p;
		Lib3dsMatrix invMeshMatrix;
		lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
		lib3ds_matrix_inv(invMeshMatrix);

		for (p = 0; p < mesh->faces; ++p) {
			Lib3dsFace &f = mesh->faceL[p];
			//...


			{
				//..
				int i;
				Lib3dsVector v[3];
				for (i = 0; i < 3; i++) {
					lib3ds_vector_transform(v[i], invMeshMatrix, mesh->pointL[f.points[i]].pos);
				}
	glColor3f(1.0,0.0,0.0);
	glPushMatrix();
 glScalef(0.01,0.01,0.01);
				glBegin(GL_TRIANGLES);
//					glNormal3fv(f.normal);
//					kdDebug() << v[0][0] << "," << v[0][1] << ","<<v[0][2] << endl;
					glVertex3fv(v[0]);
					glVertex3fv(v[1]);
					glVertex3fv(v[2]);
				glEnd();
	glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
			}
		}
		glEndList();
	}
	if (node->user.d) {
	glColor3f(1.0,0.0,0.0);
		glPushMatrix();
		Lib3dsObjectData* d = &node->data.object;
		glMultMatrixf(&node->matrix[0][0]);
		glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);
		glCallList(node->user.d);
		glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
	}
 }
}

