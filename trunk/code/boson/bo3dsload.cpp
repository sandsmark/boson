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

#include "bo3dsload.h"

#include "defines.h"
#include "bo3dtools.h"
#include "bosonglwidget.h"
#include "bomesh.h"
#include "bodebug.h"
#include "bosonmodel.h"

#include <ksimpleconfig.h>
#include <kstaticdeleter.h>

#include <qimage.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qintdict.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/material.h>

Bo3DSLoad::Bo3DSLoad(const QString& dir, const QString& file, BosonModel* data)
{
 init();
 mDirectory = dir;
 mFile = file;
 mData = data;
}

void Bo3DSLoad::init()
{
 m3ds = 0;
}

Bo3DSLoad::~Bo3DSLoad()
{
 boDebug(100) << k_funcinfo << endl;
 finishLoading();
 boDebug(100) << k_funcinfo << "done" << endl;
}

const QString& Bo3DSLoad::baseDirectory() const
{
 return mDirectory;
}

QString Bo3DSLoad::file() const
{
 return baseDirectory() + mFile;
}

QStringList Bo3DSLoad::textures() const
{
 return textures(m3ds);
}

QStringList Bo3DSLoad::textures(Lib3dsFile* file)
{
 QStringList list;
 if (!file) {
	boError(100) << k_funcinfo << "NULL file" << endl;
	return list;
 }

 // note that it is not neceassary to loop through all frames, since other
 // frames do *not* (never!) contain other textures
 Lib3dsMaterial* mat;
 for (mat = file->materials; mat; mat = mat->next) {
	Lib3dsTextureMap* t = &mat->texture1_map;
	QString texName = t->name;
	if (texName.isEmpty()) {
		continue;
	}
	list.append(texName);
 }
 return list;
}

void Bo3DSLoad::loadModel()
{
 boDebug(100) << k_funcinfo << endl;
 if (mFile.isEmpty() || mDirectory.isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return;
 }
 m3ds = lib3ds_file_load(file());
 if (!m3ds) {
	boError(100) << k_funcinfo << "Can't load " << file() << endl;
	return;
 }
 Lib3dsNode* node = m3ds->nodes;;
 if (!node) {
	boError(100) << k_funcinfo << "Could not load file " << file() << " correctly" << endl;
	return;
 }
 if (m3ds->frames < 1) {
	boError(100) << k_funcinfo << "No frames in " << file() << endl;
	return;
 }
 for (; node; node = node->next) {
	loadMesh(node);
 }
 if (mData->addFrames(m3ds->frames) != 0) {
	boError() << "offset != 0 is not supported here" << endl;
	return;
 }
 for (int i = 0; i < m3ds->frames; i++) {
	loadFrame(i);
 }
 boDebug(100) << k_funcinfo << "loaded from " << file() << endl;
}

void Bo3DSLoad::loadMesh(Lib3dsNode* node)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		loadMesh(p);
	}
 }
 if (node->type != LIB3DS_OBJECT_NODE) {
	return;
 }
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
 if (mMesh2Mesh[mesh]) {
	// already loaded
	return;
 }
 if (mesh->faces < 1) {
	boWarning() << k_funcinfo << "no faces in " << mesh->name << " of " << file() << endl;
	return;
 }
 if (mesh->texels != 0 && mesh->texels != mesh->points) {
	boError(100) << k_funcinfo << "hmm.. points: " << mesh->points
			<< " , texels: " << mesh->texels << endl;
	return;
 }

 QString textureName = BoMesh::textureName(mesh, m3ds);

 BoMesh* boMesh = new BoMesh(mesh);
 mMesh2Mesh.insert(mesh, boMesh);
 mData->addMesh(boMesh);
 mData->setTexture(boMesh, textureName);

 boMesh->setMaterial(BoMesh::material(mesh, m3ds));
 boMesh->setTextured(!textureName.isEmpty()); // this may get changed if BosonModel::cleanTextureName() can't find our texture

 boMesh->loadPoints();

// boMesh->setTextureObject(myTex);
}

void Bo3DSLoad::loadFrame(int frame)
{
 // A .3ds file can contain several frames. A different frame of the same file
 // looks slightly differnt - e.g. useful for animation (mainly even).
 // You can read a frame with lib3ds by calling lib3ds_file_eval() first for
 // every frame.
 // lib3ds_file_eval modifies node->matrix and the node->data only! it doesn't
 // change the meshes or textures!
 lib3ds_file_eval(m3ds, frame);
 m3ds->current_frame = frame;
 BoFrame* f = mData->frame(frame);
 BO_CHECK_NULL_RET(f);

 int nodes = 0;
 Lib3dsNode* node;
 for (node = m3ds->nodes; node; node = node->next) {
	countNodes(node, &nodes);
 }

 // note: a single mesh can be used multiple times per frame!
 f->allocMeshes(nodes);

 int index = 0;
 for (node = m3ds->nodes; node; node = node->next) {
	loadFrameNode(f, &index, node);
 }
}

void Bo3DSLoad::countNodes(Lib3dsNode* node, int* nodes)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		countNodes(p, nodes);
	}
 }
 if (node->type != LIB3DS_OBJECT_NODE) {
	return;
 }
 if (strcmp(node->name, "$$$DUMMY") == 0) {
	return;
 }
 Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(m3ds, node->name);
 if (!mesh) {
	return;
 }
 if (mesh->faces < 1) {
	// can actually happen! e.g. our ship does that currently!
	// (i consider these files broken!)
	return;
 }
 (*nodes)++;
}

void Bo3DSLoad::loadFrameNode(BoFrame* frame, int* index, Lib3dsNode* node)
{
 BO_CHECK_NULL_RET(frame);
 BO_CHECK_NULL_RET(index);
 BO_CHECK_NULL_RET(node);
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		loadFrameNode(frame, index, p);
	}
 }
 if (node->type != LIB3DS_OBJECT_NODE) {
	return;
 }
 if (strcmp(node->name, "$$$DUMMY") == 0) {
	return;
 }
 Lib3dsMesh* mesh3ds = lib3ds_file_mesh_by_name(m3ds, node->name);
 if (!mesh3ds) {
	return;
 }
 if (mesh3ds->faces < 1) {
	return;
 }
 BoMesh* mesh = mMesh2Mesh[mesh3ds];
 BO_CHECK_NULL_RET(mesh);
 frame->setMesh(*index, mesh);

 // the matrix is already allocated in the frame - we just need to modify it.
 BoMatrix* m = frame->matrix(*index);

 // according to the lib3ds code (node.c, lib3ds_node_eval()) this is the
 // position, the rotation and the scaling of this node and if this node has a
 // parent the matrix of the parent is applied as well (first parent, then this
 // node).
 m->loadMatrix(&node->matrix[0][0]);

 // the pivot point is the center of the object, I guess.
 Lib3dsObjectData* d = &node->data.object;
 m->translate(-d->pivot[0], -d->pivot[1], -d->pivot[2]);



 (*index)++;
}

void Bo3DSLoad::finishLoading()
{
 if (m3ds) {
	lib3ds_file_free(m3ds);
	m3ds = 0;
 }
}

void Bo3DSLoad::dumpVector(Lib3dsVector v)
{
 boDebug(100) << "Vector: " << v[0] << "," << v[1] << "," << v[2] << endl;
}

void Bo3DSLoad::dumpTriangle(Lib3dsVector* v, GLuint texture, Lib3dsTexel* tex)
{
 BoVector3 vector[3];
 for (int i = 0; i < 3; i++) {
	vector[i].set(v[i]);
 }
 dumpTriangle(vector, texture, tex);
}

void Bo3DSLoad::dumpTriangle(BoVector3* v, GLuint texture, Lib3dsTexel* tex)
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
 boDebug(100) << text << endl;
}


void debugTex(Lib3dsMaterial* mat, Lib3dsMesh* mesh)
{
 Lib3dsTextureMap* t = &mat->texture1_map;
 boDebug(100) << mat->name << " -- " << t->name << endl;
 boDebug(100) << "rot: " << t->rotation
		<< " offset: " << t->offset[0] << "," << t->offset[1]
		<< " scale: " << t->scale[0] << "," << t->scale[1]
		<< " flags: " << t->flags
		<< endl;
 boDebug(100) << "map_data scale: " << mesh->map_data.scale
		<< " pos: " << mesh->map_data.pos[0]
			<< "," << mesh->map_data.pos[1]
			<< "," << mesh->map_data.pos[2]
		<< " type: " << mesh->map_data.maptype
		<< " tile: " << mesh->map_data.tile[0] << "," << mesh->map_data.tile[1]
		<< endl;
}

