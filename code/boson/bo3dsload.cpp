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

#include "bo3dsload.h"

#include "bo3dtools.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonmodel.h"
#include "bosonmodelloader.h"
#include "bodebug.h"

#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/material.h>

Bo3DSLoad::Bo3DSLoad(const QString& dir, const QString& file, BosonModelLoaderData* data)
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

bool Bo3DSLoad::loadModel()
{
 boDebug(100) << k_funcinfo << endl;
 if (mFile.isEmpty() || mDirectory.isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return false;
 }
 if (!mData) {
	BO_NULL_ERROR(mData);
	return false;
 }
 m3ds = lib3ds_file_load(file());
 if (!m3ds) {
	boError(100) << k_funcinfo << "Can't load " << file() << endl;
	return false;
 }

 if (!loadMaterials(mData, m3ds->materials)) {
	boError() << k_funcinfo << "unable to load materials" << endl;
	return false;
 }

 Lib3dsNode* node = m3ds->nodes;;
 if (!node) {
	boError(100) << k_funcinfo << "Could not load file " << file() << " correctly" << endl;
	return false;
 }
 if (m3ds->frames < 1) {
	boError(100) << k_funcinfo << "No frames in " << file() << endl;
	return false;
 }
 for (; node; node = node->next) {
	loadMesh(node);
 }
 for (int i = 0; i < m3ds->frames; i++) {
	int index = (int)mData->addFrame();
	if (index != i) {
		boError() << k_funcinfo << "frame " << i << " has unexpected index " << index << endl;
		return false;
	}
	loadFrame(i);
 }
 boDebug(100) << k_funcinfo << "loaded from " << file() << endl;
 return true;
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
	boWarning() << k_funcinfo << "no faces in mesh " << mesh->name << " of " << file() << endl;
	return;
 }
 if (mesh->points < 1) {
	boWarning() << k_funcinfo << "no points in mesh " << mesh->name << " of " << file() << endl;
	return;
 }
 if (mesh->texels != 0 && mesh->texels != mesh->points) {
	boError(100) << k_funcinfo << "hmm.. points: " << mesh->points
			<< " , texels: " << mesh->texels << endl;
	return;
 }

 QString textureName = Bo3DSLoad::textureName(mesh, m3ds);

 BoMesh* boMesh = new BoMesh(mesh->faces, node->name);
 mMesh2Mesh.insert(mesh, boMesh);
 mData->addMesh(boMesh);

 Lib3dsMaterial* mat = Bo3DSLoad::material(mesh, m3ds);
 BoMaterial* material = 0;
 if (mat) {
	unsigned int i = 0;
	Lib3dsMaterial* m = m3ds->materials;
	while (m && m != mat) {
		i++;
		m = m->next;
	}
	if (!m) {
		// we must use one valid index.
		i = 0;
	}
	material = mData->material(i);
 } else {
	// the mesh has no material. most probably a teamcolor object.
	material = 0;
 }
 boMesh->setMaterial(material);

 boMesh->setIsTeamColor(Bo3DSLoad::isTeamColor(mesh));

 boMesh->allocatePoints(mesh->points);
 loadVertices(boMesh, mesh);
 loadTexels(boMesh, mesh, Bo3DSLoad::material(mesh, m3ds));

 loadFaces(boMesh, mesh);
}

bool Bo3DSLoad::loadFrame(int frame)
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
 if (!f) {
	BO_NULL_ERROR(f);
	return false;
 }

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
 return true;
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
	boWarning(100) << k_funcinfo << "NULL mesh for node " << node->name << endl;
	return;
 }
 if (mesh3ds->faces < 1) {
	boWarning(100) << k_funcinfo << "no faces in node " << node->name << endl;
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
 BoVector3Float vector[3];
 for (int i = 0; i < 3; i++) {
	vector[i].set(v[i]);
 }
 dumpTriangle(vector, texture, tex);
}

void Bo3DSLoad::makeVectors(BoVector3Float* v, const Lib3dsMesh* mesh, const Lib3dsFace* face)
{
  // Lib3dsFace stores only the position (index) of the
  // actual point. the actual points are in mesh->pointL
  v[0].set(mesh->pointL[ face->points[0] ].pos);
  v[1].set(mesh->pointL[ face->points[1] ].pos);
  v[2].set(mesh->pointL[ face->points[2] ].pos);
}

void Bo3DSLoad::dumpTriangle(BoVector3Float* v, GLuint texture, Lib3dsTexel* tex)
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

void Bo3DSLoad::findAdjacentFaces(QPtrList<Lib3dsFace>* adjacentFaces, Lib3dsMesh* mesh, Lib3dsFace* search)
{
 if (!adjacentFaces || !mesh) {
	return;
 }

 // add all available faces to a list.
 QPtrList<Lib3dsFace> faces;
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* face = &mesh->faceL[i];
	if (face == search) {
		// no need to add this to the list of available faces
		continue;
	}
	faces.append(face);
 }

 if (!search) {
	search = &mesh->faceL[0];
 }
 adjacentFaces->append(search); // always adjacent to itself :)

 for (unsigned int i = 0; i < adjacentFaces->count(); i++) {
	QPtrList<Lib3dsFace> found; // these need to get removed from faces list
	BoVector3Float current[3]; // the triangle/face we search for
	makeVectors(current, mesh, adjacentFaces->at(i));

	QPtrListIterator<Lib3dsFace> it(faces);
	for (; it.current(); ++it) {
		BoVector3Float v[3];
		makeVectors(v, mesh, it.current());
		if (BoVector3Float::isAdjacent(current, v)) {
			adjacentFaces->append(it.current());
			found.append(it.current());
		}
	}
	for (unsigned j = 0; j < found.count(); j++) {
		faces.removeRef(found.at(j));
	}
 }

 boDebug(100) << k_funcinfo << "adjacent: " << adjacentFaces->count() << " of " << mesh->faces << endl;
}

bool Bo3DSLoad::isTeamColor(const Lib3dsMesh* mesh)
{
 if (!mesh) {
	BO_NULL_ERROR(mesh);
	return false;
 }
 if (QString::fromLatin1(mesh->name).find("teamcolor", 0, false) == 0) {
	return true;
 }
 return false;
}

QString Bo3DSLoad::textureName(const Lib3dsMesh* mesh, Lib3dsFile* file)
{
 if (!mesh || mesh->faces == 0) {
	return QString::null;
 }
 if (mesh->texels == 0) {
	return QString::null;
 }
 Lib3dsMaterial* mat = Bo3DSLoad::material(mesh, file);
 if (!mat) {
	return QString::null;
 }
 if (Bo3DSLoad::isTeamColor(mesh)) {
	// teamcolor objects are not textured.
	return QString::null;
 }

 // this is the texture map of the object.
 // t->name is the (file-)name and in
 // mesh->texelL you can find the texture
 // coordinates for glTexCoord*()
 // note that mesh->texels can be 0 - then the
 // mesh doesn't have any texture. otherwise it
 // must be equal to mesh->points
 Lib3dsTextureMap* t = &mat->texture1_map;

 // AB: note that we use BosonModel::cleanTextureName() for the final name. here
 // we return the name from the 3ds file only.
 return QString(t->name);
}

Lib3dsMaterial* Bo3DSLoad::material(const Lib3dsMesh* mesh, Lib3dsFile* file)
{
if (!mesh || mesh->faces == 0) {
	return 0;
 }
 // AB: all faces in this mesh must use the same material!
 Lib3dsFace* f = &mesh->faceL[0];
 Lib3dsMaterial* mat = 0;
 if (f->material[0]) {
	mat = lib3ds_file_material_by_name(file, f->material);
 }
 return mat;
}

void Bo3DSLoad::loadVertices(BoMesh* boMesh, Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(boMesh);
 BO_CHECK_NULL_RET(mesh);
 if (mesh->points < 1) {
	boError() << k_funcinfo << "no points in mesh" << endl;
	return;
 }
 Lib3dsMatrix invMeshMatrix;
 lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
 lib3ds_matrix_inv(invMeshMatrix);
 BoMatrix matrix(&invMeshMatrix[0][0]);

 QValueVector<BoVector3Float> vertices(mesh->points);
 BoVector3Float vector;
 BoVector3Float v;
 for (unsigned int i = 0; i < mesh->points; i++) {
	vector.set(mesh->pointL[i].pos);
	matrix.transform(&v, &vector);
	vertices[i] = v;
 }
 boMesh->setVertices(vertices);
}

void Bo3DSLoad::loadTexels(BoMesh* boMesh, Lib3dsMesh* mesh, Lib3dsMaterial* material)
{
 BO_CHECK_NULL_RET(boMesh);
 BO_CHECK_NULL_RET(mesh);
 if (mesh->texels == 0) {
	return;
 }
 if (mesh->faces == 0) {
	return;
 }
 if (mesh->points == 0) {
	return;
 }
 if (!material) {
	return;
 }
 if (mesh->texels != mesh->points) {
	boError(100) << k_funcinfo << "texels != points" << endl;
	return;
 }

 BoMatrix texMatrix;
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
 glLoadIdentity(); // should already be there
 Lib3dsTextureMap* t = &material->texture1_map;
 if ((t->scale[0] || t->scale[1]) && (t->scale[0] != 1.0 || t->scale[1] != 1.0)) {
	// 3ds does these things pretty unhandy. it doesn't
	// scale as opengl does, but rather emulates scaling the
	// texture itself (i.e. when the texture is centered on
	// an object it will still be centered after scaling).
	// so we need to translate them before scaling.
	glTranslatef((1.0 - t->scale[0]) / 2, (1.0 - t->scale[1]) / 2, 0.0);
	glScalef(t->scale[0], t->scale[1], 1.0);
 }
 if (t->rotation != 0.0) {
	glRotatef(-t->rotation, 0.0, 0.0, 1.0);
 }
 glTranslatef(-t->offset[0], -t->offset[1], 0.0);
 glTranslatef(mesh->map_data.pos[0], mesh->map_data.pos[1], mesh->map_data.pos[2]);
 float scale = mesh->map_data.scale;
 if (scale != 0.0 && scale != 1.0) {
	// doesn't seem to be used in our models
	glScalef(scale, scale, 1.0);
 }
 texMatrix.loadMatrix(GL_TEXTURE_MATRIX);
 if (texMatrix.isNull()) {
	boWarning(100) << k_funcinfo << "Invalid texture matrix was generated!" << endl;
	texMatrix.loadIdentity();
 }
 // AB: we don't use glPushMatrix() / glPopMatrix() here to make sure that the
 // texture matrix is *always* the identity! we don't use the texture matrix
 glLoadIdentity(); // reset to identity
 glMatrixMode(GL_MODELVIEW);


 // now we have the final texture matrix in texMatrix.
 // all texel coordinates have to be transformed using this matrix.
 QValueVector<BoVector3Float> texels(mesh->points);
 BoVector3Float a;
 BoVector3Float b;
 for (unsigned int i = 0; i < mesh->points; i++) {
	a.set(mesh->texelL[i][0], mesh->texelL[i][1], 0.0);
	texMatrix.transform(&b, &a);
	texels[i] = b;
 }
 boMesh->setTexels(texels);
}

void Bo3DSLoad::loadFaces(BoMesh* boMesh, Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(boMesh);
 BO_CHECK_NULL_RET(mesh);
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* f = &mesh->faceL[i];
	BoFace face;
	int points[3];
	points[0] = f->points[0];
	points[1] = f->points[1];
	points[2] = f->points[2];
	face.setPointIndex(points);
	face.setSmoothGroup(f->smoothing);
	boMesh->setFace(i, face);
 }
}

bool Bo3DSLoad::loadMaterials(BosonModelLoaderData* modelData, Lib3dsMaterial* firstMaterial)
{
 if (!modelData) {
	BO_NULL_ERROR(modelData);
	return false;
 }
 if (!firstMaterial) {
	BO_NULL_ERROR(firstMaterial);
	return false;
 }
 Lib3dsMaterial* m = firstMaterial;
 if (modelData->materialCount() != 0) {
	boError() << k_funcinfo << "materials already loaded" << endl;
	return false;
 }
 while (m) {
	modelData->addMaterial();
	m = m->next;
 }
 m = firstMaterial;
 for (unsigned int i = 0; i < modelData->materialCount(); i++, m = m->next) {
	BoMaterial* mat = modelData->material(i);
	if (!mat) {
		boError() << k_funcinfo << "NULL mat at index=" << i << endl;
		return false;
	}
	if (!m) {
		boError() << k_funcinfo << "NULL m at index=" << i << endl;
		return false;
	}

	mat->setName(QString(m->name));

	mat->setAmbient(BoVector4Float(m->ambient));
	mat->setDiffuse(BoVector4Float(m->diffuse));
	mat->setSpecular(BoVector4Float(m->specular));
	mat->setShininess(m->shininess);

	mat->setShinStrength(m->shin_strength);
	mat->setBlur(m->blur);
	mat->setTransparency(m->transparency);
	mat->setFallOff(m->falloff);
	mat->setAdditive(m->additive);
	mat->setUseFallOff(m->use_falloff);
	mat->setSelfIllum(m->self_illum);
	mat->setShading(m->shading);
	mat->setSoften(m->soften);
	mat->setFaceMap(m->face_map);
	mat->setTwoSided(m->two_sided);
	mat->setMapDecal(m->map_decal);
	mat->setUseWire(m->use_wire);
	mat->setUseWireAbs(m->use_wire_abs);
	mat->setWireSize(m->wire_size);

	// now the texture relevant things:
	mat->setTextureName(QString(m->texture1_map.name));
	// AB: we don't set all the other textures, as I have no idea what they
	// are for.
 }
 return true;
}

