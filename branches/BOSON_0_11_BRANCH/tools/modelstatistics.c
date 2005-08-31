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


#include <lib3ds/file.h>
#include <lib3ds/material.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>

#include <stdio.h>
#include <string.h>

struct NodesSummary {
	int mCount;
	int mFaceCount;
};
struct Summary {
	int mModelCount;
	int mMeshCount;
	int mMeshFaceCount; // faces of all meshes (if one mesh appears several times it is counted only once)

	int mNodeCount;
	int mNodeFaceCount; // more relevant value - actual number of faces in the model

	int mMaxNodeFaceCount;
	const char* mMaxNodeFaceFile; // will be a pointer only!!

	int mMaxNodeCount;
	const char* mMaxNodeFile; // will be a pointer only!!

	int mMaxFrameCount;
	const char* mMaxFrameFile; // will be a pointer only!!
};


void usage(const char* command);

void addModel(char** list, int pos, const char* file); // add a model file to the list
void initSummary(struct Summary* summary);
void initNodesSummary(struct NodesSummary* summary);

void parseModel(const char* file, struct Summary* summary);
void parseNode(Lib3dsFile* file, Lib3dsNode* node, struct NodesSummary* summary);
void compareTextures(const char* texturesDir);
int checkPresence(const char* textureFile); // 0 == not there, 1 == its there

void findTextureDir(const char* dir, char* textures);


int debug;


int main(int argc, const char** argv)
{
 int i;
 int arg;
 int pathIndex = 0;
 char** models = (char**)malloc(sizeof(void*) * argc - 1); // we'll never have more than argc-1 model arguments
 int modelCount = 0;
 struct Summary summary;

 debug = 0;
 initSummary(&summary);

 arg = 0;
 for (i = 1; i < argc; i++) {
//	if (strcmp("-t", argv[i]) == 0) {
//		arg = 1;
//		continue;
//	}
	if (strcmp("-v", argv[i]) == 0) {
		debug = 1;
		continue;
	}
	if (strcmp("-vv", argv[i]) == 0) {
		debug = 2;
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
//		case 1:
//			strncpy(texturesDir, argv[i], 255);
//			break;
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

 for (i = 0; i < modelCount; i++) {
	parseModel(models[i], &summary);
 }

 printf("Models: %d\n", summary.mModelCount);
 printf("Total meshes: %d\n", summary.mMeshCount);
 printf("Total faces in meshes: %d\n", summary.mMeshFaceCount);
 printf("Total nodes: %d\n", summary.mNodeCount);
 printf("Total faces in models (all nodes): %d\n", summary.mNodeFaceCount);
 printf("Maximal node faces (%d) in %s\n", summary.mMaxNodeFaceCount, summary.mMaxNodeFaceFile);
 printf("Maximal nodes (%d) in %s\n", summary.mMaxNodeCount, summary.mMaxNodeFile);
 printf("Maximal frames (%d) in %s\n", summary.mMaxFrameCount, summary.mMaxFrameFile);

 return 0;
}

void usage(const char* command)
{
 printf("Usage: %s [OPTIONS] [.3ds files]\n", command);
 printf("Options:\n");
 printf("	-h		this help\n");
 printf("	-v		be verbose\n");
}

void initSummary(struct Summary* summary)
{
 summary->mModelCount = 0;
 summary->mMeshCount = 0;
 summary->mMeshFaceCount = 0;
 summary->mNodeCount = 0;
 summary->mNodeFaceCount = 0;

 summary->mMaxNodeFaceCount = 0;
 summary->mMaxNodeFaceFile = 0;
 summary->mMaxNodeCount = 0;
 summary->mMaxNodeFile = 0;
 summary->mMaxFrameCount = 0;
 summary->mMaxFrameFile = 0;
}

void initNodesSummary(struct NodesSummary* summary)
{
 summary->mCount = 0;
 summary->mFaceCount = 0;
}

void parseModel(const char* filename, struct Summary* summary)
{
 Lib3dsFile* file;
 Lib3dsMaterial* mat = 0;
 Lib3dsMesh* mesh = 0;
 Lib3dsNode* node = 0;
 int meshCount = 0;
 int meshFaceCount = 0;
 struct NodesSummary nodesSummary;
 initNodesSummary(&nodesSummary);
 file = lib3ds_file_load(filename);
 if (debug) {
	printf("Parsing %s\n", filename);
 }
 if (!file) {
	printf("Error: could not load file %s\n", filename);
	return;
 }
 for (mat = file->materials; mat; mat = mat->next) {
 }
 for (mesh = file->meshes; mesh; mesh = mesh->next) {
	if (debug > 1) {
		printf("Faces in mesh %d (%s): %d\n", meshCount, mesh->name, mesh->faces);
	}
	meshFaceCount += mesh->faces;
	meshCount++;
 }
 for (node = file->nodes; node; node = node->next) {
	parseNode(file, node, &nodesSummary);
 }
 if (debug) {
	printf("Faces in %d meshes: %d\n", meshCount, meshFaceCount);
	printf("Faces in %d nodes: %d\n", nodesSummary.mCount, nodesSummary.mFaceCount);
	printf("Frames: %d\n", file->frames);
 }
 summary->mModelCount++;
 summary->mMeshCount += meshCount;
 summary->mMeshFaceCount += meshFaceCount;
 summary->mNodeCount += nodesSummary.mCount;
 summary->mNodeFaceCount += nodesSummary.mFaceCount;
 if (nodesSummary.mFaceCount > summary->mMaxNodeFaceCount) {
	summary->mMaxNodeFaceCount = nodesSummary.mFaceCount;
	summary->mMaxNodeFaceFile = filename;
 }
 if (nodesSummary.mCount > summary->mMaxNodeCount) {
	summary->mMaxNodeCount = nodesSummary.mCount;
	summary->mMaxNodeFile = filename;
 }
 if (file->frames > summary->mMaxFrameCount) {
	summary->mMaxFrameCount = file->frames;
	summary->mMaxFrameFile = filename;
 }
 lib3ds_file_free(file);
}

void parseNode(Lib3dsFile* file, Lib3dsNode* node, struct NodesSummary* summary)
{
 Lib3dsNode* p;
 Lib3dsMesh* mesh;
 for (p = node->childs; p; p = p->next) {
	parseNode(file, p, summary);
 }
 if (node->type != LIB3DS_OBJECT_NODE) {
	return;
 }
 if (strcmp(node->name, "$$$DUMMY") == 0) {
	return;
 }
 if (debug > 1) {
	printf("Parsing node\n");
 }
 mesh = lib3ds_file_mesh_by_name(file, node->name);
 if (!mesh) {
	return;
 }
 summary->mCount++;
 summary->mFaceCount += mesh->faces;
 if (debug > 1) {
	printf("Faces in node %s: %d\n", node->name, mesh->faces);
 }
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


