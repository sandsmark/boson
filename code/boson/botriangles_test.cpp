
#include "bomesh.h"
#include "bo3dtools.h"
#include <bodebug.h>

#include <string.h>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>

Lib3dsFile* lib3ds;
void loadNode(Lib3dsNode* node);

int main(int argc, char** argv)
{
 char file[255];
 if (argc >= 2) {
	strncpy(file, argv[1], 255);
 } else {
	boError() << "You must provide a filename as first argument" << endl;
	exit(1);
 }
 lib3ds = lib3ds_file_load(file);
 if (!lib3ds) {
	boError() << "Unable to load file " << file << endl;
	exit(1);
 }

 Lib3dsNode* node = lib3ds->nodes;
 for (; node; node = node->next) {
	loadNode(node);
 }
 return 0;
}

void loadNode(Lib3dsNode* node3ds)
{
 {
	Lib3dsNode* p = node3ds->childs;
	for (; p; p = p->next) {
		loadNode(p);
	}
 }
 if (node3ds->type != LIB3DS_OBJECT_NODE) {
	return;
 }
 if (strcmp(node3ds->name, "$$$DUMMY") == 0) {
	return;
 }

 Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(lib3ds, node3ds->name);
 if (!mesh) {
	return;
 }
 if (mesh->faces < 1) {
	boError() << "no faces in mesh " << mesh->name << endl;
	return;
 }

 BoMesh* boMesh = new BoMesh(mesh);
 boMesh->connectFaces();
 BoNode* node = boMesh->faces();
 if (!node) {
	boError() << "no nodes in mesh " << mesh->name << endl;
	return;
 }
 if (node->previous()) {
	boError() << "not the first node for " << mesh->name << endl;
	return;
 }
 boDebug() << "Result from connecting faces:" << endl;
 int faceCount = 1;
 for (; node; node = node->next(), faceCount++) {
	BoVector3 v[3];
	BoVector3::makeVectors(v, mesh, node->face());
	QString s = QString("Face %1: ").arg(faceCount);
	for (int i = 0; i < 3; i++) {
		s += BoVector3::debugString(v[i]) + "  ";
	}
	boDebug() << s << endl;
 }
 boDebug() << "in order:" << endl;
 for (node = boMesh->faces(); node; node = node->next(), faceCount++) {
	BoVector3 v[3];
	BoVector3::makeVectors(v, mesh, node->face());
	if (node != boMesh->faces()) {
		boDebug() << v[node->relevantPoint()].debugString() << endl;
	} else {
		int f,s,t;
		node->decodeRelevantPoint(&f, &s, &t);
		boDebug() << v[f].debugString() << endl;
		boDebug() << v[s].debugString() << endl;
		boDebug() << v[t].debugString() << endl;
	}
 }
}

