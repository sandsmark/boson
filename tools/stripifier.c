/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "stripifier.h"

#include <lib3ds/file.h>
#include <lib3ds/material.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 0.001f // zero for floating point numbers

void usage(const char* command);

void addModel(char** list, int pos, const char* file); // add a model file to the list

void parseModel(const char* file);
void parseMesh(Lib3dsMesh* mesh);

void optimizeMesh(Lib3dsMesh* mesh);
void optimizePoints(Lib3dsMesh* mesh);
void optimizeFaces(Lib3dsMesh* mesh);
void dumpFace(Lib3dsFace* face, int newline);
void dumpMesh(Lib3dsMesh* mesh);
void createAdjacencyLists(Lib3dsMesh* mesh, struct Face* faces);
void stripify(struct Face* faces, int facesCount);
void makeStrip(struct Face* face, struct Face* prev, struct Face* prev2, struct List* strip);

int hasPoint(Lib3dsFace* face, int point);
int isPointEqual(Lib3dsMesh* mesh, int p1, int p2); // compares point (vertex), texel, flag


int debug;
int first_mesh_only;
struct StripifyData stripifyData;
const char* outputFile = 0;

struct List* new_list_item(struct List* previous)
{
 struct List* l;
 l = (struct List*)malloc(sizeof(struct List));
 if (previous) {
	if (previous->next != 0) {
		printf("ERROR: previous->next must be NULL!\n");
	} else {
		previous->next = l;
	}
 }
 l->next = 0;
 l->data = 0;
 return l;
}

struct List* append_to_list(struct List* list, int data)
{
 struct List* item = 0;
 struct List* prev = list;
 if (prev) {
	while (prev->next) {
		prev = prev->next;
	}
 }
 item = new_list_item(prev);
 item->data = data;
 if (!list) {
	list = item;
 }
 return list;
}

void free_list_item(struct List* list)
{
 if (!list) {
	return;
 }
 free_list_item(list->next);
 free(list);
}

void debug_list(struct List* list)
{
 struct List* l = list;
 if (!list) {
	return;
 }
 while (l) {
	printf("%d ", l->data);
	l = l->next;
 }
 printf("\n");
}

int has_item(struct List* list, int data)
{
 struct List* l = list;
 while (l) {
	if (l->data == data) {
		return 1;
	}
	l = l->next;
 }
 return 0;
}

struct Face* new_face_item(int face_index)
{
 struct Face* f = (struct Face*)malloc(sizeof(struct Face));
 f->f1 = 0;
 f->f2 = 0;
 f->f3 = 0;
 f->index = face_index;
 return f;
}



int main(int argc, const char** argv)
{
 int i;
 int arg;
 int pathIndex = 0;
 char** models = (char**)malloc(sizeof(void*) * argc - 1); // we'll never have more than argc-1 model arguments
 int modelCount = 0;

 debug = 0;
 first_mesh_only = 0;

 stripifyData.allFaces = 0;
 stripifyData.allFacesCount = 0;
 stripifyData.facesTaken = 0;
 stripifyData.usedFacesCount = 0;

 arg = 0;
 for (i = 1; i < argc; i++) {
	if (strcmp("-vvv", argv[i]) == 0) {
		debug = 3;
		continue;
	}
	if (strcmp("-vv", argv[i]) == 0) {
		debug = 2;
		continue;
	}
	if (strcmp("-v", argv[i]) == 0) {
		debug = 1;
		continue;
	}
	if (strcmp("-f", argv[i]) == 0) {
		first_mesh_only = 1;
		continue;
	}
	if (strcmp("-s", argv[i]) == 0) {
		arg = 1;
		continue;
	}
	if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
		usage(argv[0]);
		exit(0);
	}
	switch (arg) {
		case 0:
			// this should be a model file
			addModel(models, modelCount, argv[i]);
			modelCount++;
			break;
		case 1:
			// "-s" was given, i.e. save to specified file
			if (outputFile) {
				printf("ERROR: outputFile not NULL. did you give -s twice?\n");
				exit(1);
			}
			outputFile = argv[i];
			arg = 0;
			break;
		default:
			// oops - what happened here??
			break;
	}
	arg = 0;
 }
 if (modelCount < 1) {
	printf("No model specified!\n");
	usage(argv[0]);
	exit(0);
 }
 if (outputFile && modelCount > 1) {
	printf("ERROR: can open only one model when save is requested\n");
	exit(1);
 }

 for (i = 0; i < modelCount; i++) {
	parseModel(models[i]);
 }

 return 0;
}

void usage(const char* command)
{
 printf("Usage: %s [OPTIONS] [.3ds files]\n", command);
 printf("Note: this is a testing program, there is no real use for the user!\n");
 printf("Options:\n");
 printf("	-h		this help\n");
 printf("	-v		be verbose\n");
 printf("	-vv		be more verbose\n");
 printf("	-vvv		be even more verbose\n");
}

void parseModel(const char* filename)
{
 Lib3dsFile* file;
 Lib3dsMesh* mesh = 0;

 file = lib3ds_file_load(filename);
 if (debug) {
	printf("Parsing %s\n", filename);
 }
 if (!file) {
	printf("Error: could not load file %s\n", filename);
	return;
 }
 for (mesh = file->meshes; mesh; mesh = mesh->next) {
	parseMesh(mesh);
	if (first_mesh_only) {
		break;
	}
 }
 if (outputFile) {
	lib3ds_file_save(file, outputFile);
	printf("Saved to %s\n", outputFile);
 }
 lib3ds_file_free(file);
}

void addModel(char** list, int pos, const char* filename)
{
 int i;
 int len;
 if (strcmp(filename, "") == 0) {
	return;
 }
 if (list[pos]) {
	printf("Error: memory at pos=%d already used\n", pos);
	return;
 }
 len = strlen(filename);
 list[pos] = (char*)malloc(sizeof(char) * (len + 1));
 strncpy(list[pos], filename, len + 1);
}

int hasPoint(Lib3dsFace* face, int point)
{
 if (!face) {
	return 0;
 }
 if (face->points[0] == point) {
	return 1;
 }
 if (face->points[1] == point) {
	return 1;
 }
 if (face->points[2] == point) {
	return 1;
 }
 return 0;
}

void parseMesh(Lib3dsMesh* mesh)
{
 int i = 0;
 int j = 0;
 struct Face* faces = 0;
 struct List* l = 0;

 if (!mesh) {
	printf("NULL mesh\n");
	return;
 }
 if (debug >= 2) {
	dumpMesh(mesh);
 }
 int originalCount = mesh->points;
 optimizeMesh(mesh);
 if (mesh->points < originalCount) {
	printf("Removed (i.e. merged) %d of %d points\n", originalCount - mesh->points, originalCount);
 }
 if (debug >= 2) {
	dumpMesh(mesh);
 }

 faces = (struct Face*)malloc(sizeof(struct Face) * mesh->faces);
 createAdjacencyLists(mesh, faces);

 if (debug >= 3) {
	dumpAdjacent(mesh, faces, mesh->faces);
 }

 stripify(faces, mesh->faces);

 for (i = 0; i < mesh->faces; i++) {
	free_list_item(faces[i].f1);
	free_list_item(faces[i].f2);
	free_list_item(faces[i].f3);
 }
 free(faces);
}

void createAdjacencyLists(Lib3dsMesh* mesh, struct Face* faces)
{
 Lib3dsFace* l3ds_f = 0;
 Lib3dsFace* l3ds_f2 = 0;
 unsigned int face = 0;
 unsigned int face2 = 0;
 int match = 0;
 struct Face* f = 0;
 int i = 0;

 // naive implementation is sufficient.
 for (face = 0; face < mesh->faces; face++) {
	match = 0;
	l3ds_f = &mesh->faceL[face];
	f = &faces[face];
	f->f1 = 0;
	f->f2 = 0;
	f->f3 = 0;
	f->index = face;
	for (face2 = 0; face2 < mesh->faces; face2++) {
		if (face == face2) {
			continue;
		}
		l3ds_f2 = &mesh->faceL[face2];
		match = 0;
		for (i = 0; i < 3; i++) {
			if (hasPoint(l3ds_f2, l3ds_f->points[i]) != 0) {
				match++;
			}
		}
		if (match == 3) {
			printf("WARNING: 3 points matched! face might be redundant! face=%d, face2=%d mesh=%s\n", face, face2, mesh->name);
			printf("face points: %d,%d,%d   %d,%d,%d\n", l3ds_f->points[0], l3ds_f->points[1], l3ds_f->points[2], l3ds_f2->points[0], l3ds_f2->points[1], l3ds_f2->points[2]);
			continue;
		} else if (match != 2) {
			continue;
		}

		// at this point we know that exactly 2 points are matching,
		// i.e. we have found an adjacent face.
		if (hasPoint(l3ds_f2, l3ds_f->points[0]) != 0) {
			if (hasPoint(l3ds_f2, l3ds_f->points[1])) {
				f->f2 = append_to_list(f->f2, face2);
			} else {
				f->f1 = append_to_list(f->f1, face2);
			}
		} else {
			// point 0 not matching - 1 and 2 must be, as exactly 2
			// are matching.
			f->f3 = append_to_list(f->f3, face2);
		}
	}

#if 0
	j = 0;
	i = 0;
	l = f->f1;
	while (l) {
		i++;
		l = l->next;
	}
	if (i > 1) {
		printf("face %d: f1 count: %d - mesh: %s\n", face, i, mesh->name);
		debug_list(f->f1);
	} else if (i == 1) {
//		printf("face %d: entry in f1: %d - %s\n", face, f->f1->data, mesh->name);
	}

	i = 0;
	l = f->f2;
	while (l) {
		i++;
		l = l->next;
	}
	if (i > 1) {
		printf("face %d: f2 count: %d - mesh: %s\n", face, i, mesh->name);
		debug_list(f->f2);
	}

	i = 0;
	l = f->f3;
	while (l) {
		i++;
		l = l->next;
	}
	if (i > 1) {
		printf("face %d: f3 count: %d - mesh: %s\n", face, i, mesh->name);
		debug_list(f->f3);
	}
#endif
 }
}

void dumpMesh(Lib3dsMesh* mesh)
{
 int i = 0;
 Lib3dsFace* f = 0;
 if (!mesh) {
	printf("ERROR: NULL mesh\n");
	return;
 }
 printf("mesh: %s  faces: \n", mesh->name);
 for (i = 0; i < mesh->faces; i++) {
	f = &mesh->faceL[i];
	printf("%d %d %d\n", f->points[0], f->points[1], f->points[2]);
 }
}

void dumpAdjacent(Lib3dsMesh* mesh, struct Face* faces, int facesCount)
{
 int i = 0;
 struct List* l = 0;
 Lib3dsFace* f;
 printf("adjacent lists of all faces in mesh \"%s\":\n", mesh->name);
 for (i = 0; i < facesCount; i++) {
	f = &mesh->faceL[i];
	dumpFace(&mesh->faceL[i], 1);
	dumpFaceList(mesh, faces[i].f1);
	dumpFaceList(mesh, faces[i].f2);
	dumpFaceList(mesh, faces[i].f3);
 }
}

void dumpFace(Lib3dsFace* f, int newline)
{
 printf("%d %d %d", f->points[0],f->points[1], f->points[2]);
 if (newline) {
	printf("\n");
 }
}

void dumpFaceList(Lib3dsMesh* mesh, struct List* first)
{
 struct List* l = first;
 printf("  ");
 if (!l) {
	printf("<null>");
 }
 while (l) {
	dumpFace(&mesh->faceL[l->data], 0);
	l = l->next;
	if (l) {
		printf(" -> ");
	}
 }
 printf("\n");
}

void optimizeMesh(Lib3dsMesh* mesh)
{
 optimizePoints(mesh);
 optimizeFaces(mesh);
}

void optimizePoints(Lib3dsMesh* mesh)
{
 int i = 0;
 int j = 0;
 struct List* duplicatedPoints = 0;
 struct List* l = 0;
 if (debug) {
	printf("Optimizing / Fixing mesh, if necessary\n");
 }
 if (mesh->points == 0) {
	printf("ERROR: no points in mesh. remove the mesh!\n");
	return;
 }
 int* duplicates = (int*)malloc(sizeof(int) * mesh->points); // maps mesh index to the index that it is duplicating
 for (i = 0; i < mesh->points; i++) {
	duplicates[i] = -1;
 }
 for (i = 0; i < mesh->points; i++) {
	if (duplicates[i] >= 0) {
		// we already fixed this point. nothing left to do.
		continue;
	}
	duplicates[i] = i; // not a duplicate by default
	for (j = i + 1; j < mesh->points; j++) {
		if (isPointEqual(mesh, i, j) != 0) {
			int hasPoint = 0;
			l = duplicatedPoints;
			while (l) {
				if (l->data == j) {
					hasPoint = 1;
				}
				l = l->next;
			}
			if (!hasPoint) {
				// add to the list only, if it is not yet there
				duplicatedPoints = append_to_list(duplicatedPoints, j);
			}

			// j is a duplicate of i
			duplicates[j] = i;
		}
	}
 }

 int pointIndicesCount = 0;
 int* pointIndices = (int*)malloc(sizeof(int) * mesh->points); // maps new mesh point index to mesh point index
 int* replacementPoints = (int*)malloc(sizeof(int) * mesh->points); // maps a mesh point index to the new mesh point index
 for (i = 0; i < mesh->points; i++) {
	if (duplicates[i] == i) {
		// point i is not a duplicate.
		pointIndices[pointIndicesCount] = i;
		replacementPoints[i] = pointIndicesCount;
		duplicates[i] = pointIndicesCount;
		pointIndicesCount++;
	} else {
		// i is just a duplicate of duplicates[i]
		if (i < duplicates[i]) {
			printf("ERROR - i must be < duplicates[i]\n");
			exit(1);
		}
		// duplicates[i] is already in replacementPoints, so we can just
		// reference it
		replacementPoints[i] = replacementPoints[duplicates[i]];
	}
 }

 Lib3dsPoint* points = (Lib3dsPoint*)malloc(sizeof(Lib3dsPoint) * pointIndicesCount);
 Lib3dsWord* flags = 0;
 Lib3dsTexel* texels = 0;
 if (mesh->flags == mesh->points) {
	flags = (Lib3dsWord*)malloc(sizeof(Lib3dsWord) * pointIndicesCount);
 } else if (mesh->flags != 0) {
	printf("ERROR: mesh->flags != mesh->points and not 0!\n");
	exit(1);
 }
 if (mesh->texels == mesh->points) {
	texels = (Lib3dsTexel*)malloc(sizeof(Lib3dsTexel) * pointIndicesCount);
 } else if (mesh->flags != 0) {
	printf("ERROR: mesh->flags != mesh->points and not 0!\n");
	exit(1);
 }
 for (i = 0; i < pointIndicesCount; i++) {
	points[i].pos[0] = mesh->pointL[pointIndices[i]].pos[0];
	points[i].pos[1] = mesh->pointL[pointIndices[i]].pos[1];
	points[i].pos[2] = mesh->pointL[pointIndices[i]].pos[2];
	if (texels) {
		texels[i][0] = mesh->texelL[pointIndices[i]][0];
		texels[i][1] = mesh->texelL[pointIndices[i]][1];
	}
	if (flags) {
		flags[i] = mesh->flagL[pointIndices[i]];
	}
 }
 for (i = 0; i < mesh->faces; i++) {
	Lib3dsWord* points = mesh->faceL[i].points;
	points[0] = replacementPoints[points[0]];
	points[1] = replacementPoints[points[1]];
	points[2] = replacementPoints[points[2]];
 }
 lib3ds_mesh_free_point_list(mesh);
 lib3ds_mesh_free_texel_list(mesh);
 lib3ds_mesh_free_flag_list(mesh);
 mesh->pointL = points;
 mesh->points = pointIndicesCount;
 if (texels) {
	mesh->texelL = texels;
	mesh->texels = pointIndicesCount;
 }
 if (flags) {
	mesh->flagL = flags;
	mesh->flags = pointIndicesCount;
 }

 free(pointIndices);
 free(duplicates);
 free(replacementPoints);
 free_list_item(duplicatedPoints);
}

void optimizeFaces(Lib3dsMesh* mesh)
{
 if (!mesh) {
	printf("ERROR: NULL mesh\n");
	return;
 }
 int i = 0;
 int* redundant = (int*)malloc(sizeof(int) * mesh->faces);
 int redundantCount = 0;
 for (i = 0; i < mesh->faces; i++) {
	Lib3dsWord* p = mesh->faceL[i].points;
	if (p[0] == p[1] || p[0] == p[2] || p[1] == p[2]) {
		if (debug) {
			printf("face %d uses at least 2 equal points: %d %d %d - removing\n", i, p[0], p[1], p[2]);
		}
		redundant[redundantCount] = i;
		redundantCount++;
	}
 }

 if (redundantCount != 0) {
	Lib3dsFace* faces = (Lib3dsFace*)malloc(sizeof(Lib3dsFace) * mesh->faces - redundantCount);
	int facesCount = 0;
	int j = 0;
	for (i = 0; i < mesh->faces; i++) {
		if (i == redundant[j]) {
			j++;
			continue;
		}
		Lib3dsFace* f = &mesh->faceL[i];
		faces[facesCount].user = f->user;
		faces[facesCount].points[0] = f->points[0];
		faces[facesCount].points[1] = f->points[1];
		faces[facesCount].points[2] = f->points[2];
		faces[facesCount].flags = f->flags;
		faces[facesCount].smoothing = f->smoothing;
		faces[facesCount].normal[0] = f->normal[0];
		faces[facesCount].normal[1] = f->normal[1];
		faces[facesCount].normal[2] = f->normal[2];
		strncpy(faces[facesCount].material, f->material, 64);
		facesCount++;
	}
	printf("removed %d redundant faces from mesh %s. faces left: %d\n", redundantCount, mesh->name, facesCount);
	lib3ds_mesh_free_face_list(mesh);
	mesh->faceL = faces;
	mesh->faces = facesCount;
 }
 free(redundant);
}

int isPointEqual(Lib3dsMesh* mesh, int p1, int p2)
{
 Lib3dsPoint* p = mesh->pointL;
 Lib3dsWord* f = mesh->flagL;
 Lib3dsTexel* t = mesh->texelL;
 if (p1 >= mesh->points || p2 >= mesh->points) {
	printf("ERROR: invalid parameters: %d %d\n", p1, p2);
	return 0;
 }
 if (fabsf(p[p1].pos[0] - p[p2].pos[0]) > EPSILON) {
	return 0;
 }
 if (fabsf(p[p1].pos[1] - p[p2].pos[1]) > EPSILON) {
	return 0;
 }
 if (fabsf(p[p1].pos[2] - p[p2].pos[2]) > EPSILON) {
	return 0;
 }
 if (mesh->flags > 0) {
	if (mesh->flags != mesh->points) {
		printf("WARNING: flags != points! is that valid?\n");
	} else {
		if (f[p1] != f[p2]) {
			return 0;
		}
	}
 }
 if (mesh->texels > 0) {
	if (mesh->texels != mesh->points) {
		printf("ERROR: texels != points, with texels != 0! this is not valid!\n");
		return 0;
	}
	if (fabsf(t[p1][0] - t[p2][0]) > EPSILON) {
		return 0;
	}
	if (fabsf(t[p1][1] - t[p2][1]) > EPSILON) {
		return 0;
	}
 }
 if (debug >= 2) {
	printf("points are equal: %d %d\n", p1, p2);
 }
 return 1;
}

void newStripifyData(struct StripifyData* data, struct Face* faces, int facesCount)
{
 // do NOT free data->allFaces! should be done elsewhere
 free(data->facesTaken);
 data->allFaces = faces;
 data->allFacesCount = facesCount;
 data->usedFacesCount = 0;
 data->facesTaken = (int*)malloc(sizeof(int) * facesCount);
 int i = 0;
 for (i = 0; i < facesCount; i++) {
	data->facesTaken[i] = 0;
 }
}

void markFaceUsed(struct StripifyData* data, int face)
{
 if (!data) {
	printf("NULL data\n");
	return;
 }
 if (face < 0 || face >= data->allFacesCount) {
	printf("ERROR: invalid face %d\n", face);
	return;
 }
 if (data->facesTaken[face] == 0) {
	data->usedFacesCount++;
 }
 data->facesTaken[face] = 1;
}

void stripify(struct Face* faces, int facesCount)
{
 struct List* list = 0;
 if (debug) {
	printf("stripify %d faces...\n", facesCount);
	printf("(not yet implemented)\n");
 }
 if (facesCount <= 1) {
	printf("Done - no faces to be stripified.\n");
	return;
 }
 list = new_list_item(0);

 // AB: it doesn't matter which face we start with. we always have to provide
 // code for all cases:
 // - the face has no adjacent faces
 // - it has one adjacent faces (end/start of a strip)
 // - it has two adjacent faces (inside a strip)
 // - it has more than two adjacent faces (pick two)
 // also we can never know in which direction is possible at all. faces (1,2,3)
 // and (1,2,4) are adjacent, but when we start with (1,2,3), we would need a
 // face with points (2,3,x) next. eventually we will always end up in such a
 // situation, it doesn't matter if we pick a random face as "first" or one with
 // only 1 adjacent face (if present).
 newStripifyData(&stripifyData, faces, facesCount);
 struct Face* face = &faces[0];
 list->data = 0;

 makeStrip(face, 0, 0, list);

 free_list_item(list);
}

void makeStrip(struct Face* face, struct Face* prev, struct Face* prev2, struct List* strip)
{
 if (!face) {
	return;
 }
 markFaceUsed(&stripifyData, face->index);
 int next = -1;
 if (!prev) { // implies prev2 == 0
	// we just pick a random adjacent face that is still available.
	struct List* l;
	l = face->f1;
	while (next == -1 && l) {
		if (stripifyData.facesTaken[l->data] == 0) {
			next = l->data;
		}
		l = l->next;
	}
	l = face->f2;
	while (next == -1 && l) {
		if (stripifyData.facesTaken[l->data] == 0) {
			next = l->data;
		}
		l = l->next;
	}
	l = face->f3;
	while (next == -1 && l) {
		if (stripifyData.facesTaken[l->data] == 0) {
			next = l->data;
		}
		l = l->next;
	}
 } else {
	if (prev2) {
		if (has_item(prev2->f1, prev->index)) {
			// we came from prev2 to prev through f1 (0-1)
			// => we must have come from prev to face through either through f2 (0-2) or through f3 (0-3)
			if (has_item(prev->f2, face->index)) {
			} else if (has_item(prev->f3, face->index)) {
			} else {
				printf("EVIL ERROR! prev is not a valid previous of face\n");
				exit(1);
			}
		}
	}
 }
 if (next >= 0) {
	printf("appending %d to strip after %d\n", next, face->index);
	append_to_list(strip, next);
	makeStrip(&stripifyData.allFaces[next], face, prev, strip);
 } else {
	printf("strip ends here: ");
	debug_list(strip);
	return;
 }
}

